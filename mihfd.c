/*
	MIH Function: provides handover services including the Event Service (ES),
	Information Service (IS), and Command Service (CS)
 */


/*
 * sysfs information about network device interfaces
/sys/class/net/:
lo eth0

/sys/class/net/eth0/:
address carrier dormant flags iflink operstate statistics type
addr_ len device duplex ifalias link_mode power subsystem uevent
broadcast dev_id features ifindex mtu speed tx_queue_len
*/
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <sys/poll.h>

#include "mih.h"
#include "mih_protocol.c"


// maximum number os socket descriptors allowed in the poll structure
#define MAX_FDS 4
// indexes for the poll structure
#define TCP4   0
#define UDP4   1
#define TCP6   2
#define UDP6   3

#define MIHF_SOCK_NAME "mih_uxsock"

// socket descriptors for incomming data
struct pollfd _fds[MAX_FDS];


// a global variable to stop the main loop if we receive a signal
int _time_to_quit=0;

int _verbose=0;


// function prototypes

int SockInit(void);

int CapabilityDiscover(char *in_buf, int in_len, char *out_buf, int *out_len);
int Register(char *in_buf, int in_len, char *out_buf, int *out_len);
int Deregister(char *in_buf, int in_len, char *out_buf, int *out_len);
int EventSubscribe(char *in_buf, int in_len, char *out_buf, int *out_len);
int EventUnsubscribe(char *in_buf, int in_len, char *out_buf, int *out_len);

int LinkDetected(char *in_buf, int in_len, char *out_buf, int *out_len);
int LinkUp(char *in_buf, int in_len, char *out_buf, int *out_len);
int LinkDown(char *in_buf, int in_len, char *out_buf, int *out_len);
int LinkParametersReport(char *in_buf, int in_len, char *out_buf, int *out_len);
int LinkGoingDown(char *in_buf, int in_len, char *out_buf, int *out_len);
int LinkHandoverImminent(char *in_buf, int in_len, char *out_buf, int *out_len);
int LinkHandoverComplete(char *in_buf, int in_len, char *out_buf, int *out_len);

int LinkGetParameters(char *in_buf, int in_len, char *out_buf, int *out_len);
int LinkConfigureThresholds(char *in_buf, int in_len, char *out_buf, int *out_len);
int LinkActions(char *in_buf, int in_len, char *out_buf, int *out_len);
int NetHOCandidateQuery(char *in_buf, int in_len, char *out_buf, int *out_len);
int MNHOCandidateQuery(char *in_buf, int in_len, char *out_buf, int *out_len);
int N2NHOQueryResources(char *in_buf, int in_len, char *out_buf, int *out_len);
int MNHOCommit(char *in_buf, int in_len, char *out_buf, int *out_len);
int NetHOCommit(char *in_buf, int in_len, char *out_buf, int *out_len);
int N2NHOCommit(char *in_buf, int in_len, char *out_buf, int *out_len);
int MNHOComplete(char *in_buf, int in_len, char *out_buf, int *out_len);
int N2NHOComplete(char *in_buf, int in_len, char *out_buf, int *out_len);



// Improve that to replace all the "if(_verbose) printf" in the code...
void
ShowMsg(char *msg)
{
	if(_verbose) {
		printf("%s\n",msg);
	}
}


int
SockInit()
{
	int opt, i, naddr;
	char port[6];
	struct addrinfo hints;
	struct addrinfo *result, *rp;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;  // Allow IPv4 or IPv6 (AF_INET, AF_INET6)
	hints.ai_socktype = 0;        // Both SOCK_STREAM and SOCK_DGRAM
	hints.ai_protocol = 0;        // (auto) or IPPROTO_TCP, IPPROTO_UDP
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;   // use my IP address

	sprintf(port,"%d",MIHF_PORT);

	// get information about local addresses
	if(getaddrinfo(NULL, port, &hints, &result) != 0) {
		return -1;
	}

	// initial setting fot the fds
   for(i=0;i<MAX_FDS;i++) {
      _fds[i].fd=0;
      _fds[i].events=0;
   }

	// create and bind a socket with each possible local IP (4 or 6) addr
	naddr=0;
	for (rp = result; rp != NULL; rp = rp->ai_next) {

		if(rp->ai_family == AF_INET) {	// IPv4

			if(rp->ai_socktype == SOCK_STREAM) {	// TCP
				_fds[TCP4].fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
				if(_fds[TCP4].fd == -1)
					return -1;
				if(bind( _fds[TCP4].fd, rp->ai_addr, rp->ai_addrlen) != 0)
					return -1;
				if(listen(_fds[TCP4].fd,5) == -1)
					return -1;
				// set socket options for TCP
				opt = 1;
				if (setsockopt(_fds[TCP4].fd,SOL_SOCKET,SO_REUSEADDR,
					(char *)&opt,sizeof(opt)) == -1)
					return -1;
				// to avoid TCP concatenating short MIH messages
				opt = 1;
				if (setsockopt(_fds[TCP4].fd,IPPROTO_TCP,TCP_NODELAY,
					(char *)&opt,sizeof(opt)) == -1)
					return -1;
				else {
					_fds[TCP4].events = POLLIN;
					naddr++;
				}
			} else { 	// UDP
				_fds[UDP4].fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
				if(_fds[UDP4].fd == -1)
					return -1;
				if(bind(_fds[UDP4].fd, rp->ai_addr, rp->ai_addrlen) != 0)
					return -1;
				else {
					_fds[UDP4].events = POLLIN;
					naddr++;
				}
			}
		}else if(rp->ai_family == AF_INET6) {	// IPv6

			if(rp->ai_socktype == SOCK_STREAM) {	// TCP
				_fds[TCP6].fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
				if(_fds[TCP6].fd == -1)
					return -1;
				if(bind(_fds[TCP6].fd, rp->ai_addr, rp->ai_addrlen) != 0)
					return -1;
				if(listen(_fds[TCP6].fd,5) == -1)
					return -1;
				// set socket options for TCP
				opt = 1;
				if (setsockopt(_fds[TCP6].fd,SOL_SOCKET,SO_REUSEADDR,
					(char *)&opt,sizeof(opt)) == -1)
					return -1;
				// to avoid TCP concatenating short MIH messages
				opt = 1;
				if (setsockopt(_fds[TCP6].fd,IPPROTO_TCP,TCP_NODELAY,
					(char *)&opt,sizeof(opt)) == -1)
					return -1;
				else {
					_fds[TCP6].events = POLLIN;
					naddr++;
				}
			} else { 	// UDP
				_fds[UDP6].fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
				if(_fds[UDP6].fd == -1)
					return -1;
				if(bind(_fds[UDP6].fd, rp->ai_addr, rp->ai_addrlen) != 0)
					return -1;
				else {
					_fds[UDP6].events = POLLIN;
					naddr++;
				}
			}
		}
	}
	freeaddrinfo(result);

	// see if we were able to bind to at least one local address
	if(naddr == 0)
		return -1;

	return 0;
}


// Which parameter to use? Get the base name from a constant?
int
UnixSockInit(int *sockfd, char *s_name, int n_len)
{
	struct sockaddr_un bind_addr;
	int addr_len;

	// creates Unix stream socket: datagram would provide reliable communications
	// preserving message boundaries, but would require 1 pair for each local client
	if ((*sockfd=socket(AF_UNIX,SOCK_STREAM,0)) == -1) {
		if(_verbose) {
			perror("(MIHF) error creating Unix stream socket");
		}
		return -1;
	}
	// binds to local address (file). File system entry isn't created without calling bind
	memset((char *)&bind_addr,0, sizeof(struct sockaddr_un));
	memcpy(bind_addr.sun_path, s_name, n_len);
	bind_addr.sun_family = AF_UNIX;
	addr_len = sizeof(bind_addr.sun_family) + strlen(bind_addr.sun_path);

	unlink(s_name); // remove the FS entry if it's already there, preventing a binding error

	if ((bind(*sockfd, (struct sockaddr *)&bind_addr, addr_len)) == -1) {
		if(_verbose) {
			perror("(MIHF) error binding Unix socket");
		}
		return -1;
	}
	if (listen(*sockfd,10) == -1) {
		if(_verbose) {
			perror("(MIHF) error setting unix socket to listen to incoming connections");
		}
		return -1;
	}

	return 0;
}

void
Quit()
{
	// remove unix domain socket. Should we bother seeing if it exists first?
	if (unlink(MIHF_SOCK_NAME) == -1) {
		if(_verbose) {
			perror("(MIHF) error removing unix socket");
		}
	}
}

void
SigHandler(int signo)
{
	_time_to_quit=1;
}

// do some basic signal handling.
// should we restart from a SIGHUP?
int
SigInit()
{
	struct sigaction sact;

	sact.sa_handler=SigHandler;
	sigemptyset(&sact.sa_mask);

	if (sigaction(SIGINT, &sact, NULL) == -1) {
		return -1;
	}
	if (sigaction(SIGTERM, &sact, NULL) == -1) {
		return -1;
	}
	if (sigaction(SIGHUP, &sact, NULL) == -1) {
		return -1;
	}
	return 0;
}

int
ProcessService(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("(MIH) ProcessService\n");

	if(header->sid != SERVICE_MANAGEMENT)
		return -1;

// MIH_Capability_Discover: Discover the cababilities of a local or remote MIHF
// MIH_Register: Register with a remote MIHF
// MIH_DeRegister: Deregister from a remote MIHF
// MIH_Event_Subscribe: Subscribe for one or more MIH events with a local or remote MIHF
// MIH_Event_Unsubscribe: Unsubscribe for one or more MHI events from a local or remote MIHF
	switch (header->aid) {
		case (MIH_CAPABILITY_DISCOVER):
			res=CapabilityDiscover(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_REGISTER):
			res=Register(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_DEREGISTER):
			res=Deregister(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_EVENT_SUBSCRIBE):
			res=EventSubscribe(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_EVENT_UNSUBSCRIBE):
			res=EventUnsubscribe(in_buf, in_len, out_buf, out_len);
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown service management request\n");
			break;
	}
	return res;
}

int
ProcessEvent(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("ProcessEvent\n");

	if(header->sid != EVENT_SERVICE)
		return -1;

	switch (header->aid) {
		case (MIH_LINK_DETECTED):
			res=LinkDetected(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_LINK_UP):
			res=LinkUp(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_LINK_DOWN):
			res=LinkDown(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_LINK_PARAMETERS_REPORT):
			res=LinkParametersReport(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_LINK_GOING_DOWN):
			res=LinkGoingDown(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_LINK_HANDOVER_IMMINENT):
			res=LinkHandoverImminent(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_LINK_HANDOVER_COMPLETE):
			res=LinkHandoverComplete(in_buf, in_len, out_buf, out_len);
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown event service request\n");
			break;
	}
	return res;
}

int
ProcessCmd(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("ProcessCmd\n");

	if(header->sid != COMMAND_SERVICE)
		return -1;

	switch (header->aid) {
		case (MIH_LINK_GET_PARAMETERS):
			res=LinkGetParameters(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_LINK_CONFIGURE_THRESHOLDS):
			res=LinkConfigureThresholds(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_LINK_ACTIONS):
			res=LinkActions(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_NET_HO_CANDIDATE_QUERY):
			res=NetHOCandidateQuery(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_MN_HO_CANDIDATE_QUERY):
			res=MNHOCandidateQuery(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_N2N_HO_QUERY_RESOURCES):
			res=N2NHOQueryResources(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_MN_HO_COMMIT):
			res=MNHOCommit(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_NET_HO_COMMIT):
			res=NetHOCommit(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_N2N_HO_COMMIT):
			res=N2NHOCommit(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_MN_HO_COMPLETE):
			res=MNHOComplete(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_N2N_HO_COMPLETE):
			res=N2NHOComplete(in_buf, in_len, out_buf, out_len);
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown command service request\n");
			break;
	}
	return res;
}

int
ProcessInfo(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("ProcessInfo\n");

	if(header->sid != INFORMATION_SERVICE)
		return -1;

	switch (header->aid) {
		case (MIH_CAPABILITY_DISCOVER):
			res=CapabilityDiscover(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_REGISTER):
			res=Register(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_DEREGISTER):
			res=Deregister(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_EVENT_SUBSCRIBE):
			res=EventSubscribe(in_buf, in_len, out_buf, out_len);
			break;
		case (MIH_EVENT_UNSUBSCRIBE):
			res=EventUnsubscribe(in_buf, in_len, out_buf, out_len);
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Information service request\n");
			break;
	}
	return res;
}


int
ProcessRequest(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int i,res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) {
		// in_buf[in_len]='\0';
		// printf("Processing request (%d): %s\n",in_len,in_buf);
		printf("Processing request (%d)\n",in_len);
	}
	for(i=0;i<in_len;i++)
		// printf("%d(%c)",(int)in_buf[i],in_buf[i]);
		printf("%c",in_buf[i]);
	printf("\n"); fflush(stdout);

	// do we need a structure for holding the information extracted from the PDU?
	switch (header->sid) {
		case (SERVICE_MANAGEMENT):
			res=ProcessService(in_buf, in_len, out_buf, out_len);
			break;
		case (EVENT_SERVICE):
			res=ProcessEvent(in_buf, in_len, out_buf, out_len);
			break;
		case (COMMAND_SERVICE):
			res=ProcessCmd(in_buf, in_len, out_buf, out_len);
			break;
		case (INFORMATION_SERVICE):
			res=ProcessInfo(in_buf, in_len, out_buf, out_len);
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown message received\n");
			break;
	}

	return res;
}


void *
HandleTcpRequest(void *arg)
{
	int newsock;
	socklen_t addr_len;
	struct sockaddr_storage rem_addr;
	char in_buf[MIH_PDU_LEN];       // input buffer
	char out_buf[MIH_PDU_LEN];      // output buffer
	int in_len, out_len;     // input and output buffers lenghts
	char cliname[100];
	int ind, nfds;

	printf("(MIHF) Tcp handle thread\n");

	// set trhead's detach state
	if (pthread_detach(pthread_self()) == -1) {
		if(_verbose) {
			perror("(MIHF) error setting detach state");
		}
		pthread_exit(NULL);
	}

	if(_fds[TCP4].fd != 0 && _fds[TCP4].revents & POLLIN)
		ind=TCP4;
	else if(_fds[TCP6].fd != 0 && _fds[TCP6].revents & POLLIN)
		ind=TCP6;
	else
		pthread_exit(NULL);

	if(_fds[ind].fd != 0 && _fds[ind].revents & POLLIN) {
		// accepts pending connection requests
		addr_len=sizeof(rem_addr);
		if ((newsock=accept(_fds[ind].fd,(struct sockaddr *)&rem_addr,&addr_len)) == -1) {
			perror("(MIHF) error in accept");

		}else {
			if(_verbose) {
				getnameinfo((struct sockaddr *)&rem_addr,addr_len,cliname,sizeof(cliname),
					NULL, 0,NI_NUMERICHOST);
				printf("(MIHF) TCP connection established with %s\n",cliname);
			}
			// waits up to 3 seconds to read data after connection establishment
			nfds = poll (_fds, MAX_FDS, 3000);
			if (nfds > 0 && _fds[ind].fd != 0 && _fds[ind].revents & POLLIN) {
				// how to know all the data has been read as tcp might do fragmentation?
				if ((in_len=read(newsock,in_buf,MIH_PDU_LEN)) == -1) {
					if(_verbose) {
						perror("(MIHF) error reading from connected socket\n");
					}
				} else {
					// in_buf[n_bytes]='\0'; printf("(MIHF) TCP request: %s\n",in_buf);
					if (ProcessRequest(in_buf,in_len,out_buf,&out_len) == -1) {
						if(_verbose) {
							printf("(MIHF) error processing incoming tcp request\n");
						}
					}
				}
			}
			// close socket
			close(newsock);
		}
	}
	pthread_exit(NULL);
}

void *
HandleUdpRequest(void *arg)
{
	socklen_t addr_len;
	struct sockaddr_storage rem_addr;
	char in_buf[MIH_PDU_LEN];       // input buffer
	char out_buf[MIH_PDU_LEN];      // output buffer
	int in_len, out_len;     // input and output buffers lenghts
	int ind;

	if(_verbose) {
		printf("(MIHF) UDP handle thread\n");
	}

	// set trhead's detach state
	if (pthread_detach(pthread_self()) == -1) {
		if(_verbose) {
			perror("(MIHF) error setting detach state");
		}
		pthread_exit(NULL);
	}

	if(_fds[UDP4].fd != 0 && _fds[UDP4].revents & POLLIN)
		ind=UDP4;
	else if(_fds[UDP6].fd != 0 && _fds[UDP6].revents & POLLIN)
		ind=UDP6;
	else
		pthread_exit(NULL);

	if(_fds[ind].fd != 0 && _fds[ind].revents & POLLIN) {

		// read data (ONE event or command) from udp socket
		addr_len=sizeof(struct sockaddr_in);
		if ((in_len=recvfrom(_fds[ind].fd,in_buf,MIH_PDU_LEN,0,
			(struct sockaddr *)&rem_addr,&addr_len)) == -1) {
			if(_verbose) {
				perror("(MIHF) error reading from udp socket\n");
			}
		}else {
			// UDP requests should include an ACK to the originating client...

			if (ProcessRequest(in_buf,in_len,out_buf,&out_len) == -1) {
				if(_verbose) {
					perror("(MIHF) error processing incoming UDP request");
				}
			}
		}
	}

	pthread_exit(NULL);
}


int
main(int argc, char **argv)
{
	int nfds;
	pthread_t th;	// pointer to the new thread being created
/*
	Possible files of interest for probind ntwork conditions
	int type_fd, device_fd, address_fd, addr_len_fd, broadcast_fd, ifalias_fd,
		 dev_id_fd, ifindex_fd, mtu_fd, tx_queue_len_fd, speed_fd, power_fd,
		 subsystem_fd, uevent_fd, features_fd, duplex_fd, statistics_fd;
	int carrier_fd, dormant_fd, flags_fd, iflink_fd, operstate_fd, link_mode_fd;
	char carrier[LEN], dormant[LEN], flags[LEN], iflink[LEN], operstate[LEN], link_mode[LEN];
*/

	if(argc>1) {
		if(!strcmp(argv[1],"-v")) {
			_verbose=1;
			printf("Running %s in verbose mode\n",argv[0]);
		}
	}

	if (SigInit() == -1) {
		Quit();
	}
	if (SockInit() == -1) {
		Quit();
	}

	// interactions will occur via INET/INET6 sockets
	do {

		nfds = poll (_fds, MAX_FDS, -1);

		if (nfds > 0) {
			// for (i = 0; i < nfds; ++i)
				// if (fds[i].revents & POLLIN) {
			if( (_fds[TCP4].fd != 0 && _fds[TCP4].revents & POLLIN) ||
				 (_fds[TCP6].fd != 0 && _fds[TCP6].revents & POLLIN) ) {

				// printf("(MIHF) tcp request received\n");
				if (pthread_create(&th,NULL,HandleTcpRequest,NULL) == -1) {
					if(_verbose) {
						perror("(MIHF) error creating TCP thread");
					}
				}
      	}
			if( (_fds[UDP4].fd != 0 && _fds[UDP4].revents & POLLIN) ||
				 (_fds[UDP6].fd != 0 && _fds[UDP6].revents & POLLIN) ) {

				// printf("(MIHF) udp request received\n");
				if (pthread_create(&th,NULL,HandleUdpRequest,NULL) == -1) {
					if(_verbose) {
						perror("(MIHF) error creating UDP thread");
					}
				}
      	}
		}
	} while (!_time_to_quit);

	Quit();

	return EXIT_SUCCESS;
}


int
CapabilityDiscover(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Capability Discovery\n");

	if(header->aid != MIH_CAPABILITY_DISCOVER)
		return -1;

	switch (header->opcode) {
		// MIH_Capability_Discover.request (	// pp 77
		//		DestinationIdentifier,
		//		LinkAddressList,           (optional)
		// 	SupportedMihEventList,     (optional)
		//		SupportedMihCommandList,   (optional)
		//		SupportedIsQueryTypeList,  (optional)
		//		SupportedTransportList,    (optional)
		//		MBBHandoverSupport	)     (optional)
		case (REQUEST):
			res=0;
			break;

		// MIH_Capability_Discover.indication (	// pp 78
		//		SourceIdentifier,
		//		LinkAddressList,           (optional)
		// 	SupportedMihEventList,     (optional)
		//		SupportedMihCommandList,   (optional)
		//		SupportedIsQueryTypeList,  (optional)
		//		SupportedTransportList,    (optional)
		//		MBBHandoverSupport	)     (optional)
		case (INDICATION):
			res=0;
			break;

		default:
			if(_verbose)
				printf("(MIH) Unknown Capability Discovery PDU\n");
			break;
	}

	return res;
}


int
Register(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Register\n");

	if(header->aid != MIH_REGISTER)
		return -1;

	switch (header->opcode) {
		// MIH_Register.request (
		//		DestinationIdentifier,
		//		LinkIdentifierList,
		//		REquestCode )
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Register PDU\n");
			break;
	}

	return res;
}
int
Deregister(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Deregister\n");

	if(header->aid != MIH_DEREGISTER)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Deregister PDU\n");
			break;
	}

	return res;
}

int
EventSubscribe(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Event Subscribe\n");

	if(header->aid != MIH_EVENT_SUBSCRIBE)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			// MIH_Event_Subscribe.request (
			//		DestinationIdentifier,
			//		LinkIdentifier,
			//		RequestedMihEventList,
			//		EventConfigurationInfoList )
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Event Subscribe PDU\n");
			break;
	}

	return res;
}

int
EventUnsubscribe(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Event Unsubscribe\n");

	if(header->aid != MIH_EVENT_UNSUBSCRIBE)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Event Unsubscribe PDU\n");
			break;
	}

	return res;
}

int
LinkDetected(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Link Detected\n");

	if(header->aid != MIH_LINK_DETECTED)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Link Detected PDU\n");
			break;
	}
	return res;
}

int
LinkUp(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Link Up\n");

	if(header->aid != MIH_LINK_UP)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Link Up PDU\n");
			break;
	}
	return res;
}

int
LinkDown(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Link Down\n");

	if(header->aid != MIH_LINK_DOWN)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Link Down PDU\n");
			break;
	}
	return res;
}

int
LinkParametersReport(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Link Parameters Report\n");

	if(header->aid != MIH_LINK_PARAMETERS_REPORT)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Link Parameter Report PDU\n");
			break;
	}
	return res;
}

int
LinkGoingDown(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Link Going Down\n");

	if(header->aid != MIH_LINK_GOING_DOWN)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Link Going Down PDU\n");
			break;
	}
	return res;
}

int
LinkHandoverImminent(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Link Handover Imminent\n");

	if(header->aid != MIH_LINK_HANDOVER_IMMINENT)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Link Handover Imminent PDU\n");
			break;
	}
	return res;
}

int
LinkHandoverComplete(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Link Handover Complete\n");

	if(header->aid != MIH_LINK_HANDOVER_COMPLETE)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Link Handover Complete PDU\n");
			break;
	}
	return res;
}

int
LinkGetParameters(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Link Get Parameters\n");

	if(header->aid != MIH_LINK_GET_PARAMETERS)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Link Get Parameters PDU\n");
			break;
	}
	return res;
}
int
LinkConfigureThresholds(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Link Configure Thresholds\n");

	if(header->aid != MIH_LINK_CONFIGURE_THRESHOLDS)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Link Configure Thresholds PDU\n");
			break;
	}
	return res;
}
int
LinkActions(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Link Actions\n");

	if(header->aid != MIH_LINK_ACTIONS)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Link Actions PDU\n");
			break;
	}
	return res;
}

int
NetHOCandidateQuery(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Net HO Candidate\n");

	if(header->aid != MIH_NET_HO_CANDIDATE_QUERY)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Net HO Candidate Query PDU\n");
			break;
	}
	return res;
}

int
MNHOCandidateQuery(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("MN HO Candidate Query\n");

	if(header->aid != MIH_MN_HO_CANDIDATE_QUERY)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown MN HO Candidate Query PDU\n");
			break;
	}
	return res;
}

int
N2NHOQueryResources(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("N2N HO Query Resources\n");

	if(header->aid != MIH_N2N_HO_QUERY_RESOURCES)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown N2N HO Query Resources PDU\n");
			break;
	}
	return res;
}

int
MNHOCommit(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("MN HO Commit\n");

	if(header->aid != MIH_MN_HO_COMMIT)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown MN HO Commit PDU\n");
			break;
	}
	return res;
}

int
NetHOCommit(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("Net HO Commit\n");

	if(header->aid != MIH_NET_HO_COMMIT)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown Net HO Commit PDU\n");
			break;
	}
	return res;
}

int
N2NHOCommit(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("N2N HO Commit\n");

	if(header->aid != MIH_N2N_HO_COMMIT)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown N2N HO Commit PDU\n");
			break;
	}
	return res;
}

int
MNHOComplete(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("MN HO Complete\n");

	if(header->aid != MIH_MN_HO_COMPLETE)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown MN HO Complete PDU\n");
			break;
	}
	return res;
}

int
N2NHOComplete(char *in_buf, int in_len, char *out_buf, int *out_len)
{
	int res;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(_verbose) printf("N2N HO Complete\n");

	if(header->aid != MIH_N2N_HO_COMPLETE)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(_verbose)
				printf("(MIH) Unknown N2N HO Complete PDU\n");
			break;
	}
	return res;
}




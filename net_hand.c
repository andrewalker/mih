/*
	Network notification handler: accepts registration, and subscription calls to local L2 device monitoring, and forwards indication and confirmation calls to ho_engine thread
*/


#include "proto.c"

/*
	wait for request from the network: remote clients' requests or remote
updates on subscribed events.

*/

// Global data structures
struct socket * _insock;  	// the socket for incoming conections

// Function prototypes
static int SockInit(void);


static int
SockInit()
{
	int s;
	int opt;
	struct sockaddr_in rxaddr;

	// the inbound address
	memset(&rxaddr,0, sizeof(rxaddr));
	rxaddr.sin_family = AF_INET;
	rxaddr.sin_port = htons(MIHF_PORT);
	rxaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// the inbound TCP socket
	s = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &_insock);
	if (s < 0) {
		printk(KERN_ERR "MIH NetHand: error %d creating input socket\n",s);
		return -1;
	}
	// kernel_setsockopt locks and checks for the correct function to call
	// int kernel_setsockopt(struct socket *sock, int level, int optname,
	//		char *optval, unsigned int optlen)
	opt=1;
	s = kernel_setsockopt(_insock,IPPROTO_TCP,TCP_NODELAY,(char *)&opt,sizeof(opt));
	if (s < 0) {
		printk(KERN_ERR "MIH NetHand: error %d setting socket option\n",s);
		// goto err;
	}
	s = kernel_setsockopt(_insock,SOL_SOCKET,SO_REUSEADDR,(char *)&opt,sizeof(opt));
	if (s < 0) {
		printk(KERN_ERR "MIH NetHand: error %d setting socket option\n",s);
		// goto err;
	}
	s = _insock->ops->bind(_insock, (struct sockaddr *)&rxaddr, sizeof(rxaddr));
	if(s < 0) {
		printk(KERN_ERR "MIH NetHand: error %d binding port to input socket\n",s);
		goto err;
	}
	s = _insock->ops->listen(_insock, 3);
	if(s < 0) {
		printk(KERN_ERR "MIH NetHand: error %d in listening for input socket\n",s);
		goto err;
	}

	// should we also use UDP?

	return 0;

err:
	sock_release(_insock);

	return -1;
}



int
NetHandler(void *data)
{
	struct socket *new_sock = NULL;
	// char address[128];
	// char *a;
	int len = 0;

	struct msghdr msg;
	struct iovec iov;

	// trying to deal with IPv6...
	struct inet_sock *inet;

	// printk(KERN_INFO "MIH: Network handler thread active\n");

	// Should we do the network initializations here?
	if(SockInit()<0) {
		printk(KERN_INFO "MIH NetHand: fail creating socket\n");
		return -1;
	}
	while(1) {
		// if (kthread_should_stop())
		// waits for messages from the network

		// forwards indications and confirmation to HO engine?
		// calls apropriate primitives...

		if(sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &new_sock) == -1) {
			printk(KERN_INFO "MIH NetHand: error creating data socket\n");
		} else {

			new_sock->type = _insock->type;
			new_sock->ops = _insock->ops;

			// Blocking prevents finishing this thread and the module to unload...
			// Non blocking implies high cpu utilization...
			// if(_insock->ops->accept(_insock, new_sock, SOCK_NONBLOCK) < 0) {
			if(_insock->ops->accept(_insock, new_sock, 0) < 0) {
				printk(KERN_INFO "MIH NetHand: error accepting connection on data socket\n");
				// sleep to avoid cpu use... bad: delays accepting new connections
				// msleep(5000);
			} else {

				// new_sock->ops->getname(new_sock, (struct sockaddr *)address, &len, 2);
				// a = (char *)&(((struct sockaddr_in *)address)->sin_addr.s_addr);
				// printk(KERN_INFO "MIH: connection from %d.%d.%d.%d\n",
				//	(int)(a[0]),(int)(a[1]),(int)(a[2]),(int)(a[3]));

				// trying to deal with ipv6...
				// %pI4 : kernel support for printing IPv4 addresses
				inet = inet_sk(new_sock->sk);
				if(inet) {
					printk(KERN_INFO "MIH: accepted tcp %pI4:%u -> %pI4:%u\n",
						// &inet->saddr, ntohs(inet->sport),  // if k < 2.6.32....
						// &inet->daddr, ntohs(inet->dport)); // if k < 2.6.32....
						&inet->inet_saddr, ntohs(inet->inet_sport),
						&inet->inet_daddr, ntohs(inet->inet_dport));
				}

				msg.msg_name     = 0;
				msg.msg_namelen  = 0;
				msg.msg_iov      = &iov;
				msg.msg_iovlen   = 1;
				msg.msg_control  = NULL;
				msg.msg_controllen = 0;
				msg.msg_flags    = 0;

				msg.msg_iov->iov_base = _rcv_buf;
				msg.msg_iov->iov_len  = MIH_PDU_LEN;

			read_again:
				len = sock_recvmsg(new_sock, &msg, MIH_PDU_LEN, 0); // MSG_DONTWAIT);
				if (len == -EAGAIN || len == -ERESTARTSYS) {
					goto read_again;
				}
				// process PDU...
				// printk(KERN_INFO "MIH msg: %d\n",len);
				if(ProcessRequest(_rcv_buf,len) < 0) {
					// printk(KERN_INFO "MIH: error processing request\n");
				}
			}
			sock_release(new_sock);
		}
		if (kthread_should_stop())
			break;
	}
	sock_release(_insock);

	// printk(KERN_INFO "MIH: Network handler thread stopping\n");

	return 0;
}





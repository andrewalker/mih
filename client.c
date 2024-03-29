#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>

#include "mih.h"
#include "data.h"

#define LEN 128
#define PORT 4551
#define TAM_MSG	65537

int main(int argc, char **argv)
{
	struct sockaddr_in serv_addr;
	struct hostent *phe;
	int sockfd, n_bytes, porta;
	char msg[TAM_MSG];
	int i;
	mih_header_t h;

	if (argc < 2) {
		printf("Usage: %s [-s (service mgmt)] [-e (event srvc)]"
				" [-c (command srvc)] [-a AID]\n", argv[0]);
		return 0;
	}
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-s"))
			h.sid = 1; /* 1: Service Management */
		if (!strcmp(argv[i], "-e"))
			h.sid = 2; /* 2: Event Service */
		if (!strcmp(argv[i], "-c"))
			h.sid = 3; /* 3: Command Service */
		if (!strcmp(argv[i], "-a") && i < argc)
			h.aid = atoi(argv[++i]);
	}

	/* Creates TCP socket. */
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Erro na criacao do socket");
		exit(0);
	}

	/* Adjusts server address. */
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = PF_INET;
	serv_addr.sin_port = htons(4551); /* FIXME: Magic number. */

	/* Obtains IP address of serv_addr from logic name. */
	if ((phe = gethostbyname("localhost")) == NULL) {
		perror("gethostbyname error");
		exit(0);
	}

	/* Copies structure address. */
	memcpy((char *)&serv_addr.sin_addr, phe->h_addr, phe->h_length);

	/* Creates connection to server. */
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("Erro conectando com serv_addr");
		exit(0);
	}

	h.version = 1;
	h.ackreq = 0;
	h.ackrsp = 0;
	h.uir = 0;
	h.morefragment = 0;
	h.fragmentnumber = 0;
	h.rsvd1 = 0;
	/*
	 * h.sid = 4;
	 * Indentifies the different MIH services. Possible values:
	 * 1: Service Management
	 * 2: Event Service
	 * 3: Command Service
	 * 4: Information Service
	 */
	h.opcode = 1;
	/*
	 * (2) Type of operation to be performed with respect to the SID. Values:
	 * 1: Request
	 * 2: Response
	 * 3: Indication
	 */
	/* h.aid = 0; */
	h.rsvd2 = 0;
	h.tid = 0;
	h.payloadlength = 0;

	/* Sends message to server. */
	if (write(sockfd, (char *)&h, sizeof(h)) < 0) {
		perror("error writing to socket");
	}

	close(sockfd);

	return 0;
}

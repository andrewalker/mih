/*
 * Network notification handler: accepts registration, and subscription calls
 * to local L2 device monitoring, and forwards indication and confirmation
 * calls to ho_engine thread.
 */

#include "proto.c"

/* Global data structures. */
struct socket *_insock; /* The socket for incoming connections. */

/* Function prototypes. */
static int SockInit(void);
int NetHandler(void *);


static int SockInit(void)
{
	int s;
	int opt;
	struct sockaddr_in rxaddr;

	/* The inbound address. */
	memset(&rxaddr, 0, sizeof(rxaddr));
	rxaddr.sin_family = AF_INET;
	rxaddr.sin_port = htons(MIHF_PORT);
	rxaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* The inbound TCP socket. */
	s = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &_insock);
	if (s < 0) {
		printk(KERN_ERR "MIH NetHand: error %d creating input socket\n",
				s);
		return -1;
	}

	opt = 1;
	s = kernel_setsockopt(_insock, IPPROTO_TCP, TCP_NODELAY, (char *)&opt,
			sizeof(opt));
	if (s < 0) {
		printk(KERN_ERR "MIH NetHand: error %d setting socket option\n",
				s);
	}
	s = kernel_setsockopt(_insock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
			sizeof(opt));
	if (s < 0) {
		printk(KERN_ERR "MIH NetHand: error %d setting socket option\n",
				s);
	}
	s = _insock->ops->bind(_insock, (struct sockaddr *)&rxaddr,
			sizeof(rxaddr));
	if (s < 0) {
		printk(KERN_ERR "MIH NetHand: error %d binding port to input "
				"socket\n", s);
		goto err;
	}
	s = _insock->ops->listen(_insock, 3);
	if (s < 0) {
		printk(KERN_ERR "MIH NetHand: error %d in listening for input "
				"socket\n", s);
		goto err;
	}

	/* Should we also use UDP? */

	return 0;

err:
	sock_release(_insock);

	return -1;
}


void handle_connection(void *parameter)
{
	struct socket *sock = (struct socket*)parameter;

	/* Decides on starting a handover. */

	/* Process the socket and releases it. */
	if (sock)
		sock_release(sock);
}

int NetHandler(void *data)
{
	struct socket *new_sock = NULL;
	int len = 0;
	struct msghdr msg;
	struct iovec iov;
	struct inet_sock *inet;

	/* Should we do the network initializations here? */
	if (SockInit() < 0) {
		printk(KERN_INFO "MIH NetHand: fail creating socket\n");
		return -1;
	}

	/* Waits for messages from the network. */
	while (!kthread_should_stop() && !_threads_should_stop) {
		if (sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &new_sock)
				== -1) {
			printk(KERN_INFO "MIH NetHand: error creating data "
					"socket\n");
			continue;
		}

		new_sock->type = _insock->type;
		new_sock->ops = _insock->ops;

		if (_insock->ops->accept(_insock, new_sock, 0) < 0) {
			printk(KERN_INFO "MIH NetHand: error accepting "
					"connection on data socket\n");
			continue;
		}

		/* Puts socket in queue to be read by MIHF. */
		if (queue_task(&handle_connection, (void*)new_sock) == 1) {
			/* Queue's full. */
			sock_release(new_sock);
		}
	}

	sock_release(_insock);

	while (!kthread_should_stop())
		msleep(1);

	return 0;
}


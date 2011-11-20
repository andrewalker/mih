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
	DBG("Entering SockInit");
	DBG("Initializing sockaddr");
	memset(&rxaddr, 0, sizeof(rxaddr));
	rxaddr.sin_family = AF_INET;
	rxaddr.sin_port = htons(MIHF_PORT);
	rxaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* The inbound TCP socket. */
	DBG("Creating socket");
	s = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &_insock);
	if (s < 0) {
		printk(KERN_ERR "MIH NetHand: error %d creating input socket\n",
				s);
		return -1;
	}

	DBG("Configuring socket");
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
	DBG("Binding socket");
	s = _insock->ops->bind(_insock, (struct sockaddr *)&rxaddr,
			sizeof(rxaddr));
	if (s < 0) {
		printk(KERN_ERR "MIH NetHand: error %d binding port to input "
				"socket\n", s);
		goto err;
	}
	DBG("Start listening in the socket");
	s = _insock->ops->listen(_insock, 3);
	if (s < 0) {
		printk(KERN_ERR "MIH NetHand: error %d in listening for input "
				"socket\n", s);
		goto err;
	}

	/* Should we also use UDP? */

	DBG("Exiting SockInit");
	return 0;

err:
	DBG("Error occured, releasing socket");
	sock_release(_insock);

	DBG("Exiting SockInit");
	return -1;
}



int NetHandler(void *data)
{
	struct socket *new_sock = NULL;
	int len = 0;
	struct msghdr msg;
	struct iovec iov;
	struct inet_sock *inet;

	DBG("Entering NetHandler");
	/* Should we do the network initializations here? */
	if (SockInit() < 0) {
		printk(KERN_INFO "MIH NetHand: fail creating socket\n");
		return -1;
	}

	DBG("Entering handler loop");
	/* Waits for messages from the network. */
	while (!kthread_should_stop() && !_net_hand_dying) {
		DBG("Loop iteration");
		DBG("Creating socket");
		if (sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &new_sock)
				== -1) {
			printk(KERN_INFO "MIH NetHand: error creating data "
					"socket\n");
			continue;
		}

		new_sock->type = _insock->type;
		new_sock->ops = _insock->ops;

		DBG("Blocking until a connection is accepted");
		if (_insock->ops->accept(_insock, new_sock, 0) < 0) {
			printk(KERN_INFO "MIH NetHand: error accepting "
					"connection on data socket\n");
			continue;
		}

		/* Puts socket in queue to be read by MIHF. */
		DBG("Place socket in the queue");
		spin_lock(&buf_nh_tcp_spinlock);
		if (buf_nh_tcp_available_socks == _NH_TCP_QUEUE_SIZE_) {
			/* Queue's full. */
			sock_release(new_sock);
		} else {
			buf_nh_tcp[buf_nh_tcp_in] = new_sock;
			buf_nh_tcp_in++;
			buf_nh_tcp_in %= _NH_TCP_QUEUE_SIZE_;
			buf_nh_tcp_available_socks++;
		}
		spin_unlock(&buf_nh_tcp_spinlock);
	}

	DBG("Exited handler loop");
	DBG("Releasing socket");
	sock_release(_insock);
	DBG("Exiting NetHandler");
	return 0;
}

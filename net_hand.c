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

	while (1) { /* Waits for messages from the network. */

		if (sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &new_sock)
				== -1) {
			printk(KERN_INFO "MIH NetHand: error creating data "
					"socket\n");
		} else {
			new_sock->type = _insock->type;
			new_sock->ops = _insock->ops;

			/*
			 * Blocking prevents finishing this thread and the
			 * module to unload... Non blocking implies high CPU
			 * utilization...
			 *
			 * if (_insock->ops->accept(_insock, new_sock,
			 *	SOCK_NONBLOCK) < 0)
			 */
			if (_insock->ops->accept(_insock, new_sock, 0) < 0) {
				printk(KERN_INFO "MIH NetHand: error accepting"
						" connection on data socket\n");
				/* Sleep to avoid CPU use... Bad: delays
				   accepting new connections. (In case of non
				   blocking socket. */
				/* msleep(5000); */
			} else {
				inet = inet_sk(new_sock->sk);
				if (inet) {
					printk(KERN_INFO "MIH: accepted tcp %p"
							"I4:%u -> %pI4:%u\n",
						&inet->inet_saddr,
						ntohs(inet->inet_sport),
						&inet->inet_daddr,
						ntohs(inet->inet_dport));
				}

				msg.msg_name = 0;
				msg.msg_namelen = 0;
				msg.msg_iov = &iov;
				msg.msg_iovlen = 1;
				msg.msg_control = NULL;
				msg.msg_controllen = 0;
				msg.msg_flags = 0;
				msg.msg_iov->iov_base = _rcv_buf;
				msg.msg_iov->iov_len = MIH_PDU_LEN;

			read_again:
				len = sock_recvmsg(new_sock, &msg, MIH_PDU_LEN,
						0);
				if (len == -EAGAIN || len == -ERESTARTSYS) {
					goto read_again;
				}
				if (ProcessRequest(_rcv_buf, len) < 0) {
					printk(KERN_INFO "MIH: error processing"
							"request\n");
				}
			}
			sock_release(new_sock);
		}
		if (kthread_should_stop())
			break;
	}
	sock_release(_insock);

	return 0;
}

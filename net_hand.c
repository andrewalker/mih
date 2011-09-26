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
				   blocking socket.) */
				/* msleep(5000); */
			} else {
				/* Puts socket in queue to be read by MIHF. */
				spin_lock(&buf_nh_tcp_spinlock);
				if (buf_nh_tcp_available_socks ==
						_NH_TCP_QUEUE_SIZE_) {
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
		}
		if (kthread_should_stop())
			break;
	}
	sock_release(_insock);

	return 0;
}

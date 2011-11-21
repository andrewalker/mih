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


void handle_connection(void *parameter)
{
	struct socket *sock = (struct socket*)parameter;
	DBG("Entering handle_connection");

	/* Decides on starting a handover. */

	/* Process the socket and releases it. */
	DBG("Releasing socket");
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
	struct queue_task *task;

	DBG("Entering NetHandler");
	/* Should we do the network initializations here? */
	if (SockInit() < 0) {
		printk(KERN_INFO "MIH NetHand: fail creating socket\n");
		return -1;
	}

	DBG("Entering net handler loop");
	/* Waits for messages from the network. */
	while (!kthread_should_stop() && !_threads_should_stop) {
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
		DBG("Acquiring lock");
		spin_lock(&queue_spinlock);
		if (queued_tasks == _QUEUE_SIZE_) {
			/* Queue's full. */
			DBG("Queue is full, releasing socket");
			sock_release(new_sock);

			DBG("Releasing lock");
			spin_unlock(&queue_spinlock);
		} else {
			DBG("Allocating task structure");
			task = (struct task*)kmalloc(sizeof(*task), GFP_KERNEL);

			DBG("Preparing task to handle the socket");
			task->parameter = (void*)new_sock;
			task->task = &handle_connection;
			
			DBG("Placing task in the queue");
			task_queue[queue_in] = task;
			queue_in++;
			queue_in %= _QUEUE_SIZE_;
			queued_tasks++;

			DBG("Releasing lock");
			spin_unlock(&queue_spinlock);

			DBG("Notifying the semaphore");
			up(&queue_semaphore);
		}
	}

	DBG("Exited net handler loop");
	DBG("Releasing socket");
	sock_release(_insock);

	DBG("Waiting for proper termination signal");
	while (!kthread_should_stop())
		msleep(1);

	DBG("Exiting NetHandler");
	return 0;
}


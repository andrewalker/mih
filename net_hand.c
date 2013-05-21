/*
 * Network notification handler: accepts registration, and subscription calls
 * to local L2 device monitoring, and forwards indication and confirmation
 * calls to ho_engine thread.
 */

#include "proto.c"
#include "message.h"

struct received_message {
	struct socket		*sock;
	int			len;
	char			*msg;
	struct reply_handler	*reply;
};

/* Global data structures. */
struct socket *_tcp_insock;	/* The socket for incoming connections. */
struct socket *_udp_insock;	/* The socket for incoming connections. */

/* Function prototypes. */
static int UdpSockInit(struct sockaddr_in *rxaddr);
static int TcpSockInit(void);
int UdpHandler(void *);
int TcpHandler(void *);
int udp_send(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf,
		int length);


static int UdpSockInit(struct sockaddr_in *rxaddr)
{
	int s;

	/* The inbound address. */
	memset(rxaddr, 0, sizeof(*rxaddr));
	rxaddr->sin_family = AF_INET;
	rxaddr->sin_port = htons(MIHF_PORT);
	rxaddr->sin_addr.s_addr = htonl(INADDR_ANY);

	/* The inbound UDP socket. */
	s = sock_create(PF_INET, SOCK_DGRAM, IPPROTO_UDP, &_udp_insock);
	if (s < 0) {
		printk(KERN_ERR "MIH UdpHand: error %d creating input socket\n",
				s);
		return -1;
	}

	s = _udp_insock->ops->bind(_udp_insock, (struct sockaddr *)rxaddr,
			sizeof(*rxaddr));
	if (s < 0) {
		printk(KERN_ERR "MIH UdpHand: error %d binding port to input "
				"UDP socket\n", s);
		goto err;
	}

	return 0;

err:
	sock_release(_udp_insock);

	return -1;
}

static int TcpSockInit(void)
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
	s = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &_tcp_insock);
	if (s < 0) {
		printk(KERN_ERR "MIH TcpHand: error %d creating input socket\n",
				s);
		return -1;
	}

	opt = 1;
	s = kernel_setsockopt(_tcp_insock, IPPROTO_TCP, TCP_NODELAY, (char *)&opt,
			sizeof(opt));
	if (s < 0) {
		printk(KERN_ERR "MIH TcpHand: error %d setting socket option\n",
				s);
	}
	s = kernel_setsockopt(_tcp_insock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
			sizeof(opt));
	if (s < 0) {
		printk(KERN_ERR "MIH TcpHand: error %d setting socket option\n",
				s);
	}
	s = _tcp_insock->ops->bind(_tcp_insock, (struct sockaddr *)&rxaddr,
			sizeof(rxaddr));
	if (s < 0) {
		printk(KERN_ERR "MIH TcpHand: error %d binding port to input "
				"TCP socket\n", s);
		goto err;
	}
	s = _tcp_insock->ops->listen(_tcp_insock, 3);
	if (s < 0) {
		printk(KERN_ERR "MIH TcpHand: error %d in listening for input "
				"socket\n", s);
		goto err;
	}

	return 0;

err:
	sock_release(_tcp_insock);

	return -1;
}


void handle_message(void *parameter)
{
	struct received_message *p = (struct received_message*)parameter;
	int s = 0;
	mih_message_t message;

	s = unpack_mih_message(p->msg, p->len, &message);
	if (s < 0)
		goto out;

	ProcessRequest(&message, p->reply);
	free_tlvs(&message.tlvs);
out:
	kfree(p->msg);
	kfree(parameter);
}

void handle_connection(void *parameter)
{
	struct socket *sock = (struct socket*)parameter;

	/* Decides on starting a handover. */

	/* Process the socket and releases it. */
	if (sock)
		sock_release(sock);
}

void handle_udp_reply(void *parameter)
{
	struct reply_parameter *p = (struct reply_parameter*)parameter;
	struct socket *sock;
	char *buffer;
	int s;

	buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);

	if (p->message == NULL) {
		kfree(p->param);
		goto param_out;
	}

	s = sock_create(PF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock);
	if (s < 0) {
		printk(KERN_ERR "Failed to create UDP socket to send reply");
		goto msg_out;
	}

	s = pack_mih_message(p->message, buffer, BUFFER_SIZE);
	if (s < 1)
		goto sock_out;

	udp_send(sock, (struct sockaddr_in*)p->param, buffer, s);

sock_out:
	sock_release(sock);
msg_out:
	free_tlvs(&p->message->tlvs);
	kfree(p->message);
param_out:
	kfree(p);
	kfree(buffer);
}

int TcpHandler(void *data)
{
	struct socket *new_sock = NULL;

	/* Should we do the network initializations here? */
	if (TcpSockInit() < 0) {
		printk(KERN_INFO "MIH TcpHand: failed creating socket\n");
		return -1;
	}

	/* Waits for messages from the network. */
	while (!kthread_should_stop() && !_threads_should_stop) {
		if (sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &new_sock)
				== -1) {
			printk(KERN_INFO "MIH TcpHand: error creating data "
					"socket\n");
			continue;
		}

		new_sock->type = _tcp_insock->type;
		new_sock->ops = _tcp_insock->ops;

		if (_tcp_insock->ops->accept(_tcp_insock, new_sock, 0) < 0) {
			printk(KERN_INFO "MIH TcpHand: error accepting "
					"connection on data socket\n");
			continue;
		}

		/* Puts socket in queue to be read by MIHF. */
		if (queue_task(&mihf_queue, &handle_connection,
				(void*)new_sock)) {
			sock_release(new_sock);
		}
	}

	sock_release(_tcp_insock);

	while (!kthread_should_stop())
		msleep(1);

	return 0;
}

int udp_send(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf,
		int length)
{
	struct msghdr msg;
	struct iovec iov;
	mm_segment_t oldfs;
	int size = 0;

	if (!sock->sk)
		return 0;

	iov.iov_base = buf;
	iov.iov_len = length;

	msg.msg_flags = 0;
	msg.msg_name = addr;
	msg.msg_namelen = sizeof(*addr);
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	size = sock_sendmsg(sock, &msg, length);
	set_fs(oldfs);

	return size;
}

int udp_recv(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf,
		int length)
{
	struct msghdr msg;
	struct iovec iov;
	mm_segment_t oldfs;
	int size = 0;

	if (!sock->sk)
		return 0;

	iov.iov_base = buf;
	iov.iov_len = length;

	msg.msg_flags = 0;
	msg.msg_name = addr;
	msg.msg_namelen = sizeof(*addr);
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	size = sock_recvmsg(sock, &msg, length, msg.msg_flags);
	set_fs(oldfs);

	return size;
}

int UdpHandler(void *data)
{
	char *buf;
	int received = 0;
	struct sockaddr_in bind_addr;
	struct sockaddr_in *rxaddr;
	struct reply_handler *reply_hand;
	struct received_message *parameter;

	if (UdpSockInit(&bind_addr) < 0) {
		printk(KERN_INFO "MIH UdpHand: failed creating socket\n");
		return -1;
	}

	/* Waits for messages from the network. */
	while (!kthread_should_stop() && !_threads_should_stop) {
		rxaddr = kmalloc(sizeof(*rxaddr), GFP_KERNEL);
		buf = kmalloc(BUFFER_SIZE, GFP_KERNEL);

reuse_allocations:
		memset(buf, 0, BUFFER_SIZE);
		received = udp_recv(_udp_insock, rxaddr, buf, BUFFER_SIZE);

		if (received < 0) {
			printk(KERN_ERR "MIH UdpHand: failed to receive UDP "
					"message\n");
			if (!kthread_should_stop() && !_threads_should_stop) {
				goto reuse_allocations;
			}

			kfree(rxaddr);
			kfree(buf);
			break;
		}

		reply_hand = kmalloc(sizeof(*reply_hand), GFP_KERNEL);
		reply_hand->handler = &handle_udp_reply;
		reply_hand->param = rxaddr;

		parameter = kmalloc(sizeof(*parameter), GFP_KERNEL);
		parameter->sock = NULL;
		parameter->msg = buf;
		parameter->len = received;
		parameter->reply = reply_hand;

		queue_task(&mihf_queue, &handle_message, parameter);
	}

	sock_release(_udp_insock);

	while (!kthread_should_stop())
		msleep(1);

	return 0;
}


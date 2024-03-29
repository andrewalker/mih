/* Send data with no fragmentation by TCP? Use the MSS... */
#define SNDBUF 1460
#define RCVBUF 1460

/* Function prototypes. */
int SendMsg (char *buf, size_t length, struct addrinfo *addr);
int RecvMsg (char *buf, size_t length, struct socket *sock);


int SendMsg(char *buf, size_t length, struct addrinfo *addr)
{
	struct msghdr msg;
	struct iovec iov;
	int len, written = 0, left = length;
	int s = -1;
	struct socket *ssocket = NULL;

	/* Creates internal socket. */
	s = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &ssocket);
	if (s < 0) {
		return -1;
	}

	/* FIXME: Retrieve destination address from a previous request. */
	/* s = ssocket->ops->connect(ssocket, (struct sockaddr *)&_txaddr,
		   sizeof(_txaddr), O_RDWR); */
	if (s && (s != -EINPROGRESS)) {
		printk(KERN_ERR "(MIH) connect error: %d\n", s);
		return -1;
	}

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = (unsigned long)NULL;

repeat_send:
	msg.msg_iov->iov_len = left;
	msg.msg_iov->iov_base = (char *)buf + written;

	len = sock_sendmsg(ssocket, &msg, left);
	if ((len == -ERESTARTSYS) || (!(msg.msg_flags & MSG_DONTWAIT) && (len == -EAGAIN)))
		goto repeat_send;
	if (len > 0) {
  		written += len;
		left -= len;
		if (left)
			goto repeat_send;
	}

	/*
	 * Shuting down the socket prevents other transmissions?
	 * s = ssocket->ops->shutdown(ssocket, SHUT_RDWR);
	 * Not necessary since we'll close the socket later.
	 */

	/* Releases the socket. */
	sock_release(ssocket);

	/* FIXME: Code can be improved for readability. */
	/* Return written ? written : len; */
	return (written == length);
}


int RecvMsg (char *buf, size_t length, struct socket *sock)
{
	struct msghdr msg;
	struct iovec iov;
	int len;
	int max_size = SNDBUF;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	msg.msg_iov->iov_base = buf;
	msg.msg_iov->iov_len = max_size;

/* Since we are using a kernel thread, it's ok to block here... */
read_again:
	len = sock_recvmsg(sock, &msg, max_size, 0); /* MSG_DONTWAIT */
	if (len == -EAGAIN || len == -ERESTARTSYS) {
		goto read_again;
	}
	return len;
}

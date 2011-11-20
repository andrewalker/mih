/*
 * MIHF: waits on events and decides on handovers.
 * Waits for notifications arriving from the local LINK layer or updates about
 * subscribed events from remote MIHFs.
 */

int Mihf(void *data)
{
	/* Registers with local L2 entity.
	 * Subscribes to local Link events.
	 * Detects and, if detected, register with remote MIHF / MIHIS.
	 * Subscribes to remote events.
	 */

	struct socket *sock = NULL;

	DBG("Entering Mihf");
	DBG("Entering mihf handler loop");
	while (!kthread_should_stop()) {
		DBG("Loop iteration");
		/* Waits on events notifications: indications, confirms. */

		/* Reads socket queue from NetHandler. */
		DBG("Acquiring lock");
		spin_lock(&buf_nh_tcp_spinlock);
		if (buf_nh_tcp_available_socks) {
			DBG("Retrieving socket from queue");
			sock = buf_nh_tcp[buf_nh_tcp_out];
			buf_nh_tcp_out++;
			buf_nh_tcp_out %= _NH_TCP_QUEUE_SIZE_;
			buf_nh_tcp_available_socks--;
		} else {
			DBG("Queue is empty");
			sock = NULL;
		}
		DBG("Releasing lock");
		spin_unlock(&buf_nh_tcp_spinlock);

		/* Process the socket and releases it. */
		DBG("Releasing socket");
		if (sock)
			sock_release(sock);

		DBG("Sleeping");
		msleep(5000);

		/* Decides on starting a handover. */
	}

	DBG("Exited mihf handler loop");
	DBG("Exiting Mihf");
	return 0;
}

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

	while (1) {
		/* Waits on events notifications: indications, confirms. */

		/* Reads socket queue from NetHandler. */
		spin_lock(&buf_nh_tcp_spinlock);
		if (buf_nh_tcp_available_socks) {
			sock = buf_nh_tcp[buf_nh_tcp_out];
			buf_nh_tcp_out++;
			buf_nh_tcp_out %= _NH_TCP_QUEUE_SIZE_;
			buf_nh_tcp_available_socks--;
		}
		spin_unlock(&buf_nh_tcp_spinlock);

		/* Process the socket and releases it. */
		sock_release(sock);

		msleep(5000);

		/* Decides on starting a handover. */

		if (kthread_should_stop())
			break;
	}

	return 0;
}

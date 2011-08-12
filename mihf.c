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

	while (1) {
		/* Waits on events notifications: indications, confirms. */

		msleep(5000);

		/* Decides on starting a handover. */

		if (kthread_should_stop())
			break;
	}

	return 0;
}

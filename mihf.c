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

	while (!kthread_should_stop() && !_threads_should_stop) {
		/* Waits on events notifications: indications, confirms. */

		execute_task();
	}

	while (!kthread_should_stop())
		msleep(1);

	return 0;
}

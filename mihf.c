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

	DBG("Entering Mihf");
	DBG("Entering mihf handler loop");
	while (!kthread_should_stop() && !_threads_should_stop) {
		DBG("Loop iteration");
		/* Waits on events notifications: indications, confirms. */

		execute_task();
	}

	DBG("Waiting for proper termination signal");
	while (!kthread_should_stop())
		msleep(1);

	DBG("Exited mihf handler loop");
	DBG("Exiting Mihf");
	return 0;
}

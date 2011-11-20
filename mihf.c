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

	struct queue_task *task = NULL;

	DBG("Entering Mihf");
	DBG("Entering mihf handler loop");
	while (!kthread_should_stop()) {
		DBG("Loop iteration");
		/* Waits on events notifications: indications, confirms. */

		/* Reads socket queue from NetHandler. */
		DBG("Acquiring lock");
		spin_lock(&queue_spinlock);
		if (queued_tasks) {
			DBG("Retrieving task from queue");
			task = task_queue[queue_out];
			queue_out++;
			queue_out %= _QUEUE_SIZE_;
			queued_tasks--;

			DBG("Executing task");
			(*task->task)(task->parameter);

			DBG("Freeing task structure");
			kfree(task);
		}
		DBG("Releasing lock");
		spin_unlock(&queue_spinlock);

		DBG("Sleeping");
		msleep(5000);
	}

	DBG("Exited mihf handler loop");
	DBG("Exiting Mihf");
	return 0;
}

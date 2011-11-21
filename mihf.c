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
	while (!kthread_should_stop() && !_threads_should_stop) {
		DBG("Loop iteration");
		/* Waits on events notifications: indications, confirms. */

		DBG("Waiting for semaphore");
		if (down_killable(&queue_semaphore) < 0)
			printk(KERN_ERR "Semaphore was interrupted. Fatal?");

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
	}

	DBG("Waiting for proper termination signal");
	while (!kthread_should_stop())
		msleep(1);

	DBG("Exited mihf handler loop");
	DBG("Exiting Mihf");
	return 0;
}

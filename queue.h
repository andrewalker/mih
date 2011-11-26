/* Structure to represent a queued task */
struct queued_task {
	void (*task)(void *parameter);
	void *parameter;
	struct list_head list;
};

/* Queue for tasks. */
struct queued_task task_queue;
spinlock_t queue_spinlock;
struct semaphore queue_semaphore;

int init_queue(void)
{
	DBG("Entering init_queue");

	/* Creates and sets communication structures between threads. */
	DBG("Initializing spin lock");
	spin_lock_init(&queue_spinlock);
	DBG("Initializing semaphore");
	sema_init(&queue_semaphore, 0);

	DBG("Initializing queue");
	INIT_LIST_HEAD(&task_queue.list);

	DBG("Exiting init_queue");
	return 0;
}

int queue_task(void (*handler)(void*), void *parameter)
{
	struct queued_task *task = NULL;

	DBG("Entering queue_task");
	DBG("Allocating task structure");
	task = (struct queued_task*)kmalloc(sizeof(*task), GFP_KERNEL);

	DBG("Preparing task to handle the socket");
	task->task = handler;
	task->parameter = parameter;

	DBG("Acquiring lock");
	spin_lock(&queue_spinlock);
	DBG("Placing task in the queue");
	list_add_tail(&task->list, &task_queue.list);
	DBG("Releasing lock");
	spin_unlock(&queue_spinlock);

	DBG("Notifying the semaphore");
	up(&queue_semaphore);

	DBG("Exiting queue_task");
	return 0;
}

int execute_task(void)
{
	struct queued_task *task = NULL;

	DBG("Entering execute_task");
	DBG("Waiting for semaphore");
	if (down_killable(&queue_semaphore) < 0)
		printk(KERN_ERR "Semaphore was interrupted. Fatal?");

	DBG("Acquiring lock");
	spin_lock(&queue_spinlock);
	if (!list_empty(&task_queue.list)) {
		DBG("Retrieving task from queue");
		task = list_first_entry(&task_queue.list, struct queued_task,
				list);
		DBG("Removing task from queue");
		list_del(&task->list);

		DBG("Executing task");
		(*task->task)(task->parameter);

		DBG("Freeing task structure");
		kfree(task);
	}
	DBG("Releasing lock");
	spin_unlock(&queue_spinlock);

	DBG("Exiting execute_task");
	return 0;
}

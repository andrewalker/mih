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

	/* Creates and sets communication structures between threads. */
	spin_lock_init(&queue_spinlock);
	sema_init(&queue_semaphore, 0);

	INIT_LIST_HEAD(&task_queue.list);

	return 0;
}

int queue_task(void (*handler)(void*), void *parameter)
{
	struct queued_task *task = NULL;

	task = (struct queued_task*)kmalloc(sizeof(*task), GFP_KERNEL);

	task->task = handler;
	task->parameter = parameter;

	spin_lock(&queue_spinlock);
	list_add_tail(&task->list, &task_queue.list);
	spin_unlock(&queue_spinlock);

	up(&queue_semaphore);

	return 0;
}

int execute_task(void)
{
	struct queued_task *task = NULL;

	if (down_killable(&queue_semaphore) < 0)
		printk(KERN_ERR "Semaphore was interrupted. Fatal?");

	spin_lock(&queue_spinlock);
	if (!list_empty(&task_queue.list)) {
		task = list_first_entry(&task_queue.list, struct queued_task,
				list);
		list_del(&task->list);

		(*task->task)(task->parameter);

		kfree(task);
	}
	spin_unlock(&queue_spinlock);

	return 0;
}

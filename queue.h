/* Structure to represent a queued task */
struct queued_task {
	void (*task)(void *parameter);
	void *parameter;
	struct list_head list;
};

struct task_queue {
	struct queued_task task;
	struct semaphore sem;
	spinlock_t lock;
};

int init_queue(struct task_queue *queue)
{

	/* Creates and sets communication structures between threads. */
	spin_lock_init(&queue->lock);
	sema_init(&queue->sem, 0);

	INIT_LIST_HEAD(&queue->task.list);

	return 0;
}

int queue_task(struct task_queue *queue, void (*handler)(void*),
		void *parameter)
{
	struct queued_task *task = NULL;

	task = (struct queued_task*)kmalloc(sizeof(*task), GFP_KERNEL);

	task->task = handler;
	task->parameter = parameter;

	spin_lock(&queue->lock);
	list_add_tail(&task->list, &queue->task.list);
	spin_unlock(&queue->lock);

	up(&queue->sem);

	return 0;
}

int execute_task(struct task_queue *queue)
{
	struct queued_task *task = NULL;

	if (down_killable(&queue->sem) < 0)
		printk(KERN_ERR "Semaphore was interrupted. Fatal?");

	spin_lock(&queue->lock);
	if (!list_empty(&queue->task.list)) {
		task = list_first_entry(&queue->task.list, struct queued_task,
				list);
		list_del(&task->list);

		(*task->task)(task->parameter);

		kfree(task);
	}
	spin_unlock(&queue->lock);

	return 0;
}

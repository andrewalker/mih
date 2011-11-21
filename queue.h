#define _QUEUE_SIZE_ 16

/* Structure to represent a queued task */
struct queued_task {
	void (*task)(void *parameter);
	void *parameter;
};

/* Queue for tasks. */
struct queued_task *task_queue[_QUEUE_SIZE_] = {NULL};
int queue_in = 0; /* Point of entrance in the queue. */
int queue_out = 0; /* Point of withdrawal from the queue. */

int queued_tasks = 0;
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

	DBG("Exiting init_queue");
	return 0;
}

int queue_task(void (*handler)(void*), void *parameter)
{
	struct queued_task *task = NULL;

	DBG("Entering queue_task");
	DBG("Acquiring lock");
	spin_lock(&queue_spinlock);

	if (queued_tasks == _QUEUE_SIZE_) {
		DBG("Queue is full, releasing lock");
		spin_unlock(&queue_spinlock);
		DBG("Exiting queue_task");
		return 1;
	}

	DBG("Allocating task structure");
	task = (struct queued_task*)kmalloc(sizeof(*task), GFP_KERNEL);

	DBG("Preparing task to handle the socket");
	task->task = handler;
	task->parameter = parameter;

	DBG("Placing task in the queue");
	task_queue[queue_in] = task;
	queue_in++;
	queue_in %= _QUEUE_SIZE_;
	queued_tasks++;

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

	if (!queued_tasks) {
		DBG("Queue's empty, releasing lock");
		spin_unlock(&queue_spinlock);
		DBG("Exiting execute_task");
		return 1;
	}

	DBG("Retrieving task from queue");
	task = task_queue[queue_out];
	queue_out++;
	queue_out %= _QUEUE_SIZE_;
	queued_tasks--;

	DBG("Executing task");
	(*task->task)(task->parameter);

	DBG("Freeing task structure");
	kfree(task);

	DBG("Releasing lock");
	spin_unlock(&queue_spinlock);

	DBG("Exiting execute_task");
	return 0;
}

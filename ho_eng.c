/*
 * Handover engine: monitors thresholds and decides upon handover.
 * Waits for notifications arriving from the local LINK layer or updates about
 * subscribed events from remote MIHFs.
 */

int HOEngine(void *data)
{
	printk(KERN_INFO "Handover Engine thread active\n");

	/*
	 * Register with local L2 entity.
	 * Register with remote MIHF / MIHIS.
	 * Subscribes to local events.
	 * Subscribes to remote events.
	 */

	while (1) {
		/* Waits on events notifications: indications. */

		msleep(5000);

		/* Decides on starting a handover. */

		if (kthread_should_stop())
			break;
	}

	printk(KERN_INFO "Handover Engine thread stopping\n");

	return 0;
}

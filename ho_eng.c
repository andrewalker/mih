/*
	Handover engine: monitors thresholds and decides upon handover
*/

/*
	Waits for notifications arriving from the local LINK layer
or updates about subscribed events from remote MIHFs
*/

int
HOEngine(void *data)
{
	printk(KERN_INFO "Handover Engine thread active\n");

	// register with local L2 entity
	// register with remote MIHF / MIHIS
	// subscribes to local events
	// subscribes to remote events

	while(1) {
		// waits on events notifications: indications

		msleep(5000);
		// printk(KERN_INFO "HOEngine");

		// decides on starting a handover

		if (kthread_should_stop())
			break;
	}
	printk(KERN_INFO "Handover Engine thread stopping\n");

	return 0;
}



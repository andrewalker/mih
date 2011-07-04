/*
	MIHF: waits on events and decides on handovers
*/

/*
	Waits for notifications arriving from the local LINK layer
or updates about subscribed events from remote MIHFs
*/

int
Mihf(void *data)
{
	// printk(KERN_INFO "Media Independent Handover Function active\n");

	// registers with local L2 entity
	// subscribes to local Link events
	// detects and, if detected, register with remote MIHF / MIHIS
	// subscribes to remote events

	while(1) {
		// waits on events notifications: indications, confirms

		msleep(5000);
		// printk(KERN_INFO "MIHF");

		// decides on starting a handover

		if (kthread_should_stop())
			break;
	}
	// printk(KERN_INFO "MIHF thread stopping\n");

	return 0;
}



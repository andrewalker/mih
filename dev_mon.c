/*
 * Device monitor handler: monitor the status of local devices and processes
 * the executions of the callback function for device monitoring.
 *
 * Detect local interfaces.
 * Start monitoring changes in local interfaces status.
 * Send indications to subscribers.
 */


/* Function prototypes. */
void DevLinkUp(struct net_device *dev);
void DevLinkDown(struct net_device *dev);
void DevLinkGoingDown(struct net_device *dev);

static int GetIf(void);

static int
NetDevNotifier(struct notifier_block *nb, unsigned long state, void *arg);


static struct notifier_block mih_netdev_notifier = {
	.notifier_call = NetDevNotifier,
};

static int
InAddrNotifier(struct notifier_block *nb, unsigned long event, void *arg);

static struct notifier_block mih_inaddr_notifier = {
	.notifier_call = InAddrNotifier,
};


static int GetIf()
{
	return 0;
}

static int
NetDevNotifier(struct notifier_block *nb, unsigned long event, void *arg)
{
	struct net_device *dev = arg;
	u8 ssid[IEEE80211_MAX_SSID_LEN + 1];
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct iw_statistics *iw_stats;

	if (wdev) { /* It's a wireless interface... */

		/* ssid is only valid if wdev->sme_state==...CONNECTED */
		if (wdev->sme_state == CFG80211_SME_CONNECTED) {
			memcpy(ssid,wdev->ssid,wdev->ssid_len);
			ssid[wdev->ssid_len]='\0';
			printk(KERN_INFO "MIH: (%s) state:%d ssid: %s\n",
				dev->name,wdev->sme_state,ssid);
		}

		if ((dev->wireless_handlers != NULL) &&
			 (dev->wireless_handlers->get_wireless_stats != NULL)) {

			iw_stats =
				dev->wireless_handlers->get_wireless_stats(dev);

			printk(KERN_INFO "MIH: (%s): signal qual: %d level: %d"
					" noise: %d\n",
				dev->name,
				iw_stats->qual.qual,
				iw_stats->qual.level,  /* signal level (dBm) */
				iw_stats->qual.noise); /* noise level (dBm) */
		} else
			printk(KERN_INFO "MIH: null iw_stats\n");
	}

	if (dev->type != ARPHRD_ETHER) {
		return NOTIFY_DONE;
	}

	switch (event) {

		case NETDEV_REGISTER:
			/* Called once for each available device. */
			/* We can use this to create a list of the devices
			   available. */
			break;
		case NETDEV_GOING_DOWN:
			DevLinkGoingDown(dev);
			break;
		case NETDEV_DOWN:
			DevLinkDown(dev);
			break;
		case NETDEV_UP:
			DevLinkUp(dev);
			break;
		case NETDEV_PRE_UP:
			break;
		case NETDEV_CHANGE:
			/* NETDEV_CHANGE also is notified when device is made
			   Up and Down... */
			if (netif_carrier_ok(dev)) {
				DevLinkUp(dev);
			} else {
				DevLinkDown(dev);
			}
			break;
		case NETDEV_CHANGEADDR: /* MAC address (L2) has changed. */
			break;
	}

	return NOTIFY_DONE;
}


/* Event handler for inet address addition/deletion events. */
static int
InAddrNotifier(struct notifier_block *nb, unsigned long event, void *arg)
{
	struct in_ifaddr *ifa = (struct in_ifaddr *)arg;
	struct net_device *dev = ifa->ifa_dev->dev;
	unsigned char *addrp;

	/*
	 * Events:
	 *	NETDEV_UP: layer 3 addr added to interf;
	 *	NETDEV_DOWN: L3 addr removed.
	 * Will also happen for a virtual interface (e.g. eth0:0, wlan3:0, ...).
	 */

	printk(KERN_INFO "MIH: InAddrNotifier - dev:%s, event:%d\n",
			dev->name,(int)event);

	/* Example of how to access the IPv4 address: */
	if (ifa != NULL) {
		addrp = (unsigned char *)&ifa->ifa_address;
		if (addrp != NULL)
			printk(KERN_INFO "(MIH) Device: %s address: "
					"%d.%d.%d.%d\n",
				dev->name,(int)addrp[0],(int)addrp[1],
				(int)addrp[2],(int)addrp[3]);
	}

	switch (event) {
		case NETDEV_UP:
			break;
		case NETDEV_DOWN:
			break;
	}
	return NOTIFY_DONE;
}


int DevMon(void *data)
{
	/* Detects local interfaces to watch. */
	if (GetIf() < 0) {
		printk(KERN_INFO "MIH DevMon: failure detecting the "
				"interfaces\n");
	}

	/* Loops on the periodic inspection of detected devices. */

	while (1) {
		/*
		 * Polls (subscribed) devices and sends notifications.
		 *
		 * Locks a semaphore.
		 * Instead of checking the devices, check the list of
		 * subscriptions...
		 *
		 * for(dev=dev_list; dev!=NULL; dev = dev->next) {
		 *
		 *	Keep the list of subcribers and thresholds in each
		 *	device?
		 *
		 *	for(sub=dev->subscribers; sub != NULL; sub=sub->next) {
		 *		// Comparison may change for each parameter..
		 *		if(dev->... > sub->threshold)) {
		 *		// Sends message to subscribed node.
		 *			MIH Event Service
		 *			MIH_Link_Detected
		 *			MIH_Link_Up
		 *			MIH_Link_Down
		 *			MIH_Link_Parameters_Report
		 *			MIH_Link_Going_Down
		 *			MIH_Link_Handover_Imminent
		 *			MIH_Link_Handover_Complete
		 *			MIH_Link_PDU_Transmit_Status
		 *		}
		 *	}
		 * }
		 * Unlocks the semaphore.
		 *
		 * Sleeps before polling the devices again.
		 */
		msleep(5000);

		if (kthread_should_stop())
			break;
	}

	return 0;
}


void DevLinkUp(struct net_device *dev)
{
	transport_addr_t *mac_addr = kmalloc(sizeof(*mac_addr), GFP_KERNEL);
	struct Link_Up_indication_parameter *p =
			kmalloc(sizeof(*p), GFP_KERNEL);

	switch (dev->type) {
		case ARPHRD_ETHER: /* Ethernet and 802.11 with ethernet
					framing. */
			if (dev->ieee80211_ptr == NULL) {
				p->LinkIdentifier.link_id.link_type = ETHERNET;
			} else {
				p->LinkIdentifier.link_id.link_type =
						WIRELESS_IEEE_802_11;
			}
			break;
		case ARPHRD_IEEE80211: /* 801: IEEE 802.11 */
			p->LinkIdentifier.link_id.link_type =
					WIRELESS_IEEE_802_11;
			break;
	};

	p->LinkIdentifier.link_id.link_addr.mac_addr.addr_type = 6; /* All 802. */
	p->LinkIdentifier.link_id.link_addr.mac_addr.addr = dev->dev_addr;
	mac_addr->addr_type = 6;
	mac_addr->addr = dev->dev_addr;
	p->LinkIdentifier.link_addr = (link_addr_t*)mac_addr;

	p->OldAccessRouter.mac_addr.addr_type = 6;
	p->NewAccessRouter.mac_addr.addr_type = 6;

	p->IPRenewalFlag = FALSE; /* How do we know this? */
	p->MobilityManagementSupport = 0;

	queue_task(&Link_Up_indication, (void*)p);
}

void
DevLinkDown(struct net_device *dev)
{
	transport_addr_t *mac_addr = kmalloc(sizeof(*mac_addr), GFP_KERNEL);
	struct Link_Down_indication_parameter *p =
			kmalloc(sizeof(*p), GFP_KERNEL);

	p->LinkIdentifier.link_id.link_type = ETHERNET; /* WIRELESS_IEEE_802_11 */
	p->LinkIdentifier.link_id.link_addr.mac_addr.addr_type = 6; /* All 802. */
	p->LinkIdentifier.link_id.link_addr.mac_addr.addr = dev->dev_addr;
	mac_addr->addr_type = 6;
	mac_addr->addr = dev->dev_addr;
	p->LinkIdentifier.link_addr = (link_addr_t*)mac_addr;

	p->OldAccessRouter.mac_addr.addr_type = 6;
	p->ReasonCode = LD_EXPLICIT_DISCONNECT;

	/* Notify the local MIHF, if it has subscribed to this event... */

	queue_task(&Link_Down_indication, (void*)p);
}

void DevLinkGoingDown(struct net_device *dev)
{
	transport_addr_t *mac_addr = kmalloc(sizeof(*mac_addr), GFP_KERNEL);
	struct Link_Going_Down_indication_parameter *p =
			kmalloc(sizeof(*p), GFP_KERNEL);

	p->LinkIdentifier.link_id.link_type = ETHERNET; /* WIRELESS_IEEE_802_11 */
	p->LinkIdentifier.link_id.link_addr.mac_addr.addr_type = 6; /* All 802. */
	p->LinkIdentifier.link_id.link_addr.mac_addr.addr = dev->dev_addr;
	mac_addr->addr_type = 6;
	mac_addr->addr = dev->dev_addr;
	p->LinkIdentifier.link_addr = (link_addr_t*)mac_addr;

	/* How to find out the reason? */
	p->LinkGoingDownReason = LGD_EXPLICIT_DISCONNECT;

	/*
	 * timeinterval: time interval (ms) in which the link is expected to
	 * go down. 0 if unknown.
	 */
	p->TimeInterval = 0;

	/* Notify the local MIHF, if it has subscribed to this event... */

	queue_task(&Link_Going_Down_indication, (void*)p);
}


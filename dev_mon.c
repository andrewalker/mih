/*
	Device monitor handler: monitor the status of local devices and processes the executions of the callback function for device monitoring.
*/

/*
	Detect local interfaces
	Start monitoring changes in local interfaces status
	Send indications to subscribers
*/


// function prototypes
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


static int
GetIf()
{
	// struct net_device *dev;
	// struct net *net;

	// Not sure if this should be done this way... what if the lock changes in the future?
	// We can get this list using register_netdevice_notify as a
	// NETDEV_REGISTER event is produced for each registered interface
/*
	rtnl_lock();
	for_each_net(net) {
		for_each_netdev(net, dev) {
			// printk(KERN_INFO "Getif:%s type:%d\n",dev?dev->name:"NULL",dev?dev->type:-1);
			// fill the _list_of_networks...
		}
	}
	rtnl_unlock();
*/
	return 0;
}

static int
NetDevNotifier(struct notifier_block *nb, unsigned long event, void *arg)
{
	struct net_device *dev = arg;
	u8 ssid[IEEE80211_MAX_SSID_LEN+1];
	// u8 bssid[ETH_ALEN+1];
	// u8 prev_bssid[ETH_ALEN+1];
	// struct cfg80211_bss *curr_bss;

	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct iw_statistics *iw_stats;

/*
struct r8180_priv *priv = ieee80211_priv(dev);
struct ieee80211_device* ieee = priv->ieee80211;
struct iw_statistics* wstats = &priv->wstats;

int tmp_level = 0;
int tmp_qual = 0;
int tmp_noise = 0;
*/

	if (wdev) {	// it's a wireless interface...

		// ssid is only valid if wdev-sme_state==...CONNECTED
		// ...CONNECTING has substates (conn->state)
		// ->conn is only valid inside the code that uses it; useless here
		// "sme_state" not available if kern < 2.6.32...

		if (wdev->sme_state == CFG80211_SME_CONNECTED) {
			memcpy(ssid,wdev->ssid,wdev->ssid_len);
			ssid[wdev->ssid_len]='\0';
			printk(KERN_INFO "MIH: (%s) state:%d ssid: %s\n",
				dev->name,wdev->sme_state,ssid);
		}
/*
		// wdev->channel: useless...
		printk(KERN_INFO "MIH: (%s) curr BSS: "
			"band: %d  center_freq: %d  max_antenna_gain: %d  max_power: %d\n",
			dev->name, wdev->channel->band, wdev->channel->center_freq,
			wdev->channel->max_antenna_gain, wdev->channel->max_power );
*/

/*
	H testing:
	http://lxr.linux.no/#linux+v2.6.36/drivers/staging/rtl8187se/r8180_wx.c#L1550
*/
/*
		tmp_level = (&ieee->current_network)->stats.signal;
		tmp_qual = (&ieee->current_network)->stats.signalstrength;
		tmp_noise = (&ieee->current_network)->stats.noise;
		printk("level:%d, qual:%d, noise:%d\n", tmp_level, tmp_qual, tmp_noise);

		printk("level:%d\n", tmp_level);
		wstats->qual.level = tmp_level;
		wstats->qual.qual = tmp_qual;
		wstats->qual.noise = tmp_noise;
*/
/*
	h testing \end
*/

		if ((dev->wireless_handlers != NULL) &&
			 (dev->wireless_handlers->get_wireless_stats != NULL)) {

			iw_stats = dev->wireless_handlers->get_wireless_stats(dev);

			printk(KERN_INFO "MIH: (%s): signal qual: %d  level: %d  noise: %d\n",
				dev->name,
				iw_stats->qual.qual,   //
				iw_stats->qual.level,  // signal level (dBm)
				iw_stats->qual.noise); // noise level (dBm)
		} else
			printk(KERN_INFO "MIH: null iw_stats\n");

		/* // didn't manage to access internal structure cfg80211_internal_bss...
		*/
		// curr_bss = &wdev->current_bss->pub;
		/*
		curr_bss = wdev->current_bss;
		memcpy(bssid,curr_bss->bssid,ETH_ALEN); bssid[ETH_ALEN]='\0';
		printk(KERN_INFO "MIH: (%s) curr BSS: bssid:%s\n",dev->name, bssid);
		*/
		/*
		// cannot count on accessing Wireless extensions data (struct west)
		// doesn't work...
		memcpy(ssid,wdev->west.ssid,IEEE80211_MAX_SSID_LEN);
		ssid[IEEE80211_MAX_SSID_LEN]='\0';
		memcpy(bssid,wdev->west.bssid,ETH_ALEN); bssid[ETH_ALEN]='\0';
		memcpy(prev_ssid,wdev->west.prev_ssid,ETH_ALEN); prev_bssid[ETH_ALEN]='\0';
		printk(KERN_INFO "MIH: (%s) ssid:%s bssid:%s prev_bssid:%s\n"
			dev->name,ssid,bssid,prev_bssid);
		*/
	}

	// if (!wdev)	// Not a wireless interface...
	//		return NOTIFY_DONE;
	// if (dev->flags & IFF_LOOPBACK)
	// 	return NOTIFY_DONE;

	// http://lxr.linux.no/#linux+v2.6.35/include/linux/if_arp.h#L28
	// #define ARPHRD_ETHER    1               // Ethernet 10Mbps
	// #define ARPHRD_IEEE802  6               // IEEE 802.2 Ethernet/TR/TB
	// ...
	// #define ARPHRD_LOOPBACK 772             // Loopback device
	// ...
	// #define ARPHRD_IEEE802_TR 800           // Magic type ident for TR
	// #define ARPHRD_IEEE80211 801            // IEEE 802.11
	//
	// dev->type: we are interested on Ethernet/ARP devices (ethX, wlanX)
	// wlanx is seen as an ethernet device using ethernet framing
	if(dev->type != ARPHRD_ETHER) {
		return NOTIFY_DONE;
	}
	// printk(KERN_INFO "MIH NetDevNotifier: %s (event: %d)\n",dev->name,(int)event);

/*
	// events: http://lxr.linux.no/#linux+v2.6.35/include/linux/notifier.h#L192
	NETDEV_UP       0x0001  // For now you can't veto a device up/down
	NETDEV_DOWN     0x0002
	NETDEV_REBOOT   0x0003  // Tell a protocol stack a network interface
                        	// detected a hardware crash and restarted
                        	// we can use this eg to kick tcp sessions once done
	NETDEV_CHANGE   0x0004  // Notify device state change
	NETDEV_REGISTER 0x0005
	NETDEV_UNREGISTER       0x0006
	NETDEV_CHANGEMTU        0x0007
	NETDEV_CHANGEADDR       0x0008
	NETDEV_GOING_DOWN       0x0009
	NETDEV_CHANGENAME       0x000A
	NETDEV_FEAT_CHANGE      0x000B
	NETDEV_BONDING_FAILOVER 0x000C
	NETDEV_PRE_UP           0x000D
	NETDEV_BONDING_OLDTYPE  0x000E
	NETDEV_BONDING_NEWTYPE  0x000F
	NETDEV_POST_INIT        0x0010
	NETDEV_UNREGISTER_BATCH 0x0011
*/
	switch (event) {

		case NETDEV_REGISTER:
			// Called once for each available device
			// We can use this to create a list of the devices available...
			break;
		case NETDEV_GOING_DOWN:
			// printk(KERN_INFO "MIH: %s NIC Link going down\n", dev->name);
			DevLinkGoingDown(dev);
			break;
		case NETDEV_DOWN:
			// printk(KERN_INFO "MIH: %s NIC Link down\n", dev->name);
			DevLinkDown(dev);
			break;
		case NETDEV_UP:
			// printk(KERN_INFO "MIH: %s NIC Link up\n", dev->name);
			DevLinkUp(dev);
			break;
		case NETDEV_PRE_UP:
			// printk(KERN_INFO "MIH: %s NIC Link pre up\n", dev->name);
			// DevLinkUp(dev);
			break;
		case NETDEV_CHANGE:
			// NETDEV_CHANGE also is notified when device is made Up and Down...
			// printk(KERN_INFO "MIH: netdev_change...\n");
			if(netif_carrier_ok(dev)) {
				// printk(KERN_INFO "MIH: %s NIC Link is Up\n", dev->name);
				DevLinkUp(dev);
			} else {
				// printk(KERN_INFO "MIH: %s NIC Link is Down\n", dev->name);
				DevLinkDown(dev);
			}
			break;
		case NETDEV_CHANGEADDR: // MAC address (L2) has changed
			// printk(KERN_INFO "MIH: %s NIC changeaddr\n", dev->name);
			// DevLinkUp(dev);
			break;
	}

	return NOTIFY_DONE;
}


// Event handler for inet address addition/deletion events.
static int
InAddrNotifier(struct notifier_block *nb, unsigned long event, void *arg)
{
	struct in_ifaddr *ifa = (struct in_ifaddr *)arg;
	struct net_device *dev = ifa->ifa_dev->dev;
	unsigned char *addrp;

	// Events: NETDEV_UP: layer 3 addr added to interf; NETDEV_DOWN: L3 addr removed
	// Will also happen for a virtual interface (e.g. eth0:0, wlan3:0, ...)

	printk(KERN_INFO "MIH: InAddrNotifier - dev:%s, event:%d\n",dev->name,(int)event);

	// example of how to access the IPv4 address:
	if (ifa != NULL) {
		addrp=(unsigned char *)&ifa->ifa_address;
		if(addrp != NULL)
			printk(KERN_INFO "(MIH) Device: %s address: %d.%d.%d.%d\n",
				dev->name,(int)addrp[0],(int)addrp[1],(int)addrp[2],(int)addrp[3]);
	}

	// example: http://lxr.linux.no/#linux+v2.6.35.4/net/ipv4/fib_frontend.c#L912

	switch (event) {
		case NETDEV_UP:
			// printk(KERN_INFO "MIH: InAddrNotifier NETDEV_UP for %s\n",dev->name);
			break;
		case NETDEV_DOWN:
			// printk(KERN_INFO "MIH: InAddrNotifier NETDEV_DOWN for %s\n",dev->name);
			break;
	}
	return NOTIFY_DONE;
}


int
DevMon(void *data)
{
   // printk(KERN_INFO "MIH: Device monitor thread active\n");

	// detects local interfaces to watch
	if(GetIf()<0) {
   	printk(KERN_INFO "MIH DevMon: failure detecting the interfaces\n");
		// return -1;
	}
	// registers routine to watch for changes in the status of devices
	if(register_netdevice_notifier(&mih_netdev_notifier)) {
		printk(KERN_INFO "MIH DevMon: failure registering netdev notifier\n");
		return -1;
	}
	// if(register_netdevice_notifier(&mih_netdev_notifier)) {
	if(register_inetaddr_notifier(&mih_inaddr_notifier)) {
		printk(KERN_INFO "MIH DevMon: failure registering inetaddr notifier\n");
		return -1;
	}

   // Loops on the periodic inspection of detected devices

   while(1) {
		// Polls (subscribed) devices and sends notifications
	/*
		// locks a semaphore
		// instead of checking the devices, check the list of subscriptions...

		for(dev=dev_list; dev!=NULL; dev = dev->next) {

			// keep the list of subcribers and thresholds in each device?

			for(sub=dev->subscribers; sub != NULL; sub=sub->next) {
				// comparison may change for each parameter..
				if(dev->... > sub->threshold)) {

					// sends message to subscribed node
					// // MIH Event Service
					// MIH_Link_Detected
					// MIH_Link_Up
					// MIH_Link_Down
					// MIH_Link_Parameters_Report
					// MIH_Link_Going_Down
					// MIH_Link_Handover_Imminent
					// MIH_Link_Handover_Complete
					// MIH_Link_PDU_Transmit_Status
				}
			}
		}
		// unlocks the semaphore
	*/
		// sleeps before polling the devices again
		msleep(5000);
		// printk(KERN_INFO "DevMon");

      if (kthread_should_stop())
         break;
   }
   // printk(KERN_INFO "MIH: device monitor thread stopping\n");

	// unregisters routine
	unregister_netdevice_notifier(&mih_netdev_notifier);
	unregister_inetaddr_notifier(&mih_inaddr_notifier);

   return 0;
}

/*
	// some structures to consider when the notifiers are called:

	struct net_device *dev;
	struct wireless_dev *wdev;
	struct wiphy *wiphy;
	struct ieee80211_hw *hw;
	struct in_ifaddr *ifa = (struct in_ifaddr *)arg;

	dev = ifa->ifa_dev->dev;

	wdev = dev->ieee80211_ptr;
	if (wdev == NULL) {
		// printk(KERN_INFO "MIH InAddrNotifier: %s has no ieee80111_ptr\n",dev->name);
		return NOTIFY_DONE;
	}
	wiphy = wdev->wiphy;
	if (wiphy == NULL) {
		// printk(KERN_INFO "MIH InAddrNotifier: %s is not wifi\n",dev->name);
		return NOTIFY_DONE;
	}
	hw = wiphy_priv(wiphy);
	if (hw == NULL) {
		// printk(KERN_INFO "MIH InAddrNotifier: %s has no ieee80211_hw\n",dev->name);
		return NOTIFY_DONE;
	}
	// check http://lxr.linux.no/#linux+v2.6.35/include/net/mac80211.h#L1018
*/

void
DevLinkUp(struct net_device *dev)
{
   link_tuple_id_t   linkidentifier;
   link_addr_t       oldaccessrouter;
   link_addr_t       newaccessrouter;
   ip_renewal_flag_t iprenewalflag;
   ip_mob_mgmt_t     mobilitymanagementsupport;
	transport_addr_t  mac_addr;
	int status;

	// struct wireless_dev *wdev = dev->ieee80211_ptr;

	// printk(KERN_INFO "MIH: %s NIC Link is Up\n", dev->name);

	// Send Link_Up.indication to local MIHF entity

	// Link_Up.indication (63):
	// Generated when a layer 2 connection is established for the specified link interface

/*
typedef uint8_t link_type_t; // unsigned_int(1) (233)
#define WIRELESS_GSM              1
#define WIRELESS_GPRS             2
#define WIRELESS_EDGE             3
#define ETHERNET                 15
#define WIRELESS_OTHER           18
#define WIRELESS_IEEE_802_11     19
#define WIRELESS_CDMA2000        22
#define WIRELESS_UMTS            23
#define WIRELESS_IEEE_802_16     27
...

typedef struct {
   link_id_t link_id;
		typedef struct {
   		link_type_t link_type;
				typedef uint8_t link_type_t; // unsigned_int(1) (233)
   		link_addr_t link_addr;
				typedef struct {
   				uint16_t addr_type;  // (1) unsigned_int(2)
   				uint8_t * addr;      // octet_string
				} transport_addr_t;
				// (1) http://www.iana.org/assignments/address-family-numbers
				// 1:IP(IPv4),2:IP6(IPv6),...,6:802 (all 802 media+Eth "canonical format")
		} link_id_t;      // sequence(link_type,link_addr) (229)
   link_addr_t *link_addr;
		typedef union {
   		mac_addr_t mac_addr;
				typedef transport_addr_t mac_addr_t;
					typedef struct {
   					uint16_t addr_type;  // (1) unsigned_int(2)
   					uint8_t * addr;      // octet_string
					} transport_addr_t;
   		tgpp_3g_cell_id_t  tgpp_3g_cell_id;
 			...
		} link_addr_t;
} link_tuple_id_t;

// indicates wether MN's IP address needs to be changed or not.
typedef boolean_t ip_renewal_flag_t; // (239)

typedef uint16_t ip_mob_mgmt_t; // bitmap(16)
*/
	// link_id
	// Linux internal device types:
   // #define ARPHRD_ETHER    1               /* Ethernet 10Mbps              */
   // ...
   // #define ARPHRD_IEEE80211 801            /* IEEE 802.11
	switch (dev->type) {
		case ARPHRD_ETHER:   // Ethernet and 802.11 with ethernet framing
			if(dev->ieee80211_ptr == NULL)
				linkidentifier.link_id.link_type = ETHERNET;
			else
				linkidentifier.link_id.link_type = WIRELESS_IEEE_802_11;
			break;
		case ARPHRD_IEEE80211: // 801: IEEE 802.11
			linkidentifier.link_id.link_type = WIRELESS_IEEE_802_11;
			break;
	}
	linkidentifier.link_id.link_addr.mac_addr.addr_type = 6; // 802 (all 802)
	// point to dev's hadrware addr OR allocate mem & copy mac addr...
	linkidentifier.link_id.link_addr.mac_addr.addr = dev->dev_addr;
	// link_addr
	mac_addr.addr_type = 6;
	mac_addr.addr = dev->dev_addr;
	linkidentifier.link_addr = (link_addr_t*)&mac_addr;
	// point to dev's hadrware addr OR allocate mem & copy mac addr...

	oldaccessrouter.mac_addr.addr_type = 6;
	// oldaccessrouter.mac_addr.addr = ;	// allocate mem & copy mac addr...

	newaccessrouter.mac_addr.addr_type = 6;
	// newaccessrouter.mac_addr.addr = ;	// allocate mem & copy mac addr...

	iprenewalflag = FALSE; // How do we know this?
   mobilitymanagementsupport = 0;

	status = Link_Up_indication( linkidentifier, oldaccessrouter, newaccessrouter,
		 iprenewalflag, mobilitymanagementsupport);

	// if(status)
		//	printk(KERN_INFO "MIH: problem with Link_Up_indication\n");
}

void
DevLinkDown(struct net_device *dev)
{
	// struct wireless_dev *wdev = dev->ieee80211_ptr;

   link_tuple_id_t  linkidentifier;
   link_addr_t      oldaccessrouter;
   link_dn_reason_t reasoncode;
	transport_addr_t mac_addr;
	int status;

	// printk(KERN_INFO "MIH: %s NIC Link is Down\n", dev->name);

/*
	// Link down reason codes (235)
	#define LD_EXPLICIT_DISCONNECT       0
	#define LD_PACKET_TIMEOUT            1
	#define LD_NO_RESOURCE               2
	#define LD_NO_BROADCAST              3
	#define LD_AUTHENTICATION_FAILURE    4
	#define LD_BILLING_FAILURE           5

	typedef uint32_t link_dn_reason_t; // unsigned_int(1) (228) (235)
*/
	// link_id
	linkidentifier.link_id.link_type = ETHERNET; // WIRELESS_IEEE_802_11
	linkidentifier.link_id.link_addr.mac_addr.addr_type = 6; // 802 (all 802)
	// point to dev's hadrware addr OR allocate mem & copy mac addr...
	linkidentifier.link_id.link_addr.mac_addr.addr = dev->dev_addr;
	// link_addr
	mac_addr.addr_type = 6;
	mac_addr.addr = dev->dev_addr;
	linkidentifier.link_addr = (link_addr_t*)&mac_addr;
	// point to dev's hadrware addr OR allocate mem & copy mac addr...

	oldaccessrouter.mac_addr.addr_type = 6;
	// oldaccessrouter.mac_addr.addr = ;	// allocate mem & copy mac addr...

	reasoncode = LD_EXPLICIT_DISCONNECT; 	// ?

	// Notify the local MIHF, if it has subscribed to this event...

	status = Link_Down_indication( linkidentifier, oldaccessrouter, reasoncode);

	// if(status)
		//	printk(KERN_INFO "MIH: problem with Link_Down_indication\n");
}

void
DevLinkGoingDown(struct net_device *dev)
{
	// struct wireless_dev *wdev = dev->ieee80211_ptr;

   link_tuple_id_t  linkidentifier;
   uint16_t         timeinterval;
   link_gd_reason_t linkgoingdownreason;

	transport_addr_t mac_addr;
	int status;

	// printk(KERN_INFO "MIH: %s NIC Link is Down\n", dev->name);

/*
	// Link going down reason codes (235)
	#define LGD_EXPLICIT_DISCONNECT       0
	#define LGD_LINK_PAR_DEGRAGING        1
	#define LGD_LOW_POWER                 2
	#define LGD_NO_RESOURCE               3

	typedef uint8_t link_gd_reason_t;   // (229)
*/
	// link_id
	linkidentifier.link_id.link_type = ETHERNET; // WIRELESS_IEEE_802_11
	linkidentifier.link_id.link_addr.mac_addr.addr_type = 6; // 802 (all 802)
	// point to dev's hadrware addr OR allocate mem & copy mac addr...
	linkidentifier.link_id.link_addr.mac_addr.addr = dev->dev_addr;
	// link_addr
	mac_addr.addr_type = 6;
	mac_addr.addr = dev->dev_addr;
	linkidentifier.link_addr = (link_addr_t*)&mac_addr;
	// point to dev's hadrware addr OR allocate mem & copy mac addr...

	// how to find out the reason?
	linkgoingdownreason = LGD_EXPLICIT_DISCONNECT; 	// ?

	// timeinterval: time interval (ms) at which the link is expected to do down.
	// 0 if unknown
	timeinterval = 0;

	// Notify the local MIHF, if it has subscribed to this event...

	status = Link_Going_Down_indication(linkidentifier,timeinterval,linkgoingdownreason);

	// if(status)
		//	printk(KERN_INFO "MIH: problem with Link_Going_Down_indication\n");
}





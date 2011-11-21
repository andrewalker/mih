/*
 * IEEE 802.21 kernel module.
 *
 * MIHF entity, including the link layer and user layer functions.
 */


#include <linux/module.h>	/* Needed by all modules. */
#include <linux/moduleparam.h>	/* We use parameters in this module. */
#include <linux/init.h>		/* Needed for the macros (init_module, ...). */
#include <linux/kernel.h>	/* Needed for KERN_INFO. */
#include <linux/slab.h>		/* For kmalloc. */
#include <linux/kthread.h>	/* For kthread_{run, should_stop, stop}. */
#include <linux/inetdevice.h>	/* For register_inetaddr_notifier. */
#include <linux/if_arp.h>	/* For ARPHRD_ETHER and ARPHRD_IEEE80211. */
#include <linux/tcp.h>		/* For TCP_NODELAY. */
#include <linux/ieee80211.h>	/* For IEEE80211_MAX_SSID_LEN. */

#include <net/cfg80211.h>	/* For wireless_dev. */

#include "mih.h"		/* Local type definitions for MIH. */
#include "data.h"		/* Internal data structures for MIH. */


#define MODULE_NAME "mih_mod"

MODULE_AUTHOR("Helio Guardia");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Media Independent Handover Framework");


/*
 * Module parameter: verbosity.
 * To use this parameter:
 * # insmod mih_mod.ko verbose=1
 */
static int verbose = 0;
module_param(verbose, int, S_IWUSR | S_IRUGO);
MODULE_PARM_DESC(verbose, "Causes control messages to be sent to "
							"/var/log/messages");


/* Global variables. */

#define DBG(x) printk(KERN_INFO x)

struct task_struct *_net_hand_t = NULL;	/* Kernel thread. */
struct task_struct *_dev_mon_t = NULL;	/* Kernel thread. */
struct task_struct *_mihf_t = NULL;	/* Kernel thread. */

mih_tlv_t _src_mihf_id;
mih_tlv_t _dst_mihf_id;

int _threads_should_stop = 0;


#include "queue.h"
#include "message.c"
#include "ksock.c"
#include "mih_sap.c"
#include "mih_link_sap.c"
#include "net_hand.c"
#include "dev_mon.c"
#include "mihf.c"


void kill_net_hand_thread(void);

/*
 * Controls whether symbols are exported or not.
 * Prevents namespace pollution
 */
int SymbolExport(void)
{
	return 0;
}


int mih_Init(void)
{
	/* Allocates space for the sending and receiving buffers. */
	_snd_buf = kmalloc(MIH_PDU_LEN, GFP_KERNEL);
	_rcv_buf = kmalloc(MIH_PDU_LEN, GFP_KERNEL);

	/* Initialization of local MIH identifier: done in context.c. (?) */

	/* Global variable for Source MIHF ID. */
	_src_mihf_id.type = SRC_MIHF_ID_TLV;
	_src_mihf_id.length = strlen(_mymihfid);
	_src_mihf_id.value = kmalloc(_src_mihf_id.length, GFP_KERNEL);
	memcpy(_src_mihf_id.value, _mymihfid, strlen(_mymihfid));

	/* Global variable for Destination MIHF ID. */
	_dst_mihf_id.type = DST_MIHF_ID_TLV;
	_dst_mihf_id.length = strlen(_mymihfid);
	_dst_mihf_id.value = kmalloc(_dst_mihf_id.length, GFP_KERNEL);
	memcpy(_dst_mihf_id.value, _mymihfid, strlen(_mymihfid));

	return 0;
}


static int __init mod_Start(void)
{
	int err;

	DBG("Entering mod_Start");
	allow_signal(SIGKILL);

	err = SymbolExport();
	if (err)
		return err;

	err = mih_Init();
	if (err)
		return err;

	init_queue();

	DBG("Registering notifiers");
	/* Registers routine to watch for changes in the status of devices. */
	if (register_netdevice_notifier(&mih_netdev_notifier)) {
		printk(KERN_INFO "MIH DevMon: failure registering netdev "
								"notifier\n");
		return -1;
	}
	if (register_inetaddr_notifier(&mih_inaddr_notifier)) {
		printk(KERN_INFO "MIH DevMon: failure registering inetaddr "
								"notifier\n");
		return -1;
	}

	DBG("Creating kernel threads");
	/* Create working kernel threads. */
	if ((_net_hand_t = kthread_run(NetHandler, NULL, MODULE_NAME)) == NULL)
		goto kthread_failure;
	if ((_dev_mon_t = kthread_run(DevMon, NULL, MODULE_NAME)) == NULL)
		goto kthread_failure;
	if ((_mihf_t = kthread_run(Mihf, NULL, MODULE_NAME)) == NULL)
		goto kthread_failure;

	/*
	 * How to avoid a failure in the creation of a thread to damage
	 * the system?
	 */

	printk(KERN_INFO "MIH Module loaded\n");

	return 0;

kthread_failure:
	DBG("Failed creating a kernel thread");

	DBG("Notifying threads they should stop");
	_threads_should_stop = 1;

	kill_net_hand_thread();
	DBG("Killing dev_mon thread");
	if (_dev_mon_t)
		kthread_stop(_dev_mon_t);
	DBG("Killing mihf thread");
	if (_mihf_t)
		kthread_stop(_mihf_t);

	return err;
}

void kill_net_hand_thread(void)
{
	int s;
	struct sockaddr_in loopback_addr;
	struct socket *wakeup_socket;

	DBG("Entering kill_net_hand_thread");

	if (!_net_hand_t)
		return;

	DBG("Allocating sock_addr");
	/* Use a socket to force the net_hand thread to wake up */
	memset(&loopback_addr, 0, sizeof(loopback_addr));
	loopback_addr.sin_family = AF_INET;
	loopback_addr.sin_port = htons(MIHF_PORT);
	loopback_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	DBG("Creating socket");
	s = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &wakeup_socket);
	if (s < 0) {
		printk(KERN_ERR "MIH Main module: error %d creating wake-up "
				"socket\n", s);
		return;
	}

	DBG("Connecting");
	wakeup_socket->ops->connect(wakeup_socket,
			(struct sockaddr *)&loopback_addr,
			sizeof(loopback_addr), O_RDWR);
	DBG("Shutting down the socket");
	wakeup_socket->ops->shutdown(wakeup_socket, SHUT_RDWR);

	DBG("Stopping thread");
	kthread_stop(_net_hand_t);
}

static void __exit mod_End(void)
{
	DBG("Notifying threads they should stop");
	_threads_should_stop = 1;

	DBG("Entering mod_End");
	/* Stop kernel threads. */
	DBG("Kill net_hand thread");
	kill_net_hand_thread();
	DBG("Kill dev_mon thread");
	if (_dev_mon_t) {
		kthread_stop(_dev_mon_t);
	}
	DBG("Kill mihf thread");
	if (_mihf_t) {
		kthread_stop(_mihf_t);
	}

	/* Unregisters routine. */
	DBG("Unregistering notifiers");
	unregister_netdevice_notifier(&mih_netdev_notifier);
	unregister_inetaddr_notifier(&mih_inaddr_notifier);

	/* Frees the memory used by the sending and receiving buffers. */
	DBG("Free buffers");
	kfree(_snd_buf);
	kfree(_rcv_buf);

	printk(KERN_INFO "MIH module unloaded...\n");
}


module_init(mod_Start);
module_exit(mod_End);

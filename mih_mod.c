/*
 802.21 kernel module

 MIHF entity, including the link layer and user layer functions
*/


#include <linux/module.h>       // Needed by all modules
#include <linux/moduleparam.h>  // Should be included if we use paramters in this module
#include <linux/init.h>         // Needed for the macros (init_module, ...)
#include <linux/kernel.h>       // Needed for KERN_INFO

#include <linux/slab.h>         // for kmalloc

// #include <net/sock.h>
// #include <linux/net.h>     // for socket related calls
// #include <linux/socket.h>
// #include <linux/in.h>

#include <linux/kthread.h>    // for kthread_[run, should_stop]
#include <linux/inetdevice.h> // for register_inetaddr_notifier
#include <linux/if_arp.h>     // for ARPHRD_ETHER and ARPHRD_IEEE80211
#include <linux/tcp.h>        // for TCP_NODELAY

#include <linux/ieee80211.h>	// for IEEE80211_MAX_SSID_LEN

// #include <linux/nl80211.h>
#include <net/cfg80211.h>	// for wireless_dev

// #include "core.h"

// #include <net/iw_handler.h>  // needed fot the west stuff
// #include <linux/wireless.h>  // needed fot the west stuff

// #include <net/inet_sock.h>
// #include <net/timewait_sock.h>
// #include <net/inet6_connection_sock.h>
// #include <net/request_sock.h>
// #include <net/inet_timewait_sock.h>
// #include <net/inet_connection_sock.h>

// #include "ext_types.h"		// external types not defined in accessible .h files

#include "mih.h"	// local type definitions for mih
#include "data.h" // internal data structures for mih


#define MODULE_NAME "mih_mod"

MODULE_AUTHOR("Helio Guardia");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Media Independent Handover Framework");
// MODULE_SUPPORTED_DEVICE ();


// module parameter: verbosity
static int verbose=0;
module_param(verbose, int, 0644);
MODULE_PARM_DESC(verbose, "Causes control messages to be sent to /var/log/messages");
// # modinfo mih_module.ko
// To use the parameter:
// # insmod mih_module.ko verbose=1


// Global variables

struct task_struct *_net_hand_t = NULL; // kernel thread
struct task_struct *_dev_mon_t = NULL;  // kernel thread
struct task_struct *_mihf_t = NULL;   // kernel thread

/*
// input and output buffers are defined as global variables... in data.h
unsigned char *_snd_buf;	// sending buffer
unsigned char *_rcv_buf;	// receiving buffer
*/


mih_tlv_t _src_mihf_id;
mih_tlv_t _dst_mihf_id;


// other functions used by the module
// to be compiled separately...

#include "message.c"
#include "ksock.c"
#include "mih_sap.c"
#include "mih_link_sap.c"
#include "net_hand.c"
#include "dev_mon.c"
#include "mihf.c"

// struct net_device:
// http://lxr.linux.no/#linux+v2.6.34/include/linux/netdevice.h#L706
// ... struct wireless_dev *ieee80211_ptr; // IEEE 802.11 specific data
// struct wireless_dev:
// http://lxr.linux.no/#linux+v2.6.34/include/net/cfg80211.h#L1456


int
SymbolExport(void)
{
// Controls whether symbols are exported or not. Prevents namespace pollution
//
// register_symtab(NULL);
// same as:
// EXPORT_NO_SYMBOLS;
//
// EXPORT_SYMBOL (name);
//	EXPORT_SYMBOL_NOVERS (name);

	return 0;
}


int
mih_Init(void)
{
	// allocates space for the sending and receiving buffers
	// void * kmalloc (size_t size, int flags);
	_snd_buf = kmalloc(MIH_PDU_LEN, GFP_KERNEL);
	_rcv_buf = kmalloc(MIH_PDU_LEN, GFP_KERNEL);


	// initialization of local MIH identifier: done in context.c (?)

	// global variable for Source MIHF ID
	// mih_tlv_t _src_mihf_id;
	_src_mihf_id.type = SRC_MIHF_ID_TLV;
	_src_mihf_id.length = strlen(_mymihfid);
	_src_mihf_id.value = (unsigned char *)kmalloc(_src_mihf_id.length,GFP_KERNEL);
	memcpy(_src_mihf_id.value, _mymihfid, strlen(_mymihfid));

	// global variable for Destination MIHF ID
	// mih_tlv_t _dst_mihf_id;
	_dst_mihf_id.type = DST_MIHF_ID_TLV;
	_dst_mihf_id.length = strlen(_mymihfid);
	_dst_mihf_id.value = (unsigned char *)kmalloc(_dst_mihf_id.length,GFP_KERNEL);
	memcpy(_dst_mihf_id.value, _mymihfid, strlen(_mymihfid));

	return 0;
}


static int __init mod_Start(void)
{
	int err;

	// printk(KERN_INFO "Loading the MIH module...\n");

	// daemonize causes a dump in syslog...
	// daemonize(MODULE_NAME);
	// signals are disabled after daemonize()
	allow_signal(SIGKILL);

	// printk(KERN_INFO "The process is \"%s\" (pid %i)\n", current->comm, current->pid);

	err = SymbolExport();
	if (err)
		return err;

	// Socket initialization is now done in the NetHandler thread...
	// Netdevnotifier is now adjusted in the DevMon thread...

	err = mih_Init();
	if (err)
		return err;

	// create working kernel threads
/* por enquanto, nao criar essa thread
	if((_net_hand_t=kthread_run(NetHandler,NULL,MODULE_NAME))==NULL)
		goto kthread_failure;
*/
	if((_dev_mon_t=kthread_run(DevMon,NULL,MODULE_NAME))==NULL)
		goto kthread_failure;
	if((_mihf_t=kthread_run(Mihf,NULL,MODULE_NAME))==NULL)
		goto kthread_failure;

	// how to avoid a failure in the creation of a thread to damage the system?

	printk(KERN_INFO "MIH Module loaded\n");

	return 0;

kthread_failure:
	if(_net_hand_t)
		kthread_stop(_net_hand_t);
	if(_dev_mon_t)
		kthread_stop(_dev_mon_t);
	if(_mihf_t)
		kthread_stop(_mihf_t);

	return err;
}


static void __exit mod_End(void)
{
	// stop kernel threads
	if(_net_hand_t) {
		// printk(KERN_INFO "_net_hand's state: %ld\n",_net_hand_t->state);
		kthread_stop(_net_hand_t);
		// create a socket and connect to the socket to unblock the thread?
	}
	if(_dev_mon_t) {
		// printk(KERN_INFO "_dev_mon_t's state: %ld\n",_dev_mon_t->state);
		kthread_stop(_dev_mon_t);
	}
	if(_mihf_t){
		// printk(KERN_INFO "_mihf_t's state: %ld\n",_mihf_t->state);
		kthread_stop(_mihf_t);
	}

	// frees the memory used by the sending and receiving buffers
	kfree(_snd_buf);
	kfree(_rcv_buf);

	printk(KERN_INFO "MIH module unloaded...\n");
}


module_init(mod_Start);

module_exit(mod_End);




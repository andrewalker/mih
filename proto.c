
/*
	Protocol handling functions
 */


#include "proto.h"



int
ProcessService(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: ProcessService\n");

	if(header->sid != SERVICE_MANAGEMENT)
		return -1;

// MIH_Capability_Discover: Discover the cababilities of a local or remote MIHF
// MIH_Register: Register with a remote MIHF
// MIH_DeRegister: Deregister from a remote MIHF
// MIH_Event_Subscribe: Subscribe for one or more MIH events with a local or remote MIHF
// MIH_Event_Unsubscribe: Unsubscribe for one or more MHI events from a local or remote MIHF
	switch (header->aid) {
		case (MIH_CAPABILITY_DISCOVER):
			res=CapabilityDiscover(in_buf, in_len);
			break;
		case (MIH_REGISTER):
			res=Register(in_buf, in_len);
			break;
		case (MIH_DEREGISTER):
			res=Deregister(in_buf, in_len);
			break;
		case (MIH_EVENT_SUBSCRIBE):
			res=EventSubscribe(in_buf, in_len);
			break;
		case (MIH_EVENT_UNSUBSCRIBE):
			res=EventUnsubscribe(in_buf, in_len);
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown service management request\n");
			break;
	}
	return res;
}

int
ProcessEvent(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: ProcessEvent\n");

	if(header->sid != EVENT_SERVICE)
		return -1;

	switch (header->aid) {
		case (MIH_LINK_DETECTED):
			res=LinkDetected(in_buf, in_len);
			break;
		case (MIH_LINK_UP):
			res=LinkUp(in_buf, in_len);
			break;
		case (MIH_LINK_DOWN):
			res=LinkDown(in_buf, in_len);
			break;
		case (MIH_LINK_PARAMETERS_REPORT):
			res=LinkParametersReport(in_buf, in_len);
			break;
		case (MIH_LINK_GOING_DOWN):
			res=LinkGoingDown(in_buf, in_len);
			break;
		case (MIH_LINK_HANDOVER_IMMINENT):
			res=LinkHandoverImminent(in_buf, in_len);
			break;
		case (MIH_LINK_HANDOVER_COMPLETE):
			res=LinkHandoverComplete(in_buf, in_len);
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown event service request\n");
			break;
	}
	return res;
}

int
ProcessCmd(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: ProcessCmd\n");

	if(header->sid != COMMAND_SERVICE)
		return -1;

	switch (header->aid) {
		case (MIH_LINK_GET_PARAMETERS):
			res=LinkGetParameters(in_buf, in_len);
			break;
		case (MIH_LINK_CONFIGURE_THRESHOLDS):
			res=LinkConfigureThresholds(in_buf, in_len);
			break;
		case (MIH_LINK_ACTIONS):
			res=LinkActions(in_buf, in_len);
			break;
		case (MIH_NET_HO_CANDIDATE_QUERY):
			res=NetHOCandidateQuery(in_buf, in_len);
			break;
		case (MIH_MN_HO_CANDIDATE_QUERY):
			res=MNHOCandidateQuery(in_buf, in_len);
			break;
		case (MIH_N2N_HO_QUERY_RESOURCES):
			res=N2NHOQueryResources(in_buf, in_len);
			break;
		case (MIH_MN_HO_COMMIT):
			res=MNHOCommit(in_buf, in_len);
			break;
		case (MIH_NET_HO_COMMIT):
			res=NetHOCommit(in_buf, in_len);
			break;
		case (MIH_N2N_HO_COMMIT):
			res=N2NHOCommit(in_buf, in_len);
			break;
		case (MIH_MN_HO_COMPLETE):
			res=MNHOComplete(in_buf, in_len);
			break;
		case (MIH_N2N_HO_COMPLETE):
			res=N2NHOComplete(in_buf, in_len);
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown command service request\n");
			break;
	}
	return res;
}

int
ProcessInfo(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: ProcessInfo\n");

	if(header->sid != INFORMATION_SERVICE)
		return -1;

	switch (header->aid) {
		case (MIH_CAPABILITY_DISCOVER):
			res=CapabilityDiscover(in_buf, in_len);
			break;
		case (MIH_REGISTER):
			res=Register(in_buf, in_len);
			break;
		case (MIH_DEREGISTER):
			res=Deregister(in_buf, in_len);
			break;
		case (MIH_EVENT_SUBSCRIBE):
			res=EventSubscribe(in_buf, in_len);
			break;
		case (MIH_EVENT_UNSUBSCRIBE):
			res=EventUnsubscribe(in_buf, in_len);
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Information service request\n");
			break;
	}
	return res;
}


int
ProcessRequest(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) {
		printk(KERN_INFO "MIH: Processing request (%d)\n",in_len);
	}

	// do we need a structure for holding the information extracted from the PDU?
	switch (header->sid) {
		case (SERVICE_MANAGEMENT):
			res=ProcessService(in_buf, in_len);
			break;
		case (EVENT_SERVICE):
			res=ProcessEvent(in_buf, in_len);
			break;
		case (COMMAND_SERVICE):
			res=ProcessCmd(in_buf, in_len);
			break;
		case (INFORMATION_SERVICE):
			res=ProcessInfo(in_buf, in_len);
			break;
		default:
			if(verbose)
				// printk(KERN_INFO "MIH: Unknown message received\n");
			break;
	}

	return res;
}


int
CapabilityDiscover(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

   mihf_id_t         sourceidentifier;
   // mihf_id_t         destinationidentifier;
   status_t          status;
   net_type_addr_t * linkaddresslist;            // LIST(NET_TYPE_ADDR) // Optional
   mih_evt_list_t  * supportedmiheventlist;      // MIH_EVT_LIST        // Optional
   mih_cmd_list_t  * supportedmihcommandlist;    // MIH_CMD_LIST        // Optional
   mih_iq_type_lst_t * supportedisquerytypelist; // MIH_IQ_TYPE_LST     // Optional
   mih_trans_lst_t * supportedtransportlist;     // MIH_TRANS_LST       // Optional
   mbb_ho_supp_t   * mbbhandoversupport;

	if(verbose) printk(KERN_INFO "MIH: Capability Discovery\n");

	if(header->aid != MIH_CAPABILITY_DISCOVER)
		return -1;

	// compare destinationidentifier with local identifier (_mymihfid)?

	// requests and responses may arrive from the network
	switch (header->opcode) {
		case (REQUEST):
			// extract the parameters from the PDU...
			// call MIH_Capability_Discover_indication
			// MIH_Capability_Discover.indication (	// pp 78
			//		SourceIdentifier,
			//		LinkAddressList,           (optional)
			// 	SupportedMihEventList,     (optional)
			//		SupportedMihCommandList,   (optional)
			//		SupportedIsQueryTypeList,  (optional)
			//		SupportedTransportList,    (optional)
			//		MBBHandoverSupport	)     (optional)
			res=MIH_Capability_Discover_indication(sourceidentifier,linkaddresslist,
				supportedmiheventlist, supportedmihcommandlist,supportedisquerytypelist,
				supportedtransportlist,mbbhandoversupport);

			// respond automatically?

			break;

		case (RESPONSE):
			// extract the parameters from the PDU...
			// call MIH_Capability_Discover_confirm
			// MIH_Capability_Discover.confirm (	// pp 80
			//		SourceIdentifier,
			//		Status,
			//		LinkAddressList,           (optional)
			// 	SupportedMihEventList,     (optional)
			//		SupportedMihCommandList,   (optional)
			//		SupportedIsQueryTypeList,  (optional)
			//		SupportedTransportList,    (optional)
			//		MBBHandoverSupport	)     (optional)
			res=MIH_Capability_Discover_confirm(sourceidentifier,status,
				linkaddresslist, supportedmiheventlist, supportedmihcommandlist,
				supportedisquerytypelist, supportedtransportlist,mbbhandoversupport);
			break;

		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Capability Discover PDU\n");
			break;
	}

	return res;
}


int
Register(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Register\n");

	if(header->aid != MIH_REGISTER)
		return -1;

	switch (header->opcode) {
		// MIH_Register.request (
		//		DestinationIdentifier,
		//		LinkIdentifierList,
		//		REquestCode )
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Register PDU\n");
			break;
	}

	return res;
}
int
Deregister(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Deregister\n");

	if(header->aid != MIH_DEREGISTER)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Deregister PDU\n");
			break;
	}

	return res;
}

int
EventSubscribe(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Event Subscribe\n");

	if(header->aid != MIH_EVENT_SUBSCRIBE)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			// MIH_Event_Subscribe.request (
			//		DestinationIdentifier,
			//		LinkIdentifier,
			//		RequestedMihEventList,
			//		EventConfigurationInfoList )
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Event Subscribe PDU\n");
			break;
	}

	return res;
}

int
EventUnsubscribe(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Event Unsubscribe\n");

	if(header->aid != MIH_EVENT_UNSUBSCRIBE)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Event Unsubscribe PDU\n");
			break;
	}

	return res;
}

int
LinkDetected(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Link Detected\n");

	if(header->aid != MIH_LINK_DETECTED)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Link Detected PDU\n");
			break;
	}
	return res;
}

int
LinkUp(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Link Up\n");

	if(header->aid != MIH_LINK_UP)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Link Up PDU\n");
			break;
	}
	return res;
}

int
LinkDown(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Link Down\n");

	if(header->aid != MIH_LINK_DOWN)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Link Down PDU\n");
			break;
	}
	return res;
}

int
LinkParametersReport(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Link Parameters Report\n");

	if(header->aid != MIH_LINK_PARAMETERS_REPORT)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Link Parameter Report PDU\n");
			break;
	}
	return res;
}

int
LinkGoingDown(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Link Going Down\n");

	if(header->aid != MIH_LINK_GOING_DOWN)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Link Going Down PDU\n");
			break;
	}
	return res;
}

int
LinkHandoverImminent(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Link Handover Imminent\n");

	if(header->aid != MIH_LINK_HANDOVER_IMMINENT)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Link Handover Imminent PDU\n");
			break;
	}
	return res;
}

int
LinkHandoverComplete(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Link Handover Complete\n");

	if(header->aid != MIH_LINK_HANDOVER_COMPLETE)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Link Handover Complete PDU\n");
			break;
	}
	return res;
}

int
LinkGetParameters(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Link Get Parameters\n");

	if(header->aid != MIH_LINK_GET_PARAMETERS)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Link Get Parameters PDU\n");
			break;
	}
	return res;
}
int
LinkConfigureThresholds(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Link Configure Thresholds\n");

	if(header->aid != MIH_LINK_CONFIGURE_THRESHOLDS)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Link Configure Thresholds PDU\n");
			break;
	}
	return res;
}
int
LinkActions(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Link Actions\n");

	if(header->aid != MIH_LINK_ACTIONS)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Link Actions PDU\n");
			break;
	}
	return res;
}

int
NetHOCandidateQuery(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Net HO Candidate\n");

	if(header->aid != MIH_NET_HO_CANDIDATE_QUERY)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Net HO Candidate Query PDU\n");
			break;
	}
	return res;
}

int
MNHOCandidateQuery(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: MN HO Candidate Query\n");

	if(header->aid != MIH_MN_HO_CANDIDATE_QUERY)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown MN HO Candidate Query PDU\n");
			break;
	}
	return res;
}

int
N2NHOQueryResources(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: N2N HO Query Resources\n");

	if(header->aid != MIH_N2N_HO_QUERY_RESOURCES)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown N2N HO Query Resources PDU\n");
			break;
	}
	return res;
}

int
MNHOCommit(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: MN HO Commit\n");

	if(header->aid != MIH_MN_HO_COMMIT)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown MN HO Commit PDU\n");
			break;
	}
	return res;
}

int
NetHOCommit(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: Net HO Commit\n");

	if(header->aid != MIH_NET_HO_COMMIT)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown Net HO Commit PDU\n");
			break;
	}
	return res;
}

int
N2NHOCommit(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: N2N HO Commit\n");

	if(header->aid != MIH_N2N_HO_COMMIT)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown N2N HO Commit PDU\n");
			break;
	}
	return res;
}

int
MNHOComplete(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: MN HO Complete\n");

	if(header->aid != MIH_MN_HO_COMPLETE)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown MN HO Complete PDU\n");
			break;
	}
	return res;
}

int
N2NHOComplete(char *in_buf, int in_len)
{
	int res = -1;
	mih_header_t *header = (mih_header_t *)in_buf;

	if(verbose) printk(KERN_INFO "MIH: N2N HO Complete\n");

	if(header->aid != MIH_N2N_HO_COMPLETE)
		return -1;

	switch (header->opcode) {
		case (REQUEST):
			res=0;
			break;
		case (INDICATION):
			res=0;
			break;
		default:
			if(verbose)
				printk(KERN_INFO "MIH: Unknown N2N HO Complete PDU\n");
			break;
	}
	return res;
}




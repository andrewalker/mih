/*
 * Protocol handling functions
 */

#include "proto.h"


int ProcessService(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: ProcessService\n");

	if (message->header.sid != SERVICE_MANAGEMENT)
		return -1;

	switch (message->header.aid) {
		case (MIH_CAPABILITY_DISCOVER):
			res = CapabilityDiscover(message);
			break;
		case (MIH_REGISTER):
			res = Register(message);
			break;
		case (MIH_DEREGISTER):
			res = Deregister(message);
			break;
		case (MIH_EVENT_SUBSCRIBE):
			res = EventSubscribe(message);
			break;
		case (MIH_EVENT_UNSUBSCRIBE):
			res = EventUnsubscribe(message);
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown service "
						"management request\n");
			break;
	}
	return res;
}


int ProcessEvent(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: ProcessEvent\n");

	if (message->header.sid != EVENT_SERVICE)
		return -1;

	switch (message->header.aid) {
		case (MIH_LINK_DETECTED):
			res = LinkDetected(message);
			break;
		case (MIH_LINK_UP):
			res = LinkUp(message);
			break;
		case (MIH_LINK_DOWN):
			res = LinkDown(message);
			break;
		case (MIH_LINK_PARAMETERS_REPORT):
			res = LinkParametersReport(message);
			break;
		case (MIH_LINK_GOING_DOWN):
			res = LinkGoingDown(message);
			break;
		case (MIH_LINK_HANDOVER_IMMINENT):
			res = LinkHandoverImminent(message);
			break;
		case (MIH_LINK_HANDOVER_COMPLETE):
			res = LinkHandoverComplete(message);
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown event service "
						"request\n");
			break;
	}
	return res;
}


int ProcessCmd(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: ProcessCmd\n");

	if (message->header.sid != COMMAND_SERVICE)
		return -1;

	switch (message->header.aid) {
		case (MIH_LINK_GET_PARAMETERS):
			res = LinkGetParameters(message);
			break;
		case (MIH_LINK_CONFIGURE_THRESHOLDS):
			res = LinkConfigureThresholds(message);
			break;
		case (MIH_LINK_ACTIONS):
			res = LinkActions(message);
			break;
		case (MIH_NET_HO_CANDIDATE_QUERY):
			res = NetHOCandidateQuery(message);
			break;
		case (MIH_MN_HO_CANDIDATE_QUERY):
			res = MNHOCandidateQuery(message);
			break;
		case (MIH_N2N_HO_QUERY_RESOURCES):
			res = N2NHOQueryResources(message);
			break;
		case (MIH_MN_HO_COMMIT):
			res = MNHOCommit(message);
			break;
		case (MIH_NET_HO_COMMIT):
			res = NetHOCommit(message);
			break;
		case (MIH_N2N_HO_COMMIT):
			res = N2NHOCommit(message);
			break;
		case (MIH_MN_HO_COMPLETE):
			res = MNHOComplete(message);
			break;
		case (MIH_N2N_HO_COMPLETE):
			res = N2NHOComplete(message);
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown command service "
						"request\n");
			break;
	}
	return res;
}


int ProcessInfo(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: ProcessInfo\n");

	if (message->header.sid != INFORMATION_SERVICE)
		return -1;

	switch (message->header.aid) {
		case (MIH_CAPABILITY_DISCOVER):
			res = CapabilityDiscover(message);
			break;
		case (MIH_REGISTER):
			res = Register(message);
			break;
		case (MIH_DEREGISTER):
			res = Deregister(message);
			break;
		case (MIH_EVENT_SUBSCRIBE):
			res = EventSubscribe(message);
			break;
		case (MIH_EVENT_UNSUBSCRIBE):
			res = EventUnsubscribe(message);
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Information "
						"service request\n");
			break;
	}
	return res;
}


int ProcessRequest(mih_message_t *message)
{
	int res = -1;

	if (verbose) {
		printk(KERN_INFO "MIH: Processing request\n");
	}

	/* Do we need a structure for holding the information extracted from
	   the PDU? */
	switch (message->header.sid) {
		case (SERVICE_MANAGEMENT):
			res = ProcessService(message);
			break;
		case (EVENT_SERVICE):
			res = ProcessEvent(message);
			break;
		case (COMMAND_SERVICE):
			res = ProcessCmd(message);
			break;
		case (INFORMATION_SERVICE):
			res = ProcessInfo(message);
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown message "
						"received\n");
			break;
	}
	return res;
}


int CapabilityDiscover(mih_message_t *message)
{
	int res = -1;

	mihf_id_t sourceidentifier;
	/* mihf_id_t  destinationidentifier; */
	status_t status;
	net_type_addr_t *linkaddresslist; /* LIST(NET_TYPE_ADDR) */
	mih_evt_list_t *supportedmiheventlist; /* MIH_EVT_LIST */
	mih_cmd_list_t *supportedmihcommandlist; /* MIH_CMD_LIST */
	mih_iq_type_lst_t *supportedisquerytypelist; /* MIH_IQ_TYPE_LST */
	mih_trans_lst_t *supportedtransportlist; /* MIH_TRANS_LST */
	mbb_ho_supp_t *mbbhandoversupport;

	if (verbose)
		printk(KERN_INFO "MIH: Capability Discovery\n");

	if (message->header.aid != MIH_CAPABILITY_DISCOVER)
		return -1;

	/* Compare destinationidentifier with local identifier (_mymihfid)? */

	/* Requests and responses may arrive from the network. */
	switch (message->header.opcode) {
		case (REQUEST):
			res = MIH_Capability_Discover_indication(
					sourceidentifier,
					linkaddresslist,
					supportedmiheventlist,
					supportedmihcommandlist,
					supportedisquerytypelist,
					supportedtransportlist,
					mbbhandoversupport);

			/* Respond automatically? */

			break;

		case (RESPONSE):
			res = MIH_Capability_Discover_confirm(
					sourceidentifier,
					status,
					linkaddresslist,
					supportedmiheventlist,
					supportedmihcommandlist,
					supportedisquerytypelist,
					supportedtransportlist,
					mbbhandoversupport);
			break;

		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Capability "
						"Discover PDU\n");
			break;
	}
	return res;
}


int Register(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Register\n");

	if (message->header.aid != MIH_REGISTER)
		return -1;

	switch (message->header.opcode) {
		/*
		 * MIH_Register.request(
		 *		DestinationIdentifier,
		 *		LinkIdentifierList,
		 *		REquestCode);
		 */
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Register "
						"PDU\n");
			break;
	}

	return res;
}

int Deregister(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Deregister\n");

	if (message->header.aid != MIH_DEREGISTER)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Deregister "
						"PDU\n");
			break;
	}
	return res;
}


int EventSubscribe(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Event Subscribe\n");

	if (message->header.aid != MIH_EVENT_SUBSCRIBE)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			/*
			 * MIH_Event_Subscribe.request(
			 *		DestinationIdentifier,
			 *		LinkIdentifier,
			 *		RequestedMihEventList,
			 *		EventConfigurationInfoList);
			 */
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Event Subscribe "
						"PDU\n");
			break;
	}
	return res;
}


int EventUnsubscribe(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Event Unsubscribe\n");

	if (message->header.aid != MIH_EVENT_UNSUBSCRIBE)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Event "
						"Unsubscribe PDU\n");
			break;
	}
	return res;
}


int LinkDetected(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Link Detected\n");

	if (message->header.aid != MIH_LINK_DETECTED)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Link Detected "
						"PDU\n");
			break;
	}
	return res;
}


int LinkUp(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Link Up\n");

	if (message->header.aid != MIH_LINK_UP)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Link Up PDU\n");
			break;
	}
	return res;
}


int LinkDown(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Link Down\n");

	if (message->header.aid != MIH_LINK_DOWN)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Link Down "
						"PDU\n");
			break;
	}
	return res;
}


int LinkParametersReport(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Link Parameters Report\n");

	if (message->header.aid != MIH_LINK_PARAMETERS_REPORT)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Link Parameter "
						"Report PDU\n");
			break;
	}
	return res;
}


int LinkGoingDown(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Link Going Down\n");

	if (message->header.aid != MIH_LINK_GOING_DOWN)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Link Going Down "
						"PDU\n");
			break;
	}
	return res;
}


int LinkHandoverImminent(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Link Handover Imminent\n");

	if (message->header.aid != MIH_LINK_HANDOVER_IMMINENT)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Link Handover "
						"Imminent PDU\n");
			break;
	}
	return res;
}


int LinkHandoverComplete(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Link Handover Complete\n");

	if (message->header.aid != MIH_LINK_HANDOVER_COMPLETE)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Link Handover "
						"Complete PDU\n");
			break;
	}
	return res;
}


int LinkGetParameters(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Link Get Parameters\n");

	if (message->header.aid != MIH_LINK_GET_PARAMETERS)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Link Get "
						"Parameters PDU\n");
			break;
	}
	return res;
}


int LinkConfigureThresholds(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Link Configure Thresholds\n");

	if (message->header.aid != MIH_LINK_CONFIGURE_THRESHOLDS)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Link Configure "
						"Thresholds PDU\n");
			break;
	}
	return res;
}


int LinkActions(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Link Actions\n");

	if (message->header.aid != MIH_LINK_ACTIONS)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Link Actions "
						"PDU\n");
			break;
	}
	return res;
}


int NetHOCandidateQuery(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Net HO Candidate\n");

	if (message->header.aid != MIH_NET_HO_CANDIDATE_QUERY)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Net HO Candidate"
						" Query PDU\n");
			break;
	}
	return res;
}


int MNHOCandidateQuery(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: MN HO Candidate Query\n");

	if (message->header.aid != MIH_MN_HO_CANDIDATE_QUERY)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown MN HO Candidate "
						"Query PDU\n");
			break;
	}
	return res;
}


int N2NHOQueryResources(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: N2N HO Query Resources\n");

	if (message->header.aid != MIH_N2N_HO_QUERY_RESOURCES)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown N2N HO Query "
						"Resources PDU\n");
			break;
	}
	return res;
}


int MNHOCommit(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: MN HO Commit\n");

	if (message->header.aid != MIH_MN_HO_COMMIT)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown MN HO Commit "
						"PDU\n");
			break;
	}
	return res;
}


int NetHOCommit(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: Net HO Commit\n");

	if (message->header.aid != MIH_NET_HO_COMMIT)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown Net HO Commit "
						"PDU\n");
			break;
	}
	return res;
}


int N2NHOCommit(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: N2N HO Commit\n");

	if (message->header.aid != MIH_N2N_HO_COMMIT)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown N2N HO Commit "
						"PDU\n");
			break;
	}
	return res;
}


int MNHOComplete(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: MN HO Complete\n");

	if (message->header.aid != MIH_MN_HO_COMPLETE)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown MN HO Complete "
						"PDU\n");
			break;
	}
	return res;
}


int N2NHOComplete(mih_message_t *message)
{
	int res = -1;

	if (verbose)
		printk(KERN_INFO "MIH: N2N HO Complete\n");

	if (message->header.aid != MIH_N2N_HO_COMPLETE)
		return -1;

	switch (message->header.opcode) {
		case (REQUEST):
			res = 0;
			break;
		case (INDICATION):
			res = 0;
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown N2N HO Complete "
						"PDU\n");
			break;
	}
	return res;
}

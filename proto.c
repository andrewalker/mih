/*
 * Protocol handling functions
 */

#include "proto.h"


int queue_message(mih_message_t *message, struct reply_handler *reply)
{
	struct reply_parameter *param;

	param = kmalloc(sizeof(*param), GFP_KERNEL);
	param->message = message;
	param->param = reply->param;

	queue_task(&dispatch_queue, reply->handler, (void*)param);

	return 0;
}

int Acknowledge(int sid, int opcode, int aid, int tid, char* dst_mihf_id,
	struct reply_handler *reply)
{
	mih_tlv_t *ack_src_mihf_id_tlv;
	mih_tlv_t *ack_dst_mihf_id_tlv;
	mih_message_t* ack_msg;
	int src_mihf_id_len = strlen(_mymihfid);
	int dst_mihf_id_len = strlen(dst_mihf_id);

	ack_msg = kmalloc(sizeof(*ack_msg), GFP_KERNEL);
	ack_src_mihf_id_tlv = kmalloc(sizeof(*ack_src_mihf_id_tlv), GFP_KERNEL);
	ack_dst_mihf_id_tlv = kmalloc(sizeof(*ack_dst_mihf_id_tlv), GFP_KERNEL);

	memset(&ack_msg->header, 0, sizeof(ack_msg->header));
	ack_msg->header.version = 1;
	ack_msg->header.ackrsp = 1;
	ack_msg->header.sid = sid;
	ack_msg->header.opcode = opcode;
	ack_msg->header.aid = aid;
	ack_msg->header.tid = tid;

	ack_src_mihf_id_tlv->type = SRC_MIHF_ID_TLV;
	ack_src_mihf_id_tlv->length = src_mihf_id_len;
	ack_src_mihf_id_tlv->value = kmalloc(src_mihf_id_len + 1, GFP_KERNEL);
	strcpy(ack_src_mihf_id_tlv->value, _mymihfid);

	ack_dst_mihf_id_tlv->type = DST_MIHF_ID_TLV;
	ack_dst_mihf_id_tlv->length = dst_mihf_id_len;
	ack_dst_mihf_id_tlv->value = kmalloc(dst_mihf_id_len + 1, GFP_KERNEL);
	strcpy(ack_dst_mihf_id_tlv->value, dst_mihf_id);

	INIT_LIST_HEAD(&ack_msg->tlvs.list);
	list_add_tail(&ack_src_mihf_id_tlv->list, &ack_msg->tlvs.list);
	list_add_tail(&ack_dst_mihf_id_tlv->list, &ack_msg->tlvs.list);

	queue_message(ack_msg, reply);
	
	return 0;
}

int ProcessService(mih_message_t *message, struct reply_handler* reply)
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


int ProcessEvent(mih_message_t *message, struct reply_handler *reply)
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


int ProcessCmd(mih_message_t *message, struct reply_handler *reply)
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


int ProcessInfo(mih_message_t *message, struct reply_handler *reply)
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


int ProcessRequest(mih_message_t *message, struct reply_handler *reply)
{
	mih_tlv_t *src_mihf_id_tlv = list_first_entry(&message->tlvs.list,
			mih_tlv_t, list);
	mih_tlv_t *dst_mihf_id_tlv = list_entry(src_mihf_id_tlv->list.next,
			mih_tlv_t, list);
	char *src_mihf_id = (char*)src_mihf_id_tlv->value;
	char *dst_mihf_id = (char*)dst_mihf_id_tlv->value;
	int res = -1;

	if (verbose) {
		printk(KERN_INFO "MIH: Processing request\n");
	}

	if (strcmp(dst_mihf_id, _mymihfid) != 0) {
		goto out;
	}

	if (message->header.ackreq) {
		Acknowledge(message->header.sid, message->header.opcode,
				message->header.aid, message->header.tid,
				src_mihf_id, reply);
	}

	/* Do we need a structure for holding the information extracted from
	   the PDU? */
	switch (message->header.sid) {
		case (SERVICE_MANAGEMENT):
			res = ProcessService(message, reply);
			break;
		case (EVENT_SERVICE):
			res = ProcessEvent(message, reply);
			break;
		case (COMMAND_SERVICE):
			res = ProcessCmd(message, reply);
			break;
		case (INFORMATION_SERVICE):
			res = ProcessInfo(message, reply);
			break;
		default:
			if (verbose)
				printk(KERN_INFO "MIH: Unknown message "
						"received\n");
			break;
	}
out:
	return res;
}


int CapabilityDiscover(mih_message_t *message)
{
	int res = -1;

	mihf_id_t src_id;
	/* mihf_id_t  destinationidentifier; */
	status_t status;
	net_type_addr_t *link_addrs;
	mih_evt_list_t *supported_events;
	mih_cmd_list_t *supported_commands;
	mih_iq_type_lst_t *supported_is_query_types;
	mih_trans_lst_t *supported_transports;
	mbb_ho_supp_t *mbb_ho_support;

	if (verbose)
		printk(KERN_INFO "MIH: Capability Discovery\n");

	if (message->header.aid != MIH_CAPABILITY_DISCOVER)
		return -1;

	/* Compare destinationidentifier with local identifier (_mymihfid)? */

	switch (message->header.opcode) {
		case (REQUEST):

			/* Respond automatically? */

			break;

		case (RESPONSE):
			res = MIH_Capability_Discover_confirm(
					src_id,
					status,
					link_addrs,
					supported_events,
					supported_commands,
					supported_is_query_types,
					supported_transports,
					mbb_ho_support);
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

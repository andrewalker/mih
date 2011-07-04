

// Primitives

// // MIH Service Management
// MIH_Capability_Discover
// MIH_Register
// MIH_DeRegister
// MIH_Event_Subscribe
// MIH_Event_Unsubscribe

// // MIH Event Service
// MIH_Link_Detected
// MIH_Link_Up
// MIH_Link_Down
// MIH_Link_Parameters_Report
// MIH_Link_Going_Down
// MIH_Link_Handover_Imminent
// MIH_Link_Handover_Complete
// MIH_Link_PDU_Transmit_Status

// // MIH Command Service
// MIH_Link_Get_Parameters
// MIH_Link_Configure_Thresholds
// MIH_Link_Actions
// MIH_Net_HO_Candidate_Query
// MIH_MN_HO_Candidate_Query
// MIH_N2N_HO_Query_Resources
// MIH_MN_HO_Commit
// MIH_Net_HO_Commit
// MIH_N2N_HO_Commit
// MIH_MN_HO_Complete
// MIH_N2N_HO_Complete

// // MIH Information Service
// MIH_Get_Information
// MIH_Put_Information



// MIH_Capability_Discover
int
MIH_Capability_Discover_request(
	mihf_id_t destinationidentifier,              // MIHF_ID
	net_type_addr_t * linkaddresslist,            // LIST(NET_TYPE_ADDR) // Optional
	mih_evt_list_t  * supportedmiheventlist,      // MIH_EVT_LIST        // Optional
	mih_cmd_list_t  * supportedcommandlist,       // MIH_CMD_LIST        // Optional
	mih_iq_type_lst_t * supportedisquerytypelist, // MIH_IQ_TYPE_LST     // Optional
	mih_trans_lst_t * supportedtransportlist,     // MIH_TRANS_LST       // Optional
	mbb_ho_supp_t	* mbbhandoversupport)          // LIST(MBB_HO_SUPP)   // Optional
{
	mih_header_t header;
	mih_tlv_t *tlv_list, *tlv;

	// sends an MIH Discovery request

	memset(&header,0,sizeof(mih_header_t));

	header.version = 1;
	// header.ackreq = 0; // Used for requesting an acknowledge for the message.
	// header.ackrsp = 0; // Used for responding to the request for an ack for the message.
	// header.uir = 0;    // Unauthenticated Information Request
	// header.rsvd1 = 0;  // Should be set to '0'
	// header.rsvd2 = 0;
	// header.morefragment = 0;   // to be ajusted in the serialization phase...
	// header.fragmentnumber = 0; // to be ajusted in the serialization phase...
	// header.payloadlength = 0;  // to be adjusted in the serialization phase...
	header.sid = SERVICE_MANAGEMENT; // Indentifies the different MIH services
	header.opcode = REQUEST; // Type of operation (REQUEST, RESPONSE, INDICATION)
	header.aid = MIH_CAPABILITY_DISCOVER; // Action to be taken with regard to the SID
	header.tid = NewTid();
	header.payloadlength = 0;

	// mih_tlv_t * NewTlv(unsigned char type, unsigned char * value, int length);

	tlv_list = tlv = NewTlv(SRC_MIHF_ID_TLV, _mymihfid, strlen(_mymihfid));

	tlv->next=NewTlv(DST_MIHF_ID_TLV, destinationidentifier,strlen(destinationidentifier));
	tlv=tlv->next;

	if(linkaddresslist) {
		tlv->next = NewTlv(LINK_ADDRESS_LIST_TLV,
		                  (unsigned char *)linkaddresslist,sizeof(net_type_addr_t));
		tlv=tlv->next;
	}
	if(supportedmiheventlist) {
		tlv->next = NewTlv(MIH_EVENT_LIST_TLV,
		                  (unsigned char *)supportedmiheventlist,sizeof(mih_evt_list_t));
		tlv=tlv->next;
	}
	if(supportedcommandlist) {
		tlv->next = NewTlv(MIH_COMMAND_LIST_TLV,
		                  (unsigned char *)supportedcommandlist,sizeof(mih_cmd_list_t));
		tlv=tlv->next;
	}
	if(supportedisquerytypelist) {
		tlv->next = NewTlv(MIIS_QUERY_TYPE_LIST_TLV,
		                  (unsigned char*)supportedisquerytypelist,sizeof(mih_iq_type_lst_t));
		tlv=tlv->next;
	}
	if(supportedtransportlist) {
		tlv->next = NewTlv(TRANSPORT_OPTION_LIST_TLV,
		                  (unsigned char *)supportedtransportlist,sizeof(mih_trans_lst_t));
		tlv=tlv->next;
	}
	if(mbbhandoversupport) {
		tlv->next = NewTlv(MBB_HO_SUPP_TLV,
		                  (unsigned char *)mbbhandoversupport, sizeof(mbb_ho_supp_t));
		tlv=tlv->next;
	}

	// transmit PDU: header + tlv list


	return 0;
}

int
MIH_Capability_Discover_indication(
	mihf_id_t destinationidentifier,              // MIHF_ID
	net_type_addr_t * linkaddresslist,            // LIST(NET_TYPE_ADDR) // Optional
	mih_evt_list_t  * supportedmiheventlist,      // MIH_EVT_LIST        // Optional
	mih_cmd_list_t  * supportedcommandlist,       // MIH_CMD_LIST        // Optional
	mih_iq_type_lst_t * supportedisquerytypelist, // MIH_IQ_TYPE_LST     // Optional
	mih_trans_lst_t * supportedtransportlist,     // MIH_TRANS_LST       // Optional
	mbb_ho_supp_t	* mbbhandoversupport)          // LIST(MBB_HO_SUPP)   // Optional
{
	// notify the MIH User &
	// Respond with an MIH_Capability_Discover_response primitive
	// respond automatically?

	return 0;
}

int
MIH_Capability_Discover_response(
	mihf_id_t         destinationidentifier,      // MIHF_ID
	status_t          status,
	net_type_addr_t * linkaddresslist,            // LIST(NET_TYPE_ADDR) // Optional
	mih_evt_list_t  * supportedmiheventlist,      // MIH_EVT_LIST        // Optional
	mih_cmd_list_t  * supportedcommandlist,       // MIH_CMD_LIST        // Optional
	mih_iq_type_lst_t * supportedisquerytypelist, // MIH_IQ_TYPE_LST     // Optional
	mih_trans_lst_t * supportedtransportlist,     // MIH_TRANS_LST       // Optional
	mbb_ho_supp_t	 * mbbhandoversupport)         // LIST(MBB_HO_SUPP)   // Optional
{
	// send back the local capability information

	return 0;
}

int
MIH_Capability_Discover_confirm(
	mihf_id_t         sourceidentifier,           // MIHF_ID
	status_t          status,
	net_type_addr_t * linkaddresslist,            // LIST(NET_TYPE_ADDR) // Optional
	mih_evt_list_t  * supportedmiheventlist,      // MIH_EVT_LIST        // Optional
	mih_cmd_list_t  * supportedcommandlist,       // MIH_CMD_LIST        // Optional
	mih_iq_type_lst_t * supportedisquerytypelist, // MIH_IQ_TYPE_LST     // Optional
	mih_trans_lst_t * supportedtransportlist,     // MIH_TRANS_LST       // Optional
	mbb_ho_supp_t	 * mbbhandoversupport)         // LIST(MBB_HO_SUPP)   // Optional
{
	// notify the MIH User

	return 0;
}


// MIH_Register

// MIH_DeRegister
// MIH_Event_Subscribe
// MIH_Event_Unsubscribe




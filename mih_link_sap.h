/*
 * MIH_LINK_SAP
 */

/*
 * (Events)
 * Link_Detected
 * Link_Up
 * Link_Down
 * Link_Parameters_Report
 * Link_Going_Down
 * Link_Handover_Imminent
 * Link_Handover_Complete
 * Link_PDU_Transmit_Status

 * (Commands)
 * Link_Capability_Discover
 * Link_Event_Subscribe
 * Link_Event_Unsubscribe
 * Link_Get_Parameters
 * Link_Configure_Thresholds
 * Link_Action
*/

/* Events */

void Link_Up_indication(void *parameter);
void Link_Down_indication(void *parameter);
void Link_Going_Down_indication(void *parameter);

int Link_Detected_indication(link_det_info_t LinkDetectedInfo);

int Link_Parameters_Report_indication(
	link_tuple_id_t		LinkIdentifier,
	link_param_rpt_t*	LinkParametersReportList);

int Link_Handover_Imminent_indication(
	link_tuple_id_t		OldLinkIdentifier,
	link_tuple_id_t		NewLinkIdentifier,
	link_addr_t		OldAccessRouter,	/* optional */
	link_addr_t		NewAccessRouter);	/* optional */

int Link_Handover_Complete_indication(
	link_tuple_id_t		OldLinkIdentifier,
	link_tuple_id_t		NewLinkIdentifier,
	link_addr_t		OldAccessRouter,	/* optional */
	link_addr_t		NewAccessRouter,	/* optional */
	status_t		iLinkHandoverStatus);

int Link_PDU_Transmit_Status_indication(
	link_tuple_id_t		LinkIdentifier,
	uint16_t		PacketIdentifier,
	boolean_t		TransmissionStatus);

struct Link_Up_indication_parameter {
	link_tuple_id_t		LinkIdentifier;
	link_addr_t		OldAccessRouter;
	link_addr_t		NewAccessRouter;
	ip_renewal_flag_t	IPRenewalFlag;
	ip_mob_mgmt_t		MobilityManagementSupport;
};

struct Link_Down_indication_parameter {
	link_tuple_id_t		LinkIdentifier;
	link_addr_t		OldAccessRouter;
	link_dn_reason_t	ReasonCode;
};


struct Link_Going_Down_indication_parameter {
	link_tuple_id_t		LinkIdentifier;
	uint16_t		TimeInterval;
	link_gd_reason_t	LinkGoingDownReason;
};

/* Commands */

int Link_Capability_Discover_request(void);

int Link_Capability_Discover_confirm(
	status_t		Status,
	link_event_list_t	SupportedLinkEventList,
	link_cmd_list_t		SupportedLinkCommandList);

int Link_Event_Subscribe_request(link_event_list_t RequestedLinkEventList);

int Link_Event_Subscribe_confirm(
	status_t		Status,
	link_event_list_t	ResponseLinkEventList);

int Link_Event_Unsubscribe_request(link_event_list_t RequestedLinkEventList);

int Link_Event_Unsubscribe_confirm(
	status_t		Status,
	link_event_list_t	ResponseLinkEventList);

int Link_Get_Parameters_request(
	// LIST(link_param_type_t)	LinkParametersRequest,
	link_param_type_t*	LinkParametersRequest,
	link_states_req_t	LinkStatesRequest,
	link_desc_req_t		LinkDescriptorsRequest);

int Link_Get_Parameters_confirm(
	status_t		Status,
	// LIST(link_param_t)		LinkParametersStatusList,
	link_param_t*		LinkParametersStatusList,
	// LIST(link_states_rsp_t)	LinkStatesResponse,
	link_states_rsp_t*	LinkStatesResponse,
	// LIST(link_desc_rsp_t)	LinkDescriptorsResponse);
	link_desc_rsp_t*	LinkDescriptorsResponse);

int Link_Configure_Thresholds_request(
	// LIST(link_cfg_param_t)	LinkConfigureParameterList);
	link_cfg_param_t*	LinkConfigureParameterList);

int Link_Configure_Thresholds_confirm(
	status_t		Status,
	// LIST(link_cfg_status_t)	LinkConfigureStatusList);
	link_cfg_status_t*	LinkConfigureStatusList);

int Link_Action_request(
	link_action_t		LinkAction,
	uint16_t		ExecutionDelay,
	link_addr_t		PoALinkAddres);

int Link_Action_confirm(
	status_t		Status,
	// LIST(link_scan_rsp_t)	ScanResponseSet,
	link_scan_rsp_t*	ScanResponseSet,
	link_ac_result_t	LinkActionResult);



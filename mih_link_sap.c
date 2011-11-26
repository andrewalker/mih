
/*
	MIH_LINK_SAP
 */

/*
(Events)
Link_Detected
Link_Up
Link_Down
Link_Parameters_Report
Link_Going_Down
Link_Handover_Imminent
Link_Handover_Complete
Link_PDU_Transmit_Status

(Commands)
Link_Capability_Discover (request + confirm)
Link_Event_Subscribe (request + confirm)
Link_Event_Unsubscribe (request + confirm)
Link_Get_Parameters (request + confirm)
Link_Configure_Thresholds (request + confirm)
Link_Action (request + confirm)
*/

#include "mih_link_sap.h"


// Events

// (63)
int
Link_Detected_indication(link_det_info_t LinkDetectedInfo)
{
	return 0;
}

// (63)
void Link_Up_indication(struct kthread_work *work)
{
	struct Link_Up_indication_work *param = container_of(work,
			struct Link_Up_indication_work, work);

	free_previous_work(&mihf_finished_work_list);

	// notify local & remote subscribers

	printk(KERN_INFO "MIH: mih_link_sap Link_Up.indication\n");

	list_add(&param->finished, &mihf_finished_work_list);
}

// (64)
void Link_Down_indication(struct kthread_work *work)
{
	struct Link_Down_indication_work *param = container_of(work,
			struct Link_Down_indication_work, work);

	free_previous_work(&mihf_finished_work_list);

	// notify local & remote subscribers

	printk(KERN_INFO "MIH: mih_link_sap Link_Down.indication\n");

	list_add(&param->finished, &mihf_finished_work_list);
}

// (65)
int
Link_Parameters_Report_indication(
	link_tuple_id_t        LinkIdentifier,
	// LIST(link_param_rpt_t) LinkParametersReportList)
	link_param_rpt_t       * LinkParametersReportList)
{
	// notify local & remote subscribers

	printk(KERN_INFO "MIH: mih_link_sap Link_Parameters_Report.indication\n");

	return 0;
}

// (66)
void Link_Going_Down_indication(struct kthread_work *work)
{
	struct Link_Going_Down_indication_work *param = container_of(work,
			struct Link_Going_Down_indication_work, work);

	free_previous_work(&mihf_finished_work_list);

	// notify local & remote subscribers

	printk(KERN_INFO "MIH: mih_link_sap Link_Going_Down.indication\n");

	list_add(&param->finished, &mihf_finished_work_list);
}

// (66)
int
Link_Handover_Imminent_indication(
	link_tuple_id_t  OldLinkIdentifier,
	link_tuple_id_t  NewLinkIdentifier,
	link_addr_t      OldAccessRouter,      // optional
	link_addr_t      NewAccessRouter)      // optional
{
	return 0;
}

// (67)
int
Link_Handover_Complete_indication(
	link_tuple_id_t  OldLinkIdentifier,
	link_tuple_id_t  NewLinkIdentifier,
	link_addr_t      OldAccessRouter,      // optional
	link_addr_t      NewAccessRouter,      // optional
	status_t         iLinkHandoverStatus)
{
	return 0;
}

// (68)
int
Link_PDU_Transmit_Status_indication(
	link_tuple_id_t  LinkIdentifier,
	uint16_t         PacketIdentifier,
	boolean_t        TransmissionStatus)
{
	return 0;
}

// Commands

// (69)
int
Link_Capability_Discover_request()
{
	return 0;
}

// (70)
int Link_Capability_Discover_confirm(
	status_t          Status,
	link_event_list_t SupportedLinkEventList,
	link_cmd_list_t   SupportedLinkCommandList)
{
	return 0;
}

// (70)
int
Link_Event_Subscribe_request(link_event_list_t RequestedLinkEventList)
{
	return 0;
}

// (71)
int
Link_Event_Subscribe_confirm(
	status_t          Status,
	link_event_list_t ResponseLinkEventList)
{
	return 0;
}

// (72)
int
Link_Event_Unsubscribe_request(link_event_list_t RequestedLinkEventList)
{
	return 0;
}

int
Link_Event_Unsubscribe_confirm(
	status_t          Status,
	link_event_list_t ResponseLinkEventList)
{
	return 0;
}

// (73)
int
Link_Get_Parameters_request(
	// LIST(link_param_type_t) LinkParametersRequest,
	link_param_type_t     * LinkParametersRequest,
	link_states_req_t       LinkStatesRequest,
	link_desc_req_t         LinkDescriptorsRequest)
{
	return 0;
}

// (73)
int
Link_Get_Parameters_confirm(
	status_t                Status,
	// LIST(link_param_t)      LinkParametersStatusList,
	link_param_t             * LinkParametersStatusList,
	// LIST(link_states_rsp_t) LinkStatesResponse,
	link_states_rsp_t        * LinkStatesResponse,
	// LIST(link_desc_rsp_t)   LinkDescriptorsResponse)
	link_desc_rsp_t          * LinkDescriptorsResponse)
{
	return 0;
}


// (74)
int
Link_Configure_Thresholds_request(
	// LIST(link_cfg_param_t) LinkConfigureParameterList)
	link_cfg_param_t * LinkConfigureParameterList)
{
	return 0;
}

int
Link_Configure_Thresholds_confirm(
	status_t                Status,
	// LIST(link_cfg_status_t) LinkConfigureStatusList)
	link_cfg_status_t * LinkConfigureStatusList)
{
	return 0;
}

// (75)
int
Link_Action_request(
	link_action_t LinkAction,
	uint16_t      ExecutionDelay,
	link_addr_t   PoALinkAddres)
{
	return 0;
}

int
Link_Action_confirm(
	status_t Status,
	// LIST(link_scan_rsp_t) ScanResponseSet,
	link_scan_rsp_t * ScanResponseSet,
	link_ac_result_t      LinkActionResult)
{
	return 0;
}




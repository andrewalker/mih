/*
 * MIH Auxiliary functions
 */

#include "mih.h"
#include "message.h"

void unpack_mih_header(unsigned char* in_buf, mih_header_t *hdr)
{
	hdr->version		=  in_buf[0] >> 4;
	hdr->ackreq		= (in_buf[0] >> 3) & 0x1;
	hdr->ackrsp		= (in_buf[0] >> 2) & 0x1;
	hdr->uir		= (in_buf[0] >> 1) & 0x1;
	hdr->morefragment	=  in_buf[0]       & 0x1;
	hdr->fragmentnumber	=  in_buf[1] >> 1;
	hdr->rsvd1		=  in_buf[1]       & 0x1;
	hdr->sid		=  in_buf[2] >> 4;
	hdr->opcode		= (in_buf[2] >> 2) & 0x3;
	hdr->rsvd2		=  in_buf[4] >> 4;

	hdr->tid		= ((in_buf[4] & 0xF) << 8) | in_buf[5];
	hdr->aid		= ((in_buf[2] & 0x3) << 8) | in_buf[3];
	hdr->payloadlength	=  (in_buf[6]        << 8) | in_buf[7];
}

int parse_variable_length_field(unsigned char* in_buf, unsigned int in_len,
		unsigned int *length_value)
{
	unsigned int i;
	unsigned int length = in_buf[0];
	unsigned int length_octets;
	unsigned int length_end;

	if (length <= 128) {
		*length_value = length;
		return 1;
	}

	length_octets = length & 0x7F;
	if (length_octets > sizeof(length)) {
		return -1;
	}

	length_end = length_octets + 1;
	length = in_buf[1];

	for (i = 2; i < length_end; ++i)
		length = (length << 8) | in_buf[i];

	*length_value = length;
	return i;
}

int unpack_mih_tlv(unsigned char* in_buf, unsigned int in_len, mih_tlv_t **tlv)
{
	int read;
	int s;
	unsigned char type = in_buf[0];
	unsigned char *value;
	unsigned int tlv_length;

	if (in_len < 2) {
		printk(KERN_INFO "Invalid TLV");
		return -1;
	}

	read = parse_variable_length_field(&in_buf[1], in_len - 1, &tlv_length);
	if (read < 0)
		return read;

	value = &in_buf[++read];

	switch (type) {
	case SRC_MIHF_ID_TLV:
	case DST_MIHF_ID_TLV:
		s = unpack_mihf_id_tlv(type, tlv_length, value, tlv);
		break;
	case STATUS_TLV:
	case LINK_TYPE_TLV:
	case MIH_EVENT_LIST_TLV:
	case MIH_COMMAND_LIST_TLV:
	case MIIS_QUERY_TYPE_LIST_TLV:
	case TRANSPORT_OPTION_LIST_TLV:
	case LINK_ADDRESS_LIST_TLV:
	case MBB_HO_SUPP_TLV:
	case REG_REQUEST_CODE_TLV:
	case VALID_TIME_INTERVAL_TLV:
	case LINK_IDENTIFIER_TLV:
	case NEW_LINK_IDENTIFIER_TLV:
	case OLD_LINK_IDENTIFIER_TLV:
	case NEW_ACCESS_ROUTER_TLV:
	case IP_RENEWAL_FLAG_TLV:
	case MOBILITY_MGMT_SUPP_TLV:
	case IP_ADDR_CONFIG_MTHDS_TLV:
	case LINK_DOWN_REASON_CODE_TLV:
	case TIME_INTERVAL_TLV:
	case LINK_GOING_DOWN_REASON_TLV:
	case LINK_PARAMETER_REPORT_LIST_TLV:
	case DEVICE_STATES_REQUEST_TLV:
	case LINK_IDENTIFIER_LIST_TLV:
	case DEVICE_STATES_RESPONSE_LIST_TLV:
	case GET_STATUS_REQUEST_SET_TLV:
	case GET_STATUS_RESPONSE_LIST_TLV:
	case CONFIGURE_REQUEST_LIST_TLV:
	case CONFIGURE_RESPONSE_LIST_TLV:
	case LIST_OF_LINK_POA_LIST_TLV:
	case PREFERRED_LINK_LIST_TLV:
	case HO_RESOURCE_QUERY_LIST_TLV:
	case HO_STATUS_TLV:
	case ACCESS_ROUTER_ADDRESS_TLV:
	case DHCP_SERVER_ADDRESS_TLV:
	case FA_ADDRESS_TLV:
	case LINK_ACTIONS_LIST_TLV:
	case LINK_ACTIONS_RESULT_LIST_TLV:
	case HO_RESULT_TLV:
	case RESOURCE_STATUS_TLV:
	case RESOURCE_RETENTION_STATUS_TLV:
	case INFO_QUERY_BINARY_DATA_LIST_TLV:
	case INFO_QUERY_RDF_DATA_LIST_TLV:
	case INFO_QUERY_RDF_SCHEMA_URL_TLV:
	case INFO_QUERY_RDF_SCHEMA_LIST_TLV:
	case MAX_RESPONSE_SIZE_TLV:
	case INFO_RESPONSE_BINARY_DATA_LIST_TLV:
	case INFO_RESPONSE_RDF_DATA_LIST_TLV:
	case INFO_RESPONSE_RDF_SCHEMA_URL_LIST_TLV:
	case INFO_RESPONSE_RDF_SCHEMA_LIST_TLV:
	case MOBILE_NODE_MIHF_ID_TLV:
	case QUERY_RESOURCE_REPORT_FLAG_TLV:
	case EVENT_CONFIGURATION_INFO_LIST_TLV:
	case TARGET_NETWORK_INFO_TLV:
	case LIST_OF_TARGET_NETWORK_INFO_TLV:
	case ASSIGNED_RESOURCE_SET_TLV:
	case LINK_DETECTED_INFO_LIST_TLV:
	case MN_LINK_ID_TLV:
	case POA_TLV:
	case UNAUTHENTICATED_INFO_REQUEST_TLV:
	case NETWORK_TYPE_TLV:
	case REQUESTED_RESOURCE_SET_TLV:
	default:
		if (verbose) {
			printk(KERN_INFO "MIH: Unknown TLV type: %d\n",
					in_buf[0]);
		}
		return -1;
	};

	if (s < 0)
		read = s;
	else
		read += s;

	return read;
}

int unpack_mih_message(unsigned char* in_buf, unsigned int in_len,
		mih_message_t *msg)
{
	mih_tlv_t *tlv;
	int i = 0;
	int s;
	struct list_head *tlvs = &msg->tlvs.list;
	unsigned int payload_len = in_len - sizeof(msg->header);
	unsigned char* payload = &in_buf[sizeof(msg->header)];

	if (in_len < sizeof(msg->header)) {
		printk(KERN_INFO "Invalid MIH message");
		return -1;
	}

	unpack_mih_header(in_buf, &msg->header);

	if (in_len != (msg->header.payloadlength + sizeof(msg->header))) {
		printk(KERN_INFO "Invalid payload length");
		return -1;
	}

	INIT_LIST_HEAD(tlvs);

	while (i < msg->header.payloadlength) {
		s = unpack_mih_tlv(payload, payload_len, &tlv);
		if (s < 0) {
			free_tlvs(&msg->tlvs);
			return s;
		}

		i += s;
		payload = &payload[s];
		payload_len -= s;

		list_add_tail(&tlv->list, tlvs);
	}

	return 0;
}

int unpack_octet_string(unsigned char* buf, unsigned int buf_len, char **string)
{
	int length;
	int s;
	char *str;

	s = parse_variable_length_field(buf, buf_len, &length);
	if (s < 0) {
		return s;
	}

	str = kmalloc(length + 1, GFP_KERNEL);
	if (!str) {
		printk(KERN_ERR "Failed to allocate memory");
		return -1;
	}

	memcpy(str, &buf[s], length);
	str[length] = '\0';

	*string = str;
	return s + length;
}

int unpack_mihf_id_tlv(unsigned char type, unsigned int length,
		unsigned char* value, mih_tlv_t **tlv_addr)
{
	mih_tlv_t *tlv;
	int s;

	tlv = kmalloc(sizeof(*tlv), GFP_KERNEL);
	tlv->type = type;
	tlv->length = length;

	s = unpack_octet_string(value, length, (char**)&tlv->value);
	if (s < 0)
		goto err;
	if (s != length) {
		goto length_err;
	}

	*tlv_addr = tlv;
	return s;

length_err:
	kfree(tlv->value);
err:
	kfree(tlv);
	return -1;
}

void free_tlv(mih_tlv_t *tlv)
{
	kfree(tlv->value);
	kfree(tlv);
}

void free_tlvs(mih_tlv_t *tlvs)
{
	mih_tlv_t *tlv;

	while (!list_empty(&tlvs->list)) {
		tlv = list_first_entry(&tlvs->list, mih_tlv_t, list);
		list_del(&tlv->list);
		free_tlv(tlv);
	}
}


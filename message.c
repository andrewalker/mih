/*
 * MIH Auxiliary functions
 */

#include "mih.h"
#include "message.h"

void pack_mih_header(mih_header_t *hdr, unsigned char *out_buf)
{
	out_buf[0]		 = hdr->version		<< 4;
	out_buf[0]		|= hdr->ackreq		<< 3;
	out_buf[0]		|= hdr->ackrsp		<< 2;
	out_buf[0]		|= hdr->uir		<< 1;
	out_buf[0]		|= hdr->morefragment;
	out_buf[1]		 = hdr->fragmentnumber	<< 1;
	out_buf[2]		 = hdr->sid		<< 4;
	out_buf[2]		|= hdr->opcode		<< 2;
	out_buf[2]		|= hdr->aid >> 8;
	out_buf[3]		 = hdr->aid & 0xFF;
	out_buf[4]		 = hdr->tid >> 8;
	out_buf[5]		 = hdr->tid & 0xFF;
	out_buf[6]		 = hdr->payloadlength >> 8;
	out_buf[7]		 = hdr->payloadlength & 0xFF;
}

void unpack_mih_header(unsigned char *in_buf, mih_header_t *hdr)
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

int pack_variable_length_field(unsigned int length, unsigned char *out_buf,
		unsigned int buf_size)
{
	unsigned int i;
	unsigned int shift;
	unsigned int octets = sizeof(unsigned int);
	unsigned int mask = 0xFF << ((octets - 1) * 8);

	if (length <= 128) {
		if (buf_size < 1) {
			printk("Buffer too small for length field");
			return -1;
		}

		*out_buf = (unsigned char)length;
		return 1;
	}

	length -= 128;

	while ((length & mask) == 0) {
		mask >>= 8;
		--octets;
	}

	if (buf_size <= octets) {
		printk("Buffer too small for length field");
		return -1;
	}

	out_buf[0] = (unsigned char)octets;
	out_buf[0] |= 0x80;

	shift = (octets - 1) * 8;

	for (i = 1; i <= octets; ++i) {
		out_buf[i] = (unsigned char)((length & mask) >> shift);
		shift -= 8;
		mask >>= 8;
	}

	return i;
}

int parse_variable_length_field(unsigned char *in_buf, unsigned int in_len,
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

int pack_mih_tlv(mih_tlv_t *tlv, unsigned char *out_buf, unsigned int buf_size)
{
	int s;
	unsigned char type = tlv->type;

	if (buf_size < 1) {
		printk(KERN_INFO "Buffer to small for packing a TLV");
		return -1;
	}

	*out_buf = type;
	--buf_size;
	++out_buf;

	switch (type) {
	case SRC_MIHF_ID_TLV:
	case DST_MIHF_ID_TLV:
		s = pack_mihf_id_tlv(tlv, out_buf, buf_size);
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
					type);
		}
		return -1;
	};

	if (s >= 0)
		++s;

	return s;
}

int unpack_mih_tlv(unsigned char *in_buf, unsigned int in_len, mih_tlv_t **tlv)
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

int pack_mih_message(mih_message_t *msg, unsigned char *out_buf,
		unsigned int buf_size)
{
	int s;
	int pos = sizeof(msg->header);
	struct list_head *i;
	mih_tlv_t *tlv;

	if (buf_size < pos) {
		printk(KERN_INFO "Buffer is too small for packing the message");
		return -1;
	}

	list_for_each(i, &msg->tlvs.list) {
		tlv = list_entry(i, mih_tlv_t, list);

		s = pack_mih_tlv(tlv, &out_buf[pos], buf_size - pos);
		if (s < 0) {
			return s;
		}

		pos += s;
	}

	msg->header.payloadlength = pos - sizeof(msg->header);
	pack_mih_header(&msg->header, out_buf);

	return pos;
}

int unpack_mih_message(unsigned char *in_buf, unsigned int in_len,
		mih_message_t *msg)
{
	mih_tlv_t *tlv;
	int i = 0;
	int s;
	struct list_head *tlvs = &msg->tlvs.list;
	unsigned int payload_len = in_len - sizeof(msg->header);
	unsigned char *payload = &in_buf[sizeof(msg->header)];

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

int pack_octet_string(char *string, unsigned char *buf, unsigned int buf_len)
{
	int s;
	int length = strlen(string);

	s = pack_variable_length_field(length, buf, buf_len);
	if (s < 0) {
		return s;
	}

	if ((s + length) > buf_len) {
		printk(KERN_INFO "Buffer is too small for octet string: %s",
				string);
		return -1;
	}

	memcpy(&buf[s], string, length);

	return s + length;
}

int unpack_octet_string(unsigned char *buf, unsigned int buf_len, char **string)
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

int pack_mihf_id_tlv(mih_tlv_t *tlv, unsigned char *out_buf,
		unsigned int buf_size)
{
	unsigned char *buffer;
	int s;
	int pos = 0;

	buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);

	s = pack_octet_string((char*)tlv->value, buffer, BUFFER_SIZE);
	if (s < 0)
		goto out;

	pos = pack_variable_length_field(s, out_buf, buf_size);
	if (pos < 0) {
		s = pos;
		goto out;
	}

	if ((pos + s) > buf_size) {
		printk(KERN_INFO "Buffer is too small to store MIHF ID TLV");
		s = -1;
		goto out;
	}

	memcpy(&out_buf[pos], buffer, s);

	s += pos;

out:
	kfree(buffer);

	return s;
}

int unpack_mihf_id_tlv(unsigned char type, unsigned int length,
		unsigned char *value, mih_tlv_t **tlv_addr)
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


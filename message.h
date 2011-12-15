int parse_variable_length_field(unsigned char* in_buf, unsigned int in_len,
		unsigned int *tlv_length);

int unpack_octet_string(unsigned char* buf, unsigned int buf_len,
		char **string);

void unpack_mih_header(unsigned char* in_buf, mih_header_t *hdr);
int unpack_mih_message(unsigned char* in_buf, unsigned int in_len,
		mih_message_t *msg);

int unpack_mih_tlv(unsigned char* in_buf, unsigned int in_len, mih_tlv_t **tlv);

int unpack_mihf_id_tlv(unsigned char type, unsigned int length,
		unsigned char* value, mih_tlv_t **tlv_addr);

void free_tlv(mih_tlv_t* tlv);
void free_tlvs(mih_tlv_t* tlvs);


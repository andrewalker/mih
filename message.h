#define BUFFER_SIZE	4096

int pack_variable_length_field(unsigned int length, unsigned char *buf,
		unsigned int buf_len);
int parse_variable_length_field(unsigned char *in_buf, unsigned int in_len,
		unsigned int *tlv_length);

int pack_octet_string(char *string, unsigned char *buf, unsigned int buf_len);
int unpack_octet_string(unsigned char *buf, unsigned int buf_len,
		char **string);

void pack_mih_header(mih_header_t *hdr, unsigned char *out_buf);
int pack_mih_message(mih_message_t *msg, unsigned char *out_buf,
		unsigned int buf_len);
void unpack_mih_header(unsigned char *in_buf, mih_header_t *hdr);
int unpack_mih_message(unsigned char *in_buf, unsigned int in_len,
		mih_message_t *msg);

int pack_mih_tlv(mih_tlv_t *tlv, unsigned char *out_buf, unsigned int buf_len);
int unpack_mih_tlv(unsigned char *in_buf, unsigned int in_len, mih_tlv_t **tlv);

int pack_mihf_id_tlv(mih_tlv_t *tlv, unsigned char *out_buf,
		unsigned int buf_len);
int unpack_mihf_id_tlv(unsigned char type, unsigned int length,
		unsigned char *value, mih_tlv_t **tlv_addr);

void free_tlv(mih_tlv_t* tlv);
void free_tlvs(mih_tlv_t* tlvs);


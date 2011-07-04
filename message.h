
int TidInit(void);

int NewTid(void);

// utility functions for message handling

mih_tlv_t * NewTlv(unsigned char type, unsigned char * value, int length);

int TlvPack(unsigned char *buf, mih_tlv_t *tlv);

int TlvUnpack(unsigned char *buf, mih_tlv_t *tlv);


mih_message_t * MUnpack(unsigned char *msgseq, int seqlen);

unsigned char * MPack(mih_message_t *message, int *packet_len);



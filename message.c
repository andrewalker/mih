
// utility functions for message handling


#include "message.h"


mih_tlv_t *
NewTlv(unsigned char type, unsigned char * value, int length)
{
	mih_tlv_t *new_tlv=(mih_tlv_t *)kmalloc(sizeof(mih_tlv_t)+length, GFP_KERNEL);
	new_tlv->type = type;
	new_tlv->length = length;
	new_tlv->value = (unsigned char *)new_tlv + sizeof(mih_tlv_t);
	memcpy(new_tlv->value,value,length);
	new_tlv->next=NULL;

	return new_tlv;
}


// memory for the header and the tlv list is allocated here
mih_message_t *
MUnpack(unsigned char *msgseq, int seqlen)
{
	int tlvlen,i,noct;
	int paylen;
	unsigned char *nexttlv;
	mih_tlv_t *new_tlv, *tlvp;
	mih_message_t *message = kmalloc(sizeof(mih_message_t),GFP_KERNEL);

	message->header = kmalloc(sizeof(mih_header_t),GFP_KERNEL);

	memcpy(message->header,msgseq,sizeof(mih_header_t));
	message->tlv=NULL;
	paylen = message->header->payloadlength;
	nexttlv=msgseq+sizeof(mih_header_t);

	// what if the announced payload length is incorrect?
	// how do we end this function?
	while(paylen>0) {

		if(msgseq[1]<128) {	 // case 1
			tlvlen=(int)nexttlv[1];
			noct=0;
		} else { // cases 2 and 3
			// if(nexttlv[1] & 0x80) { // cases 2 and 3
			// if(nexttlv[1] & 0xff == 0x80) // case 3
			tlvlen=128;
			noct = (int)(nexttlv[1]&0x7f);

			// prevent errors here: return if noct > 3
			if(noct>3)
				return message;

			for(i=0; i < noct; i++)
				tlvlen += nexttlv[2+i] << (noct-i-1)*8;
		}
		// who will free the tlv memory area?
		new_tlv=(mih_tlv_t *)kmalloc(sizeof(mih_tlv_t)+tlvlen,GFP_KERNEL);
		new_tlv->type=(int)nexttlv[0];
		new_tlv->length=tlvlen;
		new_tlv->next=NULL;
		new_tlv->value=(unsigned char *)new_tlv+sizeof(mih_tlv_t);
		memcpy(new_tlv->value,nexttlv+1/*type*/+1/*tlvlen*/+noct,tlvlen);

		// add new_tlv to message tlv list
		if(message->tlv == NULL)
			message->tlv = new_tlv;
		else {
			tlvp = message->tlv;
			while(tlvp->next != NULL)
				tlvp=tlvp->next;
			tlvp->next=new_tlv;
		}

		nexttlv = nexttlv+1/*type*/+1/*len*/+noct+tlvlen;
		paylen -= tlvlen;
	}
	// should we free the serialized message here?

	return message;
}


// memory for the msg is allocated here
unsigned char *
MPack(mih_message_t *message, int *packet_len)
{
	int total_len, // total number of bytes to allocate
		noct, 	// number of bytes used to store the tlv length
		pos,		// current writing position
		msglen,
		i;
	mih_tlv_t *tlvp;
	unsigned char *msgseq; // the serialized message

	// determine how many bytes we'll need for the header,
	total_len=sizeof(mih_header_t);
	// and for all the tlvs
	tlvp = message->tlv;
	while(tlvp) {
		total_len += sizeof(tlvp->type) + sizeof(tlvp->length) + tlvp->length;
		// we might waste a few bytes when packing the length but it's easier this way
		tlvp=tlvp->next;
	}

	// prevent errors here: return if total_len > MAX...
	// if(total_len > MAX)
	//		return NULL;

	msgseq = (unsigned char *)kmalloc(total_len,GFP_KERNEL);

	// copy the header to the message sequence
	memcpy(msgseq,message->header,sizeof(mih_header_t));

	tlvp = message->tlv;
	pos = sizeof(mih_header_t);

	while(tlvp) {

		if(tlvp->length<=128)
			noct=0;
		else {	// maximum of 4 bytes
			if (tlvp->length-128 > 0xffffff)
				noct=4;
			else if (tlvp->length-128 > 0xffff)
					noct=3;
				else if (tlvp->length-128 > 0xff)
						noct=2;
					else noct=1;
		}
		msgseq[pos]=(unsigned char)tlvp->type;
		if(noct == 0)
			msgseq[pos+1]=(unsigned char)tlvp->length;
		else {
			msgseq[pos+1]=(unsigned char)(0x80 | noct);
			msglen=tlvp->length-128;
			for(i=0; i < noct; i++) {
				msgseq[pos+2+noct-i-1]=(unsigned char)msglen;
				msglen = msglen >> 8;
			}
		}
		memcpy(msgseq+pos+2+noct,tlvp->value,tlvp->length);

		pos += 1 /*type*/ + 1 /*len*/ + noct + tlvp->length;

		tlvp=tlvp->next;
	}
   // adjust the total number of bytes packed
   *packet_len=pos;

	// should we free the tlvs and the message here?

	return msgseq;
}


// serializes the data from a TLV struct
// who will deal with fragmentation?
int
TlvPack(unsigned char *buf, mih_tlv_t *tlv)
{
	int noct, 	// number of bytes used to store the tlv length
		msglen,
		i;

	buf[0]=(unsigned char)tlv->type;

	if(tlv->length<=128)
		noct=0;
	else {	// maximum of 4 bytes
		if (tlv->length-128 > 0xffffff)
			noct=4;
		else if (tlv->length-128 > 0xffff)
				noct=3;
			else if (tlv->length-128 > 0xff)
					noct=2;
				else noct=1;
	}
	if(noct == 0)
		buf[1]=(unsigned char)tlv->length;
	else {
		buf[1]=(unsigned char)(0x80 | noct);
		msglen=tlv->length-128;
		for(i=0; i < noct; i++) {
			buf[2+noct-i-1]=(unsigned char)msglen;
			msglen = msglen >> 8;
		}
	}
	memcpy(buf+2+noct,tlv->value,tlv->length);

   // adjust the total number of bytes packed
	return (2+noct+tlv->length);
}

// extracts serialized data from the message
int
TlvUnpack(unsigned char *buf, mih_tlv_t *tlv)
{
	int noct, 	// number of bytes used to store the tlv length
		msglen,
		i;

/* NOT DONE: just copied the code from TlvPack... */

	buf[0]=(unsigned char)tlv->type;

	if(tlv->length<=128)
		noct=0;
	else {	// maximum of 4 bytes
		if (tlv->length-128 > 0xffffff)
			noct=4;
		else if (tlv->length-128 > 0xffff)
				noct=3;
			else if (tlv->length-128 > 0xff)
					noct=2;
				else noct=1;
	}
	if(noct == 0)
		buf[1]=(unsigned char)tlv->length;
	else {
		buf[1]=(unsigned char)(0x80 | noct);
		msglen=tlv->length-128;
		for(i=0; i < noct; i++) {
			buf[2+noct-i-1]=(unsigned char)msglen;
			msglen = msglen >> 8;
		}
	}
	memcpy(buf+2+noct,tlv->value,tlv->length);

   // adjust the total number of bytes packed
	return (2+noct+tlv->length);
}


int
NewTid()
{
   return ++_tid % MAX_TID;
}

int TidInit()
{
   // set the initial TID
   get_random_bytes(&_tid, 1);

   return 0;
}


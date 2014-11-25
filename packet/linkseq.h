#ifndef __LINK_SEQ_H__
#define __LINK_SEQ_H__

#include "packet.h"

typedef struct Packet_Seq{
    Packet cur; //store packet node, must NOT be pointer
    struct Packet_Seq *prev;
    struct Packet_Seq *next;
}Packet_Seq;

extern Packet_Seq *executePacketSeq; //the execute send sequence
extern Packet_Seq *executePacketEnd; //the execute send end 
extern Packet_Seq *executeResultSeq; //the execute reply sequence
extern Packet_Seq *executeResultEnd; //the execute send end 

int *initExecuteSeq();
Packet_Seq *initListSeq();
Packet_Seq *appendListSeq(Packet_Seq *head, Packet packet);
Packet_Seq *insertListSeq(Packet_Seq *head, Packet packet);
Packet_Seq *findLastBlank(Packet_Seq *head);
int writeResultSeq(Packet_Seq *head, char *result_file);
int isEndTransaction(Packet_Seq *head, int trans_id);
int clearTransaction(Packet_Seq *head, int trans_id);

#endif

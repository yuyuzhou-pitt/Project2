#ifndef __EXECUTE_REPLY_H__
#define __EXECUTE_REPLY_H__

#define EXECUTE_REPLY 1

#include "packet.h"
#include "linkseq.h"
#include "../lib/libterminal.h"

//Packet_Seq *executePacketSeq; //the execute send sequence
//Packet_Seq *executeResultSeq; //the execute reply sequence

Packet_Seq *genExecuteReply(OptionsStruct *result_options, char *client_ip, char *server_ip, char *trans_id);
//Packet_Seq *genExecuteReply(Packet_Seq *recv_exec_seq);
//Packet_Seq *genExecuteReply(Packet_Seq **recv_exec_seq);
//int genExecuteReply();
int sendExecuteReply(int sockfd, struct sockaddr_in sockaddr, Packet *packet);

#endif

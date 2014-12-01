#ifndef __MAP_REDUCE_H__
#define __MAP_REDUCE_H__

#define EXECUTE_SERV 1
#include "packet.h"
#include "linkseq.h"
#include "../lib/libterminal.h"

int callMapReduce(OptionsStruct *result_options, char *client_ip, char *server_ip, Packet *received_packet);

#endif

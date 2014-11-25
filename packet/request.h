#ifndef __REQUEST_H__
#define __REQUEST_H__

#define REQUEST_SERV 1
#include "packet.h"
#include "../lib/libterminal.h"

Packet *genRequest(OptionsStruct *options, char *server_ip, char *port_mapper_ip);
Request_Reply *readPortMapperTable(Packet *request_serv, char *file);

#endif

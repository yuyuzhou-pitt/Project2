#ifndef __register_h__
#define __register_h__

#define REGISTER_SERV 1

#include "packet.h"
#include "../lib/libterminal.h"

Packet *genRegister(OptionsStruct *options, char *server_ip, char *port_mapper_ip);

#endif

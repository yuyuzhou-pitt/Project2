#ifndef __portmap_table_h__
#define __portmap_table_h__

#define REGISTER_SERV 1

#include "packet.h"
#include "../lib/libterminal.h"

int writePortMapperTable(Packet *packet_recv, char *file);

#endif

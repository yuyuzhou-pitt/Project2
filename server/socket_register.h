#ifndef __SOCKET_REGISTER_H__
#define __SOCKET_REGISTER_H__


//#include "../port-mapper/socket_port_mapper.h"
#include "../lib/libterminal.h"

int registerServices(OptionsStruct *options);
void *sockclient(void *arg);

#endif

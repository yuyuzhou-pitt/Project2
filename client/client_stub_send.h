#ifndef __client_stub_send_h__
#define __client_stub_send_h__
#include "../packet/linkseq.h"
#include "../lib/libterminal.h"
Packet_Seq *Execute_stub(OptionsStruct *options, char *client_ip, char *server_ip, char *trans_id);
#endif

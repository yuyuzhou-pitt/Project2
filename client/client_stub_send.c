#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "../packet/execute.h"
#include "../lib/liblog.h"
#include "../lib/libterminal.h"

Packet_Seq *Execute_stub(OptionsStruct *options, char *client_ip, char *server_ip, char *trans_id){
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Enter Client Stub, packing paramters\n");
    logging(LOGFILE, logmsg);
    return genExecuteServ(options, client_ip, server_ip, trans_id);
}

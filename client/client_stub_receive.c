#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "../packet/checksum.h"
#include "../lib/liblog.h"
#include "../packet/execute_reply.h"

int Unpack_result(Packet_Seq *Head)
{
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Enter Client Stub, unpacking result\n");
    logging(LOGFILE, logmsg);
    //TODO: implemenation of unpack. 



}

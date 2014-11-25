#ifndef __LINK_LOAD_H__
#define __LINK_LOAD_H__

#include "packet.h"
/*For load balance, store the last time used server record and index in Request_Reply
* i.e.: 
* Three servers matched the client request of "ScientificLibrary 1 Sort":
* (client)# request ScientificLibrary 1 Sort
* 192.168.1.1| 54321      | ScientificLibrary | 1  | Sort
* 192.168.1.2| 54322      | ScientificLibrary | 1  | Sort
* 192.168.1.3| 54323      | ScientificLibrary | 1  | Sort
*
* And two servers matched the client request of "ScientificLibrary 2 Max":
* (client)# request ScientificLibrary 1 Max
* 192.168.1.1| 54321      | ScientificLibrary | 2  | Max
* 192.168.1.2| 54322      | ScientificLibrary | 2  | Max
*
* By default we choose the first server to use, then the index is 0.
*
* The stored record and index looks like below:
* +----------------------------------+  +----------------------------------+
* | index = 0                        |  | index = 0                        | 
* | server_ip = 192.168.1.1          |  | server_ip = 192.168.1.1          |
* | port_number = 54322              |  | port_number = 54321              |
* | program_name = ScientificLibrary |  | program_name = ScientificLibrary |
* | version_number = 1               |->| version_number = 2               |->...
* | procedure_name = Sort            |  | procedure_name = Max             |
* +----------------------------------+  +----------------------------------+
* The primary key is the combination of program_name, version_number, and procedure_name.
*
* Next time, if we request the same combination, the link node will update the index to
* be the next server index (or back to the beginning) will be used to keep load balance.
*
* If we request a new combination, a new node will be added to the linklist.
*/

typedef struct LoadLink{
    PortMapperTable portMapperTable; // Must NOT be pointer, or the massage will not be record
    struct LoadLink *next;
}LoadLink;

extern LoadLink *loadBalanceLinkHead;
extern LoadLink *loadBalanceLinkEnd;

int *initLoadSeq();
LoadLink *initlist();
LoadLink *enqueue(LoadLink *head, PortMapperTable portMapperTable);
LoadLink *dequeue(LoadLink *head);
int listsize(LoadLink *head);
//LoadLink *recordLoadBalance(LoadLink *link, Request_Reply req_reply);
int recordLoadBalance(Request_Reply req_reply);

#endif

#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdint.h>

/*define the internal hidden file*/
#define PORT_MAPPER_FILE ".port_mapper"
#define PORT_MAPPER_TABLE_FILE ".port_mapper_table"
#define EXECUTE_RESULT_FILE ".port_mapper_execute_result"

/*define the hello packet interval*/
#define HELLO_INTERVAL 4 // for hello packet, in seconds

/*define the packet data length*/
#define INTMTU 256
#define FLOATMTU 128
#define CHARMTU 512

/*define the map reduce actions*/
#define SPLIT "Split"
#define INDEX "Index"
#define WORDCOUNT "Wordcount"
#define SORT "Sort"
#define REDUCE "Reduce"
#define MII "MII" //master_inverted_index"
#define SEARCH "Search"
#define SINGLE "Single"
#define MERGE "Merge"

/*define the file split block size*/
#define SPLIT_BLOCK 65536//64K
//#define SPLIT_BLOCK 16384//16k

/* client request will set these two params (for client execute) */
extern char exec_remote_ipstr[1024];
extern char exec_remote_port[6];

/*
Packet type can be determined in 3 bits as shown below:
Register service (from server to port mapper)         (000)
Register acknowledge (from port mapper to server)     (001)
Hello Packets (from server to port mapper)            (111)
Request server location (from client to portmapper)   (010)
Response server location (from portmapper to client)  (011)
Execute service (from client to server)               (100)
Execute service response (from server to client)      (101)
Execute service acknowledge (from server to client)   (110)
*/


/*the file identification*/
typedef struct DocPath{
    char name[80]; // the filename
    char file_or_dir; // 0 means file, 1 means directory
    char location[255]; // the URL of the document 
}DocPath; 

/*for port mapper table*/
typedef struct PortMapperTable{
    int loadbalance_index;
    char version_number[8]; 
    char program_name[30]; //the program name on servers
    char procedure_name[30]; //240 bit
    char server_ip[32];
    char port_number[6]; //16 bit, 1024 ~ 65535
}PortMapperTable;

typedef struct Register_Service{
    //char packet_type[3]; //000
    char version_number[8]; //why it's char?
    char port_number[6]; //16 bit, 1024 ~ 65535
    char program_name[30]; //the program name on servers
    int procedure_number; //8 bit, up to 255
    char procedure_names[255][30]; // up to 255 procedures, each occupies 240 bit
}Register_Serv;
 
typedef struct Register_Acknowledge{
    //char packet_type[3]; //001
    char version_number[8]; //why it's char?
    char port_number[6]; //16 bit, 1024 ~ 65535
    char program_name[30]; //the program name on servers
    int dup_numbers; //the program name on servers
}Register_Ack;

typedef struct Hello_Message{
    //char packet_type[3]; //111
    char version_number[8]; //why it's char?
    char port_number[6]; //16 bit, 1024 ~ 65535
    char program_name[30]; //the program name on servers
    int dup_numbers; //the program name on servers
}Hello_Msg;

typedef struct Request_Service{
    //char packet_type[3]; //010
    char version_number[8]; //why it's char?
    char program_name[30]; //the program name on servers
    char procedure_name[30]; // up to 255 procedures, each occupies 240 bit
}Request_Serv;

/*there would be several server match the request, up to 255*/
typedef struct Request_Response{
    int response_index; //which response is using
    int response_number; //how many response got
    PortMapperTable portMapperTable[255];
}Request_Reply;

extern Request_Reply *requested_servers; // to store all the requested servers, for execute use

union ParameterData{
    int data_int[INTMTU]; //up to 100*100 matrix. 
    float data_float [FLOATMTU];
    char data_str[CHARMTU];
};

union ResponseData{
    int data_int[INTMTU];
    float data_float[FLOATMTU];
    char data_str[CHARMTU];
};

typedef struct Execute_Sevice{
    char version_number[8];
    char program_name[30];
    char procedure_name[30];
    int transaction_id; // the packet in the same seq share the same transaction_id, randomly assigned
    int end_flag; // wheather this packet is the last one for one transaction. 
    uint32_t seq; //sequence number, start from 0 
    uint32_t ts; //timestamp
    int data_is_file_or_dir; // 0 is file, for multiply and etc; 1 is dir, for minigoogle
    char exec_action[30]; // Split, Index, or Search
    char search_term[1024]; // "term1 term2"
    int server_no; // for sort use
    int server_number; // for sort use
    int num_parameter;
    int para1_type; //int:0, float:1, char:2; default is 0
    int para1_dimension;
    int para1_dimen1_len;
    int para1_dimen2_len;
    int para2_type;
    int para2_dimension;
    int para2_dimen1_len;
    int para2_dimen2_len; 
    union ParameterData para_data;
}Execute_Serv;

typedef struct Execute_Ack{
    char version_number[8];
    char program_name[30];
    char procedure_name[30];
    int transaction_id;
    int end_flag; 
    uint32_t seq; // ACK sequence. ACK5 means, Packet 0~4 is received.  
    uint32_t ts; //timestamp
    int data_is_file_or_dir; // 0 is file, for multiply and etc; 1 is dir, for minigoogle
}Execute_Ack;

typedef struct Execute_Respons{ 
    char version_number[8];
    char program_name[30];
    char procedure_name[30];
    int transaction_id;
    int end_flag; // wheather this packet is the last one for one transaction. 
    uint32_t seq; 
    uint32_t ts;
    int data_is_file_or_dir; // 0 is file, for multiply and etc; 1 is dir, for minigoogle
    char exec_action[30]; // Split, Index, or Search
    char search_term[1024]; // "term1 term2"
    int server_no; // for sort use
    int server_number; // for sort use
    int num_parameter;
    int respons_type;
    int respons_dimension;
    int res_dimen1_len;
    int res_dimen2_len;
    int para2_type;
    int para2_dimension;
    int para2_dimen1_len;
    int para2_dimen2_len;
    union ResponseData res_data; 
}Execute_Reply;

//TODO: define packet for 100, 101, 110

/*all message will be wrapped into packet*/
typedef struct Packet{
    char sender_id[32];
    char receiver_id[32];
    //char netmask[32]; 
    char packet_type[4]; //note, an '\n' in the end
#ifdef REGISTER_SERV
    Register_Serv Data;
#elif REGISTER_ACK
    Register_Ack Data;
#elif HELLO_MSG
    Hello_Msg Data;
#elif REQUEST_SERV
    Request_Serv Data;
#elif REQUEST_REPLY
    Request_Reply Data;
#elif EXECUTE_SERV
    Execute_Serv Data;
#elif EXECUTE_ACK
    Execute_Ack Data;
#elif EXECUTE_REPLY
    Execute_Reply Data;
#else
    //Register_Serv Data; // use the largest packet to be the default packet
    Execute_Serv Data; // use the largest packet to be the default packet
#endif
    char PacketChecksum[32]; // crc32
}Packet;

#endif

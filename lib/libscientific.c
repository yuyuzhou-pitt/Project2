#include <stdio.h>
#include <stdlib.h>
#include "../lib/libscientific.h"

struct Remote_Program_Struct *programLibrary(){
    struct Remote_Program_Struct *remoteProgram;
    remoteProgram = (struct Remote_Program_Struct *)malloc(sizeof(struct Remote_Program_Struct));

    snprintf(remoteProgram->program_name, sizeof(remoteProgram->program_name), "ScientificLibrary");
    snprintf(remoteProgram->version_number, sizeof(remoteProgram->version_number), "1");
    snprintf(remoteProgram->procedure1, sizeof(remoteProgram->procedure1), "Multiply");
    snprintf(remoteProgram->procedure2, sizeof(remoteProgram->procedure2), "Sort");
    snprintf(remoteProgram->procedure3, sizeof(remoteProgram->procedure3), "Min");
    snprintf(remoteProgram->procedure4, sizeof(remoteProgram->procedure3), "Max");

    return remoteProgram;
}


struct Remote_Program_Struct *programLibraryv2(){
    struct Remote_Program_Struct *remoteProgram;
    remoteProgram = (struct Remote_Program_Struct *)malloc(sizeof(struct Remote_Program_Struct));

    snprintf(remoteProgram->program_name, sizeof(remoteProgram->program_name), "ScientificLibrary");
    snprintf(remoteProgram->version_number, sizeof(remoteProgram->version_number), "2");
    snprintf(remoteProgram->procedure1, sizeof(remoteProgram->procedure1), "Multiply");
    snprintf(remoteProgram->procedure2, sizeof(remoteProgram->procedure2), "Sort");
    snprintf(remoteProgram->procedure3, sizeof(remoteProgram->procedure3), "Min");
    snprintf(remoteProgram->procedure4, sizeof(remoteProgram->procedure3), "Max");

    return remoteProgram;
}

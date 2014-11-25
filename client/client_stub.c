#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "../lib/libterminal.h"
#include "../lib/libscientific.h"
#include "client_stub.h"
#include "socket_request.h"
#include "socket_execute.h"

void ScientificLibrary_Multiply_V1(char *input_filename, char*output_filename){
    /* # request   <program-name> <version-number> <procedure>
     * # execute   <program-name> <version-number> <procedure> <input-file> <output-file>
     */
    OptionsStruct *request_options, *execute_options;
    request_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));
    execute_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));

    snprintf(request_options->command, sizeof(request_options->command), "%s", "request");
    snprintf(request_options->option1, sizeof(request_options->option1), "%s", "ScientificLibrary");
    snprintf(request_options->option2, sizeof(request_options->option2), "%s", "1");
    snprintf(request_options->option3, sizeof(request_options->option3), "%s", "Multiply");

    snprintf(execute_options->command, sizeof(execute_options->command), "%s", "execute");
    snprintf(execute_options->option1, sizeof(execute_options->option1), "%s", "ScientificLibrary");
    snprintf(execute_options->option2, sizeof(execute_options->option2), "%s", "1");
    snprintf(execute_options->option3, sizeof(execute_options->option3), "%s", "Multiply");
    snprintf(execute_options->option4, sizeof(execute_options->option4), "%s", input_filename);
    snprintf(execute_options->option5, sizeof(execute_options->option5), "%s", output_filename);

    requestServices(request_options);
    executeServices(execute_options);
}

void ScientificLibrary_Sort_V1(char *input_filename, char*output_filename){
    OptionsStruct *request_options, *execute_options;
    request_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));
    execute_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));

    snprintf(request_options->command, sizeof(request_options->command), "%s", "request");
    snprintf(request_options->option1, sizeof(request_options->option1), "%s", "ScientificLibrary");
    snprintf(request_options->option2, sizeof(request_options->option2), "%s", "1");
    snprintf(request_options->option3, sizeof(request_options->option3), "%s", "Sort");

    snprintf(execute_options->command, sizeof(execute_options->command), "%s", "execute");
    snprintf(execute_options->option1, sizeof(execute_options->option1), "%s", "ScientificLibrary");
    snprintf(execute_options->option2, sizeof(execute_options->option2), "%s", "1");
    snprintf(execute_options->option3, sizeof(execute_options->option3), "%s", "Sort");
    snprintf(execute_options->option4, sizeof(execute_options->option4), "%s", input_filename);
    snprintf(execute_options->option5, sizeof(execute_options->option5), "%s", output_filename);

    requestServices(request_options);
    executeServices(execute_options);
}


void ScientificLibrary_Min_V1(char *input_filename, char*output_filename){
    OptionsStruct *request_options, *execute_options;
    request_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));
    execute_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));

    snprintf(request_options->command, sizeof(request_options->command), "%s", "request");
    snprintf(request_options->option1, sizeof(request_options->option1), "%s", "ScientificLibrary");
    snprintf(request_options->option2, sizeof(request_options->option2), "%s", "1");
    snprintf(request_options->option3, sizeof(request_options->option3), "%s", "Min");

    snprintf(execute_options->command, sizeof(execute_options->command), "%s", "execute");
    snprintf(execute_options->option1, sizeof(execute_options->option1), "%s", "ScientificLibrary");
    snprintf(execute_options->option2, sizeof(execute_options->option2), "%s", "1");
    snprintf(execute_options->option3, sizeof(execute_options->option3), "%s", "Min");
    snprintf(execute_options->option4, sizeof(execute_options->option4), "%s", input_filename);
    snprintf(execute_options->option5, sizeof(execute_options->option5), "%s", output_filename);

    requestServices(request_options);
    executeServices(execute_options);
}

void ScientificLibrary_Max_V1(char *input_filename, char*output_filename){
    OptionsStruct *request_options, *execute_options;
    request_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));
    execute_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));

    snprintf(request_options->command, sizeof(request_options->command), "%s", "request");
    snprintf(request_options->option1, sizeof(request_options->option1), "%s", "ScientificLibrary");
    snprintf(request_options->option2, sizeof(request_options->option2), "%s", "1");
    snprintf(request_options->option3, sizeof(request_options->option3), "%s", "Max");

    snprintf(execute_options->command, sizeof(execute_options->command), "%s", "execute");
    snprintf(execute_options->option1, sizeof(execute_options->option1), "%s", "ScientificLibrary");
    snprintf(execute_options->option2, sizeof(execute_options->option2), "%s", "1");
    snprintf(execute_options->option3, sizeof(execute_options->option3), "%s", "Max");
    snprintf(execute_options->option4, sizeof(execute_options->option4), "%s", input_filename);
    snprintf(execute_options->option5, sizeof(execute_options->option5), "%s", output_filename);

    requestServices(request_options);
    executeServices(execute_options);
}


void ScientificLibrary_Multiply_V2(char *input_filename, char*output_filename){
    OptionsStruct *request_options, *execute_options;
    request_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));
    execute_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));

    snprintf(request_options->command, sizeof(request_options->command), "%s", "request");
    snprintf(request_options->option1, sizeof(request_options->option1), "%s", "ScientificLibrary");
    snprintf(request_options->option2, sizeof(request_options->option2), "%s", "2");
    snprintf(request_options->option3, sizeof(request_options->option3), "%s", "Multiply");

    snprintf(execute_options->command, sizeof(execute_options->command), "%s", "execute");
    snprintf(execute_options->option1, sizeof(execute_options->option1), "%s", "ScientificLibrary");
    snprintf(execute_options->option2, sizeof(execute_options->option2), "%s", "2");
    snprintf(execute_options->option3, sizeof(execute_options->option3), "%s", "Multiply");
    snprintf(execute_options->option4, sizeof(execute_options->option4), "%s", input_filename);
    snprintf(execute_options->option5, sizeof(execute_options->option5), "%s", output_filename);

    requestServices(request_options);
    executeServices(execute_options);
}


void ScientificLibrary_Sort_V2(char *input_filename, char*output_filename){
    OptionsStruct *request_options, *execute_options;
    request_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));
    execute_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));

    snprintf(request_options->command, sizeof(request_options->command), "%s", "request");
    snprintf(request_options->option1, sizeof(request_options->option1), "%s", "ScientificLibrary");
    snprintf(request_options->option2, sizeof(request_options->option2), "%s", "2");
    snprintf(request_options->option3, sizeof(request_options->option3), "%s", "Sort");

    snprintf(execute_options->command, sizeof(execute_options->command), "%s", "execute");
    snprintf(execute_options->option1, sizeof(execute_options->option1), "%s", "ScientificLibrary");
    snprintf(execute_options->option2, sizeof(execute_options->option2), "%s", "2");
    snprintf(execute_options->option3, sizeof(execute_options->option3), "%s", "Sort");
    snprintf(execute_options->option4, sizeof(execute_options->option4), "%s", input_filename);
    snprintf(execute_options->option5, sizeof(execute_options->option5), "%s", output_filename);

    requestServices(request_options);
    executeServices(execute_options);
}


void ScientificLibrary_Min_V2(char *input_filename, char*output_filename){
    OptionsStruct *request_options, *execute_options;
    request_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));
    execute_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));

    snprintf(request_options->command, sizeof(request_options->command), "%s", "request");
    snprintf(request_options->option1, sizeof(request_options->option1), "%s", "ScientificLibrary");
    snprintf(request_options->option2, sizeof(request_options->option2), "%s", "2");
    snprintf(request_options->option3, sizeof(request_options->option3), "%s", "Min");

    snprintf(execute_options->command, sizeof(execute_options->command), "%s", "execute");
    snprintf(execute_options->option1, sizeof(execute_options->option1), "%s", "ScientificLibrary");
    snprintf(execute_options->option2, sizeof(execute_options->option2), "%s", "2");
    snprintf(execute_options->option3, sizeof(execute_options->option3), "%s", "Min");
    snprintf(execute_options->option4, sizeof(execute_options->option4), "%s", input_filename);
    snprintf(execute_options->option5, sizeof(execute_options->option5), "%s", output_filename);

    requestServices(request_options);
    executeServices(execute_options);
}

void ScientificLibrary_Max_V2(char *input_filename, char*output_filename){
    OptionsStruct *request_options, *execute_options;
    request_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));
    execute_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));

    snprintf(request_options->command, sizeof(request_options->command), "%s", "request");
    snprintf(request_options->option1, sizeof(request_options->option1), "%s", "ScientificLibrary");
    snprintf(request_options->option2, sizeof(request_options->option2), "%s", "2");
    snprintf(request_options->option3, sizeof(request_options->option3), "%s", "Max");

    snprintf(execute_options->command, sizeof(execute_options->command), "%s", "execute");
    snprintf(execute_options->option1, sizeof(execute_options->option1), "%s", "ScientificLibrary");
    snprintf(execute_options->option2, sizeof(execute_options->option2), "%s", "2");
    snprintf(execute_options->option3, sizeof(execute_options->option3), "%s", "Max");
    snprintf(execute_options->option4, sizeof(execute_options->option4), "%s", input_filename);
    snprintf(execute_options->option5, sizeof(execute_options->option5), "%s", output_filename);

    requestServices(request_options);
    executeServices(execute_options);
}

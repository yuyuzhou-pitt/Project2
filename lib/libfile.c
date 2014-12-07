#define _XOPEN_SOURCE 500
#define __USE_BSD
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include "liblog.h"

int copy_file(char *old_filename, char  *new_filename){
    FILE  *ptr_old, *ptr_new;
    int  a;

    if(access(old_filename, F_OK) < 0) {
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "copy_file: File not found: %s\n", old_filename);
        logging(LOGFILE, logmsg);
        return -1;
    }

    ptr_old = fopen(old_filename, "rb");
    ptr_new = fopen(new_filename, "wb");

    if(ptr_old < 0)
        return  -1;

    if(ptr_new < 0){
        fclose(ptr_old);
        return  -1;
    }

    while(1){
        a  =  fgetc(ptr_old);

        if(!feof(ptr_old))
            fputc(a, ptr_new);
        else
            break;
    }

    fclose(ptr_new);
    fclose(ptr_old);
    return  0;
}

/*write file*/
int writeFileStr(char *str, char *file){
    FILE *fp;
    if ((fp = fopen(file, "w")) < 0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "writefile: Failed to open file: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    if((fprintf(fp, "%s", str))<0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "writefile: Failed to write file %s.", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    fclose(fp);
    return 0;
}

/*write file*/
int writeFile(char *str, int size, char *file, char *writeMode){
    FILE *fp;
    if ((fp = fopen(file, writeMode)) < 0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "writefile: Failed to open file: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    if((fwrite(str, 1, size, fp))<0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "writefile: Failed to write file %s.", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    fclose(fp);
    return 0;
}

/*read file*/
int readFile(char *str, int size, char *file){
    FILE *fp;

    if(access(file, F_OK) < 0) {
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: File not found: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    if ((fp = fopen(file,"r")) < 0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: Failed to open file: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    if((fgets(str, size, fp))<0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: Failed to read file: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    fclose(fp);

    return 0;
}

int unlinkFile(char *file){

    if(access(file, F_OK) < 0) {
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: File not found: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    if(unlink(file) < 0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Failed to unlink file: %s \n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    return 0;
}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

int rmrf(char *path){
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

int unlinkUpperFile(char *file){

    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));

    strcpy(hostfile, "../");
    strcat(hostfile, file);

    if(access(hostfile, F_OK) < 0) {
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: File not found: %s\n", hostfile);
        logging(LOGFILE, logmsg);
        return -1;
    }

    if(unlink(hostfile) < 0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Failed to unlink file: %s \n", hostfile);
        logging(LOGFILE, logmsg);
        return -1;
    }

    return 0;
}

// trim the leading and end white space
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

/*convert str to int array
* read str from start_n into the res, until 
*   a. the end of str, or
*   b. res if full
* by default it starts from 0 (start_n = 0)
* when start_index + str_n > res_size, next time, start_n will set as:
*   start_n = res_size - start_index
*/
int str2IntArray(int res[], int res_size, int start_index, char str[], int start_n){
    char *p = strtok (str, " "); // p is the first term
    int i;
    i = start_index; //the start index of the int array
    
    int str_n = 0; //record the str length
    while (p) {
        //printf("%d?%d ", str_n, start_n);
        if(i < res_size && str_n >= start_n){
            res[i] = atoi(p);
            //printf("%d ", res[i]);
            i++;
        }
        p = strtok (NULL, " ");
        str_n ++;
    }

    free(p);
    return str_n; // retrun the lenth of the whole str
    //return i - start_index; // retrun the read number
}

/*unlink port file*/
int unlinkPortFile(char *file){
    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));
    strcpy(hostfile, ".");
    strcat(hostfile, file);

    unlinkFile(hostfile);

    return 0;
}

/*write ip:port to port mapper file*/
int writePortMapper(int port, char *hostip, char *file){

    char portstr[6];
    sprintf(portstr, "%d", port); // int to str

    char ipandport[32];
    memset(ipandport, 0, sizeof(ipandport));
    strcpy(ipandport, portstr);
    strcat(ipandport, ":");
    strcat(ipandport, hostip);

    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));

    strcpy(hostfile, "../");
    strcat(hostfile, file);

    if(writeFile(ipandport, sizeof(ipandport), hostfile, "w") < 0){
        perror("writeport");
        return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "write port to file: %s\n", hostfile);
    logging(LOGFILE, logmsg);

    return 0;
}


/*get port from host file*/
int getPortMapper(char portstr[6], char ipstr[32], char *file){
    char templine[32];
    char pm_ipstr[32];
    char pm_portstr[6]; // to store the port

    char *split1, *split2, *strsplit;
    split1 = pm_ipstr;
    split2 = pm_portstr;

    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));
    strcpy(hostfile, "../");
    strcat(hostfile, file);

    if(readFile(templine, sizeof(templine), hostfile) < 0){
        return -1;
    }

    //PORT_MAPPER_FILE is in the format ipstr:portstr
    strsplit = templine;
    split1 = strtok_r(strsplit, ":", &split2);
    snprintf(portstr, sizeof(pm_portstr), "%s", split1);
    snprintf(ipstr, sizeof(pm_ipstr), "%s", split2);

    //char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "port on %s is: %s\n", ipfile, portstr);
    //logging(LOGFILE, logmsg);

    return 0;
}

/*write port to host file*/
int writePort(int port, char *hostip){

    char portstr[6];
    sprintf(portstr, "%d", port); // int to str

    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));

    strcpy(hostfile, "../.");
    strcat(hostfile, hostip);

    if(writeFile(portstr, sizeof(portstr), hostfile, "w") < 0){
        perror("writeport");
        return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "write port %s to file: %s\n", portstr, hostfile);
    logging(LOGFILE, logmsg);

    return 0;
}

/*get port from host file*/
int getPort(char *portstr, int size, char *ipfile){
    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));
    strcpy(hostfile, "../.");
    strcat(hostfile, ipfile);

    if(readFile(portstr, size, hostfile) < 0){
        return -1;
    }
    //char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "port on %s is: %s\n", ipfile, portstr);
    //logging(LOGFILE, logmsg);

    return 0;
}

/*get port from host file*/
int getPortNumber(char *ipfile){
    char portstr[6];
    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));
    strcpy(hostfile, "../.");
    strcat(hostfile, ipfile);

    if(readFile(portstr, 6, hostfile) < 0){
        return -1;
    }
    //char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "port on %s is: %s\n", ipfile, portstr);
    //logging(LOGFILE, logmsg);

    return atoi(portstr);
}

/*check special char*/
int checkSpecialChar(char *str){
    return str[strspn(str, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_./")] != 0;
}

int getTimeStamp(){
    //#ifdef TIMESTAMP
    struct timeval timer; // use high quality timer to calculate the ping cost
    //struct timezone tzp;

    gettimeofday(&timer, NULL);
    return timer.tv_usec;
    //return timer.tv_sec;
    //printf("TIMESTAMP: %d.\n", timer.tv_sec);
    //#endif
}

struct timeval getUTimeStamp(){
    struct timeval timer; // use high quality timer to calculate the ping cost
    //struct timezone tzp;

    gettimeofday(&timer, NULL);
    return timer;
}

/* return file extention when delimiter is '.', or
 * return filename when delimiter is '/'*/
const char *getStrAfterDelimiter(const char *filename, char delimiter) {
    const char *dot = strrchr(filename, delimiter); //Locate last occurrence of character in string
    if(!dot || dot == filename) return "";
    return dot + 1;
}

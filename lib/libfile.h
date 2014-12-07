#ifndef __LIBFILE_H__
#define __LIBFILE_H__

int copy_file(char *old_filename, char  *new_filename);
int copy_dir(char *old_dir, char *new_dir);
int writeFile(char *str, int size, char *file, char *writeMode);
int readFile(char *str, int size, char *file);
int unlinkFile(char *file);
int str2IntArray(int res[], int res_size, int start_index, char str[], int start_n);
int rmrf(char *path);
int unlinkPortFile(char *file);
int writePortMapper(int port, char *hostip, char *file);
int getPortMapper(char portstr[6], char ipstr[32], char *file);
int writePort(int port, char *hostip);
int getPort(char *portstr, int size, char *ipfile);
int getPortNumber(char *ipfile);
int checkSpecialChar(char *str);
int getTimeStamp();
struct timeval getUTimeStamp();
const char *getStrAfterDelimiter(const char *filename, char delimiter);

#endif

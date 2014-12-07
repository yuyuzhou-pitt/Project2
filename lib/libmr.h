#ifndef __LIBMR_H__
#define __LIBMR_H__

int Split(char *file, char *target_dir);
int Reduce(char *file, char *target_dir);
int Search(char *file, char *term, char *target_dir);

#endif

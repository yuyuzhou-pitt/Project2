#ifndef __LIBSCIENTIFIC_H__
#define __LIBSCIENTIFIC_H__

/*for server programs*/
struct Remote_Program_Struct{
    char program_name[30]; //Remote_Scientific_Library
    char version_number[8]; //1, 2, 3, ...
    char procedure1[30]; //Multiply (A[n,m] int, B[m,l] int, C[n,l] int)=1;
    char procedure2[30]; //Sort (vi int, i int)=2;
    char procedure3[30]; //Min (xi int, i int)=3;
    char procedure4[30]; //Max (xi int, i int)=4;
}RemoteProgram;

struct Remote_Program_Struct *programLibrary();
struct Remote_Program_Struct *programLibraryv2();

#endif

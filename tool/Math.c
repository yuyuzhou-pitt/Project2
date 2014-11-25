#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../lib/libmath.h"
#include "../lib/libfile.h"

//#define TIMESTAMP 1

int main(argc, argv)
int argc;
char *argv[];
{
        int time1 = getTimeStamp();
	printf("(TS:%d) math starts.\n", time1);
	FILE* in = fopen(argv[1],"r");
	FILE* out;
	
	int choice=-1;
	if(strcmp(argv[1],"Multiply.in")==0) choice=0;
	else if(strcmp(argv[1],"Sort.in")==0) choice=1;
	else if(strcmp(argv[1],"Min.in")==0) choice=2;
	else if(strcmp(argv[1],"Max.in")==0) choice=3;
	
	int m, n, l;
	int i, j;
	int* d;
	switch(choice)
	{
		case 0: // matrix multiply
			fscanf(in,"%d %d",&m,&n);
			int* a = (int *)malloc(m * n * sizeof(int));
			for(i=0;i<m;i++)
			{
				for(j=0;j<n;j++)
				{
					fscanf(in,"%d",a+n*i+j);
				}
			}
			
			fscanf(in,"%d %d",&n,&l);
			int* b = (int *)malloc(n * l * sizeof(int));
			for(i=0;i<n;i++)
			{
				for(j=0;j<l;j++)
				{
					fscanf(in,"%d",b+l*i+j);
				}
			}
			
	printf("(TS:%d) file %s read.\n", getTimeStamp(), argv[1]);

			int* c = (int *)malloc(m * l * sizeof(int));
			memset (c, 0, sizeof(*c));
			c = Multiply(a,b,c,m,n,l);
	printf("(TS:%d) result calculated.\n", getTimeStamp());
			out = fopen("Multiply.out","w");
			printmatrix(out,c,m,l);
        int time2 = getTimeStamp();
        #ifdef TIMESTAMP
	printf("(TS:%d) wrote to file Multiply.\n", time2);
	printf("**************************\n");
	printf("** Time elapsed %d us. **\n", time2 - time1);
	printf("**************************\n");
        #endif
			break;
		case 1: // sort array
			fscanf(in,"%d",&n);
			d = (int *)malloc(n * sizeof(int));
			for(i=0;i<n;i++)
			{
				fscanf(in,"%d",d+i);
			}
			Sort(d, n);
			out = fopen("Sort.out","w");
			printarray(out, d, n);
			break;
		case 2: // min
			fscanf(in,"%d",&n);
			d = (int *)malloc(n * sizeof(int));
			for(i=0;i<n;i++)
			{
				fscanf(in,"%d",d+i);
			}
			out = fopen("Min.out","w");
			fprintf(out, "%d", Min(d,n));
			break;
		case 3: // max
			fscanf(in,"%d",&n);
			d = (int *)malloc(n * sizeof(int));
			for(i=0;i<n;i++)
			{
				fscanf(in,"%d",d+i);
			}
			out = fopen("Max.out","w");
			fprintf(out, "%d", Max(d, n));
			break;
		default:
			break;
	}
	fclose(in);
	fclose(out);
	return 0;
}

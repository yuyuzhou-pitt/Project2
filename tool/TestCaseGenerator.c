#include<stdio.h>
#include<stdlib.h>

int* gen_matrix(FILE* file, int rows, int cols)
{
	int i, j;
	for(i=0;i<rows;i++)
	{
		for(j=0;j<cols;j++)
		{
			fprintf(file, "%d ", rand()%10);
		}
		fprintf(file, "\n");
	}
}

int* gen_array(FILE* file, int l)
{	
	fprintf(file, "%d\n",l);
	int i;
	for(i=0;i<l;i++)
	{
		fprintf(file, "%d ", rand()%100);
	}
}

int main(argc, argv)
int argc;
char *argv[];
{
	int choice=-1;
	if(strcmp(argv[1],"Multiply")==0||strcmp(argv[1],"multiply")==0) choice=0;
	else if(strcmp(argv[1],"Sort")==0||strcmp(argv[1],"sort")==0) choice=1;
	else if(strcmp(argv[1],"Min")==0||strcmp(argv[1],"min")==0) choice=2;
	else if(strcmp(argv[1],"Max")==0||strcmp(argv[1],"max")==0) choice=3;
	
	FILE* in;
	int len, row1, col1, row2, col2;
	switch(choice)
	{
		case 0:
			if(argc != 6){
				printf("usage: ./test Multiply <row1> <col1> <row2> <col2>\n");
				return;
                        }
			row1=atoi(argv[2]), col1=atoi(argv[3]), row2=atoi(argv[4]), col2=atoi(argv[5]);
			if(col1!=row2) {
				printf(" <col1> != <row2> ?\n");
				return;
			}
			in=fopen("Multiply.in","w");
			fprintf(in, "%d %d\n",row1,col1);
			gen_matrix(in, row1, col1);
			fprintf(in, "%d %d\n",row2,col2);
			gen_matrix(in, row2, col2);
			break;
		case 1:
			len=atoi(argv[2]);
			in=fopen("Sort.in","w");
			gen_array(in, len);
			break;
		case 2:
			len=atoi(argv[2]);
			in=fopen("Min.in","w");
			gen_array(in, len);
			break;
		case 3:
			len=atoi(argv[2]);
			in=fopen("Max.in","w");
			gen_array(in, len);
			break;
		default:
			break;
	}
	fclose(in);
	return 0;
}

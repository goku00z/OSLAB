#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv){
	printf("\n**************************\n");
	printf("\nTOTAL ARGUMENTS = %d",argc);
	printf("\nARGUMENT LIST\n");
	for(int i=0;i<argc;i++){
		printf("\nARGV[%d] = %s",i,argv[i]);
	}
	printf("\n");
	printf("\n**************************\n");
	return 0;
}
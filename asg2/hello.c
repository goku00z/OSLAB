#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv){
	printf("\nTOTAL ARGUMENTS = %d",argc);
	printf("\nARGV[%d] = %s",1,argv[1]);
	printf("\nARGV[%d] = %s",2,argv[2]);
	printf("\nARGV[%d] = %s",3,argv[3]);
	printf("\n");
	return 0;
}
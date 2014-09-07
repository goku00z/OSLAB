#include <stdio.h>
#include <stdlib.h>

int main(){
	int n;
	// printf("\n@@@@@@@@@@@ PROG 3 @@@@@@@@@@@@@@\n");
	while(1){
		scanf("%d",&n);
		if(n!=-1){
			printf(" %d ",n*10);
		}
		if(n==-1){
			break;
		}
	}
	printf(" -1 ");
	return 0;
}
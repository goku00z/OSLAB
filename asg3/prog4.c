#include <stdio.h>
#include <stdlib.h>

int main(){
	int n;
	// printf("\nPROG 4\n");
	while(1){
		// printf("\nWAITING FOR INPUT:");
		scanf("%d",&n);
		// printf(" %d\n",n);
		// printf("\nWAITING FOR INPU323T:");
		if(n!=-1){
			printf(" %d\n",n*100);
		}
		if(n==-1){
			// printf("\nN = %d",n);
			break;
		}
		// printf("\nHERE\n");
	}
	printf("\n -1 \n");
	return 0;
}
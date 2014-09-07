#include <stdio.h>
#include <stdlib.h>

int main(){
	int n;
	// printf("\n########### PROG 2 #############\n");
	while(1){
		scanf("%d",&n);
		if(n%2 == 0){
			printf(" %d\n",n);
		}
		if(n==-1){
			break;
		}
	}
	printf(" -1\n");
	return 0;
}
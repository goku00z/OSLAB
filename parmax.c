#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
// #include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
int getMax(int *A,int i,int j){
	int max = A[i];
	int k;
	for(k=i+1;k<=j;k++){
		if(A[k]>max){
			max = A[k];
		}
	}
	return max;
}
int getParMax(int *A,int n){
	int L,R,M;
	int pidL,pidR,maxL,maxR,max;
	L = 0;
	R = n-1;
	int isRoot = 1;
	int temp;
	if(n<10){
		return getMax(A,0,n-1);
	}
	while(1){
		printf("\nR = %d L = %d # = %d\n",R,L,R-L+1);
		scanf("%d",&temp);
		if(R-L+1>10){
			M = (L+R)/2;
			pidL = fork();
			if(pidL == 0){
				// left child
				isRoot = 0;
				R = M;
			}else{
				pidR = fork();
				// printf("\nIn parent is root = %d",isRoot);
				if(pidR==0){
					// right child
					isRoot = 0;
					L = M+1;
				}else{
					waitpid(pidL,&maxL,0);
					waitpid(pidR,&maxR,0);
					maxL = WEXITSTATUS(maxL);
					maxR = WEXITSTATUS(maxR);
					if(maxL>maxR){
						max = maxL;
					}else{
						max = maxR;
					}
					// break;
				}
				if(isRoot == 1){
					break;
				}
			}
		}else{
			// if(isRoot==0){
				printf("\nWE ARE exiting!!");
				max = getMax(A,L,R);
				printf("\nMAX EXIT = %d",max);
				exit(max);
			// }
			// if(isRoot==1){
				// printf("\n*************returning !!****");
				// max = getMax(A,L,R);
				// break;
			// }
		}
	}
	return max;
}
int main(int argc,char **argv){
	int A[50];
	srand((unsigned int)time(NULL));
	int n = atoi(argv[1]);
	int i;
	if(n<=0||n>50){
		printf("\nARRAY SIZE out of Range !!\n");
		exit(0);
	}
	for(i=0;i<n;i++){
		A[i] = rand()%128;
	}
	printf("\nA[] = {");
	for(i=0;i<n;i++){
		printf(" %d ",A[i]);
	}
	printf("}");
	int max = getParMax(A,n);
	printf("\nMAX = %d\n",max);
	// printf("\nMAXIMA = %d",getMax(A,0,n-1));
	printf("\n");
	return 0;
}
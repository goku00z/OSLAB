#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h> 


int findMax(int *A, int s, int e){
	int i,max;
	max = A[s];
	for(i = s; i <= e; i++){
		if(max < A[i]){
			max = A[i];
		}
	}
	return max;
}

int main(int argc, char **argv){
    srand((unsigned int)time(NULL));
	int n = atoi(argv[1]), i;
	if(n <= 0 || n > 100){
		printf("Size should be between 0 and 100!");
		printf("\nEnter any key to exit:");
		int temp;
		scanf("%d",&temp);
		return 0;
	}
	int A[100], L, R, M, pidL, pidR, maxl, maxr, max; 
	for(i = 0; i < n; i++){
		A[i] = rand() % 128;
	}
	printf("Initial unsorted array:\n");
	for(i = 0; i < n; i++){
		printf("%d ", A[i]);
	}
	printf("\n");
	
	if(n < 10){
		printf("Max = %d\n", findMax(A,0,n-1));
		printf("\nEnter any key to exit:");
		int temp;
		scanf("%d",&temp);
		return 0;
	}
	L = 0; R = n - 1;
	int isRoot = 1;
	while(1){
		if(R - L + 1 > 10){
			M = (L + R) / 2;
			pidL = fork();
			if(pidL == 0){
				R = M;
				isRoot = 0;
			}
			else{
				pidR = fork();
				if(pidR == 0){
					L = M + 1;
					isRoot = 0;
				}
				else{
					waitpid(pidL, &maxl, 0);
					waitpid(pidR, &maxr, 0);
					maxl = WEXITSTATUS(maxl);
					maxr = WEXITSTATUS(maxr);
					if(maxl > maxr){
						max = maxl;
					}
					else{
						max = maxr;
					}
					printf("Max = %d : Process ID = %d : Parent Process ID = %d : L = %d : R = %d\n", max, getpid(), getppid(), L, R);
					if(isRoot==0){
						exit(max);
					}else{
						break;
					}
				}
			}
		}
		else{
			max = findMax(A, L, R);
			printf("Max = %d : Process ID = %d : Parent Process ID = %d : L = %d : R = %d\n", max, getpid(), getppid(), L, R);
			exit(max);
		}

	}
	printf("\nMAX = %d\n",max);
	printf("\nEnter any key to exit:");
	int temp;
	scanf("%d",&temp);
	return 0;
} 
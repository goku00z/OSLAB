#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int maxPipeCount(){
	int pid,max,isRoot;
	while(1){
		int p[2];
		pipe(p);
		pid = fork();
		if(pid == 0){
			isRoot = 1;
		}else{
			waitpid(pid,&max,0);
			max = WEXITSTATUS(max);
			if(isRoot == 0){
				exit(max+1);
			}else{
				break;
			}
		}
	}
	return max;
}

int main(){
	int n = maxPipeCount();
	printf("\nMaximum Number of Pipes : %d",n);
	printf("\n");
	return 0;
}
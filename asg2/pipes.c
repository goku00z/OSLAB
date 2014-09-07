#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define BUFFER 1000
int killChild = 0;
int killParent = 0;

int resumeChild = 0;
int resumeParent = 0;

void childSignalHandler(int sig){
	if(sig == SIGUSR1){
		printf("\nKILLCHILD = %d --> %d\n",killChild,1);
		killChild = 1;
	}else{
		resumeChild = 1;
	}
	return;
}
void parentSignalHandler(int sig){
	if(sig == SIGUSR2){
		printf("\nParent Received death signal from child\n");
		killParent = 1;
	}else{
		resumeParent = 1;
	}
	return;
}
int main(){
	int p[2];
	int pid;
	pipe(p);
	pid = fork();
	if(pid==0){
		char line[BUFFER];
		int i=1;
		signal(SIGUSR1,childSignalHandler);
		signal(SIGUSR2,childSignalHandler);
		while(1){
			if(killChild==1){
				// sleep(1);
				kill(getppid(),SIGUSR2);
				printf("\nCHILD is going to die !!\n");
				break;
			}else{
				printf("\nChild is going to write\n");
				sprintf(line,"%d",i);
				i++;
				write(p[1],line,BUFFER);
				kill(getppid(),SIGUSR1);
				while(resumeChild!=1){}
				resumeChild = 0;
			}
		}
		exit(0);

	}else{
		// sleep(1);
		char lineP[BUFFER];
		int n;
		signal(SIGUSR2,parentSignalHandler);
		signal(SIGUSR1,parentSignalHandler);
		while(1){
			while(resumeParent!=1){}
			resumeParent=0;
			if(killParent==1){
				printf("\nParent is going to die\n");
				break;
			}else{
				// sleep(1);
				printf("\nParent is trying to read !\n");
				read(p[0],lineP,BUFFER);
				sscanf(lineP,"%d",&n);
				printf("\nParent reads = %d\n",n);
				kill(pid,SIGUSR2);
				if(n==10){
					printf("\nGOING TO KILL CHILD !!\n");
					kill(pid,SIGUSR1);
					// break;
				}
			}
		}
	}
	printf("\n");
	return 0;
}
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
		printf("\nRESUMING CHILD\n");
		resumeChild = 1;
		return;
	}
	if(sig == SIGUSR2){
		printf("\nPAUSING CHILD\n");
		resumeChild = 0;
		return;
	}
}
void parentSignalHandler(int sig){
	if(sig == SIGUSR1){
		printf("\nRESUMING PARENT\n");
		resumeParent = 1;
		return;
	}
	if(sig == SIGUSR2){
		printf("\nPAUSING PARENT\n");
		resumeParent = 0;
		return;
	}
}

int main(){
	system("reset");
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
			printf("\nChild is going to write\n");
			sprintf(line,"%d",i);
			i++;
			write(p[1],line,BUFFER);
			printf("\nGOING TO RESUME PARENT\n");
			kill(getppid(),SIGUSR1);
			printf("\nPARENT RESUMED---\n");
			while(resumeChild!=1){}
			printf("\nCHILD FREED\n");
			kill(getppid(),SIGUSR2);
			printf("\nPARENT LOCKED\n");
			resumeChild = 0;
		}
		// exit(0);

	}else{
		// sleep(1);
		char lineP[BUFFER];
		int n;
		signal(SIGUSR2,parentSignalHandler);
		signal(SIGUSR1,parentSignalHandler);
		while(1){
			while(resumeParent!=1){}
			printf("\nPARENT RESUMED\n");
			kill(pid,SIGUSR2);
			resumeParent=0;
			printf("\nParent is trying to read !\n");
			read(p[0],lineP,BUFFER);
			sscanf(lineP,"%d",&n);
			printf("\nParent reads = %d\n",n);
			if(n==5){
				kill(pid,SIGKILL);
				break;
			}
			printf("\nGOING TO RESUME CHILD\n");
			kill(pid,SIGUSR1);
		}
	}
	printf("\n");
	return 0;
}
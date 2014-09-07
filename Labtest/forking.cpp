#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void signalhandler(int sig){
	if(sig == SIGCHLD){
		printf("\nSIGCHLD RECEIVED\n");
	}
}

void runChild(){
	printf("\nCHILD IS RUNNING !!\n");
	// sleep(1);
	return;
}

int createChild(){
	int pid = fork();
	if(pid != 0){
		return pid;
	}else{
		runChild();
		exit(-1);
	}
}

int main(){
	signal(SIGCHLD,&signalhandler);
	printf("\nCreating Child !!\n");
	int pid = createChild();
	printf("\nParent going to wait for child");
	int status = 12;
	// wait(&status);
	waitpid(pid,&status,0);
	// printf("%d",status>>1);
	// status = WEXITSTATUS(status);
	printf("\nEXIT STATUS = %d\n",status);
	return 0;
}
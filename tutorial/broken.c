#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX 100

void abc(int sig){
	if(sig == SIGPIPE){
		printf("\nSIG FOUND!!");
	}
	return;
}
main()
{
	signal(SIGPIPE,abc);
	char *msg="hello";
	char buff[MAX];
	int p[2],pid;
	pipe(p);
	pid=fork();
	if(pid==0)
	{	
	
		printf("child exiting");
	}
	else
	{
		sleep(1);
		close(p[0]);
		write(p[1],buff, MAX);
		printf("\nNO SIG \n");				
	}
	return 0;
}

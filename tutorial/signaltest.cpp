#include <stdio.h>
#include <signal.h>

void abc(int);
int main()
{
	signal(SIGINT,abc);
	signal(SIGQUIT,abc);
	for(;;);

}

void abc(int signo)
{
	printf("You have killed the process with signal ID=%d\n",signo);
}

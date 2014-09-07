#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define BUFFERSIZE 1000
#define MIN 0
#define MAX 1
#define MAXIMUM 10000
int killC = 0;
int killD = 0;

void C_signalHandler(int sig){
	killC = 1;
	return;
}
void D_signalHandler(int sig){
	killD = 1;
	return;
}

int main(){
	system("reset");
	int pipeC[2],pipeD[2];
	int pid_C,pid_D;
	// creating first pipe between P and C
	pipe(pipeC);
	// creating process C
	pid_C = fork();
	if(pid_C==0){
		// code for process C
		signal(SIGUSR1,C_signalHandler);
		close(pipeC[0]);
		sleep(1);
		int n;
		char line[BUFFERSIZE];
		srand((unsigned int)time(NULL));
		while(1){
			if(killC == 1){
				break;
			}
			n=rand()%MAXIMUM;
			sprintf(line,"%d",n);
			printf("\nC is going to write = %d\n",n);
			write(pipeC[1],line,BUFFERSIZE);
			printf("\nC is going to sleep\n");
			sleep(1);
		}
		exit(0);
	}else{
		// creating 2nd pipe between P and D
		pipe(pipeD);
		// creaing process D
		pid_D = fork();
		if(pid_D==0){
			// code for process D
			signal(SIGUSR1,D_signalHandler);
			close(pipeD[0]);
			srand((unsigned int)time(NULL));
			int n;
			char line[BUFFERSIZE];
			while(1){
				if(killD == 1){
					break;
				}
				n=rand()%MAXIMUM;
				sprintf(line,"%d",n);
				printf("\nD is going to write = %d\n",n);
				write(pipeD[1],line,BUFFERSIZE);
				printf("\nD is going to sleep\n");
				sleep(1);
			}
			exit(0);
		}else{
			// process P
			// sleep(1);
			close(pipeC[1]);
			close(pipeD[1]);
			int scoreC,scoreD;
			scoreC = scoreD = 0;
			int flag;
			int num_C,num_D;
			char line[BUFFERSIZE];
			int i=1;
			while(1){
				if(scoreC==10||scoreD==10){
					break;
				}
				printf("\n*******************************\n");
				printf("\nROUND = %d\n",i);
				printf("\nP is going to sleep\n");
				sleep(1);
				printf("\nP is going to read\n");
				read(pipeC[0],line,BUFFERSIZE);
				sscanf(line,"%d",&num_C);
				read(pipeD[0],line,BUFFERSIZE);
				sscanf(line,"%d",&num_D);
				printf("\nP: C gives = %d\tD gives = %d\n",num_C,num_D);
				flag=rand()%2;
				if(flag == MIN){
					printf("\nFlag = MIN\n");
					if(num_C<num_D){
						scoreC++;
						printf("\nC wins round %d [C-score = %d\tD-Score = %d]",i,scoreC,scoreD);
					}
					if(num_C>num_D){
						scoreD++;
						printf("\nD wins round %d [C-score = %d\tD-Score = %d]",i,scoreC,scoreD);
					}
				}
				if(flag == MAX){
					printf("\nFlag = MAX\n");
					if(num_C<num_D){
						scoreD++;
						printf("\nD wins round %d [C-score = %d\tD-Score = %d]",i,scoreC,scoreD);
					}
					if(num_C>num_D){
						scoreC++;
						printf("\nC wins round %d [C-score = %d\tD-Score = %d]",i,scoreC,scoreD);
					}
				}
				if(num_C == num_D){
					printf("\nRound %d is tied\n",i);
				}
				printf("\n*******************************\n");
				i++;
			}
			printf("\nC-Score = %d\tD-Score = %d\n",scoreC,scoreD);
			if(scoreC>scoreD){
				printf("\nC Wins\n");
			}
			if(scoreC<scoreD){
				printf("\nD Wins\n");
			}
			kill(pid_C,SIGUSR1);
			kill(pid_D,SIGUSR1);
			int status;
			waitpid(pid_C,&status,0);
			waitpid(pid_D,&status,0);
		}
	}
	return 0;
}
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>	
#include <time.h>
#include <stdlib.h>

#define N 5
#define LEFT ((i+N-1)%N)
#define RIGHT ((i+1)%N)
#define THINKING 0
#define HUNGRY 1
#define EATING 2

#define PM(s) semop(s,&mutp, 1)
#define VM(s) semop(s,&mutv, 1)
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)


int s, mutex;
// int state[N];
key_t key;
int *a;
int *state[5];

struct sembuf pop, vop, mutp, mutv;

void think(){
	
	int stime = rand() % 5 + 1;
	sleep(stime);
	
}

void eat(){
	
	int stime = rand() % 5 + 1;
	sleep(stime);

}


void test(int shmkey, int i){
	
	// printf("Testing for %d\n", i);
	// printf("State for %d is %d\n",i,state[shmkey][i]);
	// printf("State for %d is %d\n",LEFT,state[shmkey][LEFT]);
	// printf("State for %d is %d\n",RIGHT,state[shmkey][RIGHT]);
	if(state[shmkey][i] == HUNGRY && state[shmkey][LEFT] != EATING && state[shmkey][RIGHT] != EATING){
		state[shmkey][i] = EATING;
		pop.sem_num = vop.sem_num = i;
		// printf("Here for %d",i);
		V(s);
		// printf("Here for %d",i);
	}
}


void take_forks(int i){
	PM(mutex);
	// printf("trying to set hungry for %d\n", i);
	state[i][i] = HUNGRY;
	// printf("check if hungry %d",state[i][i]);
	test(i,i);
	VM(mutex);
	pop.sem_num = vop.sem_num = i;
	// printf("Checking for wait on %d", i);
	P(s);
	// printf("Crossed wait on %d", i);
}

void put_forks(int i){
	PM(mutex);
	state[i][i] = THINKING;
	test(i,LEFT);
	test(i,RIGHT);
	VM(mutex);
}

void philospher(int i){
	while(1){
		int ctr;
		printf("philospher %d starts thinking\n",i);
		think();
		printf("philospher %d wants to eat\n",i);
		take_forks(i);
		printf("philospher %d is starting to eat\n",i);
		eat();
		printf("philospher %d has finished eating\n",i);
		put_forks(i);
	}
}

int main(){
	srand(time(NULL));
	s = semget(IPC_PRIVATE, N, 0777|IPC_CREAT);
	mutex = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	semctl(s, 0, SETALL, 0);
	semctl(mutex, 0, SETVAL, 1);
	pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1 ; vop.sem_op = 1 ;
	mutp.sem_num = mutv.sem_num = 0;
	mutp.sem_flg = mutv.sem_flg = 0;
	mutp.sem_op = -1 ; mutv.sem_op = 1 ;
	int pid1, pid2, pid3, pid4, pid5, shmid, i;
	key = ftok("/usr/local/lib",'A');
	shmid = shmget(key, 5*sizeof(int), 0777|IPC_CREAT);
	if(shmid == -1){
		printf("shmget error!\n");
		exit(1);
	}
	a = (int *) shmat(shmid, 0, 0);
	for(i = 0; i < N; i++){
		a[i] = 0;
	}
	pid1 = fork();
	if(pid1 == 0){
		state[0] = (int *) shmat(shmid, 0, 0);
		philospher(0);
	}
	else{
		pid2 = fork();
		if(pid2 == 0){
			state[1] = (int *) shmat(shmid, 0, 0);
			philospher(1);
		}
		else{
			pid3 = fork();
			if(pid3 == 0){
				state[2] = (int *) shmat(shmid, 0, 0);
				philospher(2);
			}
			else{
				pid4 = fork();
				if(pid4 == 0){
					state[3] = (int *) shmat(shmid, 0, 0);
					philospher(3);
				}
				else{
					pid5 = fork();
					if(pid5 == 0){
						state[4] = (int *) shmat(shmid, 0, 0);
						philospher(4);
					}
					else{
						waitpid(pid1);
						waitpid(pid2);
						waitpid(pid3);
						waitpid(pid4);
						waitpid(pid5);
					}
				}
			}
		}
	}
}

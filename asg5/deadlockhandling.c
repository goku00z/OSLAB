#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 5
#define MAXTHINK 1
#define MAXEAT 1
#define DEADLOCKCHECKPERIOD 10
#define INBETWEENWAIT 5
#define PREMPTTIME 5

#define LEFT i
#define RIGHT ((i+1)%N)
#define P(semaphore,semP) semop(semaphore,&semP,1)
#define V(semaphore,semV) semop(semaphore,&semV,1)
#define DEBUG if(debug == 1)

int debug = 0;

void initSemaphoreOperation(struct sembuf *x,int sem_num,int sem_flg,int sem_op){
	x->sem_num = sem_num;
	x->sem_flg = sem_flg;
	x->sem_op = sem_op;
	return;
}
void think(){
	int stime = rand() % MAXTHINK + 1;
	sleep(stime);
	return;	
}
void eat(){
	int stime = rand() % MAXEAT + 1;
	sleep(stime);
	return;
}
void printSemaphoreStatus(int semaphore){
	ushort status[N];
	for(int i=0;i<N;i++){
		status[i] = 8;
	}
	semctl(semaphore,N,GETALL,status);
	for(int i=0;i<N;i++){
		printf("SEM[%d] = %d\t",i,status[i]);
	}
	printf("\n");
	return;
}
void printMutex(int mutex){
	ushort status[1];
	semctl(mutex,1,GETALL,status);
	printf("\nMUTEX = %d\n",status[0]);
	return;
}
void takeFork(int semaphore,int i,int shmid,int mutex){
	struct sembuf semP,mutP,mutV;
	initSemaphoreOperation(&semP,i,0,-1); // The P operation
	initSemaphoreOperation(&mutP,0,0,-1);
	initSemaphoreOperation(&mutV,0,0,1);
	
	DEBUG printf("Fork %d is going to be grabbed\n",i);
	DEBUG printf("\nBEFORE ****** \n");
	DEBUG printSemaphoreStatus(semaphore);
	
	P(semaphore,semP);
	// printf("\nJUST AFTER taking forking");
	// printMutex(mutex);
	P(mutex,mutP);
	int *a = (int *) shmat(shmid, 0, 0);
	a[i] = i;
	V(mutex,mutV);
	// printf("\nMUTEX FREED !!\n");

	DEBUG printSemaphoreStatus(semaphore);
	DEBUG printf("\n****** AFTER\n");
	DEBUG printf("Fork %d is grabbed\n",i);
	return;
}
void putFork(int semaphore,int i,int shmid,int mutex){
	struct sembuf semV,mutP,mutV;
	initSemaphoreOperation(&semV,i,0,1); // The V operation
	initSemaphoreOperation(&mutP,0,0,-1);
	initSemaphoreOperation(&mutV,0,0,1);

	DEBUG printf("Fork %d is going to be put down\n",i);
	DEBUG printf("\nBEFORE ****** \n");
	DEBUG printSemaphoreStatus(semaphore);
	
	P(mutex,mutP);
	int *a = (int *) shmat(shmid, 0, 0);
	a[i] = -1;
	V(semaphore,semV);
	V(mutex,mutV);

	DEBUG printSemaphoreStatus(semaphore);
	DEBUG printf("\n****** AFTER\n");
	DEBUG printf("Fork %d is putdown\n",i);

	return;
}
bool isForkTaken(int i,int k){
	bool flag = false;
	P(mutex,mutP);
	int *a = (int *) shmat(shmid, 0, 0);
	if(a[k] == i){
		flag = true;
	}
	V(mutex,mutV);
	return true;
}
void philosopher(int i,int semaphore,int shmid,int mutex){
	// the infinite loop for the philosopher i
	while(1){
		printf("Philospher %d starts thinking\n",i);
		think();
		printf("Philospher %d wants to eat\n",i);
		// Get the forks
		takeFork(semaphore,LEFT,shmid,mutex);
		printf("Philosopher %d grabs fork %d(LEFT)\n",i,LEFT);
		sleep(INBETWEENWAIT);
		if(!isForkTaken(i,RIGHT)){
			takeFork(semaphore,RIGHT,shmid,mutex);
		}
		if(!(isForkTaken(i))){
			sleep(PREMPTTIME);
			printf("\nPhilosopher %d prempted by Parent",i);
			continue;
		}
		printf("Philosopher %d grabs fork %d(RIGHT)\n",i,RIGHT);
		// forks reserved start eating
		printf("Philospher %d is starting eating\n",i);
		eat();
		// eating finished return forks
		putFork(semaphore,LEFT,shmid,mutex);
		putFork(semaphore,RIGHT,shmid,mutex);
		printf("Philospher %d ends eating and released fork %d(LEFT) and %d(RIGHT)\n",i,LEFT,RIGHT);
	}
	return;
}
int createPhilosopher(int i,int semaphore,int shmid,int mutex){
	int pid = fork();
	if(pid == 0){
		// child code
		// start philosopher i
		philosopher(i,semaphore,shmid,mutex);
		exit(0);
	}else{
		// parent code
		return pid;
	}
}
bool isDeadLock(int shmid,int mutex){
	// code to detect deadload
	struct sembuf mutP,mutV;
	initSemaphoreOperation(&mutP,0,0,-1);
	initSemaphoreOperation(&mutV,0,0,1);

	P(mutex,mutP);
	int *a = (int *) shmat(shmid, 0, 0);
	for(int i=0;i<N;i++){
		if(a[i]==-1){
			return false;
		}
	}
	V(mutex,mutV);
	return true;
}
void handleDeadLock(int *pid,int semaphore,int shmid,int mutex){
	// Code to handle deadlock
	int i = rand()%N; // randomly select a philosopher
	// putFork(semaphore,LEFT,shmid,mutex);
	// putFork(semaphore,RIGHT,shmid,mutex);
	return;
}
void parent(){
	// The Parent Module
	// Create Semaphores and semaphore as required
	srand(time(NULL));
	key_t key = ftok("/usr/local/lib",'A');
	int semaphore = semget(key,N,0666|IPC_CREAT);
	if(semaphore <0){
		printf("\n************ ERROR *********\n");
		exit(-1);
	}
	for(int i=0;i<N;i++){
		int flag = semctl(semaphore,i,SETVAL,(ushort)1);
		if(flag <0){
			printf("\nERROR !!!\n");
		}
	}
	printf("\nINITIAL SEMAPHORE STATUS : ");
	printSemaphoreStatus(semaphore);
	/*********** creating mutex for shared memory access ******/
	int mutex = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
	if(mutex<0){
		printf("\nERROR can not create mutex !!\n");
		exit(-1);
	}
	int flag = semctl(mutex,0,SETVAL,(ushort)1);
	if(flag <0){
		printf("\nERROR !!!\n");
	}
	/*********************************************/
	/********* create shared memory **************/
	key = ftok("dummy.txt",'A');
	int shmid = shmget(key, N*sizeof(int), 0777|IPC_CREAT);
	if(shmid == -1){
		printf("\nERROR in allocation shared memory");
		exit(-1);
	}
	int *a = (int *) shmat(shmid, 0, 0);
	for(int i = 0; i < N; i++){
		a[i] = -1; // can use without mutex as no child is not created till now
	}
	/*********************************************/
	int pid[N];
	printf("\nBEFORE FORKING : MUTEX :");
	printMutex(mutex);
	for(int i = 0;i < N;i++){
		pid[i] = createPhilosopher(i,semaphore,shmid,mutex);
	}
	// periodically check for deadlock
	while(1){
		sleep(DEADLOCKCHECKPERIOD);
		printf("\n************** INVOKING DEADLOCK DETECTION ****\n");
		if(isDeadLock(shmid,mutex)){
			printf("########### DEADLOCK DETECTED !! #############\n");
			handleDeadLock(pid,semaphore,shmid,mutex);
		}else{
			printf("\n_-_-_-_- NO Deadlock Found !!!!\n");
		}
	}
	// wait for children
	int status;
	for(int i = 0;i < N;i++){
		waitpid(pid[i],&status,0);
	}
	return;
}
int main(){
	system("reset");
	system("./clear.sh");
	parent();// launch parent
	return 0;
}
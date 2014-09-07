#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define PRODCOUNT 2
#define CONCOUNT 3

#define FULLKEYFILE "full.txt" 
#define FULLKEYID 1
#define MUTEXKEYFILE "mutex.txt"
#define MUTEXKEYID 2
#define EMPTYKEYFILE "empty.txt"
#define EMPTYKEYID 3
#define SHAREDKEYFILE "keyfile.txt"
#define SHAREDKEYID 4

#define P(semaphore,semP) semop(semaphore,&semP,1)
#define V(semaphore,semV) semop(semaphore,&semV,1)
#define IN N
#define OUT (N+1)
#define N 10

// msgctl(msgQID, IPC_RMID, NULL);
// int delete_segment(int seg_id){
//     if ((shmctl(seg_id,IPC_RMID,0))==-1){
//     std::cout<<" ERROR(C++)with shmctl(IPC_RMID): "<<strerror(errno)<<std::endl;
//     return -1;
//     }else//on success
//         return 0;
// }

void initSemaphoreOperation(struct sembuf *x,int sem_num,int sem_flg,int sem_op){
	x->sem_num = sem_num;
	x->sem_flg = sem_flg;
	x->sem_op = sem_op;
	return;
}

int getItem(){
	int n = rand()%10;
	return n * n; 
}
void produceItem(int id,int shmid){
	// must be done in mutex
	int *a = (int *)shmat(shmid,0,0);
	int in = a[IN];
	int out = a[OUT];
	a[in] = getItem();
	printf("\nPRODUCER [%d] produces = %d\n",id,a[in]);
	in = (in + 1)%N;
	a[IN] = in;
	shmdt(a);
	return;
}
int consumeItem(int shmid){
	// must be done in mutex
	int *a = (int *)shmat(shmid,0,0);
	int in = a[IN];
	int out = a[OUT];
	int item = a[out];
	out = (out + 1)%N;
	a[OUT] = out;
	shmdt(a);
	return item;
}
void producer(int id,int shmid,int full,int empty,int mutex){
	struct sembuf semV,semP;
	initSemaphoreOperation(&semV,0,0,1);
	initSemaphoreOperation(&semP,0,0,-1);
	while(1){
		// sleep(1);
		P(empty,semP);
		P(mutex,semP);
		produceItem(id,shmid);
		V(mutex,semV);
		V(full,semV);
	}
	return;
}
void consumer(int id,int shmid,int full,int empty,int mutex){
	struct sembuf semV,semP;
	initSemaphoreOperation(&semV,0,0,1);
	initSemaphoreOperation(&semP,0,0,-1);
	while(1){
		// sleep(1);
		P(full,semP);
		P(mutex,semP);
		int item = consumeItem(shmid);
		V(mutex,semV);
		V(empty,semV);
		printf("\nCONSUMER [%d] consumes = %d\n",id,item);
	}
	return;
}

int createProducer(int id,int shmid,int full,int empty,int mutex){
	int pid = fork();
	if(pid!=0){
		return pid;
	}else{
		srand(time(NULL));
		producer(id,shmid,full,empty,mutex);
		exit(0);
	}
}

int createConsumer(int id,int shmid,int full,int empty,int mutex){
	int pid = fork();
	if(pid!=0){
		return pid;
	}else{
		srand(time(NULL));
		consumer(id,shmid,full,empty,mutex);
		exit(0);
	}
}

main(){
	system("./clear.sh");
	srand(time(NULL));
	int full,empty,mutex,shmid;
	/*********** creating semaphores and shared memory *********/
	key_t keyFull = ftok(FULLKEYFILE,FULLKEYID);
	if((full = semget(keyFull,1,0666|IPC_CREAT)) < 0){
		printf("\nCANT CREATE SEMAPHORE {full}\n");
		exit(0);
	}
	key_t keyEmpty = ftok(EMPTYKEYFILE,EMPTYKEYID);
	if((empty = semget(keyEmpty,1,0666|IPC_CREAT)) < 0){
		printf("\nCANT CREATE SEMAPHORE {empty}\n");
		exit(0);
	}
	key_t keyMutex = ftok(MUTEXKEYFILE,MUTEXKEYID);
	if((mutex = semget(keyMutex,1,0666|IPC_CREAT)) < 0){
		printf("\nCANT CREATE SEMAPHORE {mutex}\n");
		exit(0);
	}
	key_t keyShared = ftok(SHAREDKEYFILE,SHAREDKEYID);
	if((shmid = shmget(keyShared,(N+2)*sizeof(int),0777|IPC_CREAT)) < 0){
		printf("\nCANT ALLOCATE SHARED MEMORY\n");
		exit(0);
	}
	/***********************************************************/

	/*********** init shared mem and semaphore *****************/
	int *a = (int *)shmat(shmid,0,0);
	a[IN] = 0;
	a[OUT] = 0;
	shmdt(a);
	if(semctl(full,0,SETVAL,(ushort)0) < 0){
		printf("\nCANT INITIALIZE FULL\n");
		exit(0);
	}
	if(semctl(empty,0,SETVAL,(ushort)N) < 0){
		printf("\nCANT INITIALIZE FULL\n");
		exit(0);
	}
	if(semctl(mutex,0,SETVAL,(ushort)1) < 0){
		printf("\nCANT INITIALIZE FULL\n");
		exit(0);
	}
	/************************************************************/

	/****************** forking *********************************/
	int prodpid[PRODCOUNT],conpid[CONCOUNT];
	for(int i=0;i<PRODCOUNT;i++){
		prodpid[i] = createProducer(i+1,shmid,full,empty,mutex);
	}
	for(int i=0;i<CONCOUNT;i++){
		conpid[i] = createConsumer(i+1,shmid,full,empty,mutex);
	}
	int status;
	for(int i=0;i<PRODCOUNT;i++){
		waitpid(prodpid[i],&status,0);
	}
	for(int i=0;i<CONCOUNT;i++){
		waitpid(conpid[i],&status,0);
	}
	/************************************************************/
	printf("\n");
	system("./clear.sh");
	return 0;
}
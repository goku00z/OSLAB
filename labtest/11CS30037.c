#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <math.h>


/*********************** Constants to control *******************/
#define BMAX 5
#define MAXPRODSLEEP 5
#define MAXCONSUMERSLEEP 5
#define ITEMSTART 0
// NOTE ACTUAL SLEEP IS rand()%maxgiven + 1
/****************************************************************/

/************** ftok files and ids ******************************/
#define FULLKEYFILE "full.txt" 
#define FULLKEYID 1
#define MUTEXKEYFILE "mutex.txt"
#define MUTEXKEYID 2
#define MUTEXKEYFILE2 "mutex2.txt"
#define MUTEXKEYID2 5
#define EMPTYKEYFILE "empty.txt"
#define EMPTYKEYID 3
#define SHAREDKEYFILE "keyfile.txt"
#define SHAREDKEYID 4
#define COUNTFILE "countfile.txt"
#define COUNTKEYID 5
/***************************************************************/

/***************************************************************/
#define P(semaphore,semP) semop(semaphore,&semP,1)
#define V(semaphore,semV) semop(semaphore,&semV,1)

#define INCREASED_SIZE (BMAX + 1)
#define IN INCREASED_SIZE
#define OUT (INCREASED_SIZE + 1)
#define ACTUALSIZE (INCREASED_SIZE - 1)

#define PRODUCERSLEEP (rand()%MAXPRODSLEEP + 1)
#define CONSUMERSLEEP (rand()%MAXCONSUMERSLEEP + 1)


// #define PRODUCERSLEEP (MAXPRODSLEEP)
// #define CONSUMERSLEEP (MAXCONSUMERSLEEP)


int itemCount = ITEMSTART;
/****************************************************************/

void initSemaphoreOperation(struct sembuf *x,int sem_num,int sem_flg,int sem_op){
	x->sem_num = sem_num;
	x->sem_flg = sem_flg;
	x->sem_op = sem_op;
	return;
}

void resetFile(const char *filename){
	FILE *fp = fopen(filename,"w");
	fclose(fp);
	return;
}

void createFile(){
	resetFile(FULLKEYFILE);
	resetFile(MUTEXKEYFILE);
	resetFile(EMPTYKEYFILE);
	resetFile(SHAREDKEYFILE);
	resetFile(MUTEXKEYFILE2);
	resetFile(COUNTFILE);
	return;
}

void getSemaphores(int *full,int *empty,int *mutex,int *shmid,int *mutex2,int *countsharedid){
	int full_,empty_,mutex_,shmid_,mutex2_,countsharedid_;
	key_t keyFull = ftok(FULLKEYFILE,FULLKEYID);
	if((full_ = semget(keyFull,1,0666|IPC_CREAT)) < 0){
		printf("\nCANT CREATE SEMAPHORE {full}\n");
		exit(0);
	}
	key_t keyEmpty = ftok(EMPTYKEYFILE,EMPTYKEYID);
	if((empty_ = semget(keyEmpty,1,0666|IPC_CREAT)) < 0){
		printf("\nCANT CREATE SEMAPHORE {empty}\n");
		exit(0);
	}
	key_t keyMutex = ftok(MUTEXKEYFILE,MUTEXKEYID);
	if((mutex_ = semget(keyMutex,1,0666|IPC_CREAT)) < 0){
		printf("\nCANT CREATE SEMAPHORE {mutex}\n");
		exit(0);
	}
	key_t keyMutex2 = ftok(MUTEXKEYFILE2,MUTEXKEYID2);
	if((mutex2_ = semget(keyMutex2,1,0666|IPC_CREAT)) < 0){
		printf("\nCANT CREATE SEMAPHORE {mutex2}\n");
		exit(0);
	}
	key_t keyShared = ftok(SHAREDKEYFILE,SHAREDKEYID);
	if((shmid_ = shmget(keyShared,(INCREASED_SIZE+2)*sizeof(int),0777|IPC_CREAT)) < 0){
		printf("\nCANT ALLOCATE SHARED MEMORY\n");
		exit(0);
	}
	key_t keyCount = ftok(COUNTFILE,COUNTKEYID);
	if((countsharedid_ = shmget(keyCount,(1)*sizeof(int),0777|IPC_CREAT)) < 0){
		printf("\nCANT ALLOCATE SHARED MEMORY\n");
		exit(0);
	}
	*full = full_;
	*empty = empty_;
	*mutex = mutex_;
	*shmid = shmid_;
	*mutex2 = mutex2_;
	*countsharedid = countsharedid_;
	return;
}

void initSharedBufferAndSemaphores(int full,int empty,int mutex,int shmid,int mutex2,int countsharedid){
	int *a = (int *)shmat(shmid,0,0);
	for(int i=0;i<INCREASED_SIZE;i++){
		a[i] = -1;
	}
	a[IN] = 0;
	a[OUT] = 0;
	shmdt(a);
	int *count = (int *)shmat(countsharedid,0,0);
	count[0] = 0;
	shmdt(count);
	if(semctl(full,0,SETVAL,(ushort)0) < 0){
		printf("\nCANT INITIALIZE FULL\n");
		exit(0);
	}
	if(semctl(empty,0,SETVAL,(ushort)(INCREASED_SIZE-1)) < 0){
		printf("\nCANT INITIALIZE EMPTY\n");
		exit(0);
	}
	if(semctl(mutex,0,SETVAL,(ushort)1) < 0){
		printf("\nCANT INITIALIZE MUTEX\n");
		exit(0);
	}
	if(semctl(mutex2,0,SETVAL,(ushort)1) < 0){
		printf("\nCANT INITIALIZE MUTEX2\n");
		exit(0);
	}
	return;
}
int getItem(){
	int n = itemCount;
	itemCount++;
	return n; 
}
void produceItem(int shmid,int item,int countsharedid){
	// must be done in mutex
	int *a = (int *)shmat(shmid,0,0);
	int in = a[IN];
	int out = a[OUT];
	a[in] = item;
	printf("\nProducer inserts item :%d\n",a[in]);
	in = (in + 1)%INCREASED_SIZE;
	a[IN] = in;
	shmdt(a);

	int *count = (int *)shmat(countsharedid,0,0);
	count[0] = count[0] + 1;
	shmdt(count);

	return;
}
int consumeItem(int shmid,int countsharedid){
	// must be done in mutex
	int *a = (int *)shmat(shmid,0,0);
	int in = a[IN];
	int out = a[OUT];
	int item = a[out];
	a[out] = -1;
	out = (out + 1)%INCREASED_SIZE;
	a[OUT] = out;
	shmdt(a);

	int *count = (int *)shmat(countsharedid,0,0);
	count[0] = count[0] - 1;
	shmdt(count);

	return item;
}

int isBufferEmpty(int shmid,int countsharedid){
	// int *a = (int *)shmat(shmid,0,0);
	// int in = a[IN];
	// int out = a[OUT];
	// shmdt(a);
	// if(in == out){
	// 	return 1;
	// }
	int *count = (int *)shmat(countsharedid,0,0);
	int xcount = count[0];
	shmdt(count);
	if(xcount == 0){
		return 1;
	}else{
		return 0;
	}
}
int isBufferFull(int shmid,int countsharedid){
	// int *a = (int *)shmat(shmid,0,0);
	// int in = a[IN];
	// int out = a[OUT];
	// shmdt(a);
	// int count;
	// count = (int)abs(in - out);
	// // if(in>out){
	// // 	count = in - out;
	// // }else{
	// // 	if(in == count){
	// // 		count = 0;
	// // 	}else{
	// // 		count = in + (ACTUALSIZE - out);
	// // 	}
	// // }
	// if(count == ACTUALSIZE){
	// 	return 1;
	// }
	// return 0;
	int *count = (int *)shmat(countsharedid,0,0);
	int xcount = count[0];
	shmdt(count);
	if(xcount == ACTUALSIZE){
		return 1;
	}else{
		return 0;
	}
}

void producer(int shmid,int full,int empty,int mutex,int mutex2,int countsharedid){
	int item;
	struct sembuf semV,semP;
	initSemaphoreOperation(&semV,0,0,1);
	initSemaphoreOperation(&semP,0,0,-1);
	while(1){
		sleep(PRODUCERSLEEP);
		item = getItem();
		P(mutex2,semP);
		if(isBufferFull(shmid,countsharedid) == 1){
			printf("\nProducer inserts item : Buffer FULL!\nProducer waits\n");
		}
		V(mutex2,semV);
		P(empty,semP);
		P(mutex,semP);
		produceItem(shmid,item,countsharedid);
		V(mutex,semV);
		V(full,semV);
	}
	return;
}

void consumer(int shmid,int full,int empty,int mutex,int mutex2,int countsharedid){
	struct sembuf semV,semP;
	initSemaphoreOperation(&semV,0,0,1);
	initSemaphoreOperation(&semP,0,0,-1);
	while(1){
		sleep(CONSUMERSLEEP);
		P(mutex2,semP);
		if(isBufferEmpty(shmid,countsharedid) == 1){
			printf("\nConsumer consumes item : Buffer EMPTY!\nConsumer waits\n");
		}
		V(mutex2,semV);
		P(full,semP);
		P(mutex,semP);
		int item = consumeItem(shmid,countsharedid);
		V(mutex,semV);
		V(empty,semV);
		printf("\nConsumer consumes item : %d\n",item);
	}
	return;
}
void runConsumerAndCreateProducer(int full,int empty,int mutex,int shmid,int mutex2,int countsharedid){
	int prodpid = fork();
	if(prodpid == 0){
		// child process process
		// need to run child code
		srand(time(NULL));
		producer(shmid,full,empty,mutex,mutex2,countsharedid);
		exit(0);
	}else{
		// need to run parent code
		sleep(1);
		srand(time(NULL));
		consumer(shmid,full,empty,mutex,mutex2,countsharedid);
		return;
	}
}

int main(){
	createFile(); // creating the files for using ftok
	system("reset");
	int full,empty,mutex,shmid,mutex2,countsharedid;
	getSemaphores(&full,&empty,&mutex,&shmid,&mutex2,&countsharedid); // all semaphores and shared memories creatred
	initSharedBufferAndSemaphores(full,empty,mutex,shmid,mutex2,countsharedid);
	runConsumerAndCreateProducer(full,empty,mutex,shmid,mutex2,countsharedid);
	return 0;
}
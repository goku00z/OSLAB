#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define TRAINCOUNT 100
#define TICKETPERTRAIN 500
#define MAX 2
#define WORKERCOUNT 5
#define MAXBOOKATATIME 10

#define USERSLEEP 20
#define MAXDELAY 3
#define MAXIDLE 3
#define SLEEPDELAY (rand()%MAXDELAY+1)
#define SLEEPIDLE (rand()%MAXIDLE+1)

#define EMPTY 0
#define BOOKED 1

/*************************************************/
struct sharedTable{
	int trainNumber;
	int type;
	long ID;
};
typedef struct sharedTable sharedTable;
struct history{
	int trainNumber;
	int seatNumber;
};
/*************************************************/

/*************************************************/
int killall;
pthread_mutex_t kill_mutex,wait_mutex;
pthread_cond_t maxcond;
int loadCount;
int reservationStatus[TRAINCOUNT][TICKETPERTRAIN];
sharedTable Table[MAX];
/*************************************************/
void initSharedVariables(){
	killall = 0;
	loadCount = 0;
	for(int i=0;i<TRAINCOUNT;i++){
		for(int j=0;j<TICKETPERTRAIN;j++){
			reservationStatus[i][j] = EMPTY;
		}
	}
	for(int i=0;i<MAX;i++){
		Table[i].trainNumber = -1;
		Table[i].type = -1;
		Table[i].ID = 0;
	}
	return;
}
void createMutex(pthread_mutex_t *mutex){
	pthread_mutex_init(mutex, NULL);
	pthread_mutex_trylock(mutex);
	pthread_mutex_unlock(mutex);
	return;
}
void initMutexes(){
	createMutex(&kill_mutex);
	createMutex(&wait_mutex);
	pthread_cond_init(&maxcond, NULL);
	return;
}
void freeMutexes(){
	pthread_mutex_destroy(&kill_mutex);
	pthread_mutex_destroy(&wait_mutex);
	pthread_cond_destroy(&maxcond);
	return;	
}
int isExit(){
	int killvalue;
	pthread_mutex_lock(&kill_mutex);
	killvalue = killall;
	pthread_mutex_unlock(&kill_mutex);
	return killvalue;
}

void setExit(){
	pthread_mutex_lock(&kill_mutex);
	killall = 1;
	pthread_mutex_unlock(&kill_mutex);
	return;
}
/***************** Train Functions ***************************/
void bookTicket(int trainNumber,int ticketCount){
	// assumes calling function is thread safe
	if(trainNumber<0||trainNumber>=TRAINCOUNT){
		printf("\n**{INVALID TRAIN NUMBER = %d}\n",trainNumber);
	}
	int booked[ticketCount];
	int bCount = 0;
	for(int i =0;i<TICKETPERTRAIN;i++){
		if(reservationStatus[trainNumber][i] == EMPTY){
			booked[bCount] = i;
			bCount++;
			if(bCount == ticketCount){
				break;
			}
		}
	}
	if(bCount != ticketCount){
		printf("\n{ONLY %d of asked %d tickets available in train %d}\n",bCount,ticketCount,trainNumber);
		return;
	}
	printf("\n{Congrats ! %d of %d tickets found in train %d}\n",bCount,ticketCount,trainNumber);
	for(int i=0;i<bCount;i++){
		reservationStatus[trainNumber][booked[i]] = BOOKED;
	}
	return;
}
void getFreeStatus(int trainNumber){
	// assumes calling function is thread safe
	if(trainNumber<0||trainNumber>=TRAINCOUNT){
		printf("\n**{INVALID TRAIN NUMBER = %d}\n",trainNumber);
	}
	int fCount = 0;
	for(int i=0;i<TICKETPERTRAIN;i++){
		if(reservationStatus[trainNumber][i] == EMPTY){
			fCount++;
		}
	}
	printf("\n{%d out of %d tickets available in train %d}\n",fCount,TICKETPERTRAIN,trainNumber);
	return;
}
void cancelTicket(int trainNumber,int seatNumber){
	// assumes calling function is thread safe
	if(trainNumber<0||trainNumber>=TRAINCOUNT){
		printf("\n**{INVALID TRAIN NUMBER = %d}\n",trainNumber);
	}
	if(seatNumber<0||seatNumber>=TICKETPERTRAIN){
		printf("\n**{INVALID SEAT NUMBER = %d}\n",seatNumber);
	}
	if(reservationStatus[trainNumber][seatNumber] == EMPTY){
		printf("\n{SORRY seat %d in train %d is already empty}\n",seatNumber,trainNumber);
	}else{
		reservationStatus[trainNumber][seatNumber] = EMPTY;
		printf("\n{seat %d in train %d is made empty and available for booking}\n",seatNumber,trainNumber);
	}
	return;
}
void getQuery(int *type,int *trainNumber,int *ticketCount,int *seatNumber){
	*trainNumber = rand()%TRAINCOUNT;
	*seatNumber = -1;
	switch(rand()%3+1){
		case 1:
			// type 1
			*type = 1;
			*ticketCount = -1;
			break;
		case 2:
			// type 2
			*type = 2;
			*ticketCount = rand()%MAXBOOKATATIME + 1;
		break;
		case 3:
			// type 3
			*type = 3;
			*ticketCount = -1;
			*seatNumber = rand()%TICKETPERTRAIN;
		break;
	}
	return;
}
void handleType1(int trainNumber){

}
void handleType2(int trainNumber,int ticketCount){

}
void handleType3(int trainNumber){

}
void makeQuery(int type,int trainNumber,int ticketCount,int seatNumber){

}
/************************************************************/
void waitForFreeSlot(){
	pthread_mutex_lock(&wait_mutex);
	loadCount++;
	if(loadCount>MAX){
		pthread_cond_wait(&maxcond, &wait_mutex);
	}
	pthread_mutex_unlock(&wait_mutex);
	return;	
}
void freeOwnSlot(){
	pthread_mutex_lock(&wait_mutex);
	loadCount--;
	pthread_cond_signal(&maxcond);
	pthread_mutex_unlock(&wait_mutex);
	return;
}
void *worker(void *targ){
	long ID = (long)(targ);
	printf("\nTHREAD %ld STARTS\n",ID);
	int type,trainNumber,ticketCount,seatNumber;
	while(1){
		// the main worker loop
		// check whether the main thread is requesting others to exit
		if(isExit() == 1){
			break;
		}
		// check for whether there are free slots or not
		waitForFreeSlot();
		printf("\n** THREAD %ld got access\n",ID);
		// get a query
		getQuery(&type,&trainNumber,&ticketCount,&seatNumber);
		// sleep for some random time to simulate bank or other delay
		sleep(SLEEPDELAY);
		// make the query
		makeQuery(type,trainNumber,ticketCount,seatNumber);
		// exit and make some other query to work
		printf("\n** THREAD %ld releasing access\n",ID);
		freeOwnSlot();
		// sleep for some time to be idle
		sleep(SLEEPIDLE);
	}
	printf("\nTHREAD %ld ENDS\n",ID);
	pthread_exit(NULL);
}


void createWorkers(pthread_t *threads,int wCount){
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	for(int i = 0;i<wCount;i++){
		long ID = i+1;
		if(pthread_create(threads + i, &attr, worker, (void *)ID)){
			fprintf(stderr, "Master thread: Unable to create thread\n");
			pthread_attr_destroy(&attr);
			exit(1);
    	}
    	printf("\nWORKER %ld Created\n",ID);
	}
	pthread_attr_destroy(&attr);
	return;
}

void waitForWorkers(pthread_t *threads,int wCount){
	/* Wait for all threads to complete */
	for(int i=0; i<wCount; i++){
		pthread_join(threads[i], NULL);
	}
	printf("Main(): Waited on %d  threads. Done.\n", wCount);
	return;
}

void printResult(){

}


int main(){
	initSharedVariables();
	pthread_t threads[WORKERCOUNT];
	createWorkers(threads,WORKERCOUNT);
	sleep(USERSLEEP);
	setExit();
	waitForWorkers(threads,WORKERCOUNT);
	printResult();
	pthread_exit(NULL);
}
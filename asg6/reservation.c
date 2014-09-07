#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define TRAINCOUNT 5
#define TICKETPERTRAIN 30
#define MAX 4
#define WORKERCOUNT 10
#define MAXBOOKATATIME 10

#define USERSLEEP 30
#define MAXDELAY 3
#define MAXIDLE 3
#define SLEEPDELAY (rand()%MAXDELAY+1)
#define SLEEPIDLE (rand()%MAXIDLE+1)


#define BOOKED (TRAINCOUNT+1)

/*************************************************/
struct sharedTable{
	int trainNumber;
	int type;
	long ID;
};
typedef struct sharedTable sharedTable;
/*************************************************/

/*************************************************/
int killall;
pthread_mutex_t kill_mutex,wait_mutex,table_mutex;
pthread_cond_t maxcond;
int loadCount;
int reservationStatus[TRAINCOUNT];
sharedTable Table[MAX];
/*************************************************/
void initSharedVariables(){
	killall = 0;
	loadCount = 0;
	for(int i=0;i<TRAINCOUNT;i++){
		reservationStatus[i] = 0;
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
	createMutex(&table_mutex);
	pthread_cond_init(&maxcond, NULL);
	return;
}
void freeMutexes(){
	pthread_mutex_destroy(&kill_mutex);
	pthread_mutex_destroy(&wait_mutex);
	pthread_mutex_destroy(&table_mutex);
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
void bookTicket(int trainNumber,int ticketCount,int *bookedHistory){
	// assumes calling function is thread safe
	if(trainNumber<0||trainNumber>=TRAINCOUNT){
		printf("\n**{INVALID TRAIN NUMBER = %d}\n",trainNumber);
	}
	// int booked[ticketCount];
	int remaining = TICKETPERTRAIN - reservationStatus[trainNumber];
	if(ticketCount > remaining){
		printf("\n{ONLY %d of asked %d tickets available in train %d}\n",remaining,ticketCount,trainNumber);
		return;
	}
	// for(int i =0;i<TICKETPERTRAIN;i++){
	// 	if(reservationStatus[trainNumber][i] == EMPTY){
	// 		booked[bCount] = i;
	// 		bCount++;
	// 		if(bCount == ticketCount){
	// 			break;
	// 		}
	// 	}
	// }

	printf("\n{Congrats ! %d of %d tickets found and booked in train %d}\n",ticketCount,ticketCount,trainNumber);
	bookedHistory[trainNumber] = bookedHistory[trainNumber] + ticketCount;
	reservationStatus[trainNumber] = reservationStatus[trainNumber] + ticketCount;	
	// for(int i=0;i<bCount;i++){
	// 	reservationStatus[trainNumber][booked[i]] = BOOKED;
	// }
	return;
}
void getFreeStatus(int trainNumber){
	// assumes calling function is thread safe
	if(trainNumber<0||trainNumber>=TRAINCOUNT){
		printf("\n**{INVALID TRAIN NUMBER = %d}\n",trainNumber);
	}
	int remaining = TICKETPERTRAIN - reservationStatus[trainNumber];
	// for(int i=0;i<TICKETPERTRAIN;i++){
	// 	if(reservationStatus[trainNumber][i] == EMPTY){
	// 		fCount++;
	// 	}
	// }
	printf("\n{%d out of %d tickets available in train %d}\n",remaining,TICKETPERTRAIN,trainNumber);
	return;
}
void cancelTicket(int trainNumber,int *bookedHistory){
	// assumes calling function is thread safe
	if(trainNumber<0||trainNumber>=TRAINCOUNT){
		printf("\n**{INVALID TRAIN NUMBER = %d}\n",trainNumber);
	}
	// if(seatNumber<0||seatNumber>=TICKETPERTRAIN){
	// 	printf("\n**{INVALID SEAT NUMBER = %d}\n",seatNumber);
	// }
	if(reservationStatus[trainNumber] == 0){
		printf("\n{SORRY all seats of train %d are empty}\n",trainNumber);
		return;
	}

	reservationStatus[trainNumber]--;
	bookedHistory[trainNumber]++;
	printf("\n{Ticket cancelled in train %d}\n",trainNumber);
	
	return;
}
void getQuery(int *type,int *trainNumber,int *ticketCount,int *bookedHistory){
	*trainNumber = rand()%TRAINCOUNT;
	// *seatNumber = -1;
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
			if(bookedHistory[*trainNumber]<=0){
				*type = 1;// making it a read query as there is no ticket to cancel
				break;
			}
			*ticketCount = -1;
			// *seatNumber = rand()%TICKETPERTRAIN;
		break;
	}
	return;
}

int isWritting(int trainNumber){
	for(int i=0;i<MAX;i++){
		if(Table[i].trainNumber==trainNumber&&(Table[i].type == 2 || Table[i].type == 3)){
			return 1;
		}
	}
	return 0;
}
int bookTable(int type,int trainNumber,long ID){
	int id = -1;
	for(int i =0;i<MAX;i++){
		if(Table[i].trainNumber == -1){
			id = i;
			break;
		}
	}
	if(id== -1 ||id<0||id >= MAX){
		printf("\n******* FATAL INCONSISTENSY CANT BOOK TABLE*********\n");
	}
	if(isWritting(trainNumber) == 1){
		return -1;
	}
	Table[id].trainNumber = trainNumber;
	Table[id].type = type;
	Table[id].ID = ID;
	return id;
}

void clearTable(int id){
	if(id < 0||id>=MAX){
		printf("\n******** FATAL ERROR CANNOT DELETE %d in table",id);
		exit(0);
	}
	Table[id].trainNumber = -1;
	return;
}

void makeQuery(int type,int trainNumber,int ticketCount,int *bookedHistory,long ID){
	// handles read query ... wait only if any write operation is done on the current train
	// atleast one entry in the table is empty search that and book it
	pthread_mutex_lock(&table_mutex);
	int tableId = bookTable(type,trainNumber,ID);
	pthread_mutex_unlock(&table_mutex);

	if(tableId == -1){
		// Not able to lock table .... going to do next query
		return;
	}

	// do query ... it will be thread safe
	switch(type){
		case 1:
			getFreeStatus(trainNumber);
			break;
		case 2:
			bookTicket(trainNumber,ticketCount,bookedHistory);
			break;
		case 3:
			cancelTicket(trainNumber,bookedHistory);
			break;
		default:
			printf("\n#### ERROR in thread %ld type = %d",ID,type);
			break;
	}
	// query done so clear the table
	pthread_mutex_lock(&table_mutex);
	clearTable(tableId);
	pthread_mutex_unlock(&table_mutex);
	return;
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
	int type,trainNumber,ticketCount;
	int bookedHistory[TRAINCOUNT];
	for(int i=0;i<TRAINCOUNT;i++){
		bookedHistory[i] = 0;
	}
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
		getQuery(&type,&trainNumber,&ticketCount,bookedHistory);
		// sleep for some random time to simulate bank or other delay
		sleep(SLEEPDELAY);
		// make the query
		makeQuery(type,trainNumber,ticketCount,bookedHistory,ID);
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
	printf("\n*********** RESERVATION STATUS ****************\n");
	printf("\nTRAIN NO\tRESERVED TICKETS");
	for(int i=0;i<TRAINCOUNT;i++){
		printf("\n%d\t%d",i,reservationStatus[i]);
	}
	return;
}


int main(){
	system("reset");
	initSharedVariables();
	pthread_t threads[WORKERCOUNT];
	createWorkers(threads,WORKERCOUNT);
	sleep(USERSLEEP);
	setExit();
	waitForWorkers(threads,WORKERCOUNT);
	freeMutexes();
	printResult();
	pthread_exit(NULL);
}
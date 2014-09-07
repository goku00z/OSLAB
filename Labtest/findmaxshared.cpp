#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

#define KEYFILE "keyfile.txt"
#define KEYID 1
#define N 1000
#define MAXDEPTH 4

// int shmctl(int shmid, int cmd, struct shmid_ds *buf);
// The process must have an effective shmid of owner, creator or superuser to perform this command. The cmd argument is one of following control commands:
// SHM_LOCK
//     -- Lock the specified shared memory segment in memory. The process must have the effective ID of superuser to perform this command. 
// SHM_UNLOCK
//     -- Unlock the shared memory segment. The process must have the effective ID of superuser to perform this command. 
// IPC_STAT
//     -- Return the status information contained in the control structure and place it in the buffer pointed to by buf. The process must have read permission on the segment to perform this command. 
// IPC_SET
//     -- Set the effective user and group identification and access permissions. The process must have an effective ID of owner, creator or superuser to perform this command. 
// IPC_RMID
//     -- Remove the shared memory segment. 
// The buf is a sructure of type struct shmid_ds which is defined in <sys/shm.h> 

// shmat() and shmdt() are used to attach and detach shared memory segments. They are prototypes as follows:
// void *shmat(int shmid, const void *shmaddr, int shmflg);
// int shmdt(const void *shmaddr);
// shmat() returns a pointer, shmaddr, to the head of the shared segment associated with a valid shmid. shmdt() detaches the shared memory segment located at the address indicated by shmaddr 
int getMax(int shmid,int i,int j,int depth);

int getMaxNormal(int *a,int i,int j){
	// printf("\nIN Normal get Max !");
	int max = a[i];
	for(int k = i+1;k<=j;k++){
		if(max < a[k]){
			max = a[k];
		}
	}
	return max;
}

int callGetmax(int shmid,int i,int j,int depth){
	int pid = fork();
	if(pid == 0){
		int max = getMax(shmid,i,j,depth);
		// printf("MAX = %d\n",max);
		exit(max);
	}else{
		return pid;
	}
}

int getMax(int shmid,int i,int j,int depth){
	// printf("INGETMAX , i = %d j = %d depth = %d\n",i,j,depth);
	// Base cases **************************
	if(depth == MAXDEPTH){
		// No more forking !!
		int *a = (int *)shmat(shmid,0,0);
		int max = getMaxNormal(a,i,j);
		// printf("Returning NORMAL %d\n",max);
		shmdt(a);
		return max;
	}
	if(i == j){
		int *a = (int *)shmat(shmid,0,0);
		int max = a[i];
		// printf("Returning IN GETMAX %d\n",max);
		shmdt(a);
		return max;	
	}
	// ********** **************************
	int left,right,middle;
	left = i;
	right = j;
	middle = (i+j)/2;

	int pidLeft = callGetmax(shmid,left,middle,depth+1);
	int pidRight = callGetmax(shmid,middle+1,right,depth+1);

	int maxLeft = 0,maxRight = 0;
	waitpid(pidLeft,&maxLeft,0);
	waitpid(pidRight,&maxRight,0);

	// printf("MAXLEFT = %d MAXRIGHT = %d\n",maxLeft,maxRight);

	maxLeft = WEXITSTATUS(maxLeft);
	// printf("LEFT : WIFEXITED: %d, WEXITSTATUS: %d, WIFSIGNALED: %d, WTERMSIG: %d, WCOREDUMP: %d, WIFSTOPPED: %d, WSTOPSIG: %d, WIFCONTINUED: %d\n",
 //    WIFEXITED(maxLeft), WEXITSTATUS(maxLeft), WIFSIGNALED(maxLeft), WTERMSIG(maxLeft), 0/*WCOREDUMP(maxLeft)*/, WIFSTOPPED(maxLeft),
 //    WSTOPSIG(maxLeft), WIFCONTINUED(maxLeft));
	maxRight = WEXITSTATUS(maxRight);
	// printf("RIGHT : WIFEXITED: %d, WEXITSTATUS: %d, WIFSIGNALED: %d, WTERMSIG: %d, WCOREDUMP: %d, WIFSTOPPED: %d, WSTOPSIG: %d, WIFCONTINUED: %d\n",
 //    WIFEXITED(maxRight), WEXITSTATUS(maxRight), WIFSIGNALED(maxRight), WTERMSIG(maxRight), 0/*WCOREDUMP(maxRight)*/, WIFSTOPPED(maxRight),
 //    WSTOPSIG(maxRight), WIFCONTINUED(maxRight));

	// printf("MAXLEFT = %d MAXRIGHT = %d\n",maxLeft,maxRight);

	if(maxLeft > maxRight){
		return maxLeft;
	}
	return maxRight;

}

int main(){
	key_t key = ftok(KEYFILE,KEYID);
	int shmid;
	if((shmid = shmget(key,N*sizeof(int),0777|IPC_CREAT)) < 0){
		printf("Can not get Shared memory !!\n");
		exit(0);
	}
	int *A = (int *)shmat(shmid,0,0);
	for(int i=0;i<N;i++){
		A[i] = rand()%100;
	}
	for(int i = 0;i<N;i++){
		// printf("A[%d] = %d\n",i,A[i]);
	}
	printf("MAX NORMAL = %d\n",getMaxNormal(A,0,N-1));
	shmdt(A);
	int max = getMax(shmid,0,N-1,0);
	printf("MAX FORKING = %d\n",max);
	shmctl(shmid, IPC_RMID, 0);
	printf("\n");
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>

#define KEYFILE "keyfile.txt"
#define KEYID 1
#define N 20

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

struct point{
	int x;
	int y;
};
typedef struct point point;

int main(){
	key_t key = ftok(KEYFILE,KEYID);
	int shmid;
	if((shmid = shmget(key,N*sizeof(point),0777|IPC_CREAT)) < 0){
		printf("\nCan not get Shared memory !!\n");
		exit(0);
	}
	point *A = (point *)shmat(shmid,0,0);
	// for(int i=0;i<N;i++){
	// 	A[i].x = (i+1);
	// 	A[i].y = (i+1)*(i+1);
	// }
	for(int i = 0;i<N;i++){
		printf("\nA[%d] = (%d,%d)",i,A[i].x,A[i].y);
	}
	shmdt(A);
	shmctl(shmid, IPC_RMID, 0);
}

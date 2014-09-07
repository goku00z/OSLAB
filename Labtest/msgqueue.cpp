#ifndef __MSG__QUEUE__
#define __MSG__QUEUE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>

// set it for exexution
#define MSGSIZE 1000

// int msgsnd(int msqid, const void *msgp, size_t msgsz,
//           int msgflg);

// int msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp,
//           int msgflg);

// The msqid argument must be the ID of an existing message queue. The msgp argument is a pointer to a structure that contains the type of the message and its text. The structure below is an example of what this user-defined buffer might look like:

//  struct mymsg {
//       long      mtype;    /* message type */
//       char mtext[MSGSZ]; /* message text of length MSGSZ */
// }

// char * gets ( char * str );
// Get string from stdin
// Reads characters from the standard input (stdin) and stores them as a C string into str until a newline character or the end-of-file is reached.
// The newline character, if found, is not copied into str.
// A terminating null character is automatically appended after the characters copied to str.


// char * fgets ( char * str, int num, FILE * stream );
// Get string from stream
// Reads characters from stream and stores them as a C string into str until (num-1) characters have been read or either a newline or the end-of-file is reached, whichever happens first.
// A newline character makes fgets stop reading, but it is considered a valid character by the function and included in the string copied to str.
// A terminating null character is automatically appended after the characters copied to str.

typedef struct msg_buf{
	long mtype;
	char mtext[MSGSIZE];
}message_buf;

void sendMessage(const char *text,int type,const char *keyfile,int keyid){
	int msqid;
    key_t key;
    int textlen = strlen(text) + 1;
    message_buf sbuf;

    if(textlen>MSGSIZE){
    	printf("\nMSGSIZE = %d textSize with \\0 = %d",MSGSIZE,textlen);
    }

    key = ftok(keyfile,keyid);

    if((msqid = msgget(key,IPC_CREAT | 0666)) < 0){
    	printf("\nCan not get MSGQUE !!\n");
    	exit(0);
    }
    sbuf.mtype = type;
    strcpy(sbuf.mtext,text);

    if(msgsnd(msqid,&sbuf,textlen,0) < 0){
    	printf("\nERROR in sending message !!\n");
    	exit(0);
    }
    return;
}

// int msgsnd(int msqid, const void *msgp, size_t msgsz,
//           int msgflg);

// int msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp,
//           int msgflg);

// The msqid argument must be the ID of an existing message queue. The msgp argument is a pointer to a structure that contains the type of the message and its text. The structure below is an example of what this user-defined buffer might look like:

//  struct mymsg {
//       long      mtype;    /* message type */
//       char mtext[MSGSZ]; /* message text of length MSGSZ */
// }
 // If (msgflg & IPC_NOWAIT) is non-zero, the message will not be sent and the calling process will return immediately.
// If (msgflg & IPC_NOWAIT) is 0, the calling process will suspend execution until one of the following occurs: 

void receiveMessage(char *msg,int *id,const char *keyfile,int keyid,int type = 0){
	int msqid;
	key_t key;
	message_buf rbuf;
	struct msqid_ds qstat;

	key = ftok(keyfile,keyid);

	if((msqid = msgget(key,0666|IPC_CREAT)) < 0){
		printf("\nCAN NOT GET MSQID(%d)\n",msqid);
		exit(0);
	}
	
	if(msgrcv(msqid,&rbuf,MSGSIZE,type,0) < 0){
		printf("\nERROR TO RECEIVE MESSAGE\n");
		exit(0);
	}
	
	// if(msgctl(msqid,IPC_STAT,&qstat) < 0){
	// 	printf("\nERROR Can not get message queue status\n");
	// 	exit(0);
	// }
	// printf("Message received at time %s",(ctime(&qstat.msg_rtime)));
	strcpy(msg,rbuf.mtext);
	*id = rbuf.mtype;

	return;
}



#endif
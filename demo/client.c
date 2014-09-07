#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MSGSZ   256


/*
 * Declare the message structure.
 */

typedef struct msgbuf {
        long    mtype;
        char    mtext[MSGSZ];
}message_buf;

int sendMessage(){
    
    //printf("*********In Send Message*************\n");
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    message_buf sbuf;
    size_t buf_length;

    /*
     * Get the message queue id for the
     * "name" 1234, which was created by
     * the server.
     */
    key = 1200;
    //key = ftok("dup.c", 'z');
    //(void) fprintf(stderr, "\nmsgget: Calling msgget(%#lx,\%#o)\n",key, msgflg);

    if ((msqid = msgget(key, msgflg )) < 0) {
        perror("msgget");
        return 1;
    }
    /*
    else 
        (void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);
    */

    /*
     * We'll send message type 1
     */
     
    sbuf.mtype = getpid();
    
    //(void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);
    
    printf("\nInsert message to send to the server: ");
    scanf("%[^\n]",sbuf.mtext);
    
    //(void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);
    
    buf_length = strlen(sbuf.mtext) + 1 ;
    
    

    /*
     * Send a message.
     */
    if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
        //printf ("%d, %d, %s, %d\n", msqid, sbuf.mtype, sbuf.mtext, buf_length);
        perror("msgsnd");
        return 1;
    }

    else 
        printf("Message: \"%s\" Sent\n", sbuf.mtext);
    
    if(!strcmp(sbuf.mtext,"KILL")){
        return 2;
    }
    //cprintf("*********Exit Send Message*************\n");  
    return 0;
}


int receiveMessage(){
    //printf("*********In Receive Message*************\n");
    int msqid;
    key_t key;
    message_buf  rbuf;

    /*
     * Get the message queue id for the
     * "name" 1234, which was created by
     * the server.
     */
    //printf("ERROR");
    //key = ftok("dup.c", 'b');
    key = 1000;
    //printf("ERROR");
    if ((msqid = msgget(key, 0666)) < 0) {
        //printf("ERROR");
        perror("msgget");
        return 1;
    }

    /*
     * Receive an answer of message type 1.
     */
     
    if (msgrcv(msqid, &rbuf, MSGSZ, getpid(), 0) < 0) {
        perror("msgrcv");
        return 1;
    }

    /*
     * Print the answer.
     */
    printf("\nMessage received at client end: %s\n", rbuf.mtext);
    //printf("*********Exit Receive Message*************\n");
    return 0;
}

int main()
{
    while(1)
    {    
        if(sendMessage() == 2){
            return 0;
        }
        printf("press any key to receive message: ");
        getchar();
        getchar();
        receiveMessage();
    }
}
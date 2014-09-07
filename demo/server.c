#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MSGSZ   256


/*
 * Declare the message structure.
 */

typedef struct msgbuf {
    long    mtype;
    char    mtext[MSGSZ];
} message_buf;

char msg[MSGSZ];
long type;

int sendMessageBack(){
    int msqid, i;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    message_buf sbuf;
    size_t buf_length;

    /*
     * Get the message queue id for the
     * "name" 1234, which was created by
     * the server.
     */
    //key = ftok("dup.c", 'b');
    key = 1000;
    //(void) fprintf(stderr, "\nmsgget: Calling msgget(%#lx,\%#o)\n",key, msgflg);

    if ((msqid = msgget(key, msgflg )) < 0) {
        perror("msgget");
        return 1;
    }
    // else 
    //     (void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);


    /*
     * We'll send message type 1
     */
     
    sbuf.mtype = type;
    
    //(void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);
    

    for(i = 0; i < strlen(msg); i++){
        if(msg[i] >= 'a' && msg[i] <= 'z'){
            msg[i] -= 32;
        }
        else if(msg[i] >= 'A' && msg[i] <= 'Z'){
            msg[i] += 32;
        }
    }

    strcpy(sbuf.mtext,msg);
    
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

//     else 
//         printf("Message: \"%s\" Sent\n", sbuf.mtext);
}


int receiveMessage(){
    int msqid;
    key_t key;
    message_buf  rbuf;
    struct msqid_ds qstat;
    /*
     * Get the message queue id for the
     * "name" 1234, which was created by
     * the server.
     */
    //key = 1234;
    //key = ftok("dup.c", 'z');
    key = 1200;
    if ((msqid = msgget(key, 0666)) < 0) {
        perror("msgget");
        return 1;
    }

    /*
     * Receive an answer of message type 1.
     */
     
    if (msgrcv(msqid, &rbuf, MSGSZ, 0, 0) < 0) {
        perror("msgrcv");
        return 1;
    }

    type = rbuf.mtype;
    /*
     * Print the answer.
     */
    //printf("\nMessage received at server end: %s\n", rbuf.mtext);

    if(msgctl(msqid, IPC_STAT,&qstat) < 0){
        perror("msgctl");
        return 1;
    }
    printf("Message received at time %s",(ctime(&qstat.msg_rtime)));

    strcpy(msg,rbuf.mtext);

    if (!strcmp(msg,"KILL"))
    {
        return 2;
    }

    return 0;

}

int main()
{
    while(1){
        if(receiveMessage() == 2){
            return 0;
        }
        sendMessageBack(msg);
    }
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>
#include "msgqueue.cpp"

#define KEYFILE "keyfile.txt"
#define KEYID 1


void client(){
	char text[MSGSIZE];
	char textFromServer[MSGSIZE];
	int dummy;
	while(1){
		printf("\nEnter String : ");
		gets(text);
		// printf("\nINPUT = {%s}",text);
		sendMessage(text,getpid(),KEYFILE,KEYID);
		receiveMessage(textFromServer,&dummy,KEYFILE,KEYID,getpid());
		printf("\nOUTPUT FROM SERVER = {%s}\n",textFromServer);
	}
}


int main(){
	client();
	return 0;
}

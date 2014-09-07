#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include "msgqueue.cpp"


#define KEYFILE "keyfile.txt"
#define KEYID 1

void reverseString_INPLACE(char *text){
	int i=0;
	while(text[i]!='\0'){
		i++;
	}
	i--;
	int j=0;
	char temp;
	while(1){
		if(i<j||i==j){
			return;
		}
		temp = text[i];
		text[i] = text[j];
		text[j] = temp;
		i--;
		j++;
	}
	return;
}

void server(){
	char text[MSGSIZE];
	int id;
	while(1){
		receiveMessage(text,&id,KEYFILE,KEYID);
		printf("\nTYPE = %d MSG = {%s}\n",id,text);
		reverseString_INPLACE(text);
		sendMessage(text,id,KEYFILE,KEYID);
	}
	return;
}
int main(){
	server();
}
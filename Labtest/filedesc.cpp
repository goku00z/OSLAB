#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// O_RDONLY
//     Open for reading only.
// O_WRONLY
//     Open for writing only.
// O_RDWR
//     Open for reading and writing. The result is undefined if this flag is applied to a FIFO.

// Any combination of the following may be used:

// O_APPEND
//     If set, the file offset shall be set to the end of the file prior to each write.
// O_CREAT

// int fileread(const  char *filename){
// 	int fid = open(filename,O_RDONLY);
// 	printf("\nFILE DESCRIPTOR = %d",fid);

// }

#define BUFFERSIZE 100

void runChild(int *fd){
	printf("\nCHILD IS RUNNING !!\n");
	// printf("\nIN CHILD : Writing to pipe");
	char line[BUFFERSIZE];
	for(int i = 0;i<10;i++){
		sprintf(line,"%s %d",line,i+1);
	}
	// sleep(1);
	write(fd[1],line,strlen(line)+1);
	printf("\nChild Going to read = \n");
	read(fd[0],line,BUFFERSIZE);
	printf("\nChild READ : {%s}",line);
	// close(fd[1]);
	return;
}

int createChild(int *fd){
	int pid = fork();
	if(pid != 0){
		return pid;
	}else{
		runChild(fd);
		exit(-1);
	}
}

int main(){
	int fd[2];
	pipe(fd);
	// createChild(fd);
	char line[BUFFERSIZE],lineout[BUFFERSIZE];
	// // close(fd[1]);
	// printf("\nFD[0] = %d FD[1] = %d",fd[0],fd[1]);

	// printf("\nParent watinging for read !!\n");
	// read(fd[0],line,BUFFERSIZE);

	// printf("\nSERVER READS = {%s}",line);
	// int j = 10;

	// while(j>0){
	// 	int n;
	// 	sscanf(line,"%d %[^\n]s",&n,line);
	// 	printf("\nn = %d",n);
	// 	sprintf(lineout,"%s %d",lineout,n * n);
	// 	// printf("\nLINE = {%s}",line);
	// 	// int temp;
	// 	// scanf("%d",&temp);
	// 	j--;
	// }
	// // sleep(1);
	// write(fd[1],lineout,strlen(lineout)+1);
	dup2(1,fd[1]);
	sprintf(lineout,"LINEOUT\n");
	write(fd[1],lineout,strlen(lineout)+1);
	dup2(0,fd[0]);
	// printf("\n")
	read(fd[0],lineout,10);
	printf("{%s}",lineout);
	printf("\n**********\n");
	return 0;	
}
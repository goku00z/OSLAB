#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <time.h>

#define BUFSIZE 10
char line[BUFSIZE];
int *a, n;
int shmid, status;
key_t key;
int ML, ms;
int MAXNUM;

void bubbleSort(int *arr, int start, int end){
	int i,j,t,k;
	int *temp,n=end-start+1;
	temp = (int*)malloc((end-start+1)*sizeof(int));
	for(i=start;i<=end;i++){
		temp[i-start]=arr[i];
	}
	for(i = 0; i < n-1; i++){
		for(j = 0; j < n-i-1; j++){
			if(temp[j] > temp[j+1]){
				t = temp[j];
				temp[j] = temp[j+1];
				temp[j+1] = t;
			}
		}
	}
	for(i=start;i<=end;i++){
		arr[i]=temp[i-start];
	}
	free(temp);
}

void setup(){
	int i;
	srand(time(NULL));
	printf("Enter the size of the array: ");
	scanf("%d", &n);
	printf("Enter the maximum level of the process tree: ");
	scanf("%d", &ML);
	printf("Enter the minimum size of the subarray: ");
	scanf("%d", &ms);
	printf("Enter the max limit of the random number generator: ");
	scanf("%d", &MAXNUM);
	key = ftok("/usr/local/lib",'A');
	shmid = shmget(key, n*sizeof(int), 0777|IPC_CREAT);
	if(shmid == -1){
		printf("shmget error!\n");
		exit(1);
	}
	a = (int *) shmat(shmid, 0, 0);
	for(i = 0; i < n; i++){
		a[i] = rand() % MAXNUM + 1;
	}
	printf("Unsorted Array: ");
	for(i = 0; i < n; i++){
		printf("%d ", a[i]);
	}
	printf("\n");
}

void execute(int i, int j, int level){
	int pid1, pid2, M, *b, *c;
	int k;	
	M = (i+j)/2;
	int fd1[2];
	int fd2[2];
	pipe(fd1);
	pipe(fd2);
	pid1 = fork();
	if(pid1 == 0){
		close(fd2[0]);
		close(fd1[1]);
		b = (int *) shmat(shmid, 0, 0);
		if(M-i+1 <= ms || level >= ML){
			bubbleSort(b,i,M);
		}
		else{
			execute(i, M, level+1);
		}
		/****Synchronization 1****/
		sprintf(line,"%d",1);
		write(fd2[1],line,BUFSIZE);
		read(fd1[0],line,BUFSIZE);
		/****Synchronization 1****/
		int p = 0,q = i,r = (M + 1);
		int *Copy1 = (int*)malloc((M - i + 1)*sizeof(int));
		while(p <= (M - i)){
			if(b[q] < b[r]){
				Copy1[p++] = b[q++];
			}
			else{
				Copy1[p++] = b[r++];
			}
		}
		/****Synchronization 2****/
		sprintf(line,"%d",1);
		write(fd2[1],line,BUFSIZE);
		read(fd1[0],line,BUFSIZE);
		/****Synchronization 2****/
		for(p=i;p<=M;p++){
			b[p] = Copy1[p-i];
		}
		free(Copy1);
		shmdt(b);
		exit(0);
	}else{
		pid2 = fork();
		if(pid2 == 0){
			close(fd1[0]);
			close(fd2[1]);
			c = (int *) shmat(shmid, 0, 0);
			if(j-M <= ms || level >= ML){
				bubbleSort(c,M+1,j);
			}
			else{
				execute(M+1, j, level+1);
			}
			/****Synchronization 1****/
			sprintf(line,"%d",1);
			write(fd1[1],line,BUFSIZE);
			read(fd2[0],line,BUFSIZE);
			/****Synchronization 1****/
			int p = j-M-1,q = M,r = j;
			int *Copy2 = (int*)malloc((j - M)*sizeof(int));
			while(p >= 0){
				if(c[q] > c[r]){
					Copy2[p--] = c[q--];
				}
				else{
					Copy2[p--] = c[r--];
				}
			}
			/****Synchronization 2****/
			sprintf(line,"%d",1);
			write(fd1[1],line,BUFSIZE);
			read(fd2[0],line,BUFSIZE);
			/****Synchronization 2****/
			for(p=M+1;p<=j;p++){
				c[p] = Copy2[p-M-1];
			}	
			free(Copy2);
			shmdt(c);
			exit(0);	
		}
		else{
			close(fd1[1]);
			close(fd1[0]);
			close(fd2[0]);
			close(fd2[1]);
			waitpid(pid1);
			waitpid(pid2);

		}
	}
}

int main(){
	int i;
	setup();
	if(n-1 <= ms || 0 >= ML){
		bubbleSort(a,0,n-1);
	}
	else{
		execute(0, n-1, 0);
	}
	printf("Sorted Array: ");
	for(i = 0; i < n; i++){
		printf("%d ", a[i]);
	}
	printf("\n");
	shmdt(a);
	shmctl(shmid, IPC_RMID, 0);
	return 0;
}
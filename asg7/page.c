#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXPAGESIZE 100
#define MAXFRAMESIZE 10
#define PAGECOUNT 1000
#define EMPTY -1

struct cQueue{
	int *a;
	int insert;
	int size;
};
typedef struct cQueue cQueue;

void initcQueue(cQueue *Q,int N){
	Q->insert = 0;
	Q->a = (int *)malloc(sizeof(int) * N);
	for(int i=0;i<N;i++){
		Q->a[i] = EMPTY;
	}
	Q->size = N;
	return;
}
void freeCQueue(cQueue *Q){
	free(Q->a);
	Q->a = 0;
	return;
}

struct Stack{
	int *a;
	int size;
};
typedef struct Stack Stack;
struct countArray{
	int *a;
	int *count;
	int size;
};
typedef struct countArray countArray;
void initCountArray(countArray *C,int N){
	C->a = (int *)malloc(sizeof(int) * N);
	C->count = (int *)malloc(sizeof(int) * N);
	for(int i=0;i<N;i++){
		C->a[i] = EMPTY;
		C->count[i] = 0;
	}
	C->size = N;
	return;
}
void freeCountArraay(countArray *C){
	C->size = 0;
	free(C->a);
	free(C->count);
	C->a = 0;
	C->count = 0;
	return;
}

void initStack(Stack *S,int N){
	S->a = (int *)malloc(sizeof(int) * N);
	for(int i=0;i<N;i++){
		S->a[i] = EMPTY;
	}
	S->size = N;
	return;
}
void freeStack(Stack *S){
	free(S->a);
	S->a = 0;
	S->size = 0;
	return;
}


/******************** main algo functions ****************************/
bool getPageFF(int page,cQueue *Q){
	for(int i=0;i<Q->size;i++){
		if(page == Q->a[i]){
			return true;
		}
	}
	Q->a[Q->insert] = page;
	Q->insert = (Q->insert+1)%Q->size;
	return false;
}
bool getPageLR(int page,Stack *S){
	bool found = false;
	int foundIndex = 0;
	for(int i=0;i<S->size;i++){
		if(S->a[i] == page){
			foundIndex = i;
			found = true;
			break;
		}
	}
	if(found == true){
		for(int i=foundIndex-1;i>=0;i--){
			S->a[i+1] = S->a[i];
		}
	}else{
		for(int i=S->size-2;i>=0;i--){
			S->a[i+1] = S->a[i];
		}
	}
	S->a[0] = page;
	// printf("\n{");
	// for(int i=0;i<S->size;i++){
	// 	printf(" %d ",S->a[i]);
	// }
	// printf("} PAGE = %d\n",page);
	return found;
}
bool getPageLF(int page,countArray *C){
	bool found = false;
	int foundIndex = -1;
	for(int i=0;i<C->size;i++){
		if(C->a[i] == page){
			foundIndex = i;
			found = true;
			break;
		}
	}
	if(found){
		C->count[foundIndex] = C->count[foundIndex]+1;
	}else{
		int minCount = C->count[0];
		int minIndex = 0;
		for(int i=1;i<C->size;i++){
			if(C->count[i]<minCount){
				minIndex = i;
				minCount = C->count[i];
			}
		}
		C->a[minIndex] = page;
		C->count[minIndex] = 1;
	}
	return found;
}
void runFF(const char *filename){
	printf("\n************* RUNNING FIFO ANALYSIS ****************\n");
	int frameSize,pageCount;
	FILE *f = fopen(filename,"r");
	fscanf(f,"%d%d",&pageCount,&frameSize);
	int page;
	cQueue Q;
	initcQueue(&Q,frameSize);
	int hit = 0,miss = 0;
	for(int i=0;i<pageCount;i++){
		fscanf(f,"%d",&page);
		if(getPageFF(page,&Q)){
			hit++;
			printf("\nPAGE = %d HIT",page);
		}else{
			miss++;
			printf("\nPAGE = %d MISS",page);
		}
	}
	fclose(f);
	freeCQueue(&Q);
	printf("\nTOTAL HIT = {%d} MISS = {%d}",hit,miss);
	return;
}
void runLF(const char *filename){
	printf("\n************** RUNNING LFU ANALYSIS ****************\n");
	int frameSize,pageCount;
	FILE *f = fopen(filename,"r");
	fscanf(f,"%d%d",&pageCount,&frameSize);
	int page;
	countArray C;
	initCountArray(&C,frameSize);
	int hit = 0,miss = 0;
	for(int i=0;i<pageCount;i++){
		fscanf(f,"%d",&page);
		if(getPageLF(page,&C)){
			hit++;
			printf("\nPAGE = %d HIT",page);
		}else{
			miss++;
			printf("\nPAGE = %d MISS",page);
		}
	}
	fclose(f);
	freeCountArraay(&C);
	printf("\nTOTAL HIT = {%d} MISS = {%d}",hit,miss);
	return;
}
void runLR(const char *filename){
	printf("\n************** RUNNING LRU ANALYSIS ****************\n");
	int frameSize,pageCount;
	FILE *f = fopen(filename,"r");
	fscanf(f,"%d%d",&pageCount,&frameSize);
	int page;
	Stack S;
	initStack(&S,frameSize);
	int hit = 0,miss = 0;
	for(int i=0;i<pageCount;i++){
		fscanf(f,"%d",&page);
		if(getPageLR(page,&S)){
			hit++;
			printf("\nPAGE = %d HIT",page);
		}else{
			miss++;
			printf("\nPAGE = %d MISS",page);
		}
	}
	fclose(f);
	freeStack(&S);
	printf("\nTOTAL HIT = {%d} MISS = {%d}",hit,miss);
	return;
}
/**********************************************************************/
void generateFile(const char *filename,int maxPageSize,int maxFrameSize,int pageCount){
	printf("\nGENERATING INPUT FILE\n");
	FILE *f = fopen(filename,"w");
	fprintf(f, "%d %d\n",pageCount,maxFrameSize );
	for(int i=0;i<pageCount;i++){
		int page = rand()%maxPageSize;
		fprintf(f, "%d ", page);
	}
	fclose(f);
	return;
}
bool getRunFlag(const char *algo1,const char *algo2,const char *algo3,const char *search){
	if(strcmp(algo1,search) == 0||strcmp(algo2,search) == 0||strcmp(algo3,search) == 0){
		return true;
	}
	return false;
}
int main(int argc,char **argv){
	char filename[100],algo1[20],algo2[20],algo3[20];
	algo3[0] = '\0';
	algo2[0] = '\0';
	algo1[0] = '\0';
	filename[0] = '\0';
	int paramCount = argc;
	int algoCount = 0;
	bool FF_flag = false,LR_flag = false,LF_flag = false;
	int maxPageSize,maxFrameSize,pageCount;
	switch(paramCount){
		case 5:{
			// all 3 are provied
			sprintf(algo3,"%s",argv[4]);
			algoCount++;
		}
		case 4:{
			// only 2 are provided
			sprintf(algo2,"%s",argv[3]);
			algoCount++;
		}
		case 3:{
			// only 1 are provied
			sprintf(algo1,"%s",argv[2]);
			algoCount++;
		}
		case 2:{
			// no algorithrm parameter given use all there
			sprintf(filename,"%s",argv[1]);
			break;
		}
		default:{
			printf("\nUSGAE = ./page [filename] [algo-paremeters ..[optional]]\n");
			exit(0);
		}
	}
	printf("\nFILE NAME = {%s} ALGO1 = {%s} ALGO2 = {%s} ALGO3 = {%s}",filename,algo1,algo2,algo3);
	FF_flag = getRunFlag(algo1,algo2,algo3,"FF");
	LF_flag = getRunFlag(algo1,algo2,algo3,"LF");
	LR_flag = getRunFlag(algo1,algo2,algo3,"LR");
	if(algoCount == 0){
		FF_flag = LF_flag = LR_flag = true;
	}
	// printf("\nEnter MAXPAGESIZE : ");
	// scanf("%d",&maxPageSize);
	// printf("\nEnter MAXFRAMESIZE : ");
	// scanf("%d",&maxFrameSize);
	// printf("\nEneter PAGECOUNT : ");
	// scanf("%d",&pageCount);
	maxPageSize = MAXPAGESIZE;
	maxFrameSize = MAXFRAMESIZE;
	pageCount = PAGECOUNT;

	generateFile(filename,maxPageSize,maxFrameSize,pageCount);

	if(FF_flag){
		runFF(filename);
	}
	if(LF_flag){
		runLF(filename);
	}
	if(LR_flag){
		runLR(filename);
	}

	printf("\n");
	return 0;
}
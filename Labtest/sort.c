#include <stdio.h>
#include <stdlib.h>

#define N 20

void printList(int *a,int i,int j){
	printf("\n{");
	for(int k = i;k<=j;k++){
		printf(" %d ",a[k]);
	}
	printf("}\n");
	return;
}
void printList(int *a,int n){
	printList(a,0,n-1);
	return;
}
int findMax(int *a,const int start,const int end){
	int max = a[start];
	int maxIndex = start;
	for(int i = start+1;i<=end;i++){
		if(a[i]>max){
			max = a[i];
			maxIndex = i;
		}
	}
	return maxIndex;
}
void selectionSort(int *a,const int start,const int end){
	int maxIndex,temp;
	for(int i = end;i >= start;i-- ){
		maxIndex = findMax(a,start,i);
		temp = a[i];
		a[i] = a[maxIndex];
		a[maxIndex] = temp;
	}
	return;
}
void bubbleSort(int *a,const int start,const int end){
	int temp;
	for(int i = end;i>start;i--){
		for(int j=start;j<i;j++){
			if(a[j]>a[j+1]){
				temp = a[j];
				a[j] = a[j+1];
				a[j+1] = temp;
			}
		}
	}
	return;
}

int main(){
	int a[N];
	for(int i=0;i<N;i++){
		a[i] = rand()%100;
	}
	printList(a,N);
	selectionSort(a,(int)N/3,(int)N/4);
	// bubbleSort(a,N/4,N/3);
	printList(a,N);

	printf("\n");
	return 0;
}
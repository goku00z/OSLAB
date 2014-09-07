#include <stdio.h>
#include <stdlib.h>

#define BUFFER 1000

int main(){
	char a[BUFFER];
	printf("\nPRINT\n");
	fgets(a, BUFFER, stdin);
	printf("\na = {%s}\n",a);

	printf("\n");
	return 0;
}
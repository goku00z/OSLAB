#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <locale.h>
#include <langinfo.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <iostream>

int main(){
	int n;
	while(1){
		scanf("%d",&n);
		if(n!=-1){
			printf(" %d\n",n*3);
		}
		if(n==-1){
			break;
		}
	}
	printf(" -1 ");
	return 0;
}
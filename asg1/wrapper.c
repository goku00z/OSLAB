#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>

int main(int argc,char **argv){
	// char call[1000];
	// sprintf(call,"xterm -e \"./parmax %s\"",argv[1]);
	// system(call);
// ./Home/sourav/OS/asg1/
	char argm[1000];
	sprintf(argm,"./parmax %s",argv[1]);
	execl("/usr/bin/xterm","/usr/bin/xterm","-e",argm,NULL);
	return 0;
}
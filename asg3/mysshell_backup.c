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
#include <signal.h>


// kill -9 `ps -A|grep myshell|awk '{print $1}'`

#define PROGRUNBUFFER 1000
#define MAXNUMOFARGUMENTS 100
#define MAXARGSIZE 100
#define MAXPATHCOUNT 100
#define MAXPATHSIZE 200


#define MAXPIPECOMMANDSIZE 200

#define BUFFER 1000


char line[BUFFER];
char words[2][BUFFER];
char fileIn[BUFFER];
char fileOut[BUFFER];

char inputStream[BUFFER];
char outputstream[BUFFER];


int sigflag;

void sighandler(int s)
{
  //printf("\ns\n"); 
}

void separateWord(char *s){
    int i = 0, j = 0;
    while(i < strlen(s) && s[i] != ' '){
        words[0][i] = s[i];
        i++;
    }
    words[0][i] = '\0';
    if(i == strlen(s)){
        // words[0][i-1] = '\0';
        strcpy(words[1],"");
        return;
    }
    i++;
    while(i < strlen(s)){
        words[1][j] = s[i];
        i++;
        j++;
    }
    words[1][j] = '\0';
    // if(j-2>=0){
    // 	if(words[1][j-2]=='\n'||words[1][j-2]==' '||words[1][j-2]=='\t'){
	   //  	words[1][j-2] = '\0';
    // 	}
    // }
    return;
}

void copyString(char *dest,const char *src){
	while((*src)!='\0'){
		(*dest)=(*src);
		dest++;
		src++;
	}
	(*dest)='\0';
	return;
}
void clearTabs(char *line){
	// removes spaces and tabs from line
	// like "   \t  bbbb   \t\t  rrrr \t  ggg\t  "--->"bbbb rrrr ggg"
	int i,j;
	// convert all tabs into spaces
	// printf("\n*****************BEFORE TAB REMOVAL : '%s'",line);
	i=j=0;
	while(line[i]!='\0'){
		if(line[i]=='\t'||line[i]==9||line[i]==11){
			// printf("\ntab detected !!");
			line[i]=' ';
		}
		i++;
	}
	// printf("\n****************AFTER TAB REMOVAL : '%s'",line);
	// all tabs are replaced by spaces,,,now we will remove redundant spaces[inplace]
	i=j=0;
	while(line[i]==' '){
		i++;
	}
	while(1){
		if(line[i]=='\0'){
			break;
		}
		if(line[i]!=' '){
			line[j]=line[i];
			j++;
			i++;
			continue;
		}
		line[j]=' ';
		i++;
		j++;
		while(line[i]==' '){
			i++;
		}
	}
	if(j-1>=0){
		if(line[j-1]==' '){
			line[j-1]='\0';
		}
	}
	line[j]='\0';
	// printf("\n****************FINAL TAB REMOVAL : '%s'",line);
	return;
}
void lineToWords(const char *xline,char **list,int *wCount,char separator){
	char word[PROGRUNBUFFER];
	char line[PROGRUNBUFFER];
	copyString(line,xline);
	clearTabs(line);
	int i=0;
	int lineend=0;
	int count=0;
	int j;
	while(1){
		j=0;
		while(1){
			if(line[i]=='\0'){
				lineend=1;
				break;
			}
			if(line[i]==separator){
				i++;
				break;
			}
			word[j]=line[i];
			i++;
			j++;
		}
		word[j]='\0';
		if(j!=0){
			copyString(list[count],word);
			count++;
		}
		if(lineend==1){
			break;
		}
	}
	(*wCount)=count;
	return;
}

int countChar(const char *line,char match){
	int spaceCount,i;
	i=spaceCount=0;
	char a=line[0];
	while(a!='\0'){
		// printf("%c\n",a);
		if(a==match){
			spaceCount++;
		}
		i++;
		a=line[i];
	}
	return spaceCount;
}
int extractWord(char *line,char x,char *word){
	int i=0;
	int count=0;
	int posx;
	word[0] = '\0';
	while(line[i]!='\0'){
		if(line[i]==x){
			count++;
			posx = i;
		}
		i++;
	}
	if(count>1){
		return -1;
	}
	if(count == 0){
		return 0;
	}
	int len = i;
	i = posx+1;
	int nextWord=0;
	while(i<len){
		if(line[i]=='<'||line[i]=='>'){
			word[0] = '\0';
			return -1;
		}
		if(line[i]!=' '&&line[i]!='\n'&&line[i]!='\t'&&line[i]!='|'){
			nextWord = i;
			break;
		}
		i++;
	}
	if(nextWord == 0){
		return -1;
	}
	// printf("\nnext word found at  = %d",nextWord);
	int j = 0;
	while(1){
		if(line[i]==' '||line[i]=='\n'||line[i]=='\t'||line[i]=='|'||line[i]=='\0'){
			nextWord = i;
			word[j] = '\0';
			break;
		}
		word[j] = line[i];
		j++;
		i++;
	}
	while(i<len){
		line[posx] = line[i];
		i++;
		posx++;
	}
	line[posx] = '\0';
	return 1;
}
void getFirstWord(char *cmd,char *part1,char *part2){
	int i = 0;
	while(cmd[i]!=' '&&cmd[i]!='|'&&cmd[i]!='\0'){
		part1[i] = cmd[i];
		i++;
	}
	part1[i] = '\0';
	int j = 0;
	if(cmd[i]=='\0'){
		part2[0] = '\0';
		return;
	}
	i++;
	while(cmd[i]!='\0'){
		part2[j] = cmd[i];
		i++;
		j++;
	}
	part2[j] = '\0';
	return;
}
int execwrapper(char *cmd){
	// printf("\nIN EXECWRAPPER\n");
	// sleep(1);

	char parameterX[BUFFER];
	char progName[PROGRUNBUFFER];
	char progPath[PROGRUNBUFFER];// useless ... may take a default path parameter
	// in future
	char progPathName[2*PROGRUNBUFFER];
	progPath[0] = '\0';
	progName[0] = '\0';
	parameterX[0] = '\0';

	getFirstWord(cmd,progName,parameterX);

	char *arglist[MAXNUMOFARGUMENTS+1];
	char *pathList[MAXPATHCOUNT];
	for(int i=0;i<MAXPATHCOUNT;i++){
		pathList[i]=(char *)malloc(sizeof(char)*MAXPATHSIZE);
	}
	for(int i=0;i<MAXNUMOFARGUMENTS+1;i++){
		arglist[i] = (char *)malloc(sizeof(char)*MAXARGSIZE);
	}
	int argCount = countChar(parameterX,' ')+1;
	if(argCount>MAXNUMOFARGUMENTS){
		// printf("\nERROR : can not have more than %d arguments\n",MAXNUMOFARGUMENTS);
		return -1;
	}
	lineToWords(parameterX,arglist+1,&argCount,' ');
	copyString(arglist[0],progName);
	argCount++;
	free(arglist[argCount]);
	arglist[argCount]=NULL;
	sprintf(progPathName,"%s%s",progPath,progName);
	// printf("\nPROGPATHNAME = {%s}",progName);
	// printf("\nparameterX = {%s}",parameterX);
	// printf("\nPROGPATHNAME = {%s}",progPathName);
	int success=0;
	// assuming prog is in current directory
	success = execv(progPathName,arglist);
	if(success==-1){
		// if the previous execv is failed
		// then try execv for different path combinations
		// printf("\nEXEC UNSUCCESSFULL !!\n");
		// sleep(1);
		char* pPath;
		pPath = getenv ("PATH");
		int pathCount;
		pathCount = countChar(pPath,':')+1;
		lineToWords(pPath,pathList,&pathCount,':');
		for(int i=0;i<pathCount;i++){
			sprintf(progPathName,"%s/%s",pathList[i],progName);
			success = execv(progPathName,arglist);
		}
		if(success == -1){
			// printf("\nERROR : INVALID COMMAND--{%s}\n",progName);
		}
	}
	for(int i=0;i<argCount;i++){
		free(arglist[i]);
	}
	for(int i=argCount+1;i<MAXNUMOFARGUMENTS+1;i++){
		free(arglist[i]);
	}
	for(int i=0;i<MAXPATHCOUNT;i++){
		free(pathList[i]);
	}
	return success;
}
void runProg(const char *progName,const char *progPath,const char *parameters,int amp_flag){
	int pid,status;
	char progPathName[PROGRUNBUFFER];
	char *arglist[MAXNUMOFARGUMENTS+1];
	char *pathList[MAXPATHCOUNT];
	for(int i=0;i<MAXPATHCOUNT;i++){
		pathList[i]=(char *)malloc(sizeof(char)*MAXPATHSIZE);
	}
	for(int i=0;i<MAXNUMOFARGUMENTS+1;i++){
		arglist[i] = (char *)malloc(sizeof(char)*MAXARGSIZE);
	}
	int pSize = strlen(parameters);
	char parameterX[pSize+1];
	copyString(parameterX,parameters);
	int argCount = countChar(parameterX,' ')+1;
	// printf("\nARG-COUNT = %d\n",argCount);
	if(argCount>MAXNUMOFARGUMENTS){
		printf("\nERROR : can not have more than %d arguments\n",MAXNUMOFARGUMENTS);
		return;
	}
	lineToWords(parameters,arglist+1,&argCount,' ');
	copyString(arglist[0],progName);
	// printf("\nargCount = %d",argCount);
	argCount++;
	// for(int i=0;i<argCount;i++){
	// 	printf("\narg[%d] = {%s}",i,arglist[i]);
	// }
	// printf("\n");
	free(arglist[argCount]);
	arglist[argCount]=NULL;
	sprintf(progPathName,"%s%s",progPath,progName);
	int success=0;
	pid = fork();
	if(pid == 0){
		// assuming prog is in current directory
		success = execv(progPathName,arglist);
		if(success==-1){
			// if the previous execv is failed
			// then try execv for different path combinations
			char* pPath;
			pPath = getenv ("PATH");
			int pathCount;
			pathCount = countChar(pPath,':')+1;
			lineToWords(pPath,pathList,&pathCount,':');
			// printf("\nPATH COUNT = %d\n",pathCount);
			// for(int i=0;i<pathCount;i++){
			// 	printf("\nPATH[%d] = {%s}",i,pathList[i]);
			// }
			for(int i=0;i<pathCount;i++){
				sprintf(progPathName,"%s/%s",pathList[i],progName);
				success = execv(progPathName,arglist);
			}
			if(success == -1){
				printf("\nERROR : INVALID COMMAND--{%s}\n",progName);
			}
		}
	}else{
		if(amp_flag == 1){
			printf("\nPID = %d",pid);
		}else{
			waitpid(pid,&status,0);
		}
	}
	for(int i=0;i<argCount;i++){
		free(arglist[i]);
	}
	for(int i=argCount+1;i<MAXNUMOFARGUMENTS+1;i++){
		free(arglist[i]);
	}
	for(int i=0;i<MAXPATHCOUNT;i++){
		free(pathList[i]);
	}
	// printf("PROCESS EXIT STATUS = %d\n",WEXITSTATUS(status));
	return;
}
void removeLastEnter(char *a){
	int i = 0;
	while(1){
		if(a[i] == '\0'){
			break;
		}
		i++;
	}
	if(i>=1){
		if(a[i-1]=='\n'){
			a[i-1] = '\0';
		}
	}
	return;
}
int isPipingNeeded(const char *cmd){
	char a;
	int i=0;
	int pCount = 0;
	while(1){
		a = cmd[i];
		// printf("\na= {%c}",a);
		if(a == '\0'){
			break;
		}
		if(a == '|'){
			pCount++;
		}
		i++;
	}
	return pCount;
}
void runPipe(char **proglist,int pCount){
	// printf("\nIN RUNPIPE\n");
	// printf("\nPCOUNT = %d",pCount);
	for(int i=0;i<pCount;i++){
		// printf("\n{%s}",proglist[i]);
	}
	// sleep(1);
    int pid;
    int status;
    int pipes[pCount - 1][2];
    int isRoot = 0;
    int isEnd = 0;
    char FileIn[100];
    char FileOut[100];
    int stdin_copy = 0,stdout_copy = 0;
    while(1){
        pid = fork();
        if(pid!=0){
            // parent process
            if(isRoot == 0){
                // code at the root
                // printf("\nWE ARE AT ROOT\n");
                waitpid(pid,&status,0);
                break;
            }else{
                // code at other places
                waitpid(pid,&status,0);
                // printf("\nIS END = %d\n",isEnd);
        		close(pipes[isEnd-1][1]);
                // sprintf(buffer,"\nCLOSING PIPE WRITE = %d",isEnd-1);
                // print(buffer);
                if(isRoot == 1){
                    
                }else{
                    close(1);
                    dup2(pipes[isEnd-2][1],1);
                }
                close(0);
                dup2(pipes[isEnd-1][0],0);
                // printf("\nGOING TO EXECWRAPPER = {%s}\n",proglist[pCount - isEnd]);
                int k = execwrapper(proglist[pCount - isEnd]);
                // int k = execlp(proglist[pCount - isEnd],
                //     proglist[pCount - isEnd],NULL);
                if(k == -1){
                    printf("\nEXEC NOT SUCCESSFUL !!\n");
                    exit(-1);
                }
            }
        }else{
            // child process
            isRoot++;
            if(isEnd == pCount - 1){
            	inp_flag = extractWord(proglist[pCount - isEnd - 1], '<' ,fileIn);
		        out_flag = extractWord(proglist[pCount - isEnd - 1],'>', fileOut);
		        
		        if(inp_flag != -1 && out_flag != -1){
		        	if(inp_flag == 1){
		        		stdin_copy = dup(0);
		        		close(0);
		        		ifd = open(fileIn, O_RDONLY);
						if (ifd < 0) {
				        	fprintf(stderr, "Unable to open input file in read mode...\n");
				        	continue;
						} 
		        	}
		        	if(out_flag == 1){
		        		stdout_copy = dup(1);
		        		close(1);
		        		ofd = open(fileOut, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
						if (ofd < 0) {
		    			    fprintf(stderr, "Unable to open output file in write mode...\n");
				        	continue;
						} 
		        	}
		        }
                close(pipes[isEnd-1][0]);
                close(1);
                dup2(pipes[isEnd-1][1],1);
                // printf("\nGOING TO EXECWRAPPER(tail) = {%s}\n",proglist[pCount - isEnd-1]);
                int k = execwrapper(proglist[pCount - isEnd - 1]);
                // int k = execlp(proglist[pCount - isEnd - 1],
                //     proglist[pCount - isEnd - 1],NULL);
                if(k == -1){
                    printf("\nEXEC NOT SUCCESSFUL !!\n");
                    // sleep(1);
                    exit(-1);
                }
            }
            else{
                // printf("\nPIPE = %d",isEnd);
                pipe(pipes[isEnd]);
                // pipeid++;
                isEnd++;
            }
        }

    }
    return;
}
int piping(char *cmd,int pipeCount){
	int processCount = pipeCount+1;
	char *process[processCount];
	for(int i=0;i<processCount;i++){
		process[i] = (char *)malloc(sizeof(char) * MAXPIPECOMMANDSIZE);
	}
	int dummy;
	lineToWords(cmd,process,&dummy,'|');
	for(int i=0;i<processCount;i++){
		// printf("\nPROCESS[%d] = {%s}",i,process[i]);
	}
	printf("\n");
	for(int i=0;i<processCount;i++){
		if(process[i][0] == '\0'){
			printf("\nERROR in %d th command !!",i+1);
			return -1;
		}
	}
	/************** main piping code *******************/
	runPipe(process,processCount);
	/***************************************************/
	for(int i=0;i<processCount;i++){
		free(process[i]);
	}
	printf("\n");
	return 0;
}


void runshell(){
    int flag, inp_flag, out_flag, ifd, ofd, stdin_copy, stdout_copy;
    char pth[BUFFER];
    char fileIn[BUFFER], fileOut[BUFFER];
    strcpy(pth,"");
    while(1){
        printf("myshell@%s> ", getcwd(line,BUFFER));
        line[0] = '\0';
        fgets(line, BUFFER, stdin);
        clearTabs(line);
        removeLastEnter(line);
        // printf("\nLINE = {%s}\n",line);

        int pipeCount = isPipingNeeded(line);
        // printf("\npipeCount = %d\n",pipeCount);
        if(pipeCount>0){
        	piping(line,pipeCount);
        	continue;
        }
        

        /************ NO PIPING IS REQUIRED ***********/
        inp_flag = extractWord(line, '<' ,fileIn);
        out_flag = extractWord(line,'>', fileOut);
        
        if(inp_flag != -1 && out_flag != -1){
        	if(inp_flag == 1){
        		stdin_copy = dup(0);
        		close(0);
        		ifd = open(fileIn, O_RDONLY);
				if (ifd < 0) {
		        	fprintf(stderr, "Unable to open input file in read mode...\n");
		        	continue;
				} 
        	}
        	if(out_flag == 1){
        		stdout_copy = dup(1);
        		close(1);
        		ofd = open(fileOut, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				if (ofd < 0) {
    			    fprintf(stderr, "Unable to open output file in write mode...\n");
		        	continue;
				} 
        	}
        }
        
        if(feof(stdin)){
        	printf("\n");
        	exit(0);
        }
        separateWord(line);
        //printf("%s",words[0]);
        if(!strcmp(words[0],"cd")){
            strcpy(pth,"");
            strcat(pth,getcwd(line,BUFFER));
            //if(words[1][0] != '/')
            strcat(pth,"/");
            strcat(pth,words[1]);
            flag = chdir(pth);
            if(flag == -1){
            	flag = chdir(words[1]);
            	if(flag == -1) printf("\nNo Such file or directory !!\n");
                	// runProg(words[0],"",words[1]);
            }       
        }
        else if(!strcmp(words[0],"pwd")){
            printf("%s\n",getcwd(line,BUFFER));
        }
        else if(!strcmp(words[0],"mkdir")){
            strcpy(pth,"");
            strcat(pth,getcwd(line,BUFFER));
            strcat(pth,"/");
            strcat(pth,words[1]);
            flag = mkdir(pth,0777);
            if(flag == -1){
            	flag = mkdir(words[1],0777);
            	if(flag == -1)
                	runProg(words[0],"",words[1],0);
            }               
        }
        else if(!strcmp(words[0],"rmdir")){
            strcpy(pth,"");
            strcat(pth,getcwd(line,BUFFER));
            strcat(pth,"/");
            strcat(pth,words[1]);
            flag = rmdir(pth);
            if(flag == -1){
            	flag = rmdir(words[1]);
            	if(flag == -1)
                	runProg(words[0],"",words[1],0);
            }
        }
        else if(!strcmp(words[0],"ls") && !(words[1][0] == '-' && words[1][1] == 'l')){
            DIR *dp;
            struct dirent *ep;
            dp = opendir ("./");
            if (dp != NULL)
            {
                while (ep = readdir (dp))
                puts (ep->d_name);
                    (void) closedir (dp);
            }
            else
                puts ("Couldn't open the directory.");
        }
        else if(!strcmp(words[0],"ls") && words[1][0] == '-' && words[1][1] == 'l'){
            DIR *d;
            struct dirent *de;
            struct stat buf;
            int i,j;
            char P[10]="rwxrwxrwx",AP[10]=" ";
            struct passwd *p;
            struct group *g;
            struct tm *t;
            char time[26];
            d=opendir(".");
            readdir(d);
            readdir(d);
            while((de=readdir(d))!=NULL)
            {
                stat(de->d_name,&buf);

                // File Type
                if(S_ISDIR(buf.st_mode))
                printf("d");
                else if(S_ISREG(buf.st_mode))
                printf("-");
                else if(S_ISCHR(buf.st_mode))
                printf("c");
                else if(S_ISBLK(buf.st_mode))
                printf("b");
                else if(S_ISLNK(buf.st_mode))
                printf("l");
                else if(S_ISFIFO(buf.st_mode))
                printf("p");
                else if(S_ISSOCK(buf.st_mode))
                printf("s");
                //File Permissions P-Full Permissions AP-Actual Permissions
                for(i=0,j=(1<<8);i<9;i++,j>>=1)
                AP[i]= (buf.st_mode & j ) ? P[i] : '-' ;
                printf("%s",AP);
                //No. of Hard Links
                printf("%lu",buf.st_nlink);
                //User Name
                p=getpwuid(buf.st_uid);
                printf(" %.8s",p->pw_name);
                //Group Name
                g=getgrgid(buf.st_gid);
                printf(" %-8.8s",g->gr_name);
                //File Size
                printf(" %lu",buf.st_size);
                //Date and Time of modification
                t=localtime(&buf.st_mtime);
                strftime(time,sizeof(time),"%b %d %H:%M",t);
                printf(" %s",time);
                //File Name
                printf(" %s\n",de->d_name);
            }
        }
        else if(!strcmp(words[0],"exit")){
            exit(0);
        }
        else if(strlen(words[0]) == 0){
        	continue;
        }
        else{
        	int amp_flag = 0;
        	char lastChar = words[1][strlen(words[1])-1];
        	if(lastChar == '&')
        		amp_flag = 1;
        	// printf("{%s}{%s}",words[0],words[1]);
            runProg(words[0],"",words[1],amp_flag);           
        }
        // printf("\nOutside\n");
        if(inp_flag != -1 && out_flag != -1){
        	if(inp_flag == 1){
        		close(ifd);
				dup2(stdin_copy,0);
				close(stdin_copy);
        	}
        	if(out_flag == 1){
        		close(ofd);
				dup2(stdout_copy,1);
				close(stdout_copy);	
        	}
        }     
    }
}
int main(){
	signal(SIGINT, sighandler);
	runshell();
	printf("\n");
	return 0;
}
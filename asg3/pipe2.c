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

void print(const char *line ){
    FILE *debug = fopen("debug.txt","a");
    fprintf(debug, "%s\n",line );
    fclose(debug);
    return;
}

char buffer[1000];

void runPipe(char **proglist,int pCount){
    int pid;
    int status;
    int pipes[pCount - 1][2];
    int isRoot = 0;
    int isEnd = 0;
    while(1){
        pid = fork();
        if(pid!=0){
            // parent process
            if(isRoot == 0){
                // code at the root
                waitpid(pid,&status,0);
                // printf("\nBreaking at root !!\n");
                break;
            }else{
                // code at other places
                // printf("\nGOING TO EXEC = {%s}",proglist[pCount - isEnd]);
                waitpid(pid,&status,0);
                // printf("\nDEPEND ENDED FOR = {%s}",proglist[pCount - isEnd]);
                // printf("\nIS END = %d\n",isEnd);
                close(pipes[isEnd-1][1]);
                sprintf(buffer,"\nCLOSING PIPE WRITE = %d",isEnd-1);
                print(buffer);
                if(isRoot == 1){
                    
                }else{
                    close(1);
                    dup2(pipes[isEnd-2][1],1);
                }
                close(0);
                dup2(pipes[isEnd-1][0],0);
                
                sprintf(buffer,"\nIS END = %d",isEnd);
                print(buffer);
                sprintf(buffer,"\nIN PARENT EXELCP = %d",pCount - isEnd);
                print(buffer);
                int k = execlp(proglist[pCount - isEnd],
                    proglist[pCount - isEnd],NULL);
                if(k == -1){
                    print("\nEXEC NOT SUCCESSFUL !!\n");
                    sleep(1);
                }
            }
        }else{
            // child process
            // printf("\nGOING TO EXEC in tail = {%s}\n",proglist[pCount - isEnd-1]);
            isRoot++;
            if(isEnd == (pCount - 1)){
                // printf("\nTAIL EXEC = {%s}\n",proglist[pCount - isEnd-1]);
                close(pipes[isEnd-1][0]);
                sprintf(buffer,"\nCLOSING PIPE READ = %d",isEnd-1);
                print(buffer);
                close(1);
                dup2(pipes[isEnd-1][1],1);
                sprintf(buffer,"\nIN CHILD EXELCP = %d",pCount - isEnd-1);
                print(buffer);
                // isEnd++;
                int k = execlp(proglist[pCount - isEnd - 1],
                    proglist[pCount - isEnd - 1],NULL);
                if(k == -1){
                    print("\nEXEC NOT SUCCESSFUL !!\n");
                    sleep(1);
                }
            }
            else{
                sprintf(buffer,"\nCREATING PIPE = %d",isEnd);
                print(buffer);
                pipe(pipes[isEnd]);
                if(pipes[isEnd][0]==-1||pipes[isEnd][1]==-1){
                    print("\nCANT CREATE NEW PIPE!!\n");
                }
                // pipeid++;
                isEnd++;
            }
        }

    }
    return;
}

int main(int argc,char **argv){
    FILE *debug = fopen("debug.txt","w");
    fclose(debug);
    int pCount = argc-1;
    // printf("\npCount = %d",pCount);
    char *cmd[pCount];
    for(int i=0;i<pCount;i++){
        cmd[i] = (char *)malloc(sizeof(char)*100);
        strcpy(cmd[i],argv[i+1]);
        printf("\nCMD[%d] = {%s}",i,cmd[i]);
    }

    runPipe(cmd,pCount);

    printf("\nIN MAIN\n");

    return 0;
}
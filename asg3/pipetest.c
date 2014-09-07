#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc,char **argv)
{
    int fd[2];
    int childpid;
    pipe(fd);
    childpid=fork();
    if (childpid == -1)
    {
        perror("Error forking...");
        exit(1);
    }
    if (childpid)   /*parent proces*/   //grep .c
    {
        // wait(&childpid);        //waits till the child send output to pipe
        close(fd[1]);
        close(0);       //stdin closed
        dup2(fd[0],0);
        execlp(argv[2],argv[2],argv[3],NULL);
    }
    if (childpid==0)  //ls
    {
        close(fd[0]);   /*Closes read side of pipe*/
        close(1);       //STDOUT closed
        dup2(fd[1],1);
        execlp(argv[1],argv[1],NULL);
    }

    return 0;
}

    // pid = fork();
    // if(pid!=0){
    //     waitpid(pid,&status,0);
    // }else{
    //     pipe(pipe2);
    //     pid = fork();
    //     if(pid!=0){
    //         waitpid(pid,&status,0);
    //         // close(pipe2[1]);
    //         close(0);
    //         dup2(pipe2[0],0);
    //         execlp(proglist[2],proglist[2],NULL);
    //     }else{
    //         pipe(pipe1);
    //         pid = fork();
    //         if(pid!=0){
    //             waitpid(pid,&status,0);
    //             close(0);
    //             close(1);
    //             dup2(pipe1[0],0);
    //             dup2(pipe2[1],1);
    //             execlp(proglist[1],proglist[1],NULL);
    //         }else{
    //             close(1);
    //             dup2(pipe1[1],1);
    //             execlp(proglist[0],proglist[0],NULL);
    //         }
    //     }
    // }
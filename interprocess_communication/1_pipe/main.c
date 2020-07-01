#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/*

Warning: The pipe is simplex communication.You should use two pipe if you want implement
	 duplex communication.

*/

int main(int argc, char* argv[])
{
	int pipefd[2];
	pid_t cpid;
	char buf;
	if(-1 == pipe(pipefd)){
		perror("pipe");
		exit(EXIT_FAILURE);
	}
	cpid = fork();
	if(-1 == cpid){
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if(0 == cpid){/*This is Child process.*/
		printf("this is child process.\r\n");
		close(pipefd[1]);
		if(read(pipefd[0],&buf,sizeof(buf)) != 0){
			write(STDOUT_FILENO,&buf,sizeof(buf));
		}
		write(STDOUT_FILENO,"\n",1);
		close(pipefd[0]);
		_exit(EXIT_SUCCESS);
	}else{ /*This is parent.*/
		printf("this is parent process.\r\n");
		close(pipefd[0]);
		write(pipefd[1],"h",sizeof("h"));
		wait(NULL);
		close(pipefd[1]);
		exit(EXIT_SUCCESS);
	}
	return 0;
}

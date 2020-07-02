#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

static const char file_name[] = "./fifo";

//#define WRITER_OR_NOT 
#ifdef WRITER_OR_NOT
/*
    this is the writer's program.
*/
int main(int argc, char* argv[])
{
    int pipe_fd;
    int ret;
    pipe_fd = open(file_name,O_RDWR);
    if(pipe_fd == -1){
        perror("error when open the pipe\r\n");
        exit(EXIT_FAILURE);
    }
    ret = write(pipe_fd,"hello this is writer.\r\n",sizeof("hello this is writer.\r\n"));
    if(ret == -1){
        perror("error when writing the pipe\r\n");
        close(pipe_fd);
        exit(EXIT_FAILURE);
    }
    printf("process %d had finished!\r\n", getpid());
    exit(EXIT_SUCCESS);
}

#else

/*
    this is the reader's program.
*/
int main(int argc, char* argv[])
{
    int pipe_fd;
    int ret;
    size_t count;
    char buf[100];
    /*Verify the FIFO exists.*/
    if(access(file_name,F_OK) == -1)
    {
        ret = mkfifo(file_name,0660);
        if (ret != 0)
        {
            perror("mkfifo fault\r\n");
            exit(EXIT_FAILURE);
        }
    }
    if((pipe_fd = open(file_name,O_RDONLY)) == -1){
        perror("error when open the pipe\r\n");
        exit(EXIT_FAILURE);
    }
    count=read(pipe_fd,buf,sizeof(buf));
    buf[count] = '\0';
    printf("read from writer: %s\r\n", buf);
    close(pipe_fd);
    printf("process %d had finished!\r\n",getpid()); 
    sleep(5);   
    exit(EXIT_FAILURE);
}

#endif


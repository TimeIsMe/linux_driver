#include "stdio.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"
#include "poll.h"
#include "sys/select.h"
#include "sys/time.h"
#include "linux/ioctl.h"


#define LEDOFF  0
#define LEDON   1

#define KEY0VALUE   0xF0
#define INVAKEY     0x00

static volatile unsigned char flag;

static int fd_g = 0;

void sighandle(int dunno){
    if(2 == dunno){
        flag = 1;
    }
}

void sigio_signal_func(int signum)
{
    int err = 0;
    unsigned int keyvalue = 0;
    err = read(fd_g, &keyvalue, sizeof(keyvalue));
    if(err < 0){
        printf("read error\r\n");
    }else{
        printf("sigio signal! key value=%d\r\n",keyvalue);
    }
}

int main(int argc, char *argv[]){
    char *filename;
    int flags;
    filename = argv[1];

    fd_g = open(filename,O_RDWR);
    if(fd_g<0){
        printf("file %s open failed!\r\n",filename);
        return -1;
    }
    signal(SIGINT, sighandle);
    signal(SIGIO, sigio_signal_func);
    fcntl(fd_g, F_SETOWN, getpid());
    flags = fcntl(fd_g, F_GETFD);
    fcntl(fd_g, F_SETFL, flags | FASYNC);

    while(1){
        sleep(2);
        if(1 == flag){
            break;
        }
    }
    printf("close fd\r\n");
    close(fd_g);
    return 0;

}
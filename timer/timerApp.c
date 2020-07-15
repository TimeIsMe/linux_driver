#include "stdio.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "linux/ioctl.h"
#include "signal.h"

#define LEDOFF  0
#define LEDON   1
#define CLOSE_CMD   (_IO(0xEF, 0x1))
#define OPEN_CMD    (_IO(0xEF, 0x2))
#define SETPERIOD_CMD (_IO(0xEF, 0x3))

static volatile int flag = 0;

void sigCallBack(int arg)
{
    if(2 == arg){
        flag = 1;
    }
}

int main(int argc, char *argv[]){
    int fd, retval;
    char *filename;
    unsigned int cmd;
    unsigned int arg;

    filename = argv[1];

    fd = open(filename,O_RDWR);
    if(fd<0){
        printf("file %s open failed!\r\n",filename);
        return -1;
    }
    signal(SIGINT, sigCallBack);
    while(1){
        printf("***CMD list***\r\n1 --- close led\r\n2 --- open led\r\n3 --- set period\r\n");
        printf("please input CMD:");
        scanf("%d", &cmd);
        if(1 == cmd){
            cmd = CLOSE_CMD;
        }else if(2 == cmd){
            cmd = OPEN_CMD;
        }else if(3 == cmd){
            cmd = SETPERIOD_CMD;
            printf("\tplease input timer period:");
            scanf("%d", &arg);
        }else{
            printf("illegal input!\r\n");
        }
        ioctl(fd, cmd, arg);
        if(1 == flag){
            break;
        }
    }

    close(fd);
    return 0;
}
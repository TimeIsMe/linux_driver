#include "stdio.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"
#include "poll.h"
#include "sys/select.h"
#include "sys/time.h"


#define LEDOFF  0
#define LEDON   1

#define KEY0VALUE   0xF0
#define INVAKEY     0x00

static volatile unsigned char flag;

void sighandle(int dunno){
    if(2 == dunno){
        flag = 1;
    }
}

int main(int argc, char *argv[]){
    int fd, retval;
    char *filename;
    char *option;
    unsigned char keyvalue;
    struct pollfd fds;
    fd_set readfds;
    struct timeval timeout;
    unsigned char data;
    flag = 0;
    signal(SIGINT, sighandle);
    filename = argv[1];

    fd = open(filename,O_RDWR | O_NONBLOCK);
    if(fd<0){
        printf("file %s open failed!\r\n",filename);
        return -1;
    }

    while(1){
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;
        retval = select(fd+1, &readfds, NULL, NULL, &timeout);
        switch(retval){
            case 0:
                printf("case :0\r\n");
                break;
            case -1:
                printf("case -1\r\n");
                break;
            default:
                if(FD_ISSET(fd, &readfds)){
                    retval = read(fd, &data, sizeof(data));
                    if(retval < 0){
                        printf("read error\r\n");
                    }else{
                        if(data){
                            printf("key value = %d\r\n", data);
                        }
                    }
                }else if(retval == 0){
                    printf("FD_ISSET: time out\r\n");
                }else if(retval < 0){
                    printf("FD_ISSET: error\r\n");
                }
        }
        if(1 == flag){
            break;
        }
    }
    printf("close fd\r\n");
    close(fd);
    return 0;

}
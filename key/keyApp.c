#include "stdio.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "signal.h"

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
    flag = 0;
    signal(SIGINT, sighandle);
    filename = argv[1];

    fd = open(filename,O_RDWR);
    if(fd<0){
        printf("file %s open failed!\r\n",filename);
        return -1;
    }

    while(1){
        read(fd, &keyvalue, sizeof(keyvalue));
        if(keyvalue == KEY0VALUE){
            printf("KEY0 Pressed, value = %#X\r\n", keyvalue);
        }
        if(1 == flag){
            break;
        }
    }
    close(fd);
    return 0;

}
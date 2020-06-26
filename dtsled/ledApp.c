#include "stdio.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

#define LEDOFF  0
#define LEDON   1

/**
  * @Function main
  * @Author   wml
  * @Brief         Thit is a Linux device driver 
  *                test program, you should compi
  *                le this file and run in your c
  *                orresponding template. The fir
  *                st step command in shell: $(CC
  *                ) testApp.c -o testApp.out. Th
  *                e second step command in corre
  *                sponding template shell: ./tes
  *                tApp.out /dev/myled 1 or ./tes
  *                tApp.out /dev/myled 0.
  * @Param    argc The number of your input param
  *                ters.
  * @Param    argv The content of you had input.
  * @Retval   int  Is your program working proper
  *                ly?
  */
int main(int argc, char *argv[]){
    int fd, retval;
    char *filename;
    char *option;
    unsigned char databuf[1];

    filename = argv[1];
    option = argv[2];

    fd = open(filename,O_RDWR);
    if(fd<0){
        printf("file %s open failed!\r\n",filename);
        return -1;
    }

    if(0 == strcmp("read",option)){
        printf("read option!\r\n");
        retval = read(fd,databuf,sizeof(databuf));
        if(retval<0){
            printf("led status read failed!\r\n");
            goto err;
        }
        if(0x00 == databuf[0]){
            printf("led status is on\r\n");
        }else{
            printf("led status is off\r\n");
        }
    }else if(0 == strcmp("write",option)){
        printf("write option!\r\n");
        if(argc<4){
            printf("missing parameters\r\n");
            goto err;
        }
        databuf[0] = atoi(argv[3]);
        retval = write(fd, databuf, sizeof(databuf));
        if(retval < 0){
            printf("led control failed!\r\n");
            goto err;
        }
    }else{
        printf("illegal option!\r\n");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
err:
    close(fd);
    return -1;
}
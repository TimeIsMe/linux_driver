#include "stdio.h"
#include "stdlib.h"
#include "fcntl.h"
#include "unistd.h"

#define LED_ON  1
#define LED_OFF 0

int main(int argc, char* argv[])
{
    int fd, retval;
    char *filename;
    unsigned char databuf[1];

    if(3 != argc){
        printf("Error Usage!\r\n");
        return -1;
    }
    filename = argv[1];
    
    fd = open(filename, O_RDWR);
    if(fd < 0){
        printf("file %s open failed!\r\n", filename);
        return -1;
    }

    databuf[0] = atoi(argv[2]);
    retval = write(fd, databuf, 1);
    if(retval < 0 ){
        printf("LED control failed!\r\n");
        close(fd);
        return -1;
    }
    retval = close(fd);
    if(retval < 0){
        printf("file %s close failed!\r\n", argv[1]);
        return -1;
    }
    return 0;
}
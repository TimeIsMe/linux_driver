#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

//If defined SWITCH_PRO will compile the program 1, else program 2.
//#define SWITCH_PRO

struct msgbuf{
    long mtype;
    char mtext[20];
};



int main(int argc, char* argv[])
{
    int id = 0;
    key_t mkey;
    struct msgbuf buf;
    if(2 != argc){
        perror("please input the message queue id!\r\n");
        exit(EXIT_FAILURE);
    }
    id = atoi(argv[1]);
    if((id<0) || (id>255)){
        perror("the id must be in 0-255.\r\n");
        exit(EXIT_FAILURE);
    }
    mkey = ftok("./",id);
    if(-1 == mkey){
        perror("error while execute ftok.\r\rn");
        exit(EXIT_FAILURE);
    }
    //The id become message id after all these.
    if((id = msgget(mkey, IPC_CREAT | 0666)) == -1){
        perror("error while execute msgget.\r\n");
        exit(EXIT_FAILURE);
    }
    buf.mtype = 1;

#ifdef SWITCH_PRO
//program sender.
    strcpy(buf.mtext,"hello world.");
    if(-1 == msgsnd(id, (void*)&buf, sizeof(buf.mtext), 0)){
        perror("error while sending message.\r\n");
        exit(EXIT_FAILURE);
    }
    printf("This is sender.\r\nhad send %s\r\n",buf.mtext);
    sleep(3);
    if(0 > msgctl(id, IPC_RMID, NULL)){
        perror("error while execute msgctl.\r\n");
        exit(EXIT_FAILURE);
    }

#else
    
    if(-1 == msgrcv(id, (void*)&buf, sizeof(buf.mtext), buf.mtype, 0)){
        perror("error while executing msgrcv.\r\n");
        exit(EXIT_FAILURE);
    }
    printf("This is reader.\r\nhad read %s\r\n",buf.mtext);
#endif
    sleep(3);
    exit(EXIT_SUCCESS);

}


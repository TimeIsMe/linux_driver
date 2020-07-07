#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>



#define SHM_SIZE 1024
struct shared_memory_struct{
    int written;
    char text[SHM_SIZE];
};


//#define READER_OR_WRITER
#ifdef READER_OR_WRITER

/*
    You should input the shared memory key from the terminal.
*/
int main(int argc, char* argv[])
{
    int shm_id;
    void *shm = NULL;
    struct shared_memory_struct *shm_str;
    if(argc != 2){
        perror("please type ./reader key\r\n");
        exit(EXIT_FAILURE);
    }
    shm_id = atoi(argv[1]);
    shm_id = shmget((key_t)shm_id, sizeof(struct shared_memory_struct), IPC_CREAT|0666);
    if(-1 == shm_id){
        perror("error while executing shmget!\r\n");
        exit(EXIT_FAILURE);
    }
    shm = shmat(shm_id, NULL, 0);
    if((void*)-1 == shm){
        perror("error while executing shmat!\r\n");
        exit(EXIT_FAILURE);
    }
    printf("Memory attached at 0x%X\r\n", (unsigned int)(long)shm);
    shm_str = (struct shared_memory_struct*)shm;
    while(1){
        if(shm_str->written){
            printf("had read:%s\r\n", shm_str->text);
            if(!strcmp(shm_str->text, "quit")){
                goto quit;
            }
            shm_str->written = 0;
        }        
        sleep(1);
    }
quit:
    //To separate the shared memory from program.
    if(shmdt(shm) == -1){
        perror("error while executing shmdt!\r\n");
        exit(EXIT_FAILURE);
    }

    if(-1 == shmctl(shm_id, IPC_RMID, 0)){
        perror("error while executing shmctl!\r\rn");
        exit(EXIT_FAILURE);
    }
    return 0;
}


#else

int main(int argc, char* argv[])
{
    int shm_id;
    void* shm = NULL;
    struct shared_memory_struct *shm_str;
    if(3 != argc){
        perror("please type: ./writer key content\r\n");
        exit(EXIT_FAILURE);
    }
    shm_id = atoi(argv[1]);
    shm_id = shmget((key_t)shm_id, sizeof(struct shared_memory_struct), IPC_CREAT);
    if(-1 == shm_id){
        perror("error while executing shmget!\r\n");
        exit(EXIT_FAILURE);
    }
    shm = shmat(shm_id, NULL, 0);
    if(NULL == shm){
        perror("error while executing shmat!\r\n");
        exit(EXIT_FAILURE);
    }
    shm_str = (struct shared_memory_struct*)shm;
    if(0 == shm_str->written){
        strcpy(shm_str->text,argv[2]);
        shm_str->written = 1;
        printf("had wrote: %s\r\n", shm_str->text);
    }else{
        printf("the memory is busying!\r\n");
    }
    if(-1 == shmdt(shm)){
        perror("error whiling executing shmdt!\r\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}


#endif
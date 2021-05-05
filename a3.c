#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define FIFO_NAME_RESP "RESP_PIPE_27208"
#define FIFO_NAME_REQ "REQ_PIPE_27208"
int sze;
void pingPong(int fd2){
        unsigned char len=strlen("PING");
        write(fd2, &len,1);
        write(fd2, "PING", len);

        unsigned char len1=strlen("PONG");
        write(fd2, &len1,1);      
        write(fd2, "PONG", len1);

        unsigned int number=27208;
        write(fd2, &number, sizeof(number));
}

char* createShm(int fd1, int fd2, char*request){
    unsigned int oct=0;
    if( read(fd1, &oct, sizeof(unsigned int)) <0){
        perror("Could not read from input file\n");
        close(fd1);
        return NULL;
    }
    int shmFd=-1;
    shmFd = shm_open("/siiQaeX", O_CREAT | O_RDWR, 0664);
    ftruncate(shmFd, oct);
    if(shmFd < 0) {
        perror("Could not aquire shm");
        return NULL;
    }
    char *sharedChar = (char*)mmap(0, oct, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    unsigned char len=strlen(request);
    write(fd2, &len,1);
    write(fd2, request, len);

    if(sharedChar == (void*)-1){
        perror("Could not map the shared memory");
        unsigned char len1=strlen("ERROR");
        write(fd2, &len1,1);
        write(fd2, "ERROR", len1);
        return NULL;
    }else{
        unsigned char len2=strlen("SUCCESS");
        write(fd2, &len2,1);
        write(fd2, "SUCCESS", len2);
    }
    return sharedChar;
}

void writeToShm(int fd1, int fd2, char *sharedMem, char *request){
    unsigned int offset=0;
    if( read(fd1, &offset, sizeof(unsigned int)) <0){
        perror("Could not read from input file\n");
        close(fd1);
    }
    unsigned int value=0;
    if( read(fd1, &value, sizeof(unsigned int)) <0){
        perror("Could not read from input file\n");
        close(fd1);
    }
    unsigned char len=strlen(request);
    write(fd2, &len,1);
    write(fd2, request, len);

    unsigned char len1=strlen("SUCCESS");
    unsigned char len2=strlen("ERROR");

    if(offset>=0 && offset<=5168754 && offset+sizeof(value)<=5168754){
        if(memcpy(sharedMem+offset,&value,sizeof(value))<0){
            write(fd2, &len2,1);
            write(fd2, "ERROR", len2);
        }else{
            write(fd2, &len1,1);
            write(fd2, "SUCCESS", len1);
        }
    }else{
        write(fd2, &len2,1);
        write(fd2, "ERROR", len2);
    }
}

char* mapFile(int fd1, int fd2, char *request){
    unsigned char sz=0;
    unsigned char len=strlen(request);
    unsigned char len1=strlen("SUCCESS");
    unsigned char len2=strlen("ERROR");

    if( read(fd1, &sz, sizeof(unsigned char)) <0){
        close(fd1);
    }
    char *fName=(char*)malloc((sz+1)*sizeof(char));
    read(fd1, fName, sz);

    write(fd2, &len,1);
    write(fd2, request, len);
    int fd=-1;
    fd = open(fName, O_RDONLY);
    if(fd == -1) {
        write(fd2, &len2, 1);
        write(fd2, "ERROR", len2);
        return NULL;
    }
    sze = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char *data = (char*)mmap(NULL, sze, PROT_READ, MAP_SHARED, fd, 0);
    
    if(data == (void*)-1) {
        close(fd);
        write(fd2, &len2, 1);
        write(fd2, "ERROR", len2);
    }else{
        write(fd2, &len1,1);
        write(fd2, "SUCCESS", len1);
    }
    return data;
    close(fd);
}

void readFromFileOffs(int fd1, int fd2,char *file, char* request, char*sharedMem){
    unsigned char len=strlen(request);
    unsigned char len1=strlen("SUCCESS");
    unsigned char len2=strlen("ERROR");

    unsigned int offset=0;
    read(fd1, &offset, sizeof(unsigned int));

    unsigned int noB=0;
    read(fd1, &noB, sizeof(unsigned int));

    write(fd2, &len,1);
    write(fd2, request, len);
    if(offset+noB > sze){
        write(fd2, &len2, 1);
        write(fd2, "ERROR", len2);
    }else{
        if(memcpy(sharedMem,file+offset,noB)<0){
            write(fd2, &len2, 1);
            write(fd2, "ERROR", len2);
        }else{
            write(fd2, &len1, 1);
            write(fd2, "SUCCESS", len1);
        }
    }
    //free(file);
    //munmap(file, sze);
}

int main(int argc, char **argv) {

    if(access(FIFO_NAME_RESP,0)==0){
        unlink(FIFO_NAME_RESP);
    }
    if(mkfifo(FIFO_NAME_RESP, 0664) != 0) {
        printf("ERROR\ncannot create the response pipe\n");
        return 1;
    }
    int fd1 = open(FIFO_NAME_REQ, O_RDONLY);
    if(fd1 == -1) {
        printf("ERROR\ncannot open the request pipe\n");
        return 1;
    }
    int fd2 = open(FIFO_NAME_RESP, O_WRONLY);
    if(fd2 == -1) {
        printf("ERROR\ncannot open the response pipe\n");
        return 1;
    }
    
    //2.2 Conexiunea prin pipe
    unsigned char len=strlen("CONNECT");
    write(fd2, &len,1);
    write(fd2, "CONNECT", len);
        
    char *sharedMem=NULL;
    char *file=NULL;
    for(;;){
        unsigned char size=0;
        if(read(fd1, &size, 1) != 1) {
            perror("Reading error");
            close(fd1);
            return 2;
        }
        char *request=(char*)malloc(size*sizeof(char));
        if(read(fd1, request, size*sizeof(char)) < 0){
            perror("Could not read from input file\n");
            close(fd1);
            return 2;
        }
        //2.3 Request PING 
        if(strcmp(request,"PING")==0){
            pingPong(fd2);
            request=NULL;
            free(request);

        //2.4 Request creare regiune de memorie partajata
        }else if(strcmp(request,"CREATE_SHM")==0){
            sharedMem =createShm(fd1,fd2,request);
            request=NULL;
            free(request);

        //2.5 Request scriere in zona de memorie partajata
        }else if(strcmp(request,"WRITE_TO_SHM")==0){
            writeToShm(fd1,fd2,sharedMem,request);
            request=NULL;
            free(request);

        //2.6 Request mapare fisier in memorie
        }else if(strcmp(request,"MAP_FILE")==0){
            file=mapFile(fd1, fd2, request);
            request=NULL;
            free(request);

        //2.7 Request citire de la un offset din fisier
        }else if(strcmp(request,"READ_FROM_FILE_OFFSET")==0){
            readFromFileOffs(fd1, fd2, file, request, sharedMem);
            request=NULL;
            free(request);

        //2.10 Request EXIT
        }else if(strcmp(request,"EXIT")){
            close(fd1);
            close(fd2);
            unlink(FIFO_NAME_RESP);
            request=NULL;
            free(request);
            break;
        }else{
            break;
        }
    }
    return 0;
}
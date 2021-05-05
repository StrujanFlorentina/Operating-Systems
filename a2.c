#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

int pid2, pid3, pid4, pid5, pid6, pid7;
pthread_mutex_t mutex1, mutex2, m , mutex3, mutex4;
sem_t semaphore;
int nrTh,done,pornit,total=0,pornitDeAltul, thOprit;
pthread_cond_t cond;

typedef struct thread_arg_t
{
	int process;
	int thread;
} INFO;

void thP47(INFO *thInfo)
{
	if (thInfo->process == 7 && thInfo->thread == 4)//A.7,4 incepe dupa 7,1
		pthread_mutex_lock(&mutex1);

	if (thInfo->process == 7 && thInfo->thread == 2)//B.7,2 porneste dupa ce se termina 4,3
		pthread_mutex_lock(&mutex3);
	if (thInfo->process == 4 && thInfo->thread == 1)//B.4,1 porneste dupa de se termina 7,2
		pthread_mutex_lock(&mutex4);


	info(BEGIN, thInfo->process, thInfo->thread);
	if (thInfo->process == 7 && thInfo->thread == 1)//A.7,1 incepe primul
		pthread_mutex_unlock(&mutex1);
	if (thInfo->process == 7 && thInfo->thread == 1)//A.7,1 se termina dupa 7,4
		pthread_mutex_lock(&mutex2);
	info(END, thInfo->process, thInfo->thread);
	if (thInfo->process == 7 && thInfo->thread == 4)//A. 7,4 se termina primul
		pthread_mutex_unlock(&mutex2);


    if (thInfo->process == 4 && thInfo->thread == 3)//B.4,3 se termina inainte sa inceapa 7,2
		pthread_mutex_unlock(&mutex3);
	if (thInfo->process == 7 && thInfo->thread == 2)//B.7,2 se termina inainte sa inceapa 4,1
		pthread_mutex_unlock(&mutex4);
}

void create_threads_P4_P7(pthread_t t[], INFO thInfo[], int nr)
{
    for (int i = 0; i < nr; i++){
        pthread_create(&t[i], NULL, (void *(*)(void *))thP47, (void *)&thInfo[i]);
    }
    for (int i = 0; i < nr; i++){
        pthread_join(t[i], NULL);
    }
}

void thP2(INFO *thInfo){
    sem_wait(&semaphore);

    if(thInfo->thread != 13){
        pthread_mutex_lock(&m);
        nrTh++;
        pthread_mutex_unlock(&m);
        info(BEGIN, 2, thInfo->thread);
    }
    /*if((thInfo->thread != 13) ||(thInfo->thread==13 && pornit==0)){
        pthread_mutex_lock(&m);
        nrTh++;
        total++;
        if(thInfo->thread==13){
            pornit=1;
        }
        pthread_mutex_unlock(&m);
        info(BEGIN, 2, thInfo->thread);
    }

    if(total==30 && pornit!=1){
        pthread_mutex_lock(&m);
        nrTh++;
        total++;
        pornit=1;
        pornitDeAltul=1;
        pthread_mutex_unlock(&m);
        info(BEGIN, 2, 13);
    }
    */
    
    if(thInfo->thread!=13){
        pthread_mutex_lock(&m);
        if(nrTh==5 && done==0){
            info(BEGIN, 2, 13);
            info(END, 2 ,13);
            done=1;
        }
        pthread_mutex_unlock(&m);
        pthread_mutex_lock(&m);
        nrTh--;
        pthread_mutex_unlock(&m);
        info(END, 2, thInfo->thread);
    }
    /*pthread_mutex_lock(&m);
    
    if(nrTh==6 && done==0 && pornit==1){
        pthread_cond_signal(&cond);
        done=1;
    }
    if((thInfo->thread==13 && nrTh<6 && pornitDeAltul==0 && done==0)){//|| pornit==0
        pthread_cond_wait(&cond,&m);
    }
    nrTh--;
    info(END, 2, thInfo->thread);

    pthread_mutex_unlock(&m);
    */
    sem_post(&semaphore);
}

void create_threads_P2(pthread_t t[], INFO thInfo[], int nr)
{
    sem_init(&semaphore, 0, 6);
    pthread_mutex_init(&m,NULL);
    pthread_cond_init(&cond, NULL);

    for (int i = 0; i < nr; i++){
        pthread_create(&t[i], NULL, (void *(*)(void *))thP2, (void *)&thInfo[i]);
    }
    for (int i = 0; i < nr; i++){
        pthread_join(t[i], NULL);
    }
}

int create_process(int id){
    int pid=fork();
    if(pid==0){
        info(BEGIN,id,0);
        if(id==2){
            pid3=create_process(3);
            
            pthread_t threads[35];
            INFO args[35];
            for(int i=1; i<=35; i++){
                args[i-1].process=2;
                args[i-1].thread=i;
            }
            create_threads_P2(threads, args, 35);

            waitpid(pid3,0,0);
        }else if(id==3){
            pid4=create_process(4);
            pid5=create_process(5);
            waitpid(pid4,0,0);
            waitpid(pid5,0,0);
        }else if(id==4){
            waitpid(pid7,0,0);
        }else if(id==5){
            pid7=create_process(7);
            waitpid(pid7,0,0);
        }else if(id==7){
            pthread_mutex_init(&mutex1, NULL);
            pthread_mutex_init(&mutex2, NULL);
            pthread_mutex_init(&mutex3, NULL);
            pthread_mutex_init(&mutex4, NULL);
            pthread_t threads[10];

            pthread_mutex_lock(&mutex1);
            INFO args[10]={{7,1},{7,2},{7,3},{7,4},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6}};

            pthread_mutex_lock(&mutex2);
            pthread_mutex_lock(&mutex3);
            pthread_mutex_lock(&mutex4);
            create_threads_P4_P7(threads,args,10);  
            waitpid(pid4,0,0);
        }
        info(END,id,0);
        exit(0);
    }else
        return pid;
}
int main(){
    init();

    info(BEGIN, 1, 0);

    pid2=create_process(2);
    pid6=create_process(6);
    waitpid(pid2,0,0);
    waitpid(pid6,0,0);

    info(END, 1, 0);

    sem_destroy(&semaphore);
    pthread_mutex_destroy(&m);
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    pthread_mutex_destroy(&mutex3);
    pthread_mutex_destroy(&mutex4);
    pthread_cond_destroy(&cond);

    return 0;
}
#ifndef HEADER_H_INCLUDED

#define HEADER_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/mman.h>
#include <mqueue.h>
#include <sys/msg.h>
#include <regex.h>
#include <sys/timeb.h>
#include <semaphore.h>



struct timeb t_inicio;

#define SEM_LOG "SEM_LOG"

#define MAX_C 150
#define PIPE "pipe"
#define SIZE 200


typedef struct vooA{
    char *nome;
    int init;
    int eta;
    int fuel;
    struct vooA *prox;

}t_vooA;



typedef struct vooD{
    char *nome;
    int init;
    int takeoff;
    struct vooD *prox;

}t_vooD;


typedef struct configs{
    int ut;
    int dDescola;
    int intDescola;
    int dAterra;
    int intAterra;
    int holdMin;
    int holdMax;
    int maxpart;
    int maxchega;
}t_config;

typedef struct queueA{
    int slot_shm;
    struct queueA *prox;
}t_queueA;


typedef struct queueD{
    int slot_shm;
    struct queueD *prox;
}t_queueD;

typedef struct stats{
    int nVoos;
    int nAterragens;
    int tempomedioAterrar;
    int nDescolagens;
    int tempomedioDescolar;
    int nmedioHoldings;
    int nmedioHoldings_urgentes;
    int nRedirecionados;
    int rejeitados;
}t_stats;


typedef struct message{
    long mtype;
    int id; 
    int eta;
    int fuel;
    int takeoff;
    int tipo; //departure 1, arrival 2
    int slot_shm;
}t_message;

typedef struct comms{
    int id;
    int eta;
    int fuel;
    int takeoff;
    int tipo; //departure 1, arrival 2
    int isCompleted; //0 incomplete, 1 complete
}t_comms; //shared memory para instruções

typedef struct cabeca_queues{
    t_queueD *D;
    t_queueA *A;
}t_cabecasqueue;



t_comms *arrayshm;


t_config *configs;
t_stats *stats;

sem_t *escreve_log;


pthread_mutex_t readpipe = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread_tempo;

pthread_t thread_check;
pthread_t tc_msq;
pthread_t tc_managefuel;

pthread_t voosChegada[1000], voosPartida[1000];


int ut;
int current_time = 0;
int shmid;
int mqid;
int fdpipe;
char comando[SIZE];
#endif // HEADER_H_INCLUDED


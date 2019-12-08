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
#define SEM_ARR "SEM_ARRAY"
#define SEM_STATS "SEM_STATS"
#define SEM_DEP "SEM_DEP"
#define SEM_ARRI "SEM_ARR"
#define SEM_ARRIVAL_EMPTY "EMPTY"
#define SEM_ARRIVAL_FULL "FULL"

#define SEM_A1 "SEM_A1"
#define SEM_A2 "SEM_A2"
#define SEM_D1 "SEM_D1"
#define SEM_D2 "SEM_D2"
#define SEM_LISTA "SEM_LISTA"

#define SEM_BUSY "SEM_BUSY"
#define SEM_BROAD "SEM_BROAD"
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


typedef struct message{
    long mtype;
    int id; 
    int eta;
    int fuel;
    int init;
    int takeoff;
    int tipo; //departure 1, arrival 2
    int slot_shm;
    int emergency; //emergency 1, not emergency 0
}t_message;

typedef struct comms{
    int id;
    int init;
    int eta;
    int fuel;
    int takeoff;
    int tipo; //departure 1, arrival 2
    int command; // 1 aterrar, 2 hold, 3 desvio, 4 descolar
    int pista; // 1- A1, 2-A2, 3-D1, 4-D2
    int hold_time;
    int isCompleted; //0 incomplete, 1 complete
    int emergency;
}t_comms; //shared memory para instruções

typedef struct cabeca_queues{
    t_queueD *D;
    t_queueA *A;
}t_cabecasqueue;



typedef struct stats{
    t_comms *ptrArray;
    int nVoos;
    int nAterragens;
    int tempomedioAterrar;
    int nDescolagens;
    int tempomedioDescolar;
    int nmedioHoldings;
    int nmedioHoldings_urgentes;
    int nRedirecionados;
    int rejeitados;
    int nUrgentes;
}t_stats;

t_comms *arrayshm;
t_config *configs;
t_stats *stats;
sem_t *escreve_log;
sem_t *sem_array;
sem_t *sem_stats;
sem_t *sem_pistas;
sem_t *sem_arrival_empty;
sem_t *sem_arrival_full;

sem_t *sem_pistaa1;
sem_t *sem_pistaa2;
sem_t *sem_pistad1;
sem_t *sem_pistad2;

sem_t *sem_lista;
sem_t *sem_broadcast;
sigset_t sigs;

pthread_mutex_t readpipe = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_ordem = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_ordem = PTHREAD_COND_INITIALIZER;

pthread_t thread_tempo;
pthread_t thread_tempoTC;
pthread_t thread_check;
pthread_t tc_msq;
pthread_t tc_managefuel;
pthread_t tc_gere_arrivals;
pthread_t tc_gere_departures;
pthread_t tc_busyManagement;
pthread_t voosChegada[1000], voosPartida[1000];
pthread_t broadcasts;

FILE *fp;
int ut;
int current_time = 0;
int time_departure;
int shmid2;
int shmid;
int mqid;
int fdpipe;
char comando[SIZE];
int isBusy;


#endif // HEADER_H_INCLUDED


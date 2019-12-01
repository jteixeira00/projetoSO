//Dinis Silva Costa Carvalho, nº 2018278118
//João Pimentel Roque Rodrigues Teixeira, nº 2018278532

#include "header.h"
t_vooA *cabeca_vooA;
t_vooD *cabeca_vooD;


t_vooA *cria_cabecalhovooA(void){
    t_vooA *lista = (t_vooA*)malloc(sizeof(t_vooA));
    if (lista != NULL){
        lista->nome = NULL;
        lista->init = -1;
        lista->eta = -1;
        lista->fuel = -1;
        lista->prox = NULL;
    }
    return lista;
}

t_vooD *cria_cabecalhovooD(void){
    t_vooD *lista = (t_vooD*)malloc(sizeof(t_vooD));
    if (lista != NULL){
        lista->nome = NULL;
        lista->init = -1;
        lista->takeoff= -1;
        lista->prox = NULL;
    }
    return lista;
}

t_queueD* cria_cabecalhoqueueD(void){
	t_queueD *lista = (t_queueD*)malloc(sizeof(t_queueD));
	if(lista!=NULL){
		lista->slot_shm = -1;
		lista->prox = NULL;
	}


    return lista;
}

t_queueA* cria_cabecalhoqueueA(void){
    t_queueA *lista = (t_queueA*)malloc(sizeof(t_queueA));
    if(lista!=NULL){
        lista->slot_shm = -1;
        lista->prox = NULL;
    }
    return lista;
}



void criarLog(){

    FILE *fp;
    fp=fopen("log.txt", "w");
    if(fp == NULL){
        printf("Error creating log file\n");
        exit(1);
    }
    fclose(fp);
}

void insereOrdenadoD(char *nome, int init, int takeoff, t_vooD *cabeca_vooD){


    t_vooD *node = cabeca_vooD;
    t_vooD *nodeNovo = (t_vooD*)malloc(sizeof(t_vooD));

    while((node->prox!=NULL)&&(node->prox->init<init)){
        node = node->prox;
    }
    nodeNovo->nome = strdup(nome);
    nodeNovo->init = init;
    nodeNovo->takeoff = takeoff;
    nodeNovo->prox = node->prox;
    node->prox = nodeNovo;
}

void insereOrdenadoA(char *nome, int init, int eta, int fuel, t_vooA *cabeca_vooA){
    

    t_vooA *node = cabeca_vooA;
    t_vooA *nodeNovo = (t_vooA*)malloc(sizeof(t_vooA));
    
    while((node->prox!=NULL)&&(node->prox->init<init)){
        node = node->prox;
    }
    nodeNovo->nome = strdup(nome);
    nodeNovo->init = init;
    nodeNovo->eta = eta;
    nodeNovo->fuel = fuel;
    nodeNovo->prox = node->prox;
    node->prox = nodeNovo;
}


void escreverEcra(char message[]){
	struct tm *timeInfo;
    time_t raw = time(NULL);
    char buffer[50];
    timeInfo=gmtime(&raw);
    strftime(buffer, 50, "%H:%M:%S",timeInfo);
    printf("%s %s", buffer, message);
    memset(buffer, 0, sizeof(buffer));

}

void escreverLog(char message[]){
    
    
    struct tm *timeInfo;
    time_t raw = time(NULL);
    char buffer[50];
    timeInfo=gmtime(&raw);
    strftime(buffer, 50, "%H:%M:%S",timeInfo);
    fprintf(fp, "%s %s", buffer, message);
    memset(buffer, 0, sizeof(buffer));
    
}


//lê o ficheiro config e coloca os valores na estrutura config
void lerConfig(){

    FILE *fp;
    char buffer[MAX_C];
    fp = fopen("config.txt", "r");

    if (fp==NULL){
        printf("Error opening config file\n");
        exit(1);
    }

    fgets(buffer, MAX_C, fp);
    configs->ut = atoi(strtok(buffer, "\n"));
    fgets(buffer, MAX_C, fp);
    configs->dDescola = atoi(strtok(buffer, ","));
    configs->intDescola= atoi(strtok(NULL, "\n"));
    fgets(buffer, MAX_C, fp);
    configs->dAterra = atoi(strtok(buffer, ","));
    configs->intAterra= atoi(strtok(NULL, "\n"));

    fgets(buffer, MAX_C, fp);
    configs->holdMin = atoi(strtok(buffer, ","));
    configs->holdMax= atoi(strtok(NULL, "\n"));
    fgets(buffer, MAX_C, fp);
    configs->maxpart = atoi(strtok(buffer, "\n"));
    fgets(buffer, MAX_C, fp);
    configs->maxchega = atoi(strtok(buffer, "\n"));

    fclose(fp);
}


void criarSHM(){

    char message[100];
    int maxvoos = configs->maxchega + configs->maxpart;
	shmid = shmget(IPC_PRIVATE, sizeof(t_comms)*maxvoos + sizeof(t_stats), IPC_CREAT|0666);
	sprintf(message, "Shared memory with ID %d created\n", shmid );
    sem_wait(escreve_log);

    escreverEcra(message);
    escreverLog(message);
    sem_post(escreve_log);
    
    arrayshm = (t_comms*)malloc(maxvoos * sizeof(t_comms));
    arrayshm = (t_comms*)shmat(shmid, NULL, 0);
    stats = (t_stats*)malloc(sizeof(t_stats));
    stats = (t_stats*)shmat(shmid, NULL, 0);   
    	
	//acrescentar error handling	
    }


void criarMQ(){
    char message[100];
	if((mqid = msgget(IPC_PRIVATE, IPC_CREAT|0777))<0){
		printf("Error creating message queue");
		exit(1);

	}
    sprintf(message, "Message Queue with ID %d created\n", shmid );
    sem_wait(escreve_log);
    escreverEcra(message);
    escreverLog(message);
    sem_post(escreve_log);
}

void criarPipe(){
    //Garante que o pipe não está pré-criado
	
	unlink(PIPE);
	remove(PIPE);
	// Cria o pipe
	
	if ((mkfifo(PIPE, O_CREAT|O_EXCL|0600) < 0) && (errno!=EEXIST)){
		perror("Cannot create pipe\n");
		exit(1);
	}
	
	// Coloca o pipe em modo de leitura - RDWR para evitar broken pipe
	if ((fdpipe = open(PIPE, O_RDWR)) < 0){
		perror("Cannot open pipe for reading\n");
		exit(1);
	}
}

int verificaD(char* string){
    regex_t regex;
    char nome[50];
    int init, takeoff;
    regcomp(&regex,"DEPARTURE TP[0-9]+ init: [0-9]+ takeoff: [0-9]+",REG_EXTENDED);
    int status = regexec(&regex, string, (size_t) 0, NULL, 0);
    sscanf(string, "DEPARTURE %s init: %d takeoff: %d", nome, &init, &takeoff);

    if((status==0) && (init >= current_time) && (takeoff >= init)){
        return 1;
    }
    else{

        return 0;
    }
}

int verificaA(char* string){
    regex_t regex;
    char nome[50];
    int init, eta, fuel;
    regcomp(&regex,"ARRIVAL TP[0-9]+ init: [0-9]+ eta: [0-9]+ fuel: [0-9]+",REG_EXTENDED);
    int status = regexec(&regex, string, (size_t) 0, NULL, 0);
    sscanf(string, "ARRIVAL %s init: %d eta: %d fuel: %d", nome, &init, &eta, &fuel);
    if((status==0) && (fuel >= eta) && (init >= current_time)){
        return 1;
    }
    else{
        return 0;
    }
}

void* lerMQ(void* cabeca){
	int i = 0;
	char str[100];
	t_cabecasqueue *cabecalho = (t_cabecasqueue*)cabeca;
	while(1){

    	t_message msg;
    
        if(msgrcv(mqid, &msg, sizeof(t_message), 99999, 0)==-1){
    		perror("Error na msgrcv memory\n");
            return 0;   		
    	}  

    	strcpy(str, "");
    	sprintf(str, "FLIGHT TP%d CONTACTED CONTROL TOWER FOR THE FIRST TIME\n", msg.id);
    	sem_wait(escreve_log);
    	escreverEcra(str);
    	escreverLog(str);
    	sem_post(escreve_log);
    	sem_wait(sem_stats);
    	stats->nVoos +=1;
    	sem_post(sem_stats);
        if(msg.tipo == 1){
        	sem_wait(sem_array);
            arrayshm[i].tipo = msg.tipo;
            arrayshm[i].takeoff = msg.takeoff;
            arrayshm[i].isCompleted = 0; 
            arrayshm[i].id = msg.id;

            t_queueD *node = cabecalho->D;
            t_queueD *nodeNovo = malloc(sizeof(t_queueD));
            while((node->prox!=NULL)&&(arrayshm[node->prox->slot_shm].takeoff<msg.takeoff)){
                node = node->prox;
            }
            sem_post(sem_array);
            nodeNovo->slot_shm = i;
            nodeNovo->prox = node->prox;
            node->prox = nodeNovo;
        }
        if(msg.tipo == 2){
        	sem_wait(sem_array);
            arrayshm[i].id = msg.id;
            arrayshm[i].eta = msg.eta;
            arrayshm[i].fuel = msg.fuel;
            arrayshm[i].tipo = msg.tipo;
            arrayshm[i].isCompleted=0;
            t_queueA *nodeA = cabecalho->A;
            t_queueA *nodeNovo = (t_queueA*)malloc(sizeof(t_queueA));
            while((nodeA->prox!=NULL)&&(arrayshm[nodeA->prox->slot_shm].eta<msg.eta)){
                nodeA = nodeA->prox;
            }
            sem_post(sem_array);
            nodeNovo->slot_shm = i;
            nodeNovo->prox = nodeA->prox;
            nodeA->prox = nodeNovo;
        }
    	msg.mtype = msg.id;
    	msg.slot_shm=i++;
    	msgsnd(mqid, &msg, sizeof(t_message), 0);
        
    }  
}

void *ManageFuel(void *cabeca){
    t_queueA *cabecaA = ((t_cabecasqueue*)cabeca)->A;
    char str[100];
    while(1){
        t_queueA *nodeA = cabecaA;
        while(nodeA->prox!=NULL){
        	sem_wait(sem_array);
            arrayshm[nodeA->prox->slot_shm].fuel -=1;
            if(arrayshm[nodeA->prox->slot_shm].fuel == 0){
    		strcpy(str,"");
		    sprintf(str,"FLIGHT TP%d FUEL LEVELS CRITICAL. DIVERTING FLIGHT TO NEAREST AIRPORT\n", arrayshm[nodeA->prox->slot_shm].id);
		    sem_wait(escreve_log);
		    escreverEcra(str);
		    escreverLog(str);
		    sem_post(escreve_log);
            arrayshm[nodeA->prox->slot_shm].isCompleted = 1;
        	nodeA->prox = nodeA->prox->prox;
        	sem_wait(sem_stats);
        	stats->nRedirecionados +=1;
        	sem_post(sem_stats);
        	}
        	sem_post(sem_array);
        	
            nodeA = nodeA->prox;
        }	
        usleep(ut*1000);
    }
}

void torreControlo(){
	sem_wait(escreve_log);
    escreverEcra("Control Tower created\n");
    escreverLog("Control Tower created\n");
    sem_post(escreve_log);
    t_queueA cabeca_queueA;
    cabeca_queueA.prox = NULL;
    t_queueD cabeca_queueD;
    cabeca_queueD.prox = NULL;
    t_cabecasqueue *heads = malloc(sizeof(t_cabecasqueue));
    heads->A = &cabeca_queueA;
    heads->D = &cabeca_queueD;
    if(pthread_create(&tc_msq, NULL, lerMQ, (void*)heads) != 0){
    	perror("Erro a criar a thread lerMQ\n");
    	exit(1);
    }
    if(pthread_create(&tc_managefuel, NULL, ManageFuel, (void*)heads) != 0){
        perror("Erro a criar a thread tc_managefuel\n");
        exit(1);
    }
    while(1);  
}

void lerPipe(){
	
	int verifier;
	char comando[100];
    
	char str[150];
    char nome[50];
    
    int init, eta, fuel, takeoff;
    t_vooA *nodeA = cabeca_vooA;
    t_vooD *nodeD = cabeca_vooD;

	while(1){ 			
		memset(str, 0, sizeof(str));
		fflush(stdin);	
		memset(comando, 0, sizeof(comando));
        char aux[2];
        memset(aux, 0, sizeof(aux));
        int k=0;

    	read(fdpipe,aux,1);
        while((strcmp(aux, "\n")!=0)){
            strcpy(comando+k, aux);
            k++;
            memset(aux, 0, sizeof(aux));
            read(fdpipe, aux, 1);

        }

		comando[100]='\0';
    	if(comando[0] =='D'){
    		verifier = verificaD(comando);
        }
        else{
            verifier = verificaA(comando);
        }
    	if (verifier==0){
    		strcpy(str, "");
    		sprintf(str, "WRONG COMMAND => %s\n", comando);
    		sem_wait(escreve_log);
            escreverEcra(str);
            escreverLog(str);
            sem_post(escreve_log);
    		strcpy(str, "");		
    	}
    	else{
    		strcpy(str, "");
    		sprintf(str, "NEW COMMAND => %s\n", comando);
            sem_wait(escreve_log);
    		escreverEcra(str);
    		escreverLog(str);
            sem_post(escreve_log);
            if(comando[0]=='A'){
                sscanf(comando, "ARRIVAL %s init: %d eta: %d fuel: %d", nome, &init, &eta, &fuel);
                insereOrdenadoA(nome, init, eta, fuel, cabeca_vooA);
                
            }
            else{
                sscanf(comando, "DEPARTURE %s init: %d takeoff: %d", nome, &init, &takeoff);
                insereOrdenadoD(nome, init, takeoff, cabeca_vooD);
            }
    		strcpy(str, "");
    	}

        printf("FUTURE ARRIVALS TO BE CREATED: \n");
        nodeA=cabeca_vooA->prox;
        nodeD = cabeca_vooD->prox;
        while(nodeA!= NULL){
            printf("%s ",nodeA->nome);
            nodeA = nodeA->prox;
        }
        printf("\n");
        printf("FUTURE DEPARTURES TO BE CREATED: \n");
        
        while(nodeD!= NULL){
            printf("%s ",nodeD->nome);
            nodeD = nodeD->prox;
        }
        printf("\n");
    }
}

void *tempo(){
    
    while(1){ 
        current_time++;
        usleep(ut*1000);
        
    }
}
/*
void *tempo(){
    
    struct timeb t_atual;
    int current_time;

    while(1){ 

        ftime(&t_atual);
        current_time = (int)((1000*(t_atual.time-t_inicio.time)+(t_atual.millitm-t_atual.millitm))/ut);
        printf("%d\n", current_time);    
    }
}
*/

void *partida(void *node){

    char str[100];
    char temp[10];
    int shm_slot;
    sscanf(((t_vooD*)node)->nome, "TP%s", temp);
    int id = atoi(temp);
    sprintf(str, "WOHOO, DEPARTURE FLIGHT %s CREATED AT MOMENT %d\n", ((t_vooD*)node)->nome, current_time);
   
    sem_wait(escreve_log);
    escreverEcra(str);
    escreverLog(str);
    sem_post(escreve_log); 
 


    t_message msg;
    msg.mtype = 99999;
    msg.id = id;
    
    msg.takeoff = ((t_vooD*)node)->takeoff;
    msg.tipo = 1; 	
    if(msgsnd(mqid, &msg, sizeof(t_message), 0)==-1){
    	perror("departure:msgsnd");
    	exit(1);
    }
    if(msgrcv(mqid, &msg, sizeof(t_message), id, 0)==-1){
    	perror("departure:msgrcv");
    	exit(1);
    }

    shm_slot = msg.slot_shm;
    strcpy(str,"");
    sprintf(str, "SHARED MEMORY SLOT ATTRIBUTED TO FLIGHT TP%d IS: %d\n", id, shm_slot);
    sem_wait(escreve_log);
    escreverEcra(str);
    escreverLog(str);
    sem_post(escreve_log);
    pthread_exit(0);
}

void *chegada(void *node){
    char str[100];
    char temp[10];
    int shm_slot;
    sscanf(((t_vooA*)node)->nome, "TP%s", temp);
    int id = atoi(temp);
    sprintf(str, "WOHOO, ARRIVAL FLIGHT %s CREATED AT MOMENT %d\n", ((t_vooA*)node)->nome, current_time);
    sem_wait(escreve_log);
    escreverEcra(str);
    escreverLog(str);
    sem_post(escreve_log);
    t_message msg;
    msg.mtype = 99999;
    msg.eta = ((t_vooA*)node)->eta;
    msg.fuel = ((t_vooA*)node)->fuel;
    msg.id = id;
    msg.tipo = 2;
    msgsnd(mqid, &msg, sizeof(t_message), 0);

    msgrcv(mqid, &msg, sizeof(t_message), id, 0);
    shm_slot = msg.slot_shm;

    strcpy(str,"");
    sprintf(str, "SHARED MEMORY SLOT ATTRIBUTED TO FLIGHT TP%d IS: %d\n", id, shm_slot);
    sem_wait(escreve_log);
    escreverEcra(str);
    escreverLog(str);
    sem_post(escreve_log);
    while(1){
    	if(arrayshm[shm_slot].isCompleted){
    		strcpy(str,"");
		    sprintf(str,"FLIGHT TP%d HAS BEEN CONCLUDED\n", msg.id);
		    sem_wait(escreve_log);
		    escreverEcra(str);
		    escreverLog(str);
		    sem_post(escreve_log);
    		break;
    	}
    }


    
    pthread_exit(0);
}




void *criavoos(){
    t_vooA *nodeA = cabeca_vooA;
    t_vooD *nodeD = cabeca_vooD;
    int i=0;
    int j=0;
    while(1){
        
        if((nodeA->prox!=NULL)&&(current_time==nodeA->prox->init)){
            pthread_create(&voosChegada[i++], NULL, chegada, (void*)(nodeA->prox));
            nodeA = nodeA->prox;
        }

        
        if((nodeD->prox!=NULL)&&(current_time == nodeD->prox->init)){
            pthread_create(&voosPartida[j++], NULL, partida, (void*)(nodeD->prox));
            nodeD = nodeD->prox; 

        }
        usleep(ut*1000);
    }

}



void acabar(){
	fclose(fp);
    msgctl(mqid,IPC_RMID,NULL);
    sem_wait(escreve_log);
    escreverEcra("Message Queue terminated successfully\n\n");
    escreverLog("Message Queue terminated successfully\n");
    sem_post(escreve_log);
    //depois colocar aqui o que dermos attach à shared memory
    shmctl(shmid, IPC_RMID, NULL);
    sem_wait(escreve_log);
    escreverEcra("Shared Memory terminated successfully\n\n");
    escreverLog("Shared Memory terminated successfully\n");
    sem_post(escreve_log);
    close(fdpipe);
    unlink(PIPE);
    sem_wait(escreve_log);
    escreverEcra("Named Pipe closed successfully\n\n");
    escreverLog("Named Pipe closed successfully\n");
    sem_post(escreve_log);
    sem_wait(escreve_log);
    escreverEcra("PROGRAM SHUT DOWN WITH CTRL+C\n\n");
    escreverLog("PROGRAM SHUT DOWN WITH CTRL+C\n");
    sem_post(escreve_log);
    kill(0, SIGKILL);
    exit(0);
}



void acabarTC(){
	
    sem_wait(escreve_log);
    escreverEcra("Control Tower shut down successfully\n");
    escreverLog("Control Tower shut down successfully\n");
    sem_post(escreve_log);
    exit(0);

}

void inicializa_stats(){
	stats->nvoos =0;
	stats->nAterragens = 0;
	stats->tempomedioAterrar=0:
	stats->nDescolagens = 0;
	stats->tempomedioDescolar = 0;
	stats->nmedioGoldings = 0;
	stats->nmedioHoldings_urgentes = 0;
	stats->nRedirecionados =0;
	stats->rejeitados =0;
}

int main()
{
 

    configs= (t_config*)malloc(sizeof(t_config));    
    lerConfig();
    ut = configs->ut;
    fp=fopen("log.txt", "a+");
    if(fp==NULL){
        printf("Error opening log file\n");
        exit(1);
    }
    ftime(&t_inicio);
    if(pthread_create(&thread_tempo, NULL, tempo, NULL)!=0){
        perror("pthread_create error");
        exit(1);
    }

    sem_unlink(SEM_LOG);
    escreve_log = sem_open(SEM_LOG, O_CREAT | O_EXCL, 0700, 1);
    sem_unlink(SEM_ARR);
    sem_array = sem_open(SEM_ARR, O_CREAT | O_EXCL, 0700, 1);
    sem_unlink(SEM_STATS);
    sem_stats = sem_open(SEM_STATS, O_CREAT | O_EXCL, 0700, 1);

    cabeca_vooA = cria_cabecalhovooA();
    cabeca_vooD = cria_cabecalhovooD();
    //cria log.txt e escreve
    criarLog();
    sem_wait(escreve_log);
    escreverEcra("Execution start\n");
    escreverLog("Execution start\n");
    sem_post(escreve_log);
    criarSHM();
    
    criarMQ();
    criarPipe();
    
    if(fork()==0){
    	if(pthread_create(&thread_tempoTC, NULL, tempo, NULL)!=0){
        	perror("pthread_create error");
       		exit(1);
    	}
    	
        signal(SIGINT, acabarTC);
        torreControlo();
        exit(0);
    }
    signal(SIGINT, acabar);

    //criar struct com a cabeca das listas para mandar para a criavoos
    if(pthread_create(&thread_check, NULL, criavoos, NULL)!=0){
        perror("pthread_create error");
        exit(1);
    }    
    lerPipe();
    return 0;
}

//Dinis Silva Costa Carvalho, nº 2018278118
//João Pimentel Roque Rodrigues Teixeira, nº 2018278532


/*
prioridade | atualizar escrever log | Fazer o Relatório  | corrigir while 1 da TC
condition variable voos chegada
mudar sistema numeraçao mensagens
demasiados voos em espera

*/

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
	shmid = shmget(IPC_PRIVATE,  sizeof(t_stats), IPC_CREAT|0666);
	shmid2 = shmget(IPC_PRIVATE, sizeof(t_comms)*maxvoos, IPC_CREAT | 0666);
	sprintf(message, "Shared memory with ID %d created\n", shmid );
    sem_wait(escreve_log);
    escreverEcra(message);
    escreverLog(message);
    sem_post(escreve_log);
    stats = (t_stats*)shmat(shmid, NULL, 0);
    stats->ptrArray = (t_comms*)shmat(shmid2, NULL, 0);

    arrayshm = stats->ptrArray;  
    	
}


void criarMQ(){
    char message[100];
	if((mqid = msgget(IPC_PRIVATE, IPC_CREAT|0777))<0){
		printf("Error creating message queue");
		exit(1);
	}
    sprintf(message, "Message Queue with ID %d created\n", mqid);
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
    regcomp(&regex,"DEPARTURE TP[0-9]+ init: [0-9]+ takeoff: [0-9]+$",REG_EXTENDED);
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
    regcomp(&regex,"ARRIVAL TP[0-9]+ init: [0-9]+ eta: [0-9]+ fuel: [0-9]+$",REG_EXTENDED);
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
        if(msgrcv(mqid, &msg, sizeof(t_message), -2, 0)==-1){
    		perror("Error na msgrcv memory\n");
            return 0;   		
    	}  
    	strcpy(str, "");
    	sprintf(str, "FLIGHT TP%d CONTACTED CONTROL TOWER FOR THE FIRST TIME \n", msg.id);
    	sem_wait(escreve_log);
    	escreverEcra(str);
    	escreverLog(str);
    	sem_post(escreve_log);
    	sem_wait(sem_stats);
    	
    	stats->nVoos = stats->nVoos +1;
    	
    	sem_post(sem_stats);
        if(msg.tipo == 1){

            stats->nDescolagens+=1;
            if(stats->nDescolagens + 1>configs->maxpart){
                msg.mtype = msg.id+3;

                msg.rejeitar = 1;
                stats->nDescolagens-=1;

            }
            else{
            	sem_wait(sem_array);
                arrayshm[i].tipo = msg.tipo;
                arrayshm[i].takeoff = msg.takeoff;
                arrayshm[i].isCompleted = 0; 
                arrayshm[i].id = msg.id;
                arrayshm[i].init = msg.init;
                t_queueD *node = cabecalho->D;
                t_queueD *nodeNovo = malloc(sizeof(t_queueD));
                while((node->prox!=NULL)&&(arrayshm[node->prox->slot_shm].takeoff<msg.takeoff)){
                    node = node->prox;
                }
                sem_post(sem_array);
                nodeNovo->slot_shm = i;
                nodeNovo->prox = node->prox;
                node->prox = nodeNovo;
                msg.mtype = msg.id + 3;
                msg.slot_shm=i++;
            }
        }
        if(msg.tipo == 2){

            stats->nAterragens +=1;
            if(stats->nAterragens>configs->maxchega){
                msg.mtype = msg.id+3;
                msg.rejeitar = 1;
                stats->nAterragens-=1;

            }
            else{
            	sem_wait(sem_array);
                arrayshm[i].init = msg.init;
                arrayshm[i].id = msg.id;
                arrayshm[i].eta = msg.eta;
                arrayshm[i].fuel = msg.fuel;
                arrayshm[i].tipo = msg.tipo;
                arrayshm[i].isCompleted=0;
                arrayshm[i].emergency = msg.emergency;
                t_queueA *nodeA = cabecalho->A;
                t_queueA *nodeNovo = (t_queueA*)malloc(sizeof(t_queueA));
                if(arrayshm[i].emergency == 0){
    	            while((nodeA->prox!=NULL)&&(arrayshm[nodeA->prox->slot_shm].emergency==1)){
    	                nodeA = nodeA->prox;
    	            }
    	            while((nodeA->prox!=NULL)&&(arrayshm[nodeA->prox->slot_shm].eta<msg.eta)){
    	                nodeA = nodeA->prox;
    	            }
    	        }
    	        else{
    	            while((nodeA->prox!=NULL)&&(arrayshm[nodeA->prox->slot_shm].eta<msg.eta)&&(arrayshm[nodeA->prox->slot_shm].emergency==1)){
    	                nodeA = nodeA->prox;
    	            }

    	        }
                sem_post(sem_array);
                nodeNovo->slot_shm = i;
                nodeNovo->prox = nodeA->prox;
                nodeA->prox = nodeNovo;
                msg.mtype = msg.id + 3;
                msg.slot_shm=i++;
            }

        }
    	
    	msgsnd(mqid, &msg, sizeof(t_message), 0);
        
    }  
}

void *ManageFuel(void *cabeca){
    t_queueA *cabecaA = ((t_cabecasqueue*)cabeca)->A;
    char str[100];
    while(1){
        sem_wait(sem_lista);
        t_queueA *nodeA = cabecaA;

        while((nodeA!=NULL) &&(nodeA->prox!=NULL)){
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
        sem_post(sem_lista);
        usleep(ut*1000);
    }
}



void reinsere(t_queueA *node){
    int randWait;
    randWait = (rand()%(configs->holdMax-configs->holdMin ))+configs->holdMin;
    int eta = arrayshm[node->prox->slot_shm].eta + randWait;

    t_queueA *novoNode = malloc(sizeof(t_queueA));
    t_queueA *aux = node;
    t_queueA *final;
    aux = node;
    novoNode->slot_shm = node->prox->slot_shm;
    while((aux->prox!=NULL)&&(arrayshm[aux->prox->slot_shm].eta<eta)){
        aux = aux->prox;
    }
    if(node->prox->prox == NULL){
        final = novoNode; 
    }
    else{
        final = node->prox->prox;
    }
    novoNode->prox = aux->prox;
    aux->prox = novoNode;
    novoNode->slot_shm = node->prox->slot_shm;
    free(node->prox);
    node->prox = final;
    arrayshm[novoNode->slot_shm].eta = eta;
}


int check_arrival(void* cabeca){
	
    t_queueA *cabecaA = ((t_cabecasqueue*)cabeca)->A;
        sem_wait(sem_array);
        if((cabecaA->prox->prox!=NULL) && (arrayshm[cabecaA->prox->prox->slot_shm].eta == arrayshm[cabecaA->prox->slot_shm].eta)){
            
            sem_post(sem_array);
            return 2;
        }
        else{
            
            sem_post(sem_array);
            return 1;
        }
    
    return 0;
    

}

int check_departure(void* cabeca){
    
    t_queueD *cabecaD = ((t_cabecasqueue*)cabeca)->D;

        sem_wait(sem_array);
        if((cabecaD->prox->prox!=NULL) && (arrayshm[cabecaD->prox->prox->slot_shm].takeoff <= arrayshm[cabecaD->prox->slot_shm].takeoff)){
            sem_post(sem_array);
            return 2;
        }
        else{
            sem_post(sem_array);
            return 1;
        }
    
    return 0;
    
}



int conta_departure(void *cabeca){
    t_queueD *cabecaD = ((t_cabecasqueue*)cabeca)->D;

        sem_wait(sem_array);
        if((cabecaD->prox->prox!=NULL) && (arrayshm[cabecaD->prox->prox->slot_shm].takeoff <= arrayshm[cabecaD->prox->slot_shm].takeoff)){
            sem_post(sem_array);
            printf("RETURN 2\n" );
            return 2;
        }
        else{
    
            sem_post(sem_array);
            printf("RETURN 1\n");
            return 1;
        }
    printf("RETURN 0\n");
    return 0;
}




void *gere_arrivals(void *cabeca){
    int ncriados;
    int pista = 1;
    time_departure = current_time;
    while(1){
    	
        t_queueA *cabecaA = ((t_cabecasqueue*)cabeca)->A;

        sem_wait(sem_arrival_full);

        if(cabecaA->prox!=NULL){    
            time_departure = arrayshm[cabecaA->prox->slot_shm].eta + arrayshm[cabecaA->prox->slot_shm].init;
            if(arrayshm[cabecaA->prox->slot_shm].eta + arrayshm[cabecaA->prox->slot_shm].init <= current_time){
                ncriados = check_arrival(cabeca);
                switch (ncriados){
        
                    case 1:
                        
                        sem_wait(sem_array);
                        arrayshm[cabecaA->prox->slot_shm].command = 1;

                        arrayshm[cabecaA->prox->slot_shm].pista = pista;
                        sem_post(sem_array);
                        if(pista==1){
                            pista = 2;
                        }
                        else{pista = 1;}
                        sem_post(sem_broadcast);
                        cabecaA->prox = cabecaA->prox->prox;

                        break;
                    
                    case 2:
                        
                        sem_wait(sem_array);
                        arrayshm[cabecaA->prox->slot_shm].command = 1;
                        arrayshm[cabecaA->prox->slot_shm].pista = pista;
                        if(pista==1){
                            pista = 2;
                        }
                        else{pista = 1;}
                        arrayshm[cabecaA->prox->prox->slot_shm].command = 1;
                        arrayshm[cabecaA->prox->prox->slot_shm].pista = pista;
                        sem_post(sem_array);
                        if(pista==1){
                            pista = 2;
                        }
                        else{pista = 1;}
                        
                        cabecaA->prox = cabecaA->prox->prox->prox;
                        int count = 0;
                        int temp_time = current_time;
                        while((cabecaA->prox != NULL) &&(arrayshm[cabecaA->prox->slot_shm].eta + arrayshm[cabecaA->prox->slot_shm].init <= temp_time)){
                            count++;
                            if(count>3){
                                reinsere(cabecaA);

                                stats->nmedioHoldings+=1;

                                

                            }
                            else{
                            cabecaA = cabecaA->prox;
                        }
                        }
                        sem_post(sem_broadcast);
                        break;
                }
            }
        }
        
        int value, value1;
        sem_getvalue(sem_pistaa1, &value);
        sem_getvalue(sem_pistaa2, &value1);
        if(((cabecaA->prox == NULL)||(time_departure - current_time >= configs->dAterra))&&((value==1) && value1==1)){
            sem_post(sem_arrival_empty);
        }
        else{
           
            sem_post(sem_arrival_full);
            
        } 
        
        usleep(ut*1000);     
    }   
}


void *gere_departures(void *cabeca){
    int ncriados;
    int pista = 3;
    time_departure = current_time;
    while(1){     
        
        sem_wait(sem_arrival_empty);
        t_queueD *cabecaD = ((t_cabecasqueue*)cabeca)->D;
        
        if(cabecaD->prox!=NULL){
            if(arrayshm[cabecaD->prox->slot_shm].takeoff <= current_time){
                ncriados = check_departure(cabeca);
                switch (ncriados){
                    case 1:
                        
                        sem_wait(sem_array);
                        arrayshm[cabecaD->prox->slot_shm].command = 4;
                        arrayshm[cabecaD->prox->slot_shm].pista = pista;
                        if(pista==3){
                            pista = 4;
                        }
                        else{pista =3;}
                        sem_post(sem_array);
                        sem_post(sem_broadcast);
                        cabecaD->prox = cabecaD->prox->prox;
                        
                        break;
                    
                    case 2:
                        sem_wait(sem_array);
                        arrayshm[cabecaD->prox->slot_shm].command = 4;
                        arrayshm[cabecaD->prox->slot_shm].pista = pista;
                        if(pista==3){
                            pista = 4;
                        }
                        else{pista =3;}
                        arrayshm[cabecaD->prox->prox->slot_shm].command = 4;
                        arrayshm[cabecaD->prox->prox->slot_shm].pista = pista;
                        sem_post(sem_array);
                        sem_post(sem_broadcast);
                        cabecaD->prox = cabecaD->prox->prox->prox;
                        
                        break;
                }
                
            }
           
        }
        sem_post(sem_arrival_full);
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
    if(pthread_create(&tc_gere_arrivals, NULL, gere_arrivals, (void*)heads) != 0){
    	perror("Erro a criar a thread gere_arrivals\n");
    	exit(1);
    }

    if(pthread_create(&tc_gere_departures, NULL, gere_departures, (void*)heads) != 0){
    	perror("Erro a criar a thread gere_departures\n");
    	exit(1);
    }

    while(1){
    	usleep(ut*1000);
    }
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
    msg.init = ((t_vooD*)node)->init;
    msg.mtype = 2;
    msg.id = id;
    
    msg.takeoff = ((t_vooD*)node)->takeoff;
    msg.tipo = 1; 	
    if(msgsnd(mqid, &msg, sizeof(t_message), 0)==-1){
    	perror("departure:msgsnd");
    	exit(1);
    }
    if(msgrcv(mqid, &msg, sizeof(t_message), id+3, 0)==-1){
    	perror("departure:msgrcv");
    	exit(1);
    }
    if(msg.rejeitar == 1){
        printf("FLIGHT TP%s REJECTED BY CONTROL TOWER, EXITING SYSTEM\n", ((t_vooD*)node)->nome);
        pthread_exit(0);  
    }

    shm_slot = msg.slot_shm;
    strcpy(str,"");
    sprintf(str, "SHARED MEMORY SLOT ATTRIBUTED TO FLIGHT TP%d IS: %d\n", id, shm_slot);
    sem_wait(escreve_log);
    escreverEcra(str);
    escreverLog(str);
    sem_post(escreve_log);

    usleep((((t_vooD*)node)->takeoff - current_time-1)*ut*1000);

    pthread_mutex_lock(&mutex_ordem);
    while(arrayshm[shm_slot].command!=4){
        pthread_cond_wait(&cond_ordem, &mutex_ordem);
    }
    pthread_mutex_unlock(&mutex_ordem);


    if(arrayshm[shm_slot].pista == 3){
        
        sem_wait(sem_arrival_empty);
       
        sem_wait(sem_pistad1);
        strcpy(str,"");

        sprintf(str, "FLIGHT TP%d IS DEPARTING FROM TRACK 0L \n", id);
        stats->tempomedioDescolar += (current_time-arrayshm[shm_slot].takeoff);
        sem_wait(escreve_log);
        escreverEcra(str);
        escreverLog(str);
        sem_post(escreve_log);
        usleep(configs->dDescola * ut*1000);
        sem_post(sem_pistad1);
    }

    if(arrayshm[shm_slot].pista == 4){
        sem_wait(sem_arrival_empty);
        sem_wait(sem_pistad2);
        strcpy(str,"");
        sprintf(str, "FLIGHT TP%d IS DEPARTING FROM TRACK 0R \n", id);
        stats->tempomedioDescolar += (current_time-arrayshm[shm_slot].takeoff);
        sem_wait(escreve_log);
        escreverEcra(str);
        escreverLog(str);
        sem_post(escreve_log);
        usleep(configs->dDescola * ut* 1000);
        sem_post(sem_pistad2);
    }
    sem_post(sem_arrival_empty);

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
    msg.init = ((t_vooA*)node)->init;
    msg.eta = ((t_vooA*)node)->eta;
    msg.fuel = ((t_vooA*)node)->fuel;
    msg.id = id;
    msg.tipo = 2;
    if(msg.fuel <= msg.eta + 4 + configs->dAterra){
    	strcpy(str, "");
        sprintf(str, "FLIGHT TP%d EMERGENCY LANDING REQUESTED \n", msg.id);
        sem_wait(escreve_log);
        escreverEcra(str);
        escreverLog(str);
        sem_post(escreve_log);

       	msg.emergency = 1;
        stats->nUrgentes+=1;
       	msg.mtype = (long)1;
    }
    else{
       	msg.emergency = 0;
       	msg.mtype = (long)2;
    }
    msgsnd(mqid, &msg, sizeof(t_message), 0);

    msgrcv(mqid, &msg, sizeof(t_message), id+3, 0);
    if(msg.rejeitar == 1){
        printf("FLIGHT TP%s REJECTED BY CONTROL TOWER, EXITING SYSTEM\n", ((t_vooA*)node)->nome);
        pthread_exit(0);  
    }
    shm_slot = msg.slot_shm;


    strcpy(str,"");
    sprintf(str, "SHARED MEMORY SLOT ATTRIBUTED TO FLIGHT TP%d IS: %d\n", id, shm_slot);
    sem_wait(escreve_log);
    escreverEcra(str);
    escreverLog(str);
    sem_post(escreve_log);
    sem_wait(sem_array);
    int ordem = arrayshm[shm_slot].command;
    sem_post(sem_array);
    int start_time;

    while((ordem != 1) && (ordem!=3)){
        
        if(ordem == 2){

            strcpy(str,"");
            sprintf(str, "FLIGHT TP%d RECEIVED COMMAND TO HOLD FOR %d TIME UNITS. FUEL => %d\n", id, arrayshm[shm_slot].hold_time,arrayshm[shm_slot].fuel);
            sem_wait(escreve_log);
            escreverEcra(str);
            escreverLog(str);
            sem_post(escreve_log);
        }

        usleep(ut*1000);
        sem_wait(sem_array);
        ordem = arrayshm[shm_slot].command;
        
        sem_post(sem_array);
    }
    
    if(ordem == 1){

        if(arrayshm[shm_slot].pista == 1){
            strcpy(str,"");
            sprintf(str, "FLIGHT TP%d RECEIVED COMMAND TO LAND IN TRACK 28L\n", id);
            start_time = current_time;
            sem_wait(escreve_log);
            escreverEcra(str);
            escreverLog(str);
            sem_post(escreve_log);

            sem_wait(sem_pistaa1);

            strcpy(str,"");
            sprintf(str, "FLIGHT TP%d WILL START LANDING IN TRACK 28L\n", id);
            sem_wait(escreve_log);
            escreverEcra(str);
            escreverLog(str);
            sem_post(escreve_log);
            usleep(ut*(configs->dAterra)*1000);
            strcpy(str,"");
            sprintf(str, "FLIGHT TP%d HAS FINISHED LANDING, FREEING UP TRACK 28L\n", id);
            
            stats->tempomedioAterrar += (current_time-start_time);
            sem_wait(escreve_log);
            escreverEcra(str);
            escreverLog(str);
            sem_post(escreve_log);
            sem_post(sem_pistaa1);
        }
        if(arrayshm[shm_slot].pista == 2){
            strcpy(str,"");
            sprintf(str, "FLIGHT TP%d RECEIVED COMMAND TO LAND IN TRACK 28R\n", id);
            start_time = current_time;
            sem_wait(escreve_log);
            escreverEcra(str);
            escreverLog(str);
            sem_post(escreve_log);

            sem_wait(sem_pistaa2);

            strcpy(str,"");
            sprintf(str, "FLIGHT TP%d WILL START LANDING IN TRACK 28R\n", id);
            sem_wait(escreve_log);
            escreverEcra(str);
            escreverLog(str);
            sem_post(escreve_log);
            
            usleep(ut*(configs->dAterra) *1000);
            strcpy(str,"");
            sprintf(str, "FLIGHT TP%d HAS FINISHED LANDING, FREEING UP TRACK 28R\n", id);
            stats->tempomedioAterrar += (current_time-start_time);
            

            sem_wait(escreve_log);
            escreverEcra(str);
            escreverLog(str);
            sem_post(escreve_log);
            sem_post(sem_pistaa2);
        }
    }
    if(ordem == 3){
        strcpy(str,"");
            sprintf(str, "FLIGHT TP%d DIVERTED TO CLOSEST AIRPORT\n", id);
            stats->nAterragens-=1;
            sem_wait(escreve_log);
            escreverEcra(str);
            escreverLog(str);
            sem_post(escreve_log);
    } 
    pthread_exit(0);
    
}


void *criavoos(){
    t_vooA *nodeA = cabeca_vooA;
    t_vooD *nodeD = cabeca_vooD;
    int i=0;
    int j=0;
    while(1){
        int ct = current_time;
        while((nodeA->prox!=NULL)&&(ct==nodeA->prox->init)){
            pthread_create(&voosChegada[i++], NULL, chegada, (void*)(nodeA->prox));
            nodeA = nodeA->prox;
        }

        
        while((nodeD->prox!=NULL)&&(ct == nodeD->prox->init)){
            pthread_create(&voosPartida[j++], NULL, partida, (void*)(nodeD->prox));
            nodeD = nodeD->prox; 

        }
        usleep(ut*1000);
    }

}

void printstats(){
    printf("-------------------------------------\n");
    double x;
    sem_wait(sem_stats);
    printf("\t STATS\n");
    printf("NUMBER OF FLIGHTS: %d\n", stats->nVoos);
    printf("NUMBER OF ARRIVALS: %d\n", stats->nAterragens);
    if(stats->nAterragens >0){
    x = ((double)stats->tempomedioAterrar/(double)stats->nAterragens);}
    else{x=0;}
    printf("AVERAGE LANDING TIME: %.2f\n", x);
    printf("NUMBER OF DEPARTURES: %d\n", stats->nDescolagens);
    if(stats->nDescolagens>0){
    x = ((double)stats->tempomedioDescolar/(double)stats->nDescolagens);}
    else{x=0;}
    printf("AVERAGE DEPARTURE TIME: %.2f\n", x);
    if(stats->nAterragens >0){
    x=((double)stats->nmedioHoldings/(double)stats->nAterragens);
    }
    else{x=0;}
    printf("AVERAGE HOLDING NUMBER: %.2f\n", x);
    if(stats->nUrgentes>0){
        x= ((double)stats->nmedioHoldings_urgentes/(double)stats->nUrgentes);
    }
    else{x=0;}
    printf("AVERAGE URGENT HOLDING NUMBER: %.2f\n", x);
    printf("NUMBER OF REDIRECTED FLIGHTS: %d\n", stats->nRedirecionados);
    printf("NUMBER OF REJECTED FLIGHTS: %d\n", stats->rejeitados);
    printf("NUMBER OF URGENT FLIGHTS %d\n", stats->nUrgentes);
    sem_post(sem_stats);
    printf("-------------------------------------\n");
    
}


void acabar(){
	printstats();
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
    sem_unlink(SEM_ARR);
    sem_close(sem_array);
    sem_unlink(SEM_ARRIVAL_EMPTY);
    sem_close(sem_arrival_empty);
    sem_unlink(SEM_STATS);
    sem_close(sem_stats);
    sem_unlink(SEM_A1);
    sem_close(sem_pistaa1);
    sem_unlink(SEM_A2);
    sem_close(sem_pistaa2);
    sem_unlink(SEM_D1);
    sem_close(sem_pistad1);
    sem_unlink(SEM_D2);
    sem_close(sem_pistad2);
    sem_unlink(SEM_ARRIVAL_FULL);
    sem_close(sem_arrival_full);
    sem_unlink(SEM_LISTA);
    sem_close(sem_lista);
    sem_unlink(SEM_BROAD);
    sem_close(sem_broadcast);
    sem_wait(escreve_log);
    escreverEcra("Sempahores closed successfully\n\n");
    escreverLog("Semaphores closed successfully\n");
    sem_post(escreve_log);
    sem_wait(escreve_log);
    escreverEcra("PROGRAM SHUT DOWN WITH CTRL+C\n\n");
    escreverLog("PROGRAM SHUT DOWN WITH CTRL+C\n");
    sem_post(escreve_log);
    sem_unlink(SEM_LOG);
    sem_close(escreve_log);
    fclose(fp);
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
	stats->nVoos =0;
	stats->nAterragens = 0;
	stats->tempomedioAterrar=0;
	stats->nDescolagens = 0;
	stats->tempomedioDescolar = 0;
	stats->nmedioHoldings = 0;
	stats->nmedioHoldings_urgentes = 0;
	stats->nRedirecionados =0;
	stats->rejeitados =0;
    stats->nUrgentes = 0;
}

void *broadcast(){
    while(1){
        sem_wait(sem_broadcast);
        pthread_mutex_lock(&mutex_ordem);
        pthread_cond_broadcast(&cond_ordem);
        pthread_mutex_unlock(&mutex_ordem);
    }

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
    srand(time(NULL));

    sem_unlink(SEM_LOG);
    escreve_log = sem_open(SEM_LOG, O_CREAT | O_EXCL, 0700, 1);
    sem_unlink(SEM_ARR);
    sem_array = sem_open(SEM_ARR, O_CREAT | O_EXCL, 0700, 1);
    sem_unlink(SEM_STATS);
    sem_stats = sem_open(SEM_STATS, O_CREAT | O_EXCL, 0700, 1);
    sem_unlink(SEM_ARRIVAL_EMPTY);
    sem_arrival_empty = sem_open(SEM_ARRIVAL_EMPTY, O_CREAT | O_EXCL, 0700,0);
    sem_unlink(SEM_ARRIVAL_FULL);
    sem_arrival_full = sem_open(SEM_ARRIVAL_FULL, O_CREAT | O_EXCL, 0700,1);
    sem_unlink(SEM_A1);
    sem_pistaa1 = sem_open(SEM_A1, O_CREAT | O_EXCL, 0700, 1);
    sem_unlink(SEM_A2);
    sem_pistaa2 = sem_open(SEM_A2, O_CREAT | O_EXCL, 0700, 1);
    sem_unlink(SEM_D1);
    sem_pistad1 = sem_open(SEM_D1, O_CREAT | O_EXCL, 0700, 1);
    sem_unlink(SEM_D2);
    sem_pistad2 = sem_open(SEM_D2, O_CREAT | O_EXCL, 0700, 1);
    sem_unlink(SEM_LISTA);
    sem_lista = sem_open(SEM_LISTA, O_CREAT | O_EXCL, 0700, 1);
    sem_unlink(SEM_BROAD);
    sem_broadcast = sem_open(SEM_BROAD, O_CREAT | O_EXCL, 0700, 0);
    sigfillset(&sigs);
    sigdelset(&sigs, SIGINT);
    sigdelset(&sigs, SIGUSR1);
    sigprocmask(SIG_SETMASK, &sigs, NULL);

    

    cabeca_vooA = cria_cabecalhovooA();
    cabeca_vooD = cria_cabecalhovooD();
    //cria log.txt e escreve
    criarLog();
    sem_wait(escreve_log);
    escreverEcra("Execution start\n");
    escreverLog("Execution start\n");
    sem_post(escreve_log);
    char mensagem[50];
    strcpy(mensagem, "");
    sprintf(mensagem, "MAIN PROCESS ID [%d]\n", getpid());
    sem_wait(escreve_log);
    escreverEcra(mensagem);
    escreverLog(mensagem);
    sem_post(escreve_log);
    criarSHM();
    inicializa_stats();
    criarMQ();
    criarPipe();
    
    
    if(fork()==0){
        signal(SIGUSR1,printstats);
        strcpy(mensagem, "");
        sprintf(mensagem, "CONTROL TOWER PROCESS ID [%d]\n", getpid());
        sem_wait(escreve_log);
        escreverEcra(mensagem);
        escreverLog(mensagem);
        sem_post(escreve_log);
    	if(pthread_create(&thread_tempoTC, NULL, tempo, NULL)!=0){
        	perror("pthread_create error");
       		exit(1);
    	}
    	
        signal(SIGINT, acabarTC);
        torreControlo();
        exit(0);
    }
    signal(SIGUSR1, SIG_IGN);
    signal(SIGINT, acabar);
    

    //criar struct com a cabeca das listas para mandar para a criavoos
    if(pthread_create(&thread_check, NULL, criavoos, NULL)!=0){
        perror("pthread_create error");
        exit(1);
    }    
    if(pthread_create(&broadcasts, NULL, broadcast, NULL)!=0){
        perror("pthread_create error");
        exit(1);
    }  
    lerPipe();
    return 0;
}

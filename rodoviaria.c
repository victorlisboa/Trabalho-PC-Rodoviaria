#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define QNT_ONIBUS_NORMAL 5
#define QNT_ONIBUS_ARTICULADO 2
#define CAP_ONIBUS_NORMAL 2
#define CAP_ONIBUS_ARTICULADO 3
#define QNT_PASSAGEIROS 20
#define QNT_BOXES 2
#define DURACAO_NORMAL 2
#define DURACAO_ARTICULADO 4

int boxes[QNT_BOXES];   // array que indica o id do onibus parado no box
int cap_box[QNT_BOXES]; // capacidade do onibus parado no box
int articulado_pend = 0;// indica a quantidade de onibus articulados querendo um box

pthread_mutex_t l1 = PTHREAD_MUTEX_INITIALIZER; // mutex para acessar articulado_pend
pthread_mutex_t l2 = PTHREAD_MUTEX_INITIALIZER; // mutex para acessar boxes
pthread_mutex_t l3 = PTHREAD_MUTEX_INITIALIZER; // mutex para acessar cap_box

sem_t vaga;                     // semaforo para onibus pegarem vaga em algum box
sem_t espera_box[QNT_BOXES];    // semaforo para onibus esperarem ate encher

// barreira para passageiros aguardarem onibus fazer percurso
pthread_barrier_t barreira_onibus[QNT_ONIBUS_NORMAL+QNT_ONIBUS_ARTICULADO];

void viagem(int id, int box) {
    pthread_mutex_lock(&l2);
    boxes[box] = -1;
    pthread_mutex_unlock(&l2);
    sem_post(&vaga);    // sinaliza que tem uma vaga livre nos boxes

    printf("O onibus %d saiu do box %d e foi fazer a viagem.\n", id, box);
    if(id >= QNT_ONIBUS_NORMAL) sleep(DURACAO_NORMAL);
    else sleep(DURACAO_ARTICULADO);
    printf("O onibus %d voltou da viagem.\n", id);

    /* entra na barreira e libera todos os passageiros
    *  que estavam aguardando o fim da viagem */
    pthread_barrier_wait(&barreira_onibus[id]);
}

int aloca_onibus(int id, int tipo) {
    // aloca onibus em box e retorna o box alocado
    int i;
    pthread_mutex_lock(&l2);
    for(i=0; i<QNT_BOXES; i++) {
        if(boxes[i] == -1) {
            boxes[i] = id;
            break;
        }
    }
    pthread_mutex_unlock(&l2);
    
    sleep(1);
    if(tipo) printf("O onibus articulado %d estacionou no box %d.\n", id, i);
    else printf("O onibus %d estacionou no box %d.\n", id, i);

    pthread_mutex_lock(&l3);
    if(tipo) {  // tipo 0 = normal e tipo 1 = articulado
        cap_box[i] = CAP_ONIBUS_ARTICULADO;
    }
    else {
        cap_box[i] = CAP_ONIBUS_NORMAL;
    }
    pthread_mutex_unlock(&l3);
    return i;
}

void * f_onibus(void * x) {
    int id = *((int *) x);
    while(1) {
        sem_wait(&vaga);    // aguarda ate conseguir uma vaga em algum box
        
        // Procedimentos para priorizar articulados
        pthread_mutex_lock(&l1);
        if(articulado_pend > 0) {
            sem_post(&vaga);    // se tem algum onibus articulado querendo vaga, libera a vaga
            pthread_mutex_unlock(&l1);
            //printf("O onibus %d iria entrar no box, mas deixou de entrar pois tem um articulado querendo.\n", id);
            continue;
        }
        pthread_mutex_unlock(&l1);

        // Onibus conseguiu pegar a vaga
        int box = aloca_onibus(id, 0);
        sem_wait(&espera_box[box]); // aguarda todos passageiros embarcarem
        viagem(id, box);
    }
}

void * f_articulado(void * x) {
    int id = *((int *) x);
    while(1) {
        // procedimento para conseguir vaga
        pthread_mutex_lock(&l1);
        articulado_pend++;
        pthread_mutex_unlock(&l1);
        sem_wait(&vaga);
        pthread_mutex_lock(&l1);
        articulado_pend--;
        pthread_mutex_unlock(&l1);
        int box = aloca_onibus(id, 1);
        sem_wait(&espera_box[box]); // aguarda todos passageiros embarcarem
        viagem(id, box);
    }
}

void * f_passageiro(void * x) {
    int id = *((int *) x);
    while(1) {
        // verifica boxes
        for(int i=0; i<QNT_BOXES; i++) {
            pthread_mutex_lock(&l2);
            if(boxes[i] != -1) {
                pthread_mutex_lock(&l3);
                if(cap_box[i] > 0) {
                    // passageiro vai entrar no onibus
                    int id_onibus = boxes[i];
                    printf("O passageiro %d pegou o onibus %d.\n", id, id_onibus);
                    cap_box[i]--;
                    if(cap_box[i] == 0) {
                        // ultimo passageiro libera a saida do onibus
                        sem_post(&espera_box[i]);
                    }
                    pthread_mutex_unlock(&l3);  // libera o lock do cap_box
                    pthread_mutex_unlock(&l2);  // libera o lock do boxes

                    // aguarda onibus fazer a viagem
                    pthread_barrier_wait(&barreira_onibus[id_onibus]);
                }
                pthread_mutex_unlock(&l3);
            }
            pthread_mutex_unlock(&l2);
        }
    }
}

int main() {
    // Cria threads e inicializa variaveis

    pthread_t onibus[QNT_ONIBUS_NORMAL];
    pthread_t articulados[QNT_ONIBUS_ARTICULADO];
    pthread_t passageiros[QNT_PASSAGEIROS];

    sem_init(&vaga, 0, QNT_BOXES);
    for(int i=0; i<QNT_BOXES; i++) {
        sem_init(&espera_box[i], 0, 0);
        boxes[i] = -1;
    }

    for(int i=0; i<QNT_ONIBUS_NORMAL; i++) {
        int *id = (int *) malloc(sizeof(int));
        *id = i;
        pthread_barrier_init(&barreira_onibus[i], NULL, CAP_ONIBUS_NORMAL+1);
        pthread_create(&onibus[i], NULL, f_onibus, (void *) id);
    }
    for(int i=0; i<QNT_ONIBUS_ARTICULADO; i++) {
        int *id = (int *) malloc(sizeof(int));
        *id = i + QNT_ONIBUS_NORMAL;
        pthread_barrier_init(&barreira_onibus[*id], NULL, CAP_ONIBUS_ARTICULADO+1);
        pthread_create(&articulados[i], NULL, f_articulado, (void *) id);
    }
    for(int i=0; i<QNT_PASSAGEIROS; i++) {
        int *id = (int *) malloc(sizeof(int));
        *id = i;
        pthread_create(&passageiros[i], NULL, f_passageiro, (void *) id);
    }

    pthread_join(onibus[0], NULL);

    return 0;
}

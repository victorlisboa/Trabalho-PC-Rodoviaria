#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define QNT_ONIBUS_NORMAL 5
#define QNT_ONIBUS_ARTICULADO 2
#define QNT_PASSAGEIROS 10
#define QNT_BOXES 2

int boxes[QNT_BOXES];

void * f_onibus(void * id) {
    int a = *((int *) id);
    
    while(1) {
        
    }
}

void * f_articulado(void * id) {
    int a = *((int *) id);
    while(1) {
        
    }
}

void * f_passageiro(void * id) {
    int a = *((int *) id);
    while(1) {
        
    }
}

int main() {

    pthread_t onibus[QNT_ONIBUS_NORMAL];
    pthread_t articulados[QNT_ONIBUS_ARTICULADO];
    pthread_t passageiros[QNT_PASSAGEIROS];

    // Criando threads e inicializando variaveis
    for(int i=0; i<QNT_ONIBUS_NORMAL; i++) {
        int *id = (int *) malloc(sizeof(int));
        *id = i;
        pthread_create(&onibus[i], NULL, f_onibus, (void *) id);
    }
    for(int i=0; i<QNT_ONIBUS_ARTICULADO; i++) {
        int *id = (int *) malloc(sizeof(int));
        *id = i + QNT_ONIBUS_NORMAL;
        pthread_create(&articulados[i], NULL, f_articulado, (void *) id);
    }
    for(int i=0; i<QNT_PASSAGEIROS; i++) {
        int *id = (int *) malloc(sizeof(int));
        *id = i;
        pthread_create(&passageiros[i], NULL, f_passageiro, (void *) id);
    }
    for(int i=0; i<QNT_BOXES; i++) {
        boxes[i] = -1;
    }

    pthread_join(onibus[0], NULL);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define QNT_ONIBUS_NORMAL 10
#define QNT_ONIBUS_ARTICULADO 2

void * f_articulado(void * id) {
    while(1) {
        
    }
}

void * f_onibus(void * id) {
    while(1) {

    }
}

int main() {

    pthread_t onibus[QNT_ONIBUS_NORMAL];
    pthread_t articulados[QNT_ONIBUS_ARTICULADO];
    for(int i=0; i<QNT_ONIBUS_NORMAL; i++) {
        int *id = (int *) malloc(sizeof(int));
        *id = i;
        pthread_create(&onibus[i], NULL, f_onibus, (void *) id);
    }
    for(int i=0; i<QNT_ONIBUS_ARTICULADO; i++) {
        int *id = (int *) malloc(sizeof(int));
        *id = i;
        pthread_create(&articulados[i], NULL, f_articulado, (void *) id);
    }


    return 0;
}
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NPISTAS 5
#define NTHREADS 5

struct carro {
    pthread_t thread;
    int thread_id;
    struct carro* prox;
}; 
typedef struct carro CARRO;

struct pista {
    int num_pista;
    CARRO* inicio;
    struct pista* prox;
};
typedef struct pista PISTA;

void *funcao (void *args){
    printf("Hello World");
}

CARRO* create_carro(int i){
    CARRO* novo = (CARRO*)malloc(sizeof(CARRO));
    novo->thread_id = i;
    novo->prox = NULL;
    return novo;
}

void append_carro(CARRO** fila, int i){
    if(*fila == NULL){
        *fila = create_carro(i);
        return;
    }
    CARRO* novo = create_carro(i);
    CARRO* aux = *fila;
    while(aux->prox != NULL){
        aux = aux->prox;
    }
    aux->prox = novo;
}

PISTA* create_pista(int i){
    PISTA* nova = (PISTA*)malloc(sizeof(PISTA));
    nova->num_pista = i;
    nova->prox = NULL;
    nova->inicio = NULL;
    return nova;
}

void iniciar_pistas(PISTA** inicio){
    int i;
    *inicio = create_pista(1);
    PISTA* aux = *inicio;
    for(i=1;i<NPISTAS;i++){
        aux->prox = create_pista(i+1);
        aux = aux->prox;
    }
}

void encher_pistas(PISTA** inicio){
    int i, num_carro=1;
    PISTA* aux_pista = *inicio;
    CARRO* aux_carro;
    while(aux_pista != NULL){
        for(i=0; i< NTHREADS; i++){
            append_carro(&aux_pista->inicio, num_carro);
            num_carro++;
        }
     aux_pista = aux_pista->prox;
    }
}

void print_fila_carros(CARRO* fila){
    CARRO *aux = fila;
    while(aux != NULL){
        printf(" - Carro %d\n", aux->thread_id);
        aux = aux->prox;
    }
}

void print_pistas(PISTA* inicio){
    PISTA* aux = inicio;
    while(aux != NULL){
        printf("Pista %d\n",aux->num_pista);
        print_fila_carros(aux->inicio);
        aux = aux->prox;
    }
}
 int main() {
     pthread_t tid[NTHREADS];
     int i;
     PISTA* inicio = NULL;
     iniciar_pistas(&inicio);
     encher_pistas(&inicio);
     print_pistas(inicio);
 }

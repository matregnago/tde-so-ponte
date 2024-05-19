#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define NPISTAS 3
#define NTHREADS 5
#define TRAVAR_PONTE 5
#define FLUXO_CARROS 1
#define TEMPO_TRAVAR_PONTE 5

pthread_mutex_t ponte_mutex;
int executados=0;


struct carro {
    pthread_t thread;
    int thread_id;
    int pode_passar;
    int num_fila;
    struct carro* prox;
}; 
typedef struct carro CARRO;

struct pista {
    int num_pista;
    int num_carros;
    CARRO* inicio;
    struct pista* prox;
};
typedef struct pista PISTA;

PISTA* pistas_inicio = NULL;

void contar_fila_carros() {
    PISTA* aux_pista = pistas_inicio;
    int cont;
    while(aux_pista != NULL){
        cont=0;
        CARRO *aux = aux_pista->inicio;
        while(aux != NULL) {
            cont++;
            aux = aux->prox;
        }
        printf("A pista %d tem %d carros\n", aux_pista->num_pista, cont);
        aux_pista = aux_pista->prox;
    }
}

void remover_carro(CARRO* carro){
    int n = carro->num_fila;
    int i;
    PISTA* aux = pistas_inicio;
    CARRO* temp = NULL;
    for(i=1;i<n;i++){
        aux = aux->prox;
    }
    temp = aux->inicio;
    if(aux->inicio->prox != NULL){
        aux->inicio = aux->inicio->prox;
    }
    else{
        aux->inicio = NULL;
    }
    free(temp);
}


void fechar_ponte(){
    printf("Ponte travou. \n");
    contar_fila_carros();
    sleep(TEMPO_TRAVAR_PONTE);
    printf("Ponte destravou. \n");
    pthread_mutex_unlock(&ponte_mutex);
}


void *funcao (void *args) {
    CARRO *carro = (CARRO *)args;
    while (1) {
        if(carro->pode_passar == 1){
            pthread_mutex_lock(&ponte_mutex);
            printf("Carro %d esta passando pela ponte\n", carro->thread_id);
            sleep(FLUXO_CARROS);
            if(carro->prox != NULL){
               carro->prox->pode_passar = 1;
            }
            remover_carro(carro);
            executados++;
            int ponte = TRAVAR_PONTE;
            if(executados % ponte == 0){
                fechar_ponte();
            }
            else{
                pthread_mutex_unlock(&ponte_mutex);
            }
            break;
        }
    }
    return NULL;
}


CARRO* create_carro(int i, int pode_passar, int num_fila) {
    CARRO* novo = (CARRO*)malloc(sizeof(CARRO));
    novo->thread_id = i;
    novo->prox = NULL;
    novo->pode_passar = pode_passar;
    novo->num_fila = num_fila;
    return novo;
}

void append_carro(CARRO** fila, int i, int num_fila) {
    if(*fila == NULL) {
        *fila = create_carro(i, 1, num_fila);
        return;
    }
    CARRO* novo = create_carro(i, 0, num_fila);
    CARRO* aux = *fila;
    while(aux->prox != NULL) {
        aux = aux->prox;
    }
    aux->prox = novo;
}

PISTA* create_pista(int i) {
    PISTA* nova = (PISTA*)malloc(sizeof(PISTA));
    nova->num_pista = i;
    nova->prox = NULL;
    nova->inicio = NULL;
    return nova;
}

void iniciar_pistas(PISTA** inicio) {
    int i;
    *inicio = create_pista(1);
    PISTA* aux = *inicio;
    for(i = 1; i < NPISTAS; i++) {
        aux->prox = create_pista(i + 1);
        aux = aux->prox;
    }
}

void encher_pistas(PISTA** inicio) {
    int i, num_carro = 1;
    PISTA* aux_pista = *inicio;
    CARRO* aux_carro;
    while(aux_pista != NULL) {
        for(i = 0; i < NTHREADS; i++) {
            append_carro(&aux_pista->inicio, num_carro, aux_pista->num_pista);
            num_carro++;
        }
        aux_pista = aux_pista->prox;
    }
}

void print_fila_carros(CARRO* fila) {
    CARRO *aux = fila;
    while(aux != NULL) {
        printf(" - Carro %d\n", aux->thread_id);
        aux = aux->prox;
    }
}


void print_pistas(PISTA* inicio) {
    PISTA* aux = inicio;
    while(aux != NULL) {
        printf("Pista %d\n", aux->num_pista);
        print_fila_carros(aux->inicio);
        aux = aux->prox;
    }
}

void iniciar_threads() {
    PISTA* aux_pista = pistas_inicio;
    while (aux_pista != NULL) {
        CARRO* aux_carro = aux_pista->inicio;
        while (aux_carro != NULL) {
            pthread_create(&aux_carro->thread, NULL, funcao, (void*)aux_carro);
            aux_carro = aux_carro->prox;
        }
        aux_pista = aux_pista->prox;
    }
}

void esperar_threads() {
    PISTA* aux_pista = pistas_inicio;
    while (aux_pista != NULL) {
        CARRO* aux_carro = aux_pista->inicio;
        while (aux_carro != NULL) {
            pthread_join(aux_carro->thread, NULL);
            aux_carro = aux_carro->prox;
        }
        aux_pista = aux_pista->prox;
    }
}

int main() {
    pthread_mutex_init(&ponte_mutex, NULL);
    iniciar_pistas(&pistas_inicio);
    encher_pistas(&pistas_inicio);
    print_pistas(pistas_inicio);
    iniciar_threads();
    esperar_threads();
    pthread_mutex_destroy(&ponte_mutex);
    printf("Acabou os carros\n");
    return 0;
}

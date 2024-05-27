#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// Variaveis de configuracao
#define NPISTAS 2        // Numero de pistas de carros
#define NTHREADS 2       // Numero de carros por pista
#define TRAVAR_PONTE 25  // Frequencia de execucoes para travar a ponte
#define FLUXO_CARROS \
  1  // Tempo que um carro demora para atravessar a ponte (segundos)
#define TEMPO_TRAVAR_PONTE 5  // Tempo que a ponte fica bloqueada

// Declaracao da struct da lista encadeada
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

// Variaveis globais
PISTA* pistas_inicio = NULL;
pthread_mutex_t ponte_mutex;
int executados = 0;

// Manipulacao da estrutura da lista
CARRO* criar_carro(int i, int pode_passar, int num_fila) {
  CARRO* novo = (CARRO*)malloc(sizeof(CARRO));
  novo->thread_id = i;
  novo->prox = NULL;
  novo->pode_passar = pode_passar;
  novo->num_fila = num_fila;
  return novo;
}

void remover_carro(CARRO* carro) {
  int n = carro->num_fila;
  int i;
  PISTA* aux = pistas_inicio;
  CARRO* temp = NULL;
  for (i = 1; i < n; i++) {
    aux = aux->prox;
  }
  temp = aux->inicio;
  if (aux->inicio->prox != NULL) {
    aux->inicio = aux->inicio->prox;
  } else {
    aux->inicio = NULL;
  }
  free(temp);
}

void adicionar_carro_lista(CARRO** fila, int i, int num_fila) {
  if (*fila == NULL) {
    *fila = criar_carro(i, 1, num_fila);
    return;
  }
  CARRO* novo = criar_carro(i, 0, num_fila);
  CARRO* aux = *fila;
  while (aux->prox != NULL) {
    aux = aux->prox;
  }
  aux->prox = novo;
}

PISTA* criar_pista(int i) {
  PISTA* nova = (PISTA*)malloc(sizeof(PISTA));
  nova->num_pista = i;
  nova->prox = NULL;
  nova->inicio = NULL;
  return nova;
}

// Inicializacao das pistas e das filas de carros
void iniciar_pistas() {
  int i;
  pistas_inicio = criar_pista(1);
  PISTA* aux = pistas_inicio;
  for (i = 1; i < NPISTAS; i++) {
    aux->prox = criar_pista(i + 1);
    aux = aux->prox;
  }
}

void encher_pistas() {
  int i, num_carro = 1;
  PISTA* aux_pista = pistas_inicio;
  CARRO* aux_carro;
  while (aux_pista != NULL) {
    for (i = 0; i < NTHREADS; i++) {
      adicionar_carro_lista(&aux_pista->inicio, num_carro,
                            aux_pista->num_pista);
      num_carro++;
    }
    aux_pista = aux_pista->prox;
  }
}

// Prints iniciais
void print_fila_carros(CARRO* fila) {
  CARRO* aux = fila;
  while (aux != NULL) {
    printf(" - Carro %d\n", aux->thread_id);
    aux = aux->prox;
  }
}

void print_pistas() {
  PISTA* aux = pistas_inicio;
  while (aux != NULL) {
    printf("Pista %d\n", aux->num_pista);
    print_fila_carros(aux->inicio);
    aux = aux->prox;
  }
}

// Logica principal do programa

// Calcula o congestionamento
void contar_fila_carros() {
  PISTA* aux_pista = pistas_inicio;
  int cont;
  while (aux_pista != NULL) {
    cont = 0;
    CARRO* aux = aux_pista->inicio;
    while (aux != NULL) {
      cont++;
      aux = aux->prox;
    }
    printf(" Pista %d: %d carros\n", aux_pista->num_pista, cont);
    aux_pista = aux_pista->prox;
  }
}

// Trava a ponte
void fechar_ponte() {
  printf("\nA ponte travou. Congestionamento: \n");
  contar_fila_carros();
  sleep(TEMPO_TRAVAR_PONTE);
  printf("A ponte destravou.\n\n");
  pthread_mutex_unlock(&ponte_mutex);
}

// Funcao principal executada por cada thread
void* cruzar_ponte(void* args) {
  CARRO* carro = (CARRO*)args;
  while (1) {
    if (carro->pode_passar == 1) {
      pthread_mutex_lock(&ponte_mutex);
      printf("Carro %d cruzou a ponte\n", carro->thread_id);
      sleep(FLUXO_CARROS);
      if (carro->prox != NULL) {
        carro->prox->pode_passar = 1;
      }
      remover_carro(carro);
      executados++;
      int ponte = TRAVAR_PONTE;
      if (executados % ponte == 0) {
        fechar_ponte();
      } else {
        pthread_mutex_unlock(&ponte_mutex);
      }
      break;
    }
  }
  return NULL;
}

// Inicializacao das threads
void iniciar_threads() {
  PISTA* aux_pista = pistas_inicio;
  while (aux_pista != NULL) {
    CARRO* aux_carro = aux_pista->inicio;
    while (aux_carro != NULL) {
      pthread_create(&aux_carro->thread, NULL, cruzar_ponte, (void*)aux_carro);
      aux_carro = aux_carro->prox;
    }
    aux_pista = aux_pista->prox;
  }
}

// Esperar threads finalizarem
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
  if (NPISTAS < 1) {
    printf("Numero de pistas precisa ser maior do que 0");
    return -1;
  }
  if (NTHREADS < 1) {
    printf("Numero de threads precisa ser maior do que 0");
    return -1;
  }
  pthread_mutex_init(&ponte_mutex, NULL);
  iniciar_pistas();
  encher_pistas();
  print_pistas();
  printf("\nPassagens pela ponte: \n");
  iniciar_threads();
  esperar_threads();
  pthread_mutex_destroy(&ponte_mutex);
  printf("Acabaram os carros. Programa finalizado.\n");
  return 0;
}

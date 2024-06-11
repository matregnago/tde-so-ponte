#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

// Variaveis de configuracao
#define NPISTAS 10   	// Numero de pistas de carros
#define NPROCESS 5	// Numero de carros por pista
#define TRAVAR_PONTE 25  // Frequencia de execucoes para travar a ponte
#define FLUXO_CARROS 1  // Tempo que um carro demora para atravessar a ponte (segundos)
#define TEMPO_TRAVAR_PONTE 5  // Tempo que a ponte fica bloqueada
#define PISTAS_KEY 100 // Chave memoria compartilhada de pistas
#define CONTROLS_KEY 200 // Chave memoria compartilhada de controle

// Declaracao da struct da lista encadeada
struct carro {
  int num;
  int pode_passar;
  int num_fila;
  int lado;
  struct carro* prox;
};
typedef struct carro CARRO;

struct pista {
  int num_pista;
  int num_carros;
  int lado;
  CARRO* inicio;
  struct pista* prox;
};
typedef struct pista PISTA;

// Variaveis globais
sem_t* sem0; // Semaphore lado 0
sem_t* sem1; // Semaphore lado 1
int num_carro = 1; // Numero do carro para encher pistas
int lado = 0; // Controla o lado da pista na pista

// Manipulacao da estrutura da lista
void criar_carro(CARRO* carro, int num, int pode_passar, int num_fila, int lado_pista) {
  carro->num = num;
  carro->pode_passar = pode_passar;
  carro->num_fila = num_fila;
  carro->lado = lado_pista;
  carro->prox = NULL;
}

void remover_carro(PISTA* pistas, CARRO* carros_base, CARRO* carro) {
  int num_pista = carro->num_fila;
  int num_carro = carro->num;
  int desl_vet_carros = 0;
  if (num_pista - 1 == 0) {
	desl_vet_carros = num_carro - 1;
  } else {
	desl_vet_carros = (num_carro - 1) % (NPROCESS * (num_pista - 1));
  }

  CARRO* lista_carros_pista = &carros_base[(num_pista - 1) * NPROCESS];
  CARRO* aux = &lista_carros_pista[desl_vet_carros];
  if (aux->prox == NULL) {
	pistas[num_pista-1].inicio = NULL;
  } else {

	aux->prox->pode_passar = 1;
	pistas[num_pista - 1].inicio = aux->prox;
  }
}

void adicionar_carros_a_pista(PISTA* pista, CARRO* carros_base, int lado_pista) {
  int pode_passar_val = 0;
  for (int j = 0; j < NPROCESS; j++) {
	if (j == 0) {
  	pode_passar_val = 1;
	}
	criar_carro(&carros_base[j], num_carro, pode_passar_val, pista->num_pista, lado_pista);
	num_carro++;
	if (j < NPROCESS - 1) {
  	carros_base[j].prox = &carros_base[j + 1];
	} else {
  	carros_base[j].prox = NULL;
	}
	pode_passar_val = 0;
  }
  pista->inicio = carros_base;
  pista->num_carros = NPROCESS;
}

void criar_pista(PISTA* pista, int i) {
  pista->num_pista = i;
  pista->lado = lado;
  pista->prox = NULL;
  pista->inicio = NULL;
}

// Inicializacao das pistas e das filas de carros
void iniciar_pistas(PISTA* pistas) {
  int i;
  for (i = 0; i < NPISTAS; i++) {
  	if(i== NPISTAS/2){
      	lado = 1;
  	}
	criar_pista(&pistas[i], i + 1);
  }
}


void encher_pistas(PISTA* pistas, CARRO* carros_base) {
  for (int i = 0; i < NPISTAS; i++) {
	CARRO* current_carro = &carros_base[i * NPROCESS];
	adicionar_carros_a_pista(&pistas[i], current_carro, pistas[i].lado);
  }
}

// Print inicial
void print_pistas(PISTA* pistas) {
  for (int i = 0; i < NPISTAS; i++) {
	printf("Pista %d lado: %d\n", pistas[i].num_pista, pistas[i].lado);
	CARRO* c = pistas[i].inicio;
	while (c != NULL) {
  	printf(" - Carro %d lado: %d\n", c->num, c->lado);
  	c = c->prox;
	}
  }
}

// Logica principal do programa

// Calcula o congestionamento
void contar_fila_carros(PISTA* pistas) {
  int cont;
  for(int i=0; i< NPISTAS; i++) {
	cont = 0;
	CARRO* aux = pistas[i].inicio;
	while (aux != NULL) {
  	cont++;
  	aux = aux->prox;
	}
	printf(" Pista %d: %d carros\n", pistas[i].num_pista, cont);
  }
}

// Trava a ponte
void fechar_ponte(PISTA* pistas, int* controls, int lado) {
  printf("\nO lado %d travou!. Congestionamento: \n", lado);
  contar_fila_carros(pistas);
  sleep(TEMPO_TRAVAR_PONTE);
  printf("A ponte destravou.\n\n");
}

// Funcao principal executada por cada processo
void cruzar_ponte(PISTA* pistas, CARRO* carro, CARRO* carros_base,int* controls) {
  int num;
  sem_t* current_sem;
  int* current_control;
  if(carro->lado == 0 ){
  	current_sem = sem0;
  	current_control = &controls[1];
  }
  else{
  	current_sem = sem1;
  	current_control = &controls[2];
  }
  while (1) {
	if (controls[1] == 0 && carro->pode_passar == 1) {
  	num = carro->num;
  	*current_control = 1;
  	sem_wait(current_sem);
  	printf("Carro %d lado (%d) cruzou a ponte\n", carro->num, carro->lado);
  	remover_carro(pistas, carros_base, carro);
  	controls[0]++;
  	if (controls[0] % TRAVAR_PONTE == 0) {
 		 fechar_ponte(pistas, controls, carro->lado);
  	}
  	sleep(FLUXO_CARROS);
  	sem_post(current_sem);
  	*current_control = 0;
  	break;
	}
  }
  kill(getpid(), SIGKILL);
}

// Inicializacao dos processos
void iniciar_processos(PISTA* pistas, int* controls, CARRO* carros_base) {
  pid_t pid;
  PISTA* aux_pista = pistas;
  for (int i = 0; i < NPISTAS; i++) {
	CARRO* lista_carros_pista = &carros_base[i * NPROCESS];
	for (int j = 0; j < NPROCESS; j++) {
  	pid = fork();
  	if (pid > 0) {
  	} else if (pid == 0) {
    	cruzar_ponte(pistas, &lista_carros_pista[j], carros_base, controls);
  	} else if (pid < 0) {
    	printf("Erro na criacao do processo!");
  	}
	}
  }
}

// Esperar processos finalizarem
void esperar_processos() {
  for (int i = 0; i < NPROCESS * NPISTAS; i++) {
	wait(NULL);
  }
}

int main() {
    
  if (NPISTAS < 1) {
	printf("Numero de pistas precisa ser maior do que 0");
	return -1;
  }
  if (NPROCESS < 1) {
	printf("Numero de threads precisa ser maior do que 0");
	return -1;
  }
 
  int shmid = shmget(
  PISTAS_KEY, sizeof(PISTA) * NPISTAS + sizeof(CARRO) * NPISTAS * NPROCESS,0666 | IPC_CREAT);
  void* shm;
 
  if (shmid < 0) {
	printf("Erro na criacao da memoria compartilhada de pistas");
	exit(1);
  }

  if ((shm = shmat(shmid, 0, 0)) < 0) {
	printf("Erro na alocacao da memoria compartilhada de pistas");
	exit(1);
  }
 
  PISTA* pistas = (PISTA*)shm;
  CARRO* carros_base = (CARRO*)(pistas + NPISTAS);  // Aritmética de ponteiros de PISTA

  shmid = shmget(CONTROLS_KEY, sizeof(int) * 3, IPC_CREAT | 0600);
  int* controls;

  if (shmid < 0) {
	printf("Erro na criacao da memoria compartilhada de controle");
	exit(1);
  }

  if ((controls = shmat(shmid, 0, 0)) < 0) {
	printf("Erro na alocacao da memoria de controle");
	exit(1);
  }
  sem0 = sem_open("/process_semaphore0", O_CREAT, 0644, 1);
  sem1 = sem_open("/process_semaphore1", O_CREAT, 0644, 1);
  if (sem0 == SEM_FAILED || sem1 == SEM_FAILED) {
	printf("Erro na criacao do semaphore");
	exit(1);
  }
 
  iniciar_pistas(pistas);
  encher_pistas(pistas, carros_base);
  print_pistas(pistas);
  printf("\nPassagens pela ponte: \n");
  iniciar_processos(pistas, controls, carros_base);
  esperar_processos(pistas);
  sem_unlink("/process_semaphore");
  printf("Acabaram os carros. Programa finalizado.\n");
  shmdt(pistas);
  shmdt(controls);
  return 0;
}




#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

int M;
long long int N;
int nCons;
long long int* buffer;

sem_t mutex;
sem_t bufferCheio;
sem_t bufferVazio;

// Função verificadora de primos
int ehPrimo(long long int n) {
  if (n <= 1) return 0;
  if (n == 2) return 1;
  if (n%2 == 0) return 0;
  for (int i = 3; i*i <= n; i += 2)
    if (n%i == 0) return 0;
  return 1;
}

// Corpo do programa da thread produtora
void* threadProd(void* args){
  long long int currN = 1; // Inteiro atual a ser adicionado no buffer
  
  // Preenchimento é feito enquanto número atual não ultrapassa o limiar
  while (currN <= N) {
    // Aguarda o buffer ficar vazio
    sem_wait(&bufferVazio);

    // Povoa o buffer com os inteiros
    for (int i = 0; i < M && currN <= N; i++)
      buffer[i] = currN++;

    // Libera as threads consumidoras
    for (int i = 0; i < M; i++)
      sem_post(&bufferCheio);
  }

  pthread_exit(NULL);
}

// Corpo do programa da thread consumidora
void* threadCons(void* args){
  static int idxBuffer = 0; // Índice atual (global) do próximo inteiro a ser lido no buffer
  static int acabou = 0; // Flag indicando se a produção já acabou
  long long int numColetado; // Cópia local do número lido no buffer
  long long int contPrimos = 0; // Contagem de primos da thread
  long long int* ret;

  ret = (long long int*)malloc(sizeof(long long int));
  if (!ret){
    printf("\nERRO: Impossível alocar variável auxiliar de retorno da thread!\n");
    pthread_exit(NULL);
  }

  while (1){
    // Espera o buffer ficar cheio
    sem_wait(&bufferCheio);
    
    // Sai do loop caso já tenha acabado de produzir
    if (acabou)
      break;

    // Zona crítica (acesso a `idxBuffer`)
    sem_wait(&mutex);
    numColetado = buffer[idxBuffer]; // Lê o próximo inteiro
    idxBuffer++;

    if (numColetado == N){ // Se o número lido é o último...
      acabou = 1; // não há mais números para testar...
      for (int i = 0; i < nCons; i++)
        sem_post(&bufferCheio); // libero todas as threads consumidoras
    }

    if (idxBuffer == M){ // Se todos os elementos do buffer já foram lidos...
      sem_post(&bufferVazio); // libera thread produtora...
      idxBuffer = 0; // e reseta o índice do buffer para 0
    }
    sem_post(&mutex);

    // Faz operação custosa (verificar se é primo)
    if (ehPrimo(numColetado))
      contPrimos++;
  }

  *ret = contPrimos;
  pthread_exit((void*)ret);
}

int main(int argc, char* argv[]){
	long long int* contPrimos;
  long long int totPrimos = 0; // Contagem total de primos
	long long int maxContPrimos; // Contagem de primos máxima dentre os consumidores
  int threadVencedora; // Índice da thread vencedora
	
	if (argc < 4){
	  printf("ERRO: Há argumentos faltantes na chamada do programa!\n"
           "Tente %s <tamanho do buffer M> <nº de inteiros N> <nº de threads consumidoras>\n", 
           argv[0]);
	  exit(EXIT_FAILURE);
	}
	
	M = atoi(argv[1]); // Tamanho do buffer
	N = atoll(argv[2]); // Limite superior do intervalo
	nCons = atoi(argv[3]); // Número de threads consumidoras
	
	buffer = (long long int*)calloc(M, sizeof(long long int));
	if (!buffer){
	  printf("ERRO: Impossível alocar memória para buffer auxiliar!\n");
	  exit(EXIT_FAILURE);
	}

  sem_init(&mutex, 0, 1); // Semáforo binário
  sem_init(&bufferCheio, 0, 0); // Começa com 0, pois nenhum consumidor pode agir de início
  sem_init(&bufferVazio, 0, 1); // Começa com 1, liberando a primeira leva de inteiros
	
	pthread_t tidProd;
	pthread_t tidsCons[nCons];
  long long int contagens[nCons];
	
  // Criando thread produtora
	if (pthread_create(&tidProd, NULL, threadProd, NULL)){
	  printf("ERRO: Impossível criar thread produtora!\n");
	  exit(EXIT_FAILURE);
	}
	
  // Criando threads consumidoras
	for (int i = 0; i < nCons; i++){
	  if (pthread_create(&tidsCons[i], NULL, threadCons, NULL)){
	    printf("ERRO: Impossível criar thread consumidora!\n");
	    exit(EXIT_FAILURE);
	  }
	}
	
  // Capturando thread produtora
	if (pthread_join(tidProd, NULL)){
	  printf("ERRO: Impossível capturar thread produtora!\n");
	  exit(EXIT_FAILURE);
	}
	
  // Capturando threads consumidoras (e assimilando seus retornos)
	for (int i = 0; i < nCons; i++){
	  if (pthread_join(tidsCons[i], (void**)&contPrimos)){
	    printf("ERRO: Impossível capturar thread consumidora!\n");
	    exit(EXIT_FAILURE);
	  }
    if (!contPrimos) // Acontece quando variável de retorno não pôde ser alocada
	    exit(EXIT_FAILURE);

    totPrimos += *contPrimos;
	  if (*contPrimos > maxContPrimos){
      threadVencedora = i;
	    maxContPrimos = *contPrimos;
    }
    contagens[i] = *contPrimos;
	  free(contPrimos);
	}

  free(buffer);
  sem_destroy(&mutex);
  sem_destroy(&bufferCheio);
  sem_destroy(&bufferVazio);

  printf("Total de primos até %lld: %lld\n", N, totPrimos);

  printf("Contagens de primos por thread:\n");
  for (int i = 0; i < nCons; i++)
    printf("Thread %d) %lld\n", i+1, contagens[i]);
  
  printf("A thread vencedora foi %d, com uma contagem de %lld primos!\n", 
          threadVencedora+1, maxContPrimos);
	
	return 0;
}
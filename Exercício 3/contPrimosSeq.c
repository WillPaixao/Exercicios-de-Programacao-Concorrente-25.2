#include <stdio.h>
#include <stdlib.h>

// Função verificadora de primos
int ehPrimo(long long int n) {
  if (n <= 1) return 0;
  if (n == 2) return 1;
  if (n%2 == 0) return 0;
  for (int i = 3; i*i <= n; i += 2)
    if (n%i == 0) return 0;
  return 1;
}

int main(int argc, char* argv[]){
  long long int N;
  long long int contPrimos = 0;

  if (argc < 2){
    printf("ERRO: Há argumento faltante!\n"
           "Tente %s <nº de inteiros N>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  N = atoll(argv[1]);

  for (long long int i = 1; i <= N; i++)
    if (ehPrimo(i))
      contPrimos++;

  printf("Contagem de primos até %lld: %lld\n", N, contPrimos);
  
  return 0;
}
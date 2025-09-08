#include <stdio.h>
#include <stdlib.h>
#include "exceptions.h"
#include "concGenerics.h"

void inc(const void* orgVal, void* destVal){
  int n = *(int*)orgVal;
  int* dest = (int*)destVal;
  *dest = n + 1;
}

void intToScream(const void* orgVal, void* destVal){
  int n = *(int*)orgVal;
  char** dest = (char**)destVal;
  *dest = (char*)malloc((n+2) * sizeof(char));
  for (int i = 0; i < n; i++)
    (*dest)[i] = 'a';
  (*dest)[n] = '\0';
}

void screamLouder(const void* orgVal, void* destVal){
  char* s = *(char**)orgVal;
  char** dest = (char**)destVal;
  for (int i = 0; s[i] != '\0'; i++)
    (*dest)[i] = s[i] - 'a' + 'A';
}

int main(int argc, char* argv[]){
  int len;
  int nWorkers;
  int* enumeration;
  char** destination;
  char flagPrint = 0;

  if (argc < 3){
    printf("To few arguments passed to program! Try %s [vec_length] [n_threads] [print_result? (OPTIONAL)]\n", argv[0]);
    return EXIT_FAILURE;
  }

  len = atoi(argv[1]);
  nWorkers = atoi(argv[2]);

  if (argc > 3)
    flagPrint = atoi(argv[3]);

  enumeration = (int*)calloc(len, sizeof(int));
  checkMalloc(enumeration);

  concEnum(enumeration, len, nWorkers);

  destination = (char**)calloc(len, sizeof(char*));
  checkMalloc(destination);

  concMap(enumeration, sizeof(int), enumeration, sizeof(int), len, inc, nWorkers);
  concMap(enumeration, sizeof(int), destination, sizeof(char*), len, intToScream, nWorkers);
  concMap(destination, sizeof(char*), destination, sizeof(char*), len, screamLouder, nWorkers);

  if (flagPrint){
    printf("Original vector:");
    for (int i = 0; i < len; i++)
      printf(" %d ", enumeration[i]);

    printf("\nMapped vector:");
    for (int i = 0; i < len; i++)
      printf(" %s ", destination[i]);
    
    putchar('\n');
  }

  free(enumeration);

  for (int i = 0; i < len; i++)
    free(destination[i]);
  free(destination);

  return EXIT_SUCCESS;
}
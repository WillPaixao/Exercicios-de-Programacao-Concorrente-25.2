#include <stdio.h>
#include <stdlib.h>
#include "exceptions.h"
#include "concGenerics.h"

void inc(const void* orgVal, void* destVal){
  int n = *(int*)orgVal;
  int* dest = (int*)destVal;
  *dest = n + 1;
}

void square(const void* orgVal, void* destVal){
  int n = *(int*)orgVal;
  int* dest = (int*)destVal;
  *dest = n * n;
}

int main(int argc, char* argv[]){
  int len;
  int nWorkers;
  int* enumeration;
  int* destination;
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

  destination = (int*)calloc(len, sizeof(int));
  checkMalloc(destination);

  concMap(enumeration, sizeof(int), destination, sizeof(int), len, square, nWorkers);

  if (flagPrint){
    printf("Original vector:");
    for (int i = 0; i < len; i++)
      printf(" %d ", enumeration[i]);

    printf("\nMapped vector:");
    for (int i = 0; i < len; i++)
      printf(" %d ", destination[i]);
    
    putchar('\n');
  }

  free(enumeration);
  free(destination);

  return EXIT_SUCCESS;
}
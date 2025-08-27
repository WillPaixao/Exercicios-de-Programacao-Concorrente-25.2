#include <stdio.h>
#include <stdlib.h>
#include "exceptions.h"
#include "concGenerics.h"

void add(void* destVal, const void* elemVal){
  int n = *(int*)elemVal;
  int* dest = (int*)destVal;
  *dest += n;
}

int main(int argc, char* argv[]){
  int len;
  int nWorkers;
  int* enumeration;
  int accumulation;
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

  concReduce(&accumulation, enumeration, sizeof(int), len, add, nWorkers);

  if (flagPrint){
    printf("Vector:");
    for (int i = 0; i < len; i++)
      printf(" %d ", enumeration[i]);
    printf("\nReduced value: %d\n", accumulation);
  }

  free(enumeration);

  return EXIT_SUCCESS;
}
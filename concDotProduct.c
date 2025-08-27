#include <stdio.h>
#include <stdlib.h>
#include "concGenerics.h"
#include "timer.h"

/**
 * @brief Auxiliar function that stores the current value in `vec1` to the x coordinate in the corresponding pair.
 */
void initPairX(void* modVal, const void* baseVal){
  float x = *(float*)baseVal;
  float* pair = *(float(*)[2])modVal;
  pair[0] = x;
}

/**
 * @brief Auxiliar function that stores the current value in `vec2` to the y coordinate in the corresponding pair.
 */
void initPairY(void* modVal, const void* baseVal){
  float y = *(float*)baseVal;
  float* pair = *(float(*)[2])modVal;
  pair[1] = y;
}

/** 
 * @brief Auxiliar argument function to `concMap()` that multiplies the values in a pair and stores in another vector.
 */
void vecMul(void* modVal, const void* baseVal){
  float* pair = *(float(*)[2])baseVal;
  float* mod = (float*)modVal;
  *mod = pair[0] * pair[1];
}

/** 
 * @brief Auxiliar argument function to `concReduce()` that accumulates the sum of `float` values in `destVal`.
 */ 
void add(void* destVal, const void* elemVal){
  float n = *(float*)elemVal;
  float* dest = (float*)destVal;
  *dest += n;
}

/**
 * @brief Function that computes the dot product between two `float` vectors concurrently.
 * 
 * @param vec1 Base pointer to the 1st vector.
 * @param vec2 Base pointer to the 2nd vector.
 * @param len Length (or dimension) of both vectors.
 * @param nWorkers Number of threads to be used.
 * @return Dot product of the two vectors.
 */
float concDotProduct(float* vec1, float* vec2, int len, int nWorkers){
  float (*arrOfPairs)[2]; // Pointer to array of two elements (pair).
  float* prods;           // Vector of products.
  float dotProd;

  if (!vec1 || !vec2 || len <= 0){
    printf("ERROR: Invalid argument(s) passed to concDotProduct()!");
    exit(EXIT_FAILURE);
  }

  arrOfPairs = (float(*)[2])calloc(len, sizeof(float[2])); // What a cast! XD
  if (!arrOfPairs){
    printf("ERROR: Error in alloccation during the computation of dot product!");
    exit(EXIT_FAILURE);
  }

  prods = (float*)calloc(len, sizeof(float));
  if (!prods){
    printf("ERROR: Error in alloccation during the computation of dot product!");
    free(arrOfPairs);
    exit(EXIT_FAILURE);
  }

  // Setting the values in the pairs from both vectors.
  concMap(arrOfPairs, sizeof(float[2]), vec1, sizeof(float), len, initPairX, nWorkers);
  concMap(arrOfPairs, sizeof(float[2]), vec2, sizeof(float), len, initPairY, nWorkers);

  // Storing the product of the values in each pair in `prods`.
  concMap(prods, sizeof(float), arrOfPairs, sizeof(float[2]), len, vecMul, nWorkers);

  // Adding the products together in a single float (the dot product).
  concReduce(&dotProd, prods, sizeof(float), len, add, nWorkers);

  free(arrOfPairs);
  free(prods);

  return dotProd;
}

int main(int argc, char* argv[]){
  FILE* bin;
  int len;
  float* vec1;
  float* vec2;
  float seqDotProd;
  float concDotProd;
  float error;
  const char* fileName;
  int nWorkers;
  double begin;
  double end;
  char flagPrint = 0;

  // Ensuring the arguments to the program are correct.
  if (argc < 3){
    printf("To few arguments passed to program! Try %s [file_path] [n_threads] [print_vectors? (OPTIONAL)]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  fileName = argv[1];
  nWorkers = atoi(argv[2]);

  if (argc > 3)
    flagPrint = atoi(argv[3]);

  // Opening the file in read-only mode.
  if (!(bin = fopen(fileName, "r"))){
    printf("ERROR: Could not read from binary file!\n");
    exit(EXIT_FAILURE);
  }

  // Reading the dimension of the vectors contained in the binary file.
  if (fread(&len, sizeof(int), 1, bin) != 1){
    printf("ERROR: Error in reading length of vectors from binary file!\n");
    fclose(bin);
    exit(EXIT_FAILURE);
  }

  // Allocating the 1st vector of `float`s according to given `len`.
  vec1 = (float*)calloc(len, sizeof(float));
  if (!vec1){
    printf("\nERROR: Failure in allocating memory for vector 1!\n");
    exit(EXIT_FAILURE);
  }

  // Reading the 1st vector from the binary file.
  if (fread(vec1, sizeof(float), len, bin) != len){
    printf("ERROR: Error in reading the 1st vector from binary!\n");
    fclose(bin);
    free(vec1);
    exit(EXIT_FAILURE);
  }

  // Allocating the 2nd vector of `float`s according to given `len`.
  vec2 = (float*)calloc(len, sizeof(float));
  if (!vec2){
    printf("\nERROR: Failure in allocating memory for vector 2!\n");
    free(vec1);
    exit(EXIT_FAILURE);
  }

  // Reading the 2nd vector from the binary file.
  if (fread(vec2, sizeof(float), len, bin) != len){
    printf("ERROR: Error in reading the 2nd vector from binary!\n");
    fclose(bin);
    free(vec1);
    free(vec2);
    exit(EXIT_FAILURE);
  }

  // Reading the sequential dot product stored in the binary file.
  if (fread(&seqDotProd, sizeof(float), 1, bin) != 1){
    printf("ERROR: Error in reading the result of sequential dot product from binary!\n");
    fclose(bin);
    free(vec1);
    free(vec2);
    exit(EXIT_FAILURE);
  }

  // Output the data read from file, if the user asked for it.
  if (flagPrint){
    printf("Vector 1:");
    for (int i = 0; i < len; i++)
      printf(" %f ", vec1[i]);
    printf("\nVector 2:");
    for (int i = 0; i < len; i++)
      printf(" %f ", vec2[i]);
    putchar('\n');
  }

  // Calculating the dot product concurrently.
  GET_TIME(begin);
  concDotProd = concDotProduct(vec1, vec2, len, nWorkers);
  GET_TIME(end);

  // Measuring the error.
  error = (seqDotProd - concDotProd) / seqDotProd;
  if (error < 0)
    error = -error;

  // Printing the results.
  printf("Sequential result: %f\n", seqDotProd);
  printf("Concurrent result: %f\n", concDotProd);
  printf("Error: %f\n", error);
  printf("Elapsed time to compute dot product: %lf s\n", end-begin);

  // Freeing memory
  fclose(bin);
  free(vec1);
  free(vec2);

  return EXIT_SUCCESS;
}
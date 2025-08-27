#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "timer.h"

#define DEFAULT_MIN -10
#define DEFAULT_MAX 10

/**
 * @brief Function that generates a random float in [0,1].
 * 
 * @return Float value from 0 to 1.
 */
float randFloat(){
  return ((float)rand()) / RAND_MAX;
}

/**
 * @brief Function that generates a random float in [`min`,`max`].
 * 
 * @return Float value from `min` to `max`.
 */
float randFloatInterval(float min, float max){
  return randFloat() * (max - min) + min; 
}

/**
 * @brief Sets the values of a `float` vector to random floats in the interval [`minVal`,`maxVal`].
 * 
 * @param vec Base pointer of the `float` vector.
 * @param len Length of the vector.
 * @param minVal Minimum value that an element of the vector can assume.
 * @param maxVal Maximum value that an element of the vector can assume.
 * 
 * @warning This function assumes that the dedicated length of the vector is, at least, equal to `len`.
 */
void setRandVec(float vec[], int len, float minVal, float maxVal){
  if (!vec || len <= 0 || minVal > maxVal)
    return;
  for (int i = 0; i < len; i++)
    vec[i] = randFloatInterval(minVal, maxVal);
}

/**
 * @brief Function that computes the dot product (sequentially) between two `float` vectors.
 * 
 * @param vec1 Base pointer to the first `float` vector.
 * @param vec1 Base pointer to the second `float` vector.
 * @param len Length (or dimension) of both vectors.
 */
float dotProduct(float vec1[], float vec2[], int len){
  if (len <= 0){
    printf("\nERROR: Invalid length %d passed to dotProduct!\n", len);
    return 0;
  }
  float accum = 0;
  for (int i = 0; i < len; i++)
    accum += vec1[i] * vec2[i];
  return accum;
}

int main(int argc, char* argv[]){
  FILE* bin;
  float* vec1;
  float* vec2;
  float dotProd;
  int len;
  const char* filePath;
  float min, max;
  char flagPrint = 0;
  double begin, end;

  srand(time(NULL));

  if (argc < 3){
    printf("To few arguments passed to program! Try %s [vec_length] [file_path] [print_result? (OPTIONAL)] [min_val (OPTIONAL)] [max_val (OPTIONAL)]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  len = atoi(argv[1]);
  filePath = argv[2];

  if (argc > 3)
    flagPrint = atoi(argv[3]);

  if (argc <= 4){
    min = DEFAULT_MIN;
    max = DEFAULT_MAX;
  }
  else if (argc == 5){
    min = atof(argv[4]);
    if (min > 0){
      max = min;
      min = -min;
    }
    else {
      max = -min;
    }
  }
  else {
    min = atof(argv[4]);
    max = atof(argv[5]);
  }

  vec1 = (float*)calloc(len, sizeof(float));
  if (!vec1){
    printf("\nERROR: Failure in allocating memory for vector 1!\n");
    exit(EXIT_FAILURE);
  }

  vec2 = (float*)calloc(len, sizeof(float));
  if (!vec2){
    printf("\nERROR: Failure in allocating memory for vector 2!\n");
    free(vec1);
    exit(EXIT_FAILURE);
  }

  setRandVec(vec1, len, min, max);
  setRandVec(vec2, len, min, max);

  GET_TIME(begin);
  dotProd = dotProduct(vec1, vec2, len);
  GET_TIME(end);

  if (flagPrint){
    printf("Vector 1:");
    for (int i = 0; i < len; i++)
      printf(" %f ", vec1[i]);
    printf("\nVector 2:");
    for (int i = 0; i < len; i++)
      printf(" %f ", vec2[i]);
    printf("\nDot product: %f\n", dotProd);
  }

  if (!(bin = fopen(filePath, "w"))){
    printf("ERROR: Could not save the results!\n");
    exit(EXIT_FAILURE);
  }

  if (fwrite(&len, sizeof(int), 1, bin) != 1){
    printf("ERROR: Error in writing the length of vectors in binary!\n");
    fclose(bin);
    free(vec1);
    free(vec2);
    exit(EXIT_FAILURE);
  }

  if (fwrite(vec1, sizeof(float), len, bin) != len){
    printf("ERROR: Error in writing the 1st vector in binary!\n");
    fclose(bin);
    free(vec1);
    free(vec2);
    exit(EXIT_FAILURE);
  }

  if (fwrite(vec2, sizeof(float), len, bin) != len){
    printf("ERROR: Error in writing the 2nd vector in binary!\n");
    fclose(bin);
    free(vec1);
    free(vec2);
    exit(EXIT_FAILURE);
  }

  if (fwrite(&dotProd, sizeof(float), 1, bin) != 1){
    printf("ERROR: Error in writing the dot product of vectors in binary!\n");
    fclose(bin);
    free(vec1);
    free(vec2);
    exit(EXIT_FAILURE);
  }

  printf("Writing in %s was successful!\n", filePath);
  printf("Time elapsed to compute the dot product: %lf s\n", end-begin);

  fclose(bin);
  free(vec1);
  free(vec2);

  return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "exceptions.h"
#include "concGenerics.h"

/**
 * @brief Structure that encapsulates the arguments passed to threadEnum().
 * 
 * @sa See threadEnum() for the function that uses this.
 * @sa See concEnum() for the main function of this.
 */
typedef struct {
  int* segBase; /**< Base pointer to the segment of `int` vector. */
  int idxBase;  /**< Absolute index value of the base of segment. */
  int segLen;   /**< Length of the segment. */
} t_args_enum;

/**
 * @brief Structure that encapsulates the arguments passed to threadMap().
 * 
 * @sa See threadMap() for the function that uses this.
 * @sa See concMap() for the main function of this.
 */
typedef struct {
  char* orgSegBase;                   /**< Base pointer to the origin segment. */
  char* destSegBase;                  /**< Base pointer to the destination segment. */
  int segLen;                         /**< Length of the segments. */
  size_t orgElemSize;                 /**< Size, in bytes, of each element in the origin segment. */
  size_t destElemSize;                /**< Size, in bytes, of each element in the destination segment. */
  void (*func)(void*, const void*);   /**< Mapping function. */
} t_args_map;

/**
 * @brief Structure that encapsulates the arguments passed to threadReduce().
 * 
 * @sa See threadReduce() for the function that uses this.
 * @sa See concReduce() for the main function of this.
 */
typedef struct {
  char* segBase;                    /**< Base pointer to the segment. */
  int segLen;                       /**< Length of the segment. */
  size_t elemSize;                  /**< Size, in bytes, of each element in the segment. */
  void (*func)(void*, const void*); /**< Reducing function. */
} t_args_reduce;

/**
 * @brief Auxiliar function that treats inconsistent values for `nWorkers`.
 * 
 * @param nWorkers Number of threads.
 * @param vecLen Length of the vector.
 * @retval 1 Se `nWorkers <= 0`.
 * @retval vecLen Se `nWorkers > vecLen`.
 * @retval nWorkers Caso contrário.
 */
static int treatNWorkers(int nWorkers, int vecLen){
  if (nWorkers > vecLen)
    return vecLen;
  if (nWorkers <= 0)
    return 1;
  return nWorkers;
}

/**
 * @brief Auxiliar thread function for writing an enumeration on a segment of a vector.
 * 
 * @param args Parameter that points to a `t_args_enum` struct.
 * @return `NULL` pointer.
 * 
 * @sa See concEnum() for the main function of this.
 */
static void* threadEnum(void* args){
  t_args_enum arg = *(t_args_enum*)args;

  for (int i = arg.idxBase; i < arg.idxBase + arg.segLen; i++)
    arg.segBase[i] = i;

  free(args);
  
  pthread_exit(NULL);
}

int concEnum(int* dest, int len, int nWorkers){
  nWorkers = treatNWorkers(nWorkers, len);
  
  pthread_t tids[nWorkers];
  t_args_enum* args;

  for (int i = 0; i < nWorkers; i++){
    args = (t_args_enum*)malloc(sizeof(t_args_enum));
    checkMalloc(args);
    
    args->idxBase = i * (len / nWorkers);
    args->segBase = dest;
    args->segLen = (len / nWorkers) + (i == nWorkers-1 ? (len % nWorkers) : 0);

    checkThreadCreate(pthread_create(&tids[i], NULL, threadEnum, args), args);
  }

  for (int i = 0; i < nWorkers; i++)
    checkThreadJoin(pthread_join(tids[i], NULL));

  return EXIT_SUCCESS;
}

/**
 * @brief Auxiliar thread function for writing the mapped version of the origin segment into the destination segment.
 * 
 * @param args Parameter that points to a `t_args_map` struct.
 * @return `NULL` pointer.
 * 
 * @sa See concMap() for the main function of this.
 */
static void* threadMap(void* args){
  t_args_map arg = *(t_args_map*)args;

  for (char *currOrg = arg.orgSegBase, *currDest = arg.destSegBase; 
       currOrg < arg.orgSegBase + arg.orgElemSize * arg.segLen; 
       currOrg += arg.orgElemSize, currDest += arg.destElemSize)
    arg.func(currDest, currOrg); // Updating value by reference
  
  free(args);

  pthread_exit(NULL);
}

int concMap(void* dest,
            size_t destElemSize,
            void* org,
            size_t orgElemSize, 
            int len, 
            void (*func)(void*, const void*), 
            int nWorkers){
  nWorkers = treatNWorkers(nWorkers, len);

  checkLength(len);
  checkSize(orgElemSize);
  checkSize(destElemSize);

  pthread_t tids[nWorkers];
  t_args_map* args;

  for (int i = 0; i < nWorkers; i++){
    args = (t_args_map*)malloc(sizeof(t_args_map));
    checkMalloc(args);

    args->orgElemSize = orgElemSize;
    args->orgSegBase = (char*)org + i * orgElemSize * (len / nWorkers);
    args->destElemSize = destElemSize;
    args->destSegBase = (char*)dest + i * destElemSize * (len / nWorkers);
    args->segLen = (len / nWorkers) + (i == nWorkers-1 ? len % nWorkers : 0);
    args->func = func;

    checkThreadCreate(pthread_create(&tids[i], NULL, threadMap, args), args);
  }

  for (int i = 0; i < nWorkers; i++)
    checkThreadJoin(pthread_join(tids[i], NULL));

  return EXIT_SUCCESS;
}

/**
 * @brief Auxiliar thread function for reducing the elements of a segment onto a single value.
 * 
 * @param args Parameter that points to a `t_args_reduce` struct.
 * @return Pointer to the reduced value.
 * 
 * @sa See concReduce() for the main function of this.
 */
static void* threadReduce(void* args){
  t_args_reduce arg = *(t_args_reduce*)args;
  void* accum = malloc(arg.elemSize); // TODO: Defend

  memcpy(accum, arg.segBase, arg.elemSize); // Copying the first value to the accumulator

  for (char* curr = arg.segBase + arg.elemSize;
       curr < arg.segBase + arg.elemSize * arg.segLen;
       curr += arg.elemSize)
    arg.func(accum, curr);
  
  pthread_exit(accum);
}

int concReduce(void* dest,
               void* vec,
               size_t elemSize,
               int len,
               void (*func)(void*, const void*),
               int nWorkers){
  nWorkers = treatNWorkers(nWorkers, len);
  checkLength(len);
  checkSize(elemSize);

  pthread_t tids[nWorkers];
  t_args_reduce* args;
  void* ret;
  
  for (int i = 0; i < nWorkers; i++){
    args = (t_args_reduce*)malloc(sizeof(t_args_reduce));
    checkMalloc(args);

    args->segBase = (char*)vec + i * elemSize * (len / nWorkers);
    args->segLen = (len / nWorkers) + (i == nWorkers-1 ? len % nWorkers : 0);
    args->elemSize = elemSize;
    args->func = func;

    checkThreadCreate(pthread_create(&tids[i], NULL, threadReduce, args), args);
  }

  for (int i = 0; i < nWorkers; i++){
    checkThreadJoin(pthread_join(tids[i], &ret));
    func(dest, ret);
    free(ret);
  }

  return EXIT_SUCCESS;
}
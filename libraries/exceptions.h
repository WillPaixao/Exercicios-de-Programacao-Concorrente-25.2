/**
 * @file exceptions.h
 * @brief Library of exception checking functions.
 * 
 * Library containing some auxiliar macros for exception checking, treating and raising.
 * 
 * @sa The <pthread.h> library contains most of the functions whose returns are treated here.
 */

#pragma once

#define ERROR_MALLOC 2          /**< Error code for malloc failure.           */
#define ERROR_THREAD_CREATE 3   /**< Error code for error in thread creation. */
#define ERROR_THREAD_JOIN 4     /**< Error code for error in thread joining.  */
#define ERROR_LENGTH 5          /**< Error code for invalid length of vector. */
#define ERROR_SIZE 6            /**< Error code for invalid size of element.  */

int _checkMalloc(void* ptr);
int _checkThreadCreate(int pthreadRet, void* threadArgs);
int _checkThreadJoin(int pthreadRet);
int _checkLength(int len);
int _checkSize(size_t size);

/**
 * @brief Auxiliar macro for defining error checking macros.
 * 
 * @param checkFunc Function that effectively checks the condition for the error.
 * @param errorID Integer identifier of the error to be raised.
 * @param errorMsg String constant that details the error.
 * @param ... Parameters passed to `checkFunc`.
 */
#define makeError(checkFunc, errorID, errorMsg, ...) do {   \
  if (checkFunc(__VA_ARGS__)){                              \
    fprintf(stderr,                                         \
            "ERROR (file %s, function %s, line %d): %s\n",  \
            __FILE__,                                       \
            __func__,                                       \
            __LINE__,                                       \
            errorMsg);                                      \
    return errorID;                                         \
  }                                                         \
} while (0)

/**
 * @brief Checks whether a pointer returned by malloc is NULL or not and raises `ERROR_MALLOC` if so.
 * @param ptr Pointer to memory address.
 */
#define checkMalloc(ptr) \
  makeError(_checkMalloc, ERROR_MALLOC, "Cannot malloc properly!", ptr) 

/**
 * @brief Checks if thread creation succeeded and raises `ERROR_THREAD_CREATE` if not (freeing its args).
 * @param pthreadRet `pthread_create` returned value.
 * @param threadArgs Pointer to memory allocated args.
 */
#define checkThreadCreate(pthreadRet, threadArgs) \
  makeError(_checkThreadCreate, ERROR_THREAD_CREATE, "Cannot create thread!", pthreadRet, threadArgs)

/**
 * @brief Checks if thread joining succeeded and raises `ERROR_THREAD_JOIN` if not.
 * @param pthreadRet `pthread_join` returned value.
 */
#define checkThreadJoin(pthreadRet) \
  makeError(_checkThreadJoin, ERROR_THREAD_JOIN, "Cannot join thread!", pthreadRet)

/**
 * @brief Checks if provided length is valid (positive).
 * @param len Length of the vector.
 */
#define checkLength(len) \
  makeError(_checkLength, ERROR_LENGTH, "Invalid length!", len)

/**
 * @brief Checks if provided size of element is valid (positive).
 * @param size Size of element.
 */
#define checkSize(size) \
  makeError(_checkSize, ERROR_SIZE, "Invalid size!", size)

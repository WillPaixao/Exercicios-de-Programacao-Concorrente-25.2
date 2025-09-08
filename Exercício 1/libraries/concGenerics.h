/**
 * @file concGenerics.h
 * @brief Library of generic concurrent vector manipulation functions.
 * 
 * Library containing the main generic list manipulation functions with support to concurrency.
 * They all expect generic arrays (of type `void*`) and functions that operate on this type.
 */

#pragma once

/**
 * @brief Function that sets an enumeration, starting from 0, on a given `int` vector.
 * 
 * @param dest Base pointer of the vector to be written.
 * @param len Intended length of the vector.
 * @param nWorkers Number of threads to be used.
 * @return 0 in success, error code otherwise.
 * 
 * @warning If `nWorkers` is less than or equal to 0, its value is taken as 1. If it is greater than the number of elements in the vector, then it is capped by the provided length of the vector.
 * @warning If `len` is less than or equal to 0, the function returns `ERROR_LENGTH`.
 */
int concEnum(int* dest, int len, int nWorkers);

/**
 * @brief Function that applies a mapping function to the elements of a vector.
 * 
 * @param org Base pointer of the origin vector.
 * @param orgElemSize Size, in bytes, of each element in the origin vector.
 * @param dest Base pointer of the destination vector.
 * @param destElemSize Size, in bytes, of each element in the destination vector.
 * @param len Length of the vectors.
 * @param func Mapping function.
 * @param nWorkers Number of threads to be used.
 * @return 0 in success, error code otherwise.
 *
 * @note The mapping function `func`, with signature `func(void* modVal, const void* baseVal)`, receives a value from the original vector (`baseVal`), as its first argument, and sets the mapped version on the memory address pointed by the second argument (`modVal`). An example of mapping function in this style, that receives an `int` and yields its incremented value as an `int`, would be: 
 * ```c
 * void inc(void* modVal, const void* baseVal){
 *    int n = *(int*)baseVal;
 *    int* mod = (int*)modVal;
 *    *mod = n + 1;
 * }
 * ```
 * Assuming that `vec` was declared as `int*`, the corresponding call to `concMap()`, incrementing all of its values on itself, would be (also with `len` being the length of `vec` and `nWorkers` the number of threads):
 * ```c
 * concMap(vec, sizeof(int), vec, sizeof(int), len, inc, nWorkers);
 * ```
 * 
 * @note Only the `dest` vector is modified by this function.
 * 
 * @warning If `nWorkers` is less than or equal to 0, its value is taken as 1. If it is greater than the number of elements in the vector, then it is capped by the provided length of the vector.
 * @warning It is assumed that the length of both vectors — `org` and `dest` — is equal to `len`.
 * @warning If `len` is less than or equal to 0, the function returns `ERROR_LENGTH`.
 * @warning If `elemSize` is less than or equal to 0, the function returns `ERROR_SIZE`.
 */
int concMap(void* dest,
            size_t destElemSize,
            void* org,
            size_t orgElemSize, 
            int len, 
            void (*func)(void*, const void*), 
            int nWorkers);

/**
 * @brief Function that reduces a vector to a single value.
 * 
 * @param dest Pointer to the variable in which the result of reducing is to be saved.
 * @param vec Base pointer of the vector.
 * @param elemSize Size, in bytes, of each element in the vector.
 * @param len Length of the vector.
 * @param func Reducing function.
 * @param nWorkers Number of threads to be used.
 * @return 0 in success, error code otherwise.
 * 
 * @note The reducing function `func`, with signature `func(void* destVal, const void* elemVal)`, has the effect of `*destVal = *destVal # *elemVal`, in which `#` is some binary operation between values of the same type. Here is an example of `add`ing function between `int`s:
 * ```c
 * void add(void* destVal, const void* elemVal){
 *    int n = *(int*)elemVal;
 *    int* dest = (int*)destVal;
 *    *dest += n;
 * }
 * ```
 * It could be passed to `concReduce()` as (with `sum`, `intVec`, `len` and `nWorkers` being predefined variables):
 * ```c
 * concReduce(&sum, intVec, sizeof(int), len, add, nWorkers);
 * ```
 * @note Only the value pointed by `dest` is modified by this function.
 * 
 * @warning If `nWorkers` is less than or equal to 0, its value is taken as 1. If it is greater than the number of elements in the vector, then it is capped by the provided length of the vector.
 * @warning If `len` is less than or equal to 0, the function returns `ERROR_LENGTH`.
 * @warning If `elemSize` is equal to 0, the function returns `ERROR_SIZE`.
 */
int concReduce(void* dest,
               void* vec,
               size_t elemSize,
               int len,
               void (*func)(void*, const void*),
               int nWorkers);
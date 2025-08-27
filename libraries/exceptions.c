#include <stdlib.h>
#include <stdio.h>
#include "exceptions.h"

int _checkMalloc(void* ptr){
  return !ptr;
}

int _checkThreadCreate(int pthreadRet, void* threadArgs){
  if (pthreadRet)
    free(threadArgs);
  return pthreadRet;
}

int _checkThreadJoin(int pthreadRet){
  return pthreadRet;
}

int _checkLength(int len){
  return len <= 0;
}

int _checkSize(size_t size){
  return size == 0;
}
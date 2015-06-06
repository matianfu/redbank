/*
 * mem.c
 *
 *  Created on: Jun 5, 2015
 *      Author: ma
 */

#include <stdlib.h>
#include "mem.h"

void * MALLOC(size_t size) {
  return malloc(size);
}

void FREE (void *ptr) {
  if (ptr)
    free(ptr);
}


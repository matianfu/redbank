/*
 * buddy.c
 *
 *  Created on: Jun 1, 2015
 *      Author: ma
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "buddy.h"

#define sizes           MAX_ORDER

// the order ranges 0..(sizes - 1), the largest memblock is 2**(sizes - 1)
#define SIZEOF_MEMPOOL  (1 << (sizes - 1))

/* memory pool */
uint8_t mempool[SIZEOF_MEMPOOL];

/* pointers to the free space lists */
pointer freelists[sizes];

/* blocks in freelists[i] are of size 2**i. */
#define BLOCKSIZE(i) (1 << (i))

/* the address of the buddy of a block from freelists[i]. */
#define BUDDYOF(b,i) ((pointer)( ((uintptr_t)b) ^ (1 << (i)) ))

pointer bmalloc(int size) {
  int i;

  /* compute i as the least integer such that i >= log2(size) */
  for (i = 0; BLOCKSIZE( i ) < size; i++)
    ;

  if (i >= sizes) {
    printf("no space available");
    return NULL;
  }
  else if (freelists[i] != NULL) {

    /* we already have the right size block on hand */
    pointer block;
    block = freelists[i];
    freelists[i] = *(pointer *) freelists[i];
    return block;

  }
  else {
    /* we need to split a bigger block */
    pointer block, buddy;
    block = bmalloc(BLOCKSIZE(i + 1));

    if (block != NULL) {
      /* split and put extra on a free list */
      buddy = BUDDYOF(block, i);
      *(pointer *) buddy = freelists[i];
      freelists[i] = buddy;
    }

    return block;
  }
}

pointer bmalloc2(int size) {

  int i, fit;
  pointer block, buddy;

  for (fit = 0; BLOCKSIZE(fit) < size; fit++)
    ;

  if (freelists[fit]) {
    block = freelists[fit];
    freelists[fit] = *(pointer*) freelists[fit];

    return block;
  }

  i = fit;
  while (freelists[i] == NULL) {
    i++;
    if (i >= sizes)
      return NULL;
  }

  block = freelists[i];
  while (i > fit) {
    i--;
    buddy = BUDDYOF(block, i);
    freelists[i] = buddy;
  }

  return block;
}

void bfree(pointer block, int size) {
  int i;
  pointer * p;
  pointer buddy;

  /* compute i as the least integer such that i >= log2(size) */
  for (i = 0; BLOCKSIZE( i ) < size; i++)
    ;

  /* see if this block's buddy is free */
  buddy = BUDDYOF(block, i);
  p = &freelists[i];
  while ((*p != NULL) && (*p != buddy))
    p = (pointer *) *p;

  if (*p != buddy) {

    /* buddy not free, put block on its free list */
    *(pointer *) block = freelists[i];
    freelists[i] = block;

  }
  else {

    /* buddy found, remove it from its free list */
    *p = *(pointer *) buddy;

    /* deallocate the block and its buddy as one block */
    if (block > buddy) {
      bfree(buddy, BLOCKSIZE(i + 1));
    }
    else {
      bfree(block, BLOCKSIZE(i + 1));
    }
  }
}

void bfree2(pointer block, int i) {

  pointer buddy;
  pointer * p;

  for (;;) {
    buddy = BUDDYOF(block, i);
    p = &freelists[i];

    while ((*p != NULL) && (*p != buddy))
      p = (pointer *) *p;

    if (*p != buddy) {
      // buddy not free
      *(pointer*)block = freelists[i];
      freelists[i] = block;
      return;
    }
    else {
      // remove it
      p = (pointer*) *p;
      i++;
    }
  }
}

void buddy_init() {

  int i;

  printf("buddy init, max order (exclusive): %d\n", sizes);
  printf("buddy init, mem pool size: %d\n", SIZEOF_MEMPOOL);

  for (i = 0; i < sizes; i++) {
    freelists[i] = NULL;
  }

  memset(mempool, 0, SIZEOF_MEMPOOL);

  freelists[sizes - 1] = mempool;
}

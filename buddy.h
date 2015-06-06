/*
 * buddy.h
 *
 *  Created on: Jun 1, 2015
 *      Author: ma
 */

#ifndef BUDDY_H_
#define BUDDY_H_

#define MAX_ORDER (5)   // exclusive
#define MIN_ORDER (2)   // inclusive

typedef void * pointer; /* used for untyped pointers */

void buddy_init();
void* bmalloc(int size);
void bfree(pointer block, int size);

#endif /* BUDDY_H_ */

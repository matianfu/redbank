/*
 * rb_hash.h
 *
 *  Created on: Jun 1, 2015
 *      Author: ma
 */

#ifndef HASH_H_
#define HASH_H_

#include <stdint.h>

// 32 bit offset_basis = 2166136261
#define FNV1A_OFFSET_BASIS_UINT32   ((uint32_t)2166136261)
#define HASH_SEED                   FNV1A_OFFSET_BASIS_UINT32

uint32_t RBHash32(uint32_t hash, int num, uint8_t* octet);

#endif /* HASH_H_ */

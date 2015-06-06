

/*
 * rb_hash.c
 *
 *  Created on: Jun 1, 2015
 *      Author: ma
 */

#include "hash.h"

#include <stdint.h>

/**
 * Reference: http://www.isthe.com/chongo/tech/comp/fnv/
 *
 * Choose 32bit fnv-1a hash
 *
 * The pseudo code:
 * hash = offset_basis
 * for each octet_of_data to be hashed
 *   hash = hash xor octet_of_data
 *   hash = hash * FNV_prime
 * return hash
 *
 */

// 32 bit FNV_prime = 224 + 28 + 0x93 = 16777619
#define FNV1A_PRIME                 ((uint32_t)16777619)
// 32 bit offset_basis = 2166136261
#define FNV1A_OFFSET_BASIS_UINT32   ((uint32_t)2166136261)

uint32_t RBHash32(uint32_t hash, int num, uint8_t* octet) {

  while (num) {
    hash = hash ^ (uint32_t)(*octet);
    hash = hash * FNV1A_PRIME;
    octet++;
    num--;
  }

  return hash;
}




















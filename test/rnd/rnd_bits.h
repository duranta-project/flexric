/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef RND_BITS_H
#define RND_BITS_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

// Uniform random value in [0, 2^n), n in [1, 64]
// rand() provides at least 15 random bits, so 16 bits are taken per call
static inline
uint64_t rand_bits(unsigned n)
{
  assert(n > 0 && n <= 64);
  uint64_t v = 0;
  for(size_t i = 0; i < 4; ++i)
    v = (v << 16) | (uint64_t)(rand() & 0xFFFF);
  return n == 64 ? v : v & ((UINT64_C(1) << n) - 1);
}

#endif

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define N_SHIFT 100
#define NORM_FACTOR  23156
#define THRESHOLD  0.3
#define N_FFT 4096
#define N_rx  8
#define NR_NB_SC_PER_RB  12

typedef struct complex16 {
  int16_t r;
  int16_t i;
} c16_t;

typedef struct complex8 {
  int8_t r;
  int8_t i;
} c8_t;

__attribute__((always_inline)) inline uint32_t c16amp2(const c16_t a) {
    return a.r * a.r + a.i * a.i;
}

#endif

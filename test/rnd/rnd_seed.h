/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef RND_SEED_H
#define RND_SEED_H

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Seed rand() for a test, with arg if not NULL (e.g., argv[1]), otherwise
// with the time and the PID, so that runs started in the same second differ.
// The seed is printed to stderr, which is unbuffered and therefore also shown
// when the test aborts. A failing run is reproduced by passing that seed as
// argument, e.g., ./test_rc_sm 1579
static inline
unsigned init_rnd_seed(char const* arg)
{
  unsigned seed = (unsigned)time(NULL) ^ (unsigned)getpid();

  if(arg != NULL){
    char* end = NULL;
    unsigned long const val = strtoul(arg, &end, 10);
    if(*arg == '\0' || *end != '\0' || val > UINT_MAX){
      fprintf(stderr, "Invalid random seed '%s'\n", arg);
      exit(EXIT_FAILURE);
    }
    seed = (unsigned)val;
  }

  fprintf(stderr, "Random seed: %u\n", seed);
  srand(seed);
  return seed;
}

#endif

/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */


#include "../../src/ric/near_ric_api.h"

#include <arpa/inet.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>

const uint16_t MAC_ran_func_id = 142;
const uint16_t RLC_ran_func_id = 143;
const uint16_t PDCP_ran_func_id = 144;
const uint16_t SLICE_ran_func_id = 145; // Not implemented yet
const uint16_t KPM_ran_func_id = 147;
const char* cmd = "5_ms";


static
void stop_and_exit()
{
  // Stop the RIC
  stop_near_ric_api();

  exit(EXIT_SUCCESS);
}

static 
pthread_once_t once = PTHREAD_ONCE_INIT;

static
void sig_handler(int sig_num)
{
  printf("\n[NEAR-RIC]: Abruptly ending with signal number = %d\n", sig_num);
  // For the impatient, do not break my code
  pthread_once(&once, stop_and_exit);
}


int main(int argc, char *argv[])
{
  // Signal handler
  signal(SIGINT, sig_handler);

  fr_args_t args = init_fr_args(argc, argv);
 
  // Init the RIC
  init_near_ric_api(&args);

  while(1){
    poll(NULL, 0, 1000);
  }
  return EXIT_SUCCESS;
}


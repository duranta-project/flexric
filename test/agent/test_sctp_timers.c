/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/*
 * The E2 agent sets its own SCTP timers, so that a nearRT-RIC that went away
 * without a SHUTDOWN is noticed within seconds instead of the minutes the kernel
 * defaults take.
 *
 * Checks where the values come from -- fr_args_t, then the config file, then the
 * defaults -- and that they reach the socket.
 */

// The asserts are the test: keep them in builds that define NDEBUG.
#undef NDEBUG

#include "agent/endpoint_agent.h"
#include "util/conf_file.h"

#include <assert.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Writes a config file and points args at it. The caller unlinks path.
static
void write_conf(fr_args_t* args, char* path, char const* content)
{
  int const fd = mkstemp(path);
  assert(fd != -1);
  FILE* fp = fdopen(fd, "w");
  assert(fp != NULL);
  fputs(content, fp);
  fclose(fp);

  strncpy(args->conf_file, path, sizeof(args->conf_file) - 1);
}

// The RAN fills fr_args_t itself and never sets conf_file.
static
void test_defaults(void)
{
  fr_args_t const args = {0};
  fr_sctp_timers_t const t = get_e2ap_sctp_timers(&args);

  assert(t.hb_interval_ms == SCTP_DEFAULT_HB_INTERVAL_MS);
  assert(t.rto_max_ms == SCTP_DEFAULT_RTO_MAX_MS);
  assert(t.max_retrans == SCTP_DEFAULT_MAX_RETRANS);

  printf("[TEST]: SCTP timers default when nothing sets them\n");
}

static
void test_conf_file(void)
{
  fr_args_t args = {0};
  char path[] = "/tmp/test_sctp_timers_XXXXXX";
  write_conf(&args, path,
             "[NEAR-RIC]\n"
             "NEAR_RIC_IP = 127.0.0.1\n"
             "# SCTP_HB_INTERVAL_MS = 9999\n"
             "SCTP_HB_INTERVAL_MS = 700\n"
             "SCTP_RTO_MAX_MS = 1500\n");

  fr_sctp_timers_t const t = get_e2ap_sctp_timers(&args);
  unlink(path);

  assert(t.hb_interval_ms == 700 && "the commented-out key is skipped");
  assert(t.rto_max_ms == 1500);
  assert(t.max_retrans == SCTP_DEFAULT_MAX_RETRANS && "a missing key falls back to the default");

  printf("[TEST]: SCTP timers read from the config file\n");
}

static
void test_args_win_over_conf_file(void)
{
  fr_args_t args = {.sctp_hb_interval_ms = 500, .sctp_rto_max_ms = 800, .sctp_max_retrans = 3};
  char path[] = "/tmp/test_sctp_timers_XXXXXX";
  write_conf(&args, path,
             "[NEAR-RIC]\n"
             "SCTP_HB_INTERVAL_MS = 700\n"
             "SCTP_RTO_MAX_MS = 1500\n"
             "SCTP_MAX_RETRANS = 9\n");

  fr_sctp_timers_t const t = get_e2ap_sctp_timers(&args);
  unlink(path);

  assert(t.hb_interval_ms == 500);
  assert(t.rto_max_ms == 800);
  assert(t.max_retrans == 3);

  printf("[TEST]: SCTP timers set in fr_args_t win over the config file\n");
}

static
void expect_on_socket(fr_sctp_timers_t t, int rto_min)
{
  e2ap_ep_ag_t ep = {0};
  e2ap_init_ep_agent(&ep, "127.0.0.1", 36421, t);
  int const fd = ep.base.fd;

  struct sctp_rtoinfo rto = {0};
  socklen_t len = sizeof(rto);
  int rc = getsockopt(fd, IPPROTO_SCTP, SCTP_RTOINFO, &rto, &len);
  assert(rc == 0);
  assert((int)rto.srto_max == t.rto_max_ms);
  assert((int)rto.srto_min == rto_min);
  assert(rto.srto_initial <= rto.srto_max);

  struct sctp_assocparams assoc = {0};
  len = sizeof(assoc);
  rc = getsockopt(fd, IPPROTO_SCTP, SCTP_ASSOCINFO, &assoc, &len);
  assert(rc == 0);
  assert(assoc.sasoc_asocmaxrxt == t.max_retrans);

  struct sctp_paddrparams paddr = {0};
  len = sizeof(paddr);
  rc = getsockopt(fd, IPPROTO_SCTP, SCTP_PEER_ADDR_PARAMS, &paddr, &len);
  assert(rc == 0);
  assert((int)paddr.spp_hbinterval == t.hb_interval_ms);
  assert(paddr.spp_pathmaxrxt == t.max_retrans);
  assert((paddr.spp_flags & SPP_HB_ENABLE) != 0);

  e2ap_free_ep_agent(&ep);
}

static
void test_timers_reach_the_socket(void)
{
  int const probe = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
  if(probe == -1){
    printf("[TEST]: SKIP SCTP timers on the socket: this kernel has no SCTP\n");
    return;
  }

  // The minimum the kernel starts from: the sysctl, 1000 ms unless changed.
  struct sctp_rtoinfo kernel = {0};
  socklen_t len = sizeof(kernel);
  int const rc = getsockopt(probe, IPPROTO_SCTP, SCTP_RTOINFO, &kernel, &len);
  assert(rc == 0);
  close(probe);

  expect_on_socket((fr_sctp_timers_t){.hb_interval_ms = 700, .rto_max_ms = 1500, .max_retrans = 4},
                   kernel.srto_min);

  // A maximum below the kernel's minimum would be refused, so the minimum is
  // lowered with it.
  expect_on_socket((fr_sctp_timers_t){.hb_interval_ms = 300, .rto_max_ms = 500, .max_retrans = 2}, 500);

  printf("[TEST]: SCTP timers reach the socket\n");
}

int main()
{
  test_defaults();
  test_conf_file();
  test_args_win_over_conf_file();
  test_timers_reach_the_socket();

  printf("[TEST]: E2 agent SCTP timers OK\n");
  return EXIT_SUCCESS;
}

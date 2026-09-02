/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "../../../../src/util/byte_array.h"
#include "../../../../src/util/alg_ds/alg/defer.h"
#include "../../../../src/sm/dapp_sm/enc/dapp_enc_asn.h"
#include "../../../../src/sm/dapp_sm/dec/dapp_dec_asn.h"
#include "../../../../src/sm/dapp_sm/e3/dapp_enc_e3.h"
#include "../../../../src/sm/dapp_sm/e3/dapp_dec_e3.h"
#include "../../../../src/sm/dapp_sm/e3/service_models/spectrum_sm/asn/encoder.h"
#include "../../../../src/sm/dapp_sm/e3/service_models/spectrum_sm/asn/decoder.h"
#include "../../../rnd/fill_rnd_data_dapp.h"
#include "../../../../src/util/time_now_us.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ===========================================================================
 * E2SM-DAPP ASN.1 codec round-trips
 * =========================================================================== */

static void test_dapp_event_trigger(void)
{
  e2sm_dapp_event_trigger_t msg = fill_rnd_dapp_ev_trigger();
  defer({ free_e2sm_dapp_event_trigger(&msg); });

  byte_array_t ba = dapp_enc_event_trigger_asn(&msg);
  defer({ free_byte_array(ba); });

  e2sm_dapp_event_trigger_t out = dapp_dec_event_trigger_asn(ba.len, ba.buf);
  defer({ free_e2sm_dapp_event_trigger(&out); });

  assert(eq_e2sm_dapp_event_trigger(&msg, &out) == true);
}

static void test_dapp_action_def(void)
{
  e2sm_dapp_action_def_t msg = fill_rnd_dapp_action_def();
  defer({ free_e2sm_dapp_action_def(&msg); });

  byte_array_t ba = dapp_enc_action_def_asn(&msg);
  defer({ free_byte_array(ba); });

  e2sm_dapp_action_def_t out = dapp_dec_action_def_asn(ba.len, ba.buf);
  defer({ free_e2sm_dapp_action_def(&out); });

  assert(eq_e2sm_dapp_action_def(&msg, &out) == true);
}

static void test_dapp_ind_hdr_format(e2sm_dapp_ind_hdr_t hdr)
{
  defer({ free_e2sm_dapp_ind_hdr(&hdr); });

  byte_array_t ba = dapp_enc_ind_hdr_asn(&hdr);
  defer({ free_byte_array(ba); });

  e2sm_dapp_ind_hdr_t out = dapp_dec_ind_hdr_asn(ba.len, ba.buf);
  defer({ free_e2sm_dapp_ind_hdr(&out); });

  assert(eq_e2sm_dapp_ind_hdr(&hdr, &out) == true);
}

static void test_dapp_ind_hdr(void)
{
  e2sm_dapp_ind_hdr_t f1 = {.format = FORMAT_1_E2SM_DAPP_IND_HDR, .frmt_1 = fill_rnd_dapp_ind_hdr_frmt_1()};
  test_dapp_ind_hdr_format(f1);

  e2sm_dapp_ind_hdr_t f2 = {.format = FORMAT_2_E2SM_DAPP_IND_HDR, .frmt_2 = fill_rnd_dapp_ind_hdr_frmt_2()};
  test_dapp_ind_hdr_format(f2);
}

static void test_dapp_ind_msg_format(e2sm_dapp_ind_msg_t msg)
{
  defer({ free_e2sm_dapp_ind_msg(&msg); });

  byte_array_t ba = dapp_enc_ind_msg_asn(&msg);
  defer({ free_byte_array(ba); });

  e2sm_dapp_ind_msg_t out = dapp_dec_ind_msg_asn(ba.len, ba.buf);
  defer({ free_e2sm_dapp_ind_msg(&out); });

  assert(eq_e2sm_dapp_ind_msg(&msg, &out) == true);
}

static void test_dapp_ind_msg(void)
{
  e2sm_dapp_ind_msg_t f1 = {.format = FORMAT_1_E2SM_DAPP_IND_MSG, .frmt_1 = fill_rnd_dapp_ind_msg_frmt_1()};
  test_dapp_ind_msg_format(f1);

  e2sm_dapp_ind_msg_t f2 = {.format = FORMAT_2_E2SM_DAPP_IND_MSG, .frmt_2 = fill_rnd_dapp_ind_msg_frmt_2()};
  test_dapp_ind_msg_format(f2);
}

static void test_dapp_ctrl_hdr(void)
{
  e2sm_dapp_ctrl_hdr_t msg = fill_rnd_dapp_ctrl_hdr();
  defer({ free_e2sm_dapp_ctrl_hdr(&msg); });

  byte_array_t ba = dapp_enc_ctrl_hdr_asn(&msg);
  defer({ free_byte_array(ba); });

  e2sm_dapp_ctrl_hdr_t out = dapp_dec_ctrl_hdr_asn(ba.len, ba.buf);
  defer({ free_e2sm_dapp_ctrl_hdr(&out); });

  assert(eq_e2sm_dapp_ctrl_hdr(&msg, &out) == true);
}

static void test_dapp_ctrl_msg(void)
{
  e2sm_dapp_ctrl_msg_t msg = fill_rnd_dapp_ctrl_msg();
  defer({ free_e2sm_dapp_ctrl_msg(&msg); });

  byte_array_t ba = dapp_enc_ctrl_msg_asn(&msg);
  defer({ free_byte_array(ba); });

  e2sm_dapp_ctrl_msg_t out = dapp_dec_ctrl_msg_asn(ba.len, ba.buf);
  defer({ free_e2sm_dapp_ctrl_msg(&out); });

  assert(eq_e2sm_dapp_ctrl_msg(&msg, &out) == true);
}

static void test_dapp_ctrl_out(void)
{
  e2sm_dapp_ctrl_out_t msg = fill_rnd_dapp_ctrl_out();
  defer({ free_e2sm_dapp_ctrl_out(&msg); });

  byte_array_t ba = dapp_enc_ctrl_out_asn(&msg);
  defer({ free_byte_array(ba); });

  e2sm_dapp_ctrl_out_t out = dapp_dec_ctrl_out_asn(ba.len, ba.buf);
  defer({ free_e2sm_dapp_ctrl_out(&out); });

  assert(eq_e2sm_dapp_ctrl_out(&msg, &out) == true);
}

static void test_dapp_ran_func_def(void)
{
  e2sm_dapp_func_def_t msg = fill_rnd_dapp_func_def();
  defer({ free_e2sm_dapp_func_def(&msg); });

  byte_array_t ba = dapp_enc_func_def_asn(&msg);
  defer({ free_byte_array(ba); });

  e2sm_dapp_func_def_t out = dapp_dec_func_def_asn(ba.len, ba.buf);
  defer({ free_e2sm_dapp_func_def(&out); });

  assert(eq_e2sm_dapp_func_def(&msg, &out) == true);
}

/* ===========================================================================
 * Spectrum SM IR helpers (cp / eq / free)
 * =========================================================================== */

static void test_spectrum_ir_helpers_report(void)
{
  /* Random non-empty case. */
  spectrum_sm_report_t a = fill_rnd_spectrum_report();
  if (a.prb_count == 0) {
    a.prb_count = 4;
    a.prbs = calloc(4, sizeof(uint16_t));
    assert(a.prbs != NULL && "Memory exhausted");
    for (int i = 0; i < 4; ++i)
      a.prbs[i] = (uint16_t)(i * 7);
  }
  defer({ free_spectrum_sm_report(&a); });

  spectrum_sm_report_t b = cp_spectrum_sm_report(&a);
  defer({ free_spectrum_sm_report(&b); });

  assert(eq_spectrum_sm_report(&a, &b) == true);

  /* Mutate the copy and verify inequality. */
  if (b.prb_count > 0)
    b.prbs[0] = (uint16_t)((a.prbs[0] + 1u) & 0xFFFFu);
  assert(eq_spectrum_sm_report(&a, &b) == false);

  /* Empty case round-trips and free-on-empty is a no-op. */
  spectrum_sm_report_t empty = {.prb_count = 0, .prbs = NULL};
  spectrum_sm_report_t empty_cp = cp_spectrum_sm_report(&empty);
  assert(eq_spectrum_sm_report(&empty, &empty_cp) == true);
  free_spectrum_sm_report(&empty_cp);
  free_spectrum_sm_report(&empty);
}

static void test_spectrum_ir_helpers_control(void)
{
  spectrum_sm_control_t a = fill_rnd_spectrum_control();
  if (a.prb_count == 0) {
    a.prb_count = 3;
    a.blockedPRBs = calloc(3, sizeof(uint16_t));
    assert(a.blockedPRBs != NULL && "Memory exhausted");
    for (int i = 0; i < 3; ++i)
      a.blockedPRBs[i] = (uint16_t)(i + 1);
  }
  defer({ free_spectrum_sm_control(&a); });

  spectrum_sm_control_t b = cp_spectrum_sm_control(&a);
  defer({ free_spectrum_sm_control(&b); });

  assert(eq_spectrum_sm_control(&a, &b) == true);

  if (b.prb_count > 0)
    b.blockedPRBs[0] = (uint16_t)((a.blockedPRBs[0] + 1u) & 0xFFFFu);
  assert(eq_spectrum_sm_control(&a, &b) == false);

  spectrum_sm_control_t empty = {.prb_count = 0, .blockedPRBs = NULL};
  spectrum_sm_control_t empty_cp = cp_spectrum_sm_control(&empty);
  assert(eq_spectrum_sm_control(&empty, &empty_cp) == true);
  free_spectrum_sm_control(&empty_cp);
  free_spectrum_sm_control(&empty);
}

/* ===========================================================================
 * Spectrum SM ASN.1 codec — IR ↔ wire round-trips
 * (production code only ships one direction per IE; the other is implemented
 *  via the asn1c-direct helpers in fill_rnd_data_dapp.c.)
 * =========================================================================== */

/* spectrum_sm_enc_control output decodes back into IR equivalent to the input. */
static void test_spectrum_enc_control_roundtrip(void)
{
  spectrum_sm_control_t in = fill_rnd_spectrum_control();
  if (in.prb_count == 0) {
    in.prb_count = 1;
    in.blockedPRBs = calloc(1, sizeof(uint16_t));
    assert(in.blockedPRBs != NULL && "Memory exhausted");
    in.blockedPRBs[0] = 42;
  }
  defer({ free_spectrum_sm_control(&in); });

  uint8_t* buf = NULL;
  size_t len = 0;
  bool const ok = spectrum_sm_enc_control(&in, &buf, &len);
  assert(ok == true);
  assert(buf != NULL);
  defer({ free(buf); });

  spectrum_sm_control_t out = {0};
  bool const dec_ok = spectrum_control_from_aper_bytes(buf, len, &out);
  assert(dec_ok == true);
  defer({ free_spectrum_sm_control(&out); });

  assert(eq_spectrum_sm_control(&in, &out) == true);
}

/* spectrum_sm_dec_report decodes the IR our asn1c-direct helper produced. */
static void test_spectrum_dec_report_roundtrip(void)
{
  spectrum_sm_report_t in = fill_rnd_spectrum_report();
  if (in.prb_count == 0) {
    in.prb_count = 5;
    in.prbs = calloc(5, sizeof(uint16_t));
    assert(in.prbs != NULL && "Memory exhausted");
    for (int i = 0; i < 5; ++i)
      in.prbs[i] = (uint16_t)((i * 11) % 273);
  }
  defer({ free_spectrum_sm_report(&in); });

  byte_array_t ba = spectrum_report_to_aper_bytes(&in);
  defer({ free_byte_array(ba); });

  spectrum_sm_report_t out = {0};
  bool const ok = spectrum_sm_dec_report(ba.buf, ba.len, &out);
  assert(ok == true);
  defer({ free_spectrum_sm_report(&out); });

  assert(eq_spectrum_sm_report(&in, &out) == true);
}

/* ===========================================================================
 * Boundary / failure paths
 * =========================================================================== */

static void test_spectrum_enc_control_oob(void)
{
  uint8_t* buf = NULL;
  size_t len = 0;

  /* Negative prb_count is rejected. */
  uint16_t dummy_prb = 0;
  spectrum_sm_control_t neg = {.prb_count = -1, .blockedPRBs = &dummy_prb, .timestamp_ns = time_now_ns()};
  assert(spectrum_sm_enc_control(&neg, &buf, &len) == false);
  assert(buf == NULL && len == 0);

  /* prb_count > 273 is rejected. */
  spectrum_sm_control_t hi = {.prb_count = 274, .blockedPRBs = &dummy_prb, .timestamp_ns = time_now_ns()};
  assert(spectrum_sm_enc_control(&hi, &buf, &len) == false);
  assert(buf == NULL && len == 0);

  /* PRB index > 272 is rejected. */
  uint16_t bad_prb[2] = {0, 273};
  spectrum_sm_control_t bad = {.prb_count = 2, .blockedPRBs = bad_prb, .timestamp_ns = time_now_ns()};
  assert(spectrum_sm_enc_control(&bad, &buf, &len) == false);
  assert(buf == NULL && len == 0);

  /* NULL ctrl is rejected. */
  assert(spectrum_sm_enc_control(NULL, &buf, &len) == false);
  assert(buf == NULL && len == 0);

  /* An unstamped control is valid: the schema marks timestamp OPTIONAL, so 0
     means "no producer reference", not "malformed". It encodes with the field
     absent. */
  spectrum_sm_control_t unstamped = {.prb_count = 0, .blockedPRBs = NULL};
  assert(spectrum_sm_enc_control(&unstamped, &buf, &len) == true);
  assert(buf != NULL && len > 0);
  free(buf);
  buf = NULL;
  len = 0;

  /* prb_count == 0 (with NULL prbs) is valid and produces output. */
  spectrum_sm_control_t zero = {.prb_count = 0, .blockedPRBs = NULL, .timestamp_ns = time_now_ns()};
  assert(spectrum_sm_enc_control(&zero, &buf, &len) == true);
  assert(buf != NULL);
  free(buf);
}

static void test_spectrum_dec_report_invalid(void)
{
  spectrum_sm_report_t out = {0};

  /* NULL buffer is rejected. */
  assert(spectrum_sm_dec_report(NULL, 0, &out) == false);

  /* Zero length is rejected. */
  uint8_t one = 0;
  assert(spectrum_sm_dec_report(&one, 0, &out) == false);

  /* Garbage bytes — the asn1c decoder may reject, or may accept with an
   * unexpected layout; both are acceptable as long as we don't crash and the
   * output is left in a consistent (free()-able) state. */
  uint8_t garbage[16] = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
                         0x00, 0xFF, 0x55, 0xAA, 0x12, 0x34, 0x56, 0x78};
  spectrum_sm_report_t r = {0};
  bool const dec_ok = spectrum_sm_dec_report(garbage, sizeof(garbage), &r);
  if (dec_ok)
    free_spectrum_sm_report(&r);
}

static void test_dapp_enc_e3_control_dispatch(void)
{
  uint8_t* buf = NULL;
  size_t len = 0;

  /* Unsupported RAN function ID returns false. */
  dapp_e3_ctrl_payload_t spec = {.type = DAPP_E3_SM_SPECTRUM};
  spec.u.spectrum.prb_count = 0;
  spec.u.spectrum.blockedPRBs = NULL;
  assert(dapp_enc_e3_control(/*ran_function_id=*/999u, &spec, &buf, &len) == false);
  assert(buf == NULL && len == 0);

  /* RAN function ID 1 with the wrong inner SM type returns false. */
  dapp_e3_ctrl_payload_t mismatched = {.type = DAPP_E3_SM_NONE};
  assert(dapp_enc_e3_control(/*ran_function_id=*/1u, &mismatched, &buf, &len) == false);
  assert(buf == NULL && len == 0);

  /* NULL payload returns false. */
  assert(dapp_enc_e3_control(/*ran_function_id=*/1u, NULL, &buf, &len) == false);
  assert(buf == NULL && len == 0);

  /* Happy path: ran_function_id=1 + DAPP_E3_SM_SPECTRUM produces bytes. */
  spectrum_sm_control_t* c = &spec.u.spectrum;
  c->timestamp_ns = time_now_ns();
  c->prb_count = 4;
  c->blockedPRBs = calloc(4, sizeof(uint16_t));
  assert(c->blockedPRBs != NULL && "Memory exhausted");
  for (int i = 0; i < 4; ++i)
    c->blockedPRBs[i] = (uint16_t)i;
  assert(dapp_enc_e3_control(/*ran_function_id=*/1u, &spec, &buf, &len) == true);
  assert(buf != NULL && len > 0);
  free(buf);
  free_spectrum_sm_control(c);
}

static void test_dapp_dec_e3_indication_dispatch(void)
{
  /* NULL/zero-length input returns false and out is reset to NONE. */
  dapp_e3_ind_payload_t out = {.type = DAPP_E3_SM_SPECTRUM};
  assert(dapp_dec_e3_indication(/*ran_function_id=*/1u, NULL, 0, &out) == false);
  assert(out.type == DAPP_E3_SM_NONE);

  uint8_t one = 0xFF;
  out.type = DAPP_E3_SM_SPECTRUM;
  assert(dapp_dec_e3_indication(/*ran_function_id=*/1u, &one, 0, &out) == false);
  assert(out.type == DAPP_E3_SM_NONE);

  /* Unsupported RAN function ID returns false (and leaves type NONE). */
  uint8_t buf[1] = {0};
  assert(dapp_dec_e3_indication(/*ran_function_id=*/999u, buf, 1, &out) == false);
  assert(out.type == DAPP_E3_SM_NONE);

  /* Happy path: encode a Spectrum report via the asn1c-direct helper and
   * verify the dispatcher routes to the spectrum decoder and matches. */
  spectrum_sm_report_t in = {.prb_count = 3};
  in.prbs = calloc(3, sizeof(uint16_t));
  assert(in.prbs != NULL && "Memory exhausted");
  in.prbs[0] = 1;
  in.prbs[1] = 17;
  in.prbs[2] = 271;
  byte_array_t ba = spectrum_report_to_aper_bytes(&in);

  out = (dapp_e3_ind_payload_t){0};
  bool const ok = dapp_dec_e3_indication(/*ran_function_id=*/1u, ba.buf, ba.len, &out);
  assert(ok == true);
  assert(out.type == DAPP_E3_SM_SPECTRUM);
  assert(eq_spectrum_sm_report(&in, &out.u.spectrum) == true);

  free_dapp_e3_ind_payload(&out);
  free_byte_array(ba);
  free_spectrum_sm_report(&in);
}

int main(void)
{
  srand((unsigned)time(NULL));

  printf("\nDAPP SM enc/dec test launched\n");

  test_dapp_event_trigger();
  printf("DAPP Event Trigger test succeeded\n");

  test_dapp_action_def();
  printf("DAPP Action Definition test succeeded\n");

  test_dapp_ind_hdr();
  printf("DAPP Indication Header test succeeded (format 1 + format 2)\n");

  test_dapp_ind_msg();
  printf("DAPP Indication Message test succeeded (format 1 + format 2)\n");

  test_dapp_ctrl_hdr();
  printf("DAPP Control Header test succeeded\n");

  test_dapp_ctrl_msg();
  printf("DAPP Control Message test succeeded\n");

  test_dapp_ctrl_out();
  printf("DAPP Control Outcome test succeeded\n");

  test_dapp_ran_func_def();
  printf("DAPP RAN Function Definition test succeeded (with ev_trig + dapp_e3_subs)\n");

  test_spectrum_ir_helpers_report();
  test_spectrum_ir_helpers_control();
  printf("Spectrum SM IR cp/eq/free test succeeded\n");

  test_spectrum_enc_control_roundtrip();
  printf("Spectrum SM control encode roundtrip test succeeded\n");

  test_spectrum_dec_report_roundtrip();
  printf("Spectrum SM report decode roundtrip test succeeded\n");

  test_spectrum_enc_control_oob();
  printf("Spectrum SM control out-of-bound rejection test succeeded\n");

  test_spectrum_dec_report_invalid();
  printf("Spectrum SM report invalid input test succeeded\n");

  test_dapp_enc_e3_control_dispatch();
  printf("DAPP E3 control dispatch test succeeded\n");

  test_dapp_dec_e3_indication_dispatch();
  printf("DAPP E3 indication dispatch test succeeded\n");

  return EXIT_SUCCESS;
}

/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "fill_rnd_data_dapp.h"

#include "../../src/sm/dapp_sm/dapp_sm_id.h"
#include "../../src/util/time_now_us.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "Spectrum-DAppReportData.h"
#include "Spectrum-PRBBlacklistReport.h"
#include "Spectrum-XAppControlData.h"
#include "Spectrum-PRBBlockedControl.h"
#include "asn_application.h"
#include "per_encoder.h"
#include "per_decoder.h"

#define DAPP_SPECTRUM_RAN_FUNCTION_ID 1u
#define DAPP_PRB_MAX_INDEX 272u
#define DAPP_PRB_MAX_COUNT 273

static byte_array_t fill_rnd_byte_array(size_t len)
{
  assert(len > 0);
  byte_array_t ba = {.len = len, .buf = malloc(len)};
  assert(ba.buf != NULL && "Memory exhausted");
  for (size_t i = 0; i < len; ++i)
    ba.buf[i] = (uint8_t)(rand() & 0xFF);
  return ba;
}

/* Event Trigger ------------------------------------------------------------ */

e2sm_dapp_event_trigger_t fill_rnd_dapp_ev_trigger(void)
{
  e2sm_dapp_event_trigger_t dst = {.format = FORMAT_1_E2SM_DAPP_EV_TRIGGER_FORMAT};
  return dst;
}

/* Action Definition -------------------------------------------------------- */

e2sm_dapp_action_def_t fill_rnd_dapp_action_def(void)
{
  e2sm_dapp_action_def_t dst = {
      .format = FORMAT_1_E2SM_DAPP_ACTION_DEF,
      .ric_style_type = (rand() & 1) ? DAPP_RIC_STYLE_E3_DATA_REPORT : DAPP_RIC_STYLE_E3_SUBSCRIPTION_MAP,
  };
  return dst;
}

/* Indication Header -------------------------------------------------------- */

e2sm_dapp_ind_hdr_frmt_1_t fill_rnd_dapp_ind_hdr_frmt_1(void)
{
  e2sm_dapp_ind_hdr_frmt_1_t dst = {0};
  dst.ran_function_id = (uint32_t)rand();
  dst.dapp_id = (uint32_t)rand();
  dst.node_type = (uint8_t)(rand() & 0x7F);
  dst.node_nb_id = (uint32_t)rand();
  for (size_t i = 0; i < 3; ++i)
    dst.node_plmn_id[i] = (uint8_t)(rand() & 0xFF);
  dst.node_cu_du_id_present = (rand() & 1) != 0;
  dst.node_cu_du_id = dst.node_cu_du_id_present ? (uint64_t)rand() : 0;
  dst.timestamp_ns = time_now_ns();
  dst.sequence_id =
      (int64_t)rand() + 1; /* never 0: that's the "absent" sentinel */
  return dst;
}

e2sm_dapp_ind_hdr_frmt_2_t fill_rnd_dapp_ind_hdr_frmt_2(void)
{
  e2sm_dapp_ind_hdr_frmt_2_t dst = {0};
  dst.node_type = (uint8_t)(rand() & 0x7F);
  dst.node_nb_id = (uint32_t)rand();
  for (size_t i = 0; i < 3; ++i)
    dst.node_plmn_id[i] = (uint8_t)(rand() & 0xFF);
  dst.node_cu_du_id_present = (rand() & 1) != 0;
  dst.node_cu_du_id = dst.node_cu_du_id_present ? (uint64_t)rand() : 0;
  dst.timestamp_ns = time_now_ns();
  dst.sequence_id =
      (int64_t)rand() + 1; /* never 0: that's the "absent" sentinel */
  return dst;
}

e2sm_dapp_ind_hdr_t fill_rnd_dapp_ind_hdr(void)
{
  e2sm_dapp_ind_hdr_t dst = {0};
  if (rand() & 1) {
    dst.format = FORMAT_1_E2SM_DAPP_IND_HDR;
    dst.frmt_1 = fill_rnd_dapp_ind_hdr_frmt_1();
  } else {
    dst.format = FORMAT_2_E2SM_DAPP_IND_HDR;
    dst.frmt_2 = fill_rnd_dapp_ind_hdr_frmt_2();
  }
  return dst;
}

/* Indication Message ------------------------------------------------------- */

e2sm_dapp_ind_msg_frmt_1_t fill_rnd_dapp_ind_msg_frmt_1(void)
{
  e2sm_dapp_ind_msg_frmt_1_t dst = {0};
  size_t const n = (size_t)((rand() % 256) + 1);
  byte_array_t ba = fill_rnd_byte_array(n);
  dst.data_size = ba.len;
  dst.data = ba.buf;
  return dst;
}

static dapp_e3_subscription_item_t fill_rnd_dapp_e3_subscription_item(void)
{
  dapp_e3_subscription_item_t dst = {0};
  dst.dapp_id = (uint32_t)rand();
  size_t const n = (size_t)((rand() % 4) + 1);
  dst.sz_subscribed_e3_ran_functions = n;
  dst.subscribed_e3_ran_functions = calloc(n, sizeof(uint32_t));
  assert(dst.subscribed_e3_ran_functions != NULL && "Memory exhausted");
  for (size_t i = 0; i < n; ++i)
    dst.subscribed_e3_ran_functions[i] = (uint32_t)rand();
  return dst;
}

static dapp_e3_subscription_list_t fill_rnd_dapp_e3_subscription_list(void)
{
  dapp_e3_subscription_list_t dst = {0};
  size_t const n = (size_t)((rand() % 3) + 1);
  dst.sz_dapp_e3_subscriptions = n;
  dst.dapp_e3_subscriptions = calloc(n, sizeof(dapp_e3_subscription_item_t));
  assert(dst.dapp_e3_subscriptions != NULL && "Memory exhausted");
  for (size_t i = 0; i < n; ++i)
    dst.dapp_e3_subscriptions[i] = fill_rnd_dapp_e3_subscription_item();
  return dst;
}

e2sm_dapp_ind_msg_frmt_2_t fill_rnd_dapp_ind_msg_frmt_2(void)
{
  e2sm_dapp_ind_msg_frmt_2_t dst = {0};
  dst.dapp_e3_subs = fill_rnd_dapp_e3_subscription_list();
  return dst;
}

e2sm_dapp_ind_msg_t fill_rnd_dapp_ind_msg(void)
{
  e2sm_dapp_ind_msg_t dst = {0};
  if (rand() & 1) {
    dst.format = FORMAT_1_E2SM_DAPP_IND_MSG;
    dst.frmt_1 = fill_rnd_dapp_ind_msg_frmt_1();
  } else {
    dst.format = FORMAT_2_E2SM_DAPP_IND_MSG;
    dst.frmt_2 = fill_rnd_dapp_ind_msg_frmt_2();
  }
  return dst;
}

/* Control Header / Message / Outcome --------------------------------------- */

e2sm_dapp_ctrl_hdr_t fill_rnd_dapp_ctrl_hdr(void)
{
  e2sm_dapp_ctrl_hdr_t dst = {.format = FORMAT_1_E2SM_DAPP_CTRL_HDR};
  dst.frmt_1.ran_function_id = (uint32_t)rand();
  dst.frmt_1.dapp_id = (uint32_t)rand();
  dst.frmt_1.timestamp_ns = time_now_ns();
  dst.frmt_1.sequence_id =
      (int64_t)rand() + 1; /* never 0: that's the "absent" sentinel */
  return dst;
}

e2sm_dapp_ctrl_msg_t fill_rnd_dapp_ctrl_msg(void)
{
  e2sm_dapp_ctrl_msg_t dst = {.format = FORMAT_1_E2SM_DAPP_CTRL_MSG};
  size_t const n = (size_t)((rand() % 256) + 1);
  byte_array_t ba = fill_rnd_byte_array(n);
  dst.frmt_1.data_size = (uint32_t)ba.len;
  dst.frmt_1.data = ba.buf;
  return dst;
}

e2sm_dapp_ctrl_out_t fill_rnd_dapp_ctrl_out(void)
{
  e2sm_dapp_ctrl_out_t dst = {.format = FORMAT_1_E2SM_DAPP_CTRL_OUT};
  dst.frmt_1.timestamp_ns = time_now_ns();
  dst.frmt_1.sequence_id =
      (int64_t)rand() + 1; /* never 0: that's the "absent" sentinel */
  return dst;
}

e2sm_dapp_ctrl_out_t fill_dapp_ctrl_out(void) { return fill_rnd_dapp_ctrl_out(); }

/* RAN Function Definition -------------------------------------------------- */

static seq_report_sty_dapp_sm_t fill_rnd_seq_report_sty(void)
{
  seq_report_sty_dapp_sm_t dst = {0};
  dst.report_type = (uint32_t)((rand() % 2) + 1);
  char const* nm = (dst.report_type == 1) ? "E3 Data Report" : "E3 Subscription Map";
  dst.name = cp_str_to_ba(nm);
  dst.ind_hdr_type = dst.report_type;
  dst.ind_msg_type = dst.report_type;
  /* Exercise the encoder/decoder branch that copies the inner subscription
   * list into ReportStyle items. */
  dst.dapp_e3_subs = calloc(1, sizeof(dapp_e3_subscription_list_t));
  assert(dst.dapp_e3_subs != NULL && "Memory exhausted");
  *dst.dapp_e3_subs = fill_rnd_dapp_e3_subscription_list();
  return dst;
}

static seq_ctrl_style_dapp_sm_t fill_rnd_seq_ctrl_style(void)
{
  seq_ctrl_style_dapp_sm_t dst = {0};
  dst.style_type = 1;
  dst.name = cp_str_to_ba("E3 Control Style");
  dst.hdr = 1;
  dst.msg = 1;
  dst.out_frmt = 1;
  dst.dapp_e3_subs = calloc(1, sizeof(dapp_e3_subscription_list_t));
  assert(dst.dapp_e3_subs != NULL && "Memory exhausted");
  *dst.dapp_e3_subs = fill_rnd_dapp_e3_subscription_list();
  return dst;
}

static e2sm_dapp_func_def_t fill_dapp_func_def_with_name(byte_array_t name, byte_array_t oid, byte_array_t desc)
{
  e2sm_dapp_func_def_t dst = {0};
  dst.name.name = name;
  dst.name.oid = oid;
  dst.name.description = desc;
  dst.name.instance = NULL;

  dst.ev_trig = calloc(1, sizeof(ran_func_def_ev_trig_dapp_sm_t));
  assert(dst.ev_trig != NULL && "Memory exhausted");

  dst.report = calloc(1, sizeof(ran_func_def_report_dapp_sm_t));
  assert(dst.report != NULL && "Memory exhausted");
  dst.report->sz_seq_report_sty = 2;
  dst.report->seq_report_sty = calloc(2, sizeof(seq_report_sty_dapp_sm_t));
  assert(dst.report->seq_report_sty != NULL && "Memory exhausted");
  dst.report->seq_report_sty[0] = fill_rnd_seq_report_sty();
  dst.report->seq_report_sty[1] = fill_rnd_seq_report_sty();

  dst.ctrl = calloc(1, sizeof(ran_func_def_ctrl_dapp_sm_t));
  assert(dst.ctrl != NULL && "Memory exhausted");
  dst.ctrl->sz_seq_ctrl_style = 1;
  dst.ctrl->seq_ctrl_style = calloc(1, sizeof(seq_ctrl_style_dapp_sm_t));
  assert(dst.ctrl->seq_ctrl_style != NULL && "Memory exhausted");
  dst.ctrl->seq_ctrl_style[0] = fill_rnd_seq_ctrl_style();

  return dst;
}

e2sm_dapp_func_def_t fill_rnd_dapp_func_def(void)
{
  return fill_dapp_func_def_with_name(cp_str_to_ba("E2SM-DAPP-Test"),
                                      cp_str_to_ba("1.3.6.1.4.1.53148.1.1.255.3"),
                                      cp_str_to_ba("Test DAPP RAN function"));
}

e2sm_dapp_func_def_t fill_dapp_ran_func_def(void)
{
  return fill_dapp_func_def_with_name(cp_str_to_ba(SM_DAPP_SHORT_NAME),
                                      cp_str_to_ba(SM_DAPP_OID),
                                      cp_str_to_ba(SM_DAPP_DESCRIPTION));
}

/* Spectrum SM helpers ------------------------------------------------------ */

spectrum_sm_report_t fill_rnd_spectrum_report(void)
{
  spectrum_sm_report_t dst = {0};
  dst.timestamp_ns = time_now_ns();
  long const n = (long)(rand() % (DAPP_PRB_MAX_COUNT + 1));
  dst.prb_count = n;
  if (n == 0) {
    dst.prbs = NULL;
    return dst;
  }
  dst.prbs = calloc((size_t)n, sizeof(uint16_t));
  assert(dst.prbs != NULL && "Memory exhausted");
  for (long i = 0; i < n; ++i)
    dst.prbs[i] = (uint16_t)(rand() % (DAPP_PRB_MAX_INDEX + 1));
  return dst;
}

spectrum_sm_control_t fill_rnd_spectrum_control(void)
{
  spectrum_sm_control_t dst = {0};
  dst.timestamp_ns = time_now_ns();
  long const n = (long)(rand() % (DAPP_PRB_MAX_COUNT + 1));
  dst.prb_count = n;
  if (n == 0) {
    dst.blockedPRBs = NULL;
    return dst;
  }
  dst.blockedPRBs = calloc((size_t)n, sizeof(uint16_t));
  assert(dst.blockedPRBs != NULL && "Memory exhausted");
  for (long i = 0; i < n; ++i)
    dst.blockedPRBs[i] = (uint16_t)(rand() % (DAPP_PRB_MAX_INDEX + 1));
  return dst;
}

byte_array_t spectrum_report_to_aper_bytes(spectrum_sm_report_t const* r)
{
  assert(r != NULL);
  assert(r->prb_count >= 0 && r->prb_count <= DAPP_PRB_MAX_COUNT);

  Spectrum_DAppReportData_t asn;
  memset(&asn, 0, sizeof(asn));
  /* OPTIONAL now: absent when the producer did not stamp. */
  if (r->timestamp_ns != 0) {
    asn.timestamp = calloc(1, sizeof(*asn.timestamp));
    assert(asn.timestamp != NULL && "Memory exhausted");
    *asn.timestamp = (long)r->timestamp_ns;
  }
  asn.reportPayload.present = Spectrum_DAppReportPayload_PR_prbBlacklistReport;

  Spectrum_PRBBlacklistReport_t* rep = (Spectrum_PRBBlacklistReport_t*)calloc(1, sizeof(*rep));
  assert(rep != NULL && "Memory exhausted");
  asn.reportPayload.choice.prbBlacklistReport = rep;

  for (long i = 0; i < r->prb_count; ++i) {
    long* elem = (long*)calloc(1, sizeof(*elem));
    assert(elem != NULL && "Memory exhausted");
    *elem = (long)r->prbs[i];
    int const rc = ASN_SEQUENCE_ADD(&rep->blacklistedPRBs.list, elem);
    assert(rc == 0 && "ASN_SEQUENCE_ADD failed");
  }

  uint8_t tmp[2048];
  asn_enc_rval_t const er = aper_encode_to_buffer(&asn_DEF_Spectrum_DAppReportData, NULL, &asn, tmp, sizeof(tmp));
  assert(er.encoded > 0 && "aper_encode_to_buffer failed");

  size_t const n = (size_t)((er.encoded + 7) / 8);
  byte_array_t ba = {.len = n, .buf = malloc(n)};
  assert(ba.buf != NULL && "Memory exhausted");
  memcpy(ba.buf, tmp, n);

  ASN_STRUCT_RESET(asn_DEF_Spectrum_DAppReportData, &asn);
  return ba;
}

bool spectrum_control_from_aper_bytes(uint8_t const* buf, size_t len, spectrum_sm_control_t* out)
{
  assert(out != NULL);
  out->prb_count = 0;
  out->blockedPRBs = NULL;

  if (buf == NULL || len == 0)
    return false;

  Spectrum_XAppControlData_t asn;
  memset(&asn, 0, sizeof(asn));
  Spectrum_XAppControlData_t* asn_ref = &asn;

  asn_dec_rval_t const ret = aper_decode(NULL, &asn_DEF_Spectrum_XAppControlData, (void**)&asn_ref, buf, len, 0, 0);
  if (ret.code != RC_OK) {
    ASN_STRUCT_RESET(asn_DEF_Spectrum_XAppControlData, &asn);
    return false;
  }

  out->timestamp_ns = asn.timestamp != NULL ? (int64_t)*asn.timestamp : 0;

  if (asn.controlPayload.present != Spectrum_XAppControlPayload_PR_prbBlockedControl
      || asn.controlPayload.choice.prbBlockedControl == NULL) {
    ASN_STRUCT_RESET(asn_DEF_Spectrum_XAppControlData, &asn);
    return false;
  }

  Spectrum_PRBBlockedControl_t const* blocked = asn.controlPayload.choice.prbBlockedControl;
  int const n = blocked->blockedPRBs.list.count;
  if (n < 0 || n > DAPP_PRB_MAX_COUNT) {
    ASN_STRUCT_RESET(asn_DEF_Spectrum_XAppControlData, &asn);
    return false;
  }

  out->prb_count = (long)n;
  if (n > 0) {
    out->blockedPRBs = calloc((size_t)n, sizeof(uint16_t));
    assert(out->blockedPRBs != NULL && "Memory exhausted");
    for (int i = 0; i < n; ++i) {
      long const v = *blocked->blockedPRBs.list.array[i];
      if (v < 0 || v > (long)DAPP_PRB_MAX_INDEX) {
        ASN_STRUCT_RESET(asn_DEF_Spectrum_XAppControlData, &asn);
        free(out->blockedPRBs);
        out->blockedPRBs = NULL;
        out->prb_count = 0;
        return false;
      }
      out->blockedPRBs[i] = (uint16_t)v;
    }
  }

  ASN_STRUCT_RESET(asn_DEF_Spectrum_XAppControlData, &asn);
  return true;
}

/* Top-level containers ----------------------------------------------------- */

dapp_sub_data_t fill_rnd_dapp_subscription(void)
{
  dapp_sub_data_t dst = {0};
  dst.et = fill_rnd_dapp_ev_trigger();
  dst.action_def = calloc(1, sizeof(e2sm_dapp_action_def_t));
  assert(dst.action_def != NULL && "Memory exhausted");
  *dst.action_def = fill_rnd_dapp_action_def();
  return dst;
}

static dapp_ind_data_t fill_rnd_dapp_ind_data_fmt2(void)
{
  dapp_ind_data_t dst = {0};
  dst.hdr.format = FORMAT_2_E2SM_DAPP_IND_HDR;
  dst.hdr.frmt_2 = fill_rnd_dapp_ind_hdr_frmt_2();

  dst.msg.format = FORMAT_2_E2SM_DAPP_IND_MSG;
  dst.msg.frmt_2 = fill_rnd_dapp_ind_msg_frmt_2();

  dst.e3.type = DAPP_E3_SM_NONE;
  return dst;
}

static dapp_ind_data_t fill_rnd_dapp_ind_data_fmt1_spectrum(void)
{
  spectrum_sm_report_t report = fill_rnd_spectrum_report();
  byte_array_t ba = spectrum_report_to_aper_bytes(&report);

  while (ba.len == 0) {
    free_byte_array(ba);
    free_spectrum_sm_report(&report);
    report = fill_rnd_spectrum_report();
    if (report.prb_count == 0) {
      report.prb_count = 1;
      report.prbs = calloc(1, sizeof(uint16_t));
      assert(report.prbs != NULL && "Memory exhausted");
      report.prbs[0] = (uint16_t)(rand() % (DAPP_PRB_MAX_INDEX + 1));
    }
    ba = spectrum_report_to_aper_bytes(&report);
  }

  dapp_ind_data_t dst = {0};
  dst.hdr.format = FORMAT_1_E2SM_DAPP_IND_HDR;
  dst.hdr.frmt_1 = fill_rnd_dapp_ind_hdr_frmt_1();
  dst.hdr.frmt_1.ran_function_id = DAPP_SPECTRUM_RAN_FUNCTION_ID;

  dst.msg.format = FORMAT_1_E2SM_DAPP_IND_MSG;
  dst.msg.frmt_1.data_size = ba.len;
  dst.msg.frmt_1.data = ba.buf;

  dst.e3.type = DAPP_E3_SM_SPECTRUM;
  dst.e3.u.spectrum = report;
  return dst;
}

dapp_ind_data_t fill_rnd_dapp_ind_data(void)
{
  return (rand() & 1) ? fill_rnd_dapp_ind_data_fmt1_spectrum() : fill_rnd_dapp_ind_data_fmt2();
}

dapp_ctrl_req_data_t fill_dapp_ctrl(void)
{
  dapp_ctrl_req_data_t dst = {0};

  dst.hdr.format = FORMAT_1_E2SM_DAPP_CTRL_HDR;
  dst.hdr.frmt_1.ran_function_id = DAPP_SPECTRUM_RAN_FUNCTION_ID;
  dst.hdr.frmt_1.dapp_id = (uint32_t)rand();
  dst.hdr.frmt_1.timestamp_ns = time_now_ns();
  dst.hdr.frmt_1.sequence_id =
      (int64_t)rand() + 1; /* never 0: that's the "absent" sentinel */

  dst.msg.format = FORMAT_1_E2SM_DAPP_CTRL_MSG;
  dst.msg.frmt_1.data = NULL;
  dst.msg.frmt_1.data_size = 0;

  dst.e3.type = DAPP_E3_SM_SPECTRUM;
  dst.e3.u.spectrum.timestamp_ns = time_now_ns();
  long const prb_count = (long)((rand() % 16) + 1);
  dst.e3.u.spectrum.prb_count = prb_count;
  dst.e3.u.spectrum.blockedPRBs = calloc((size_t)prb_count, sizeof(uint16_t));
  assert(dst.e3.u.spectrum.blockedPRBs != NULL && "Memory exhausted");
  for (long i = 0; i < prb_count; ++i)
    dst.e3.u.spectrum.blockedPRBs[i] = (uint16_t)(rand() % (DAPP_PRB_MAX_INDEX + 1));

  return dst;
}

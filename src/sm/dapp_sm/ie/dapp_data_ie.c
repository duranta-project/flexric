/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "dapp_data_ie.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////
// RIC Event Trigger Definition
/////////////////////////////////////

void free_e2sm_dapp_event_trigger(e2sm_dapp_event_trigger_t* src)
{
  assert(src != NULL);

  if (src->format == FORMAT_1_E2SM_DAPP_EV_TRIGGER_FORMAT) {
    free_e2sm_dapp_ev_trg_frmt_1(&src->frmt_1);

  } else {
    assert(0 != 0 && "Not implemented");
  }
}

bool eq_e2sm_dapp_event_trigger(e2sm_dapp_event_trigger_t const* m0, e2sm_dapp_event_trigger_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->format == FORMAT_1_E2SM_DAPP_EV_TRIGGER_FORMAT) {
    return eq_e2sm_dapp_ev_trg_frmt_1(&m0->frmt_1, &m1->frmt_1);

  } else {
    assert(0 != 0 && "Not implemented");
  }

  assert(0 != 0 && "Impossible path");
  return false;
}

e2sm_dapp_event_trigger_t cp_e2sm_dapp_event_trigger(e2sm_dapp_event_trigger_t const* src)
{
  assert(src != NULL);

  e2sm_dapp_event_trigger_t dst = {0};
  dst.format = src->format;

  if (src->format == FORMAT_1_E2SM_DAPP_EV_TRIGGER_FORMAT) {
    dst.frmt_1 = cp_e2sm_dapp_ev_trg_frmt_1(&src->frmt_1);

  } else {
    assert(0 != 0 && "Not implemented");
  }

  return dst;
}

//////////////////////////////////////
// RIC Action Definition
/////////////////////////////////////

void free_e2sm_dapp_action_def(e2sm_dapp_action_def_t* src)
{
  assert(src != NULL);

  if (src->format == FORMAT_1_E2SM_DAPP_ACTION_DEF) {
    free_e2sm_dapp_action_def_frmt_1(&src->frmt_1);
  } else {
    assert(0 != 0 && "Unknown format");
  }
}

e2sm_dapp_action_def_t cp_e2sm_dapp_action_def(e2sm_dapp_action_def_t const* src)
{
  assert(src != NULL);

  e2sm_dapp_action_def_t dst = {0};
  dst.format = src->format;
  dst.ric_style_type = src->ric_style_type;

  if (src->format == FORMAT_1_E2SM_DAPP_ACTION_DEF) {
    dst.frmt_1 = cp_e2sm_dapp_action_def_frmt_1(&src->frmt_1);
  } else {
    assert(0 != 0 && "Unknown format");
  }

  return dst;
}

bool eq_e2sm_dapp_action_def(e2sm_dapp_action_def_t const* m0, e2sm_dapp_action_def_t const* m1)
{
  if (m0 == m1)
    return true;
  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->format != m1->format)
    return false;
  if (m0->ric_style_type != m1->ric_style_type)
    return false;

  if (m0->format == FORMAT_1_E2SM_DAPP_ACTION_DEF) {
    return eq_e2sm_dapp_action_def_frmt_1(&m0->frmt_1, &m1->frmt_1);
  } else {
    assert(0 != 0 && "Unknown format");
  }

  return false;
}

//////////////////////////////////////
// RIC Indication Header
/////////////////////////////////////

void free_e2sm_dapp_ind_hdr(e2sm_dapp_ind_hdr_t* src)
{
  assert(src != NULL);

  if (src->format == FORMAT_1_E2SM_DAPP_IND_HDR) {
    free_e2sm_dapp_ind_hdr_frmt_1(&src->frmt_1);

  } else if (src->format == FORMAT_2_E2SM_DAPP_IND_HDR) {
    free_e2sm_dapp_ind_hdr_frmt_2(&src->frmt_2);

  } else {
    assert(0 != 0 && "Unknown type");
  }
}

e2sm_dapp_ind_hdr_t cp_e2sm_dapp_ind_hdr(e2sm_dapp_ind_hdr_t const* src)
{
  assert(src != NULL);
  e2sm_dapp_ind_hdr_t dst = {.format = src->format};

  if (src->format == FORMAT_1_E2SM_DAPP_IND_HDR) {
    dst.frmt_1 = cp_e2sm_dapp_ind_hdr_frmt_1(&src->frmt_1);

  } else if (src->format == FORMAT_2_E2SM_DAPP_IND_HDR) {
    dst.frmt_2 = cp_e2sm_dapp_ind_hdr_frmt_2(&src->frmt_2);

  } else {
    assert(0 != 0 && "Unknown type");
  }

  return dst;
}

bool eq_e2sm_dapp_ind_hdr(e2sm_dapp_ind_hdr_t const* m0, e2sm_dapp_ind_hdr_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->format != m1->format)
    return false;

  if (m0->format == FORMAT_1_E2SM_DAPP_IND_HDR) {
    return eq_e2sm_dapp_ind_hdr_frmt_1(&m0->frmt_1, &m1->frmt_1);

  } else if (m0->format == FORMAT_2_E2SM_DAPP_IND_HDR) {
    return eq_e2sm_dapp_ind_hdr_frmt_2(&m0->frmt_2, &m1->frmt_2);

  } else {
    assert(0 != 0 && "Unknown format");
  }

  assert(0 != 0 && "Not implemented");

  return true;
}

//////////////////////////////////////
// RIC Indication Message
/////////////////////////////////////

void free_e2sm_dapp_ind_msg(e2sm_dapp_ind_msg_t* src)
{
  assert(src != NULL);

  if (src->format == FORMAT_1_E2SM_DAPP_IND_MSG) {
    free_e2sm_dapp_ind_msg_frmt_1(&src->frmt_1);

  } else if (src->format == FORMAT_2_E2SM_DAPP_IND_MSG) {
    free_e2sm_dapp_ind_msg_frmt_2(&src->frmt_2);

  } else {
    assert(0 != 0 && "Unknown format");
  }
}

e2sm_dapp_ind_msg_t cp_e2sm_dapp_ind_msg(e2sm_dapp_ind_msg_t const* src)
{
  assert(src != NULL);

  e2sm_dapp_ind_msg_t dst = {.format = src->format};

  if (src->format == FORMAT_1_E2SM_DAPP_IND_MSG) {
    dst.frmt_1 = cp_e2sm_dapp_ind_msg_frmt_1(&src->frmt_1);

  } else if (src->format == FORMAT_2_E2SM_DAPP_IND_MSG) {
    dst.frmt_2 = cp_e2sm_dapp_ind_msg_frmt_2(&src->frmt_2);
  } else {
    assert(0 != 0 && "Unknown format");
  }

  return dst;
}

bool eq_e2sm_dapp_ind_msg(e2sm_dapp_ind_msg_t const* m0, e2sm_dapp_ind_msg_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->format != m1->format)
    return false;

  if (m0->format == FORMAT_1_E2SM_DAPP_IND_MSG) {
    return eq_e2sm_dapp_ind_msg_frmt_1(&m0->frmt_1, &m1->frmt_1);

  } else if (m0->format == FORMAT_2_E2SM_DAPP_IND_MSG) {
    return eq_e2sm_dapp_ind_msg_frmt_2(&m0->frmt_2, &m1->frmt_2);

  } else {
    assert(0 != 0 && "Unknown format");
  }

  assert(0 != 0 && "Impossible path");

  return true;
}

//////////////////////////////////////
// RIC Control Header
/////////////////////////////////////

void free_e2sm_dapp_ctrl_hdr(e2sm_dapp_ctrl_hdr_t* src)
{
  assert(src != NULL);
  if (src->format == FORMAT_1_E2SM_DAPP_CTRL_HDR) {
    free_e2sm_dapp_ctrl_hdr_frmt_1(&src->frmt_1);

  } else {
    assert(0 != 0 && "Unknown type");
  }
}

e2sm_dapp_ctrl_hdr_t cp_e2sm_dapp_ctrl_hdr(e2sm_dapp_ctrl_hdr_t const* src)
{
  assert(src != NULL);
  e2sm_dapp_ctrl_hdr_t dst = {.format = src->format};

  if (dst.format == FORMAT_1_E2SM_DAPP_CTRL_HDR) {
    dst.frmt_1 = cp_e2sm_dapp_ctrl_hdr_frmt_1(&src->frmt_1);

  } else {
    assert(0 != 0 && "Unknown format type");
  }

  return dst;
}

bool eq_e2sm_dapp_ctrl_hdr(e2sm_dapp_ctrl_hdr_t const* m0, e2sm_dapp_ctrl_hdr_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->format != m1->format)
    return false;

  if (m0->format == FORMAT_1_E2SM_DAPP_CTRL_HDR) {
    if (eq_e2sm_dapp_ctrl_hdr_frmt_1(&m0->frmt_1, &m1->frmt_1) == false)
      return false;

  } else {
    assert(0 != 0 && "Unknown type");
  }

  return true;
}

//////////////////////////////////////
// RIC Control Message
/////////////////////////////////////

void free_e2sm_dapp_ctrl_msg(e2sm_dapp_ctrl_msg_t* src)
{
  assert(src != NULL);

  if (src->format == FORMAT_1_E2SM_DAPP_CTRL_MSG) {
    free_e2sm_dapp_ctrl_msg_frmt_1(&src->frmt_1);

  } else {
    assert(0 && "Unknown format");
  }
}

e2sm_dapp_ctrl_msg_t cp_e2sm_dapp_ctrl_msg(e2sm_dapp_ctrl_msg_t const* src)
{
  assert(src != NULL);

  e2sm_dapp_ctrl_msg_t dst = {.format = src->format};

  if (src->format == FORMAT_1_E2SM_DAPP_CTRL_MSG) {
    dst.frmt_1 = cp_e2sm_dapp_ctrl_msg_frmt_1(&src->frmt_1);

  } else {
    assert(0 != 0 && "Unknown format");
  }

  return dst;
  ;
}

bool eq_e2sm_dapp_ctrl_msg(e2sm_dapp_ctrl_msg_t const* m0, e2sm_dapp_ctrl_msg_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->format != m1->format)
    return false;

  if (m0->format == FORMAT_1_E2SM_DAPP_CTRL_MSG) {
    if (eq_e2sm_dapp_ctrl_msg_frmt_1(&m0->frmt_1, &m1->frmt_1) == false)
      return false;

  } else {
    assert(0 && "Unknown format");
  }

  return true;
}

//////////////////////////////////////
// RIC Control Outcome
/////////////////////////////////////

void free_e2sm_dapp_ctrl_out(e2sm_dapp_ctrl_out_t* src)
{
  assert(src != NULL);

  if (src->format == FORMAT_1_E2SM_DAPP_CTRL_OUT) {
    free_e2sm_dapp_ctrl_out_frmt_1(&src->frmt_1);

  } else {
    assert(0 != 0 && "Unknown format");
  }
}

e2sm_dapp_ctrl_out_t cp_e2sm_dapp_ctrl_out(e2sm_dapp_ctrl_out_t const* src)
{
  assert(src != NULL);

  e2sm_dapp_ctrl_out_t dst = {.format = src->format};

  if (dst.format == FORMAT_1_E2SM_DAPP_CTRL_OUT) {
    dst.frmt_1 = cp_e2sm_dapp_ctrl_out_frmt_1(&src->frmt_1);

  } else {
    assert(0 != 0 && "Unknown format");
  }

  return dst;
}

bool eq_e2sm_dapp_ctrl_out(e2sm_dapp_ctrl_out_t const* m0, e2sm_dapp_ctrl_out_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->format != m1->format)
    return false;

  if (m0->format == FORMAT_1_E2SM_DAPP_CTRL_OUT) {
    if (eq_e2sm_dapp_ctrl_out_frmt_1(&m0->frmt_1, &m1->frmt_1) == false)
      return false;

  } else {
    assert(0 != 0 && "Unknown format");
  }

  return true;
}

//////////////////////////////////////
// RAN Function Definition
/////////////////////////////////////

void free_e2sm_dapp_func_def(e2sm_dapp_func_def_t* src)
{
  assert(src != NULL);
  //  RAN Function Name
  free_ran_function_name(&src->name);

  // RAN Function Definition for EVENT TRIGGER
  if (src->ev_trig != NULL) {
    free_ran_func_def_ev_trig_dapp_sm(src->ev_trig);
    free(src->ev_trig);
  }

  // RAN Function Definition for REPORT
  if (src->report != NULL) {
    free_ran_func_def_report_dapp_sm(src->report);
    free(src->report);
  }

  // RAN Function Definition for CONTROL
  if (src->ctrl != NULL) {
    free_ran_func_def_ctrl_dapp_sm(src->ctrl);
    free(src->ctrl);
  }
}

e2sm_dapp_func_def_t cp_e2sm_dapp_func_def(e2sm_dapp_func_def_t const* src)
{
  assert(src != NULL);

  e2sm_dapp_func_def_t dst = {0};

  //  RAN Function Name
  dst.name = cp_ran_function_name(&src->name);

  // RAN Function Definition for EVENT TRIGGER
  if (src->ev_trig != NULL) {
    dst.ev_trig = calloc(1, sizeof(ran_func_def_ev_trig_dapp_sm_t));
    assert(dst.ev_trig != NULL && "Memory exhausted");
    *dst.ev_trig = cp_ran_func_def_ev_trig_dapp_sm(src->ev_trig);
  }

  // RAN Function Definition for REPORT
  if (src->report != NULL) {
    dst.report = calloc(1, sizeof(ran_func_def_report_dapp_sm_t));
    assert(dst.report != NULL && "Memory exhausted");
    *dst.report = cp_ran_func_def_report_dapp_sm(src->report);
  }

  // RAN Function Definition for CONTROL
  if (src->ctrl != NULL) {
    dst.ctrl = calloc(1, sizeof(ran_func_def_ctrl_dapp_sm_t));
    assert(dst.ctrl != NULL && "Memory exhausted");
    *dst.ctrl = cp_ran_func_def_ctrl_dapp_sm(src->ctrl);
  }

  return dst;
}

bool eq_e2sm_dapp_func_def(e2sm_dapp_func_def_t const* m0, e2sm_dapp_func_def_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  //  RAN Function Name
  if (eq_ran_function_name(&m0->name, &m1->name) == false)
    return false;

  // RAN Function Definition for EVENT TRIGGER
  if (eq_ran_func_def_ev_trig_dapp_sm(m0->ev_trig, m1->ev_trig) == false)
    return false;

  // RAN Function Definition for REPORT
  if (eq_ran_func_def_report_dapp_sm(m0->report, m1->report) == false)
    return false;

  // RAN Function Definition for CONTROL
  if (eq_ran_func_def_ctrl_dapp_sm(m0->ctrl, m1->ctrl) == false)
    return false;

  return true;
}

///////////////
/// RIC Subscription
///////////////

void free_dapp_sub_data(dapp_sub_data_t* src)
{
  assert(src != NULL);
  free_e2sm_dapp_event_trigger(&src->et);

  if (src->action_def != NULL) {
    free_e2sm_dapp_action_def(src->action_def);
    free(src->action_def);
  }
}

bool eq_dapp_sub_data(dapp_sub_data_t const* m0, dapp_sub_data_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (eq_e2sm_dapp_event_trigger(&m0->et, &m1->et) == false)
    return false;

  return true;
}

dapp_sub_data_t cp_dapp_sub_data(dapp_sub_data_t const* src)
{
  assert(src != NULL);
  dapp_sub_data_t dst = {0};

  dst.et = cp_e2sm_dapp_event_trigger(&src->et);

  return dst;
}

///////////////
// RIC Indication
///////////////

void free_dapp_ind_data(dapp_ind_data_t* ind)
{
  assert(ind != NULL);

  free_e2sm_dapp_ind_hdr(&ind->hdr);
  free_e2sm_dapp_ind_msg(&ind->msg);
  free_dapp_e3_ind_payload(&ind->e3);
}

bool eq_dapp_ind_data(dapp_ind_data_t const* m0, dapp_ind_data_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (eq_e2sm_dapp_ind_hdr(&m0->hdr, &m1->hdr) == false) {
    assert(0 != 0 && "Debug hdr");
    return false;
  }

  if (eq_e2sm_dapp_ind_msg(&m0->msg, &m1->msg) == false) {
    assert(0 != 0 && "Debug msg");
    return false;
  }

  if (eq_dapp_e3_ind_payload(&m0->e3, &m1->e3) == false) {
    assert(0 != 0 && "Debug msg");
    return false;
  }

  return true;
}

dapp_ind_data_t cp_dapp_ind_data(dapp_ind_data_t const* src)
{
  assert(src != NULL);
  dapp_ind_data_t dst = {0};

  dst.hdr = cp_e2sm_dapp_ind_hdr(&src->hdr);
  dst.msg = cp_e2sm_dapp_ind_msg(&src->msg);
  dst.e3 = cp_dapp_e3_ind_payload(&src->e3);

  assert(eq_dapp_ind_data(src, &dst) == true);

  return dst;
}

///////////////
// RIC Control
///////////////

void free_dapp_ctrl_req_data(dapp_ctrl_req_data_t* src)
{
  assert(src != NULL);

  free_e2sm_dapp_ctrl_hdr(&src->hdr);
  free_e2sm_dapp_ctrl_msg(&src->msg);
  free_dapp_e3_ctrl_payload(&src->e3);
}

bool eq_dapp_ctrl_req_data(dapp_ctrl_req_data_t const* m0, dapp_ctrl_req_data_t const* m1)
{
  if (m0 == m1)
    return true;

  if (!m0 || !m1)
    return false;

  if (!eq_e2sm_dapp_ctrl_hdr(&m0->hdr, &m1->hdr))
    return false;

  if (!eq_e2sm_dapp_ctrl_msg(&m0->msg, &m1->msg))
    return false;

  if (!eq_dapp_e3_ctrl_payload(&m0->e3, &m1->e3))
    return false;

  return true;
}

dapp_ctrl_req_data_t cp_dapp_ctrl_req_data(dapp_ctrl_req_data_t const* src)
{
  assert(src != NULL);
  dapp_ctrl_req_data_t dst = {0};

  dst.hdr = cp_e2sm_dapp_ctrl_hdr(&src->hdr);
  dst.msg = cp_e2sm_dapp_ctrl_msg(&src->msg);
  dst.e3 = cp_dapp_e3_ctrl_payload(&src->e3);

  assert(eq_dapp_ctrl_req_data(src, &dst) == true);
  return dst;
}

//////////////////////////////////////
// DAPP E3 Indication Payload
/////////////////////////////////////

void free_dapp_e3_ind_payload(dapp_e3_ind_payload_t* src)
{
  assert(src != NULL);

  switch (src->type) {
    case DAPP_E3_SM_SPECTRUM:
      free_spectrum_sm_report(&src->u.spectrum);
      break;

    case DAPP_E3_SM_NONE:
    default:
      break;
  }

  src->type = DAPP_E3_SM_NONE;
}

dapp_e3_ind_payload_t cp_dapp_e3_ind_payload(dapp_e3_ind_payload_t const* src)
{
  assert(src != NULL);
  dapp_e3_ind_payload_t dst = {0};

  dst.type = src->type;

  switch (src->type) {
    case DAPP_E3_SM_SPECTRUM:
      dst.u.spectrum = cp_spectrum_sm_report(&src->u.spectrum);
      break;

    case DAPP_E3_SM_NONE:
    default:
      break;
  }

  return dst;
}

bool eq_dapp_e3_ind_payload(dapp_e3_ind_payload_t const* m0, dapp_e3_ind_payload_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->type != m1->type)
    return false;

  switch (m0->type) {
    case DAPP_E3_SM_NONE:
      break;

    case DAPP_E3_SM_SPECTRUM:
      if (!eq_spectrum_sm_report(&m0->u.spectrum, &m1->u.spectrum))
        return false;
      break;

    default:
      assert(0 != 0 && "Unknown DAPP E3 SM type");
      return false;
  }

  return true;
}

//////////////////////////////////////
// DAPP E3 Control Payload
/////////////////////////////////////

void free_dapp_e3_ctrl_payload(dapp_e3_ctrl_payload_t* src)
{
  assert(src != NULL);

  switch (src->type) {
    case DAPP_E3_SM_SPECTRUM:
      free_spectrum_sm_control(&src->u.spectrum);
      break;

    case DAPP_E3_SM_NONE:
    default:
      break;
  }

  src->type = DAPP_E3_SM_NONE;
}

dapp_e3_ctrl_payload_t cp_dapp_e3_ctrl_payload(dapp_e3_ctrl_payload_t const* src)
{
  assert(src != NULL);
  dapp_e3_ctrl_payload_t dst = {0};

  dst.type = src->type;

  switch (src->type) {
    case DAPP_E3_SM_SPECTRUM:
      dst.u.spectrum = cp_spectrum_sm_control(&src->u.spectrum);
      break;

    case DAPP_E3_SM_NONE:
    default:
      break;
  }

  return dst;
}

bool eq_dapp_e3_ctrl_payload(dapp_e3_ctrl_payload_t const* m0, dapp_e3_ctrl_payload_t const* m1)
{
  if (m0 == m1)
    return true;

  if (m0 == NULL || m1 == NULL)
    return false;

  if (m0->type != m1->type)
    return false;

  switch (m0->type) {
    case DAPP_E3_SM_NONE:
      break;

    case DAPP_E3_SM_SPECTRUM:
      if (!eq_spectrum_sm_control(&m0->u.spectrum, &m1->u.spectrum))
        return false;
      break;

    default:
      assert(0 != 0 && "Unknown DAPP E3 SM type");
      return false;
  }

  return true;
}
#include "dapp_enc_asn.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libe3/e2sm_dapp_c.h>

#include "../../../util/byte_array.h"

/* maxE2SMDAPPIndMsgSize in the E2SM-DAPP grammar, which libe3 compiles. */
#define DAPP_MAX_IND_MSG_SIZE 32768

/* The encoders borrow the IR's buffers: only the arrays that libe3's structs need
 * and the IR does not have in the same shape are allocated, and freed after the
 * encode. libe3 owns the ASN.1; the result is malloc'ed bytes for the caller. */

static libe3_e2sm_dapp_bytes_t borrow_ba(byte_array_t ba)
{
  return (libe3_e2sm_dapp_bytes_t){.data = ba.buf, .len = ba.len};
}

/* The bytes of a successful encode, to be released with free_byte_array(). */
static byte_array_t take_encoded(e3_error_t rc, libe3_e2sm_dapp_bytes_t out)
{
  assert(rc == E3_SUCCESS && "E2SM-DAPP encode failed");
  (void)rc;
  return (byte_array_t){.len = out.len, .buf = out.data};
}

static libe3_e2sm_dapp_sub_list_t borrow_subs(dapp_e3_subscription_list_t const* src)
{
  libe3_e2sm_dapp_sub_list_t dst = {.n_items = src->sz_dapp_e3_subscriptions};

  if (dst.n_items > 0) {
    dst.items = calloc(dst.n_items, sizeof(*dst.items));
    assert(dst.items != NULL && "Memory exhausted");
  }

  for (size_t i = 0; i < dst.n_items; ++i) {
    dapp_e3_subscription_item_t const* ir = &src->dapp_e3_subscriptions[i];
    dst.items[i] = (libe3_e2sm_dapp_sub_item_t){.dapp_id = ir->dapp_id,
                                                .n_ran_functions = ir->sz_subscribed_e3_ran_functions,
                                                .ran_functions = ir->subscribed_e3_ran_functions};
  }

  return dst;
}

byte_array_t dapp_enc_event_trigger_asn(e2sm_dapp_event_trigger_t const* src)
{
  assert(src != NULL);
  assert(src->format == FORMAT_1_E2SM_DAPP_EV_TRIGGER_FORMAT && "Unknown format");

  libe3_e2sm_dapp_event_trigger_t in = {.format = LIBE3_E2SM_DAPP_EVENT_TRIGGER_FORMAT_1};

  libe3_e2sm_dapp_bytes_t out = {0};
  e3_error_t rc = libe3_e2sm_dapp_encode_event_trigger(&in, &out);
  return take_encoded(rc, out);
}

byte_array_t dapp_enc_action_def_asn(e2sm_dapp_action_def_t const* src)
{
  assert(src != NULL);
  assert(src->format == FORMAT_1_E2SM_DAPP_ACTION_DEF && "Unknown action definition format");

  libe3_e2sm_dapp_action_definition_t in = {.format = LIBE3_E2SM_DAPP_ACTION_DEFINITION_FORMAT_1,
                                            .ric_style_type = src->ric_style_type};

  libe3_e2sm_dapp_bytes_t out = {0};
  e3_error_t rc = libe3_e2sm_dapp_encode_action_definition(&in, &out);
  return take_encoded(rc, out);
}

byte_array_t dapp_enc_ind_hdr_asn(e2sm_dapp_ind_hdr_t const* src)
{
  assert(src != NULL);

  // Same contract as dapp_enc_ind_msg_asn: an empty byte_array_t means declined.
  libe3_e2sm_dapp_ind_hdr_t in = {0};

  if (src->format == FORMAT_1_E2SM_DAPP_IND_HDR) {
    e2sm_dapp_ind_hdr_frmt_1_t const* f = &src->frmt_1;
    /* The grammar's node-cu-du-id is 32 bits wide; libe3 would truncate silently. */
    if (f->node_cu_du_id_present && f->node_cu_du_id > UINT32_MAX) {
      printf("[E2SM-DAPP]: IndicationHeader node_cu_du_id %llu does not fit in 32 bits, dropping\n",
             (unsigned long long)f->node_cu_du_id);
      return (byte_array_t){0};
    }

    in.format = LIBE3_E2SM_DAPP_IND_HDR_FORMAT_1;
    in.frmt_1 = (libe3_e2sm_dapp_ind_hdr_format1_t){.ran_function_id = f->ran_function_id,
                                                    .dapp_id = f->dapp_id,
                                                    .node_type = f->node_type,
                                                    .node_nb_id = f->node_nb_id,
                                                    .has_node_cu_du_id = f->node_cu_du_id_present,
                                                    .node_cu_du_id = (uint32_t)f->node_cu_du_id,
                                                    /* OPTIONAL: 0 means the producer did not stamp, which is a
                                                     * normal message. Encode it as absent. */
                                                    .has_timestamp = f->timestamp_ns != 0,
                                                    .timestamp = f->timestamp_ns,
                                                    /* Same convention: 0 means no correlation id was assigned. */
                                                    .has_sequence_id = f->sequence_id != 0,
                                                    .sequence_id = f->sequence_id};
    memcpy(in.frmt_1.node_plmn_id, f->node_plmn_id, 3);
  } else if (src->format == FORMAT_2_E2SM_DAPP_IND_HDR) {
    e2sm_dapp_ind_hdr_frmt_2_t const* f = &src->frmt_2;
    if (f->node_cu_du_id_present && f->node_cu_du_id > UINT32_MAX) {
      printf("[E2SM-DAPP]: IndicationHeader node_cu_du_id %llu does not fit in 32 bits, dropping\n",
             (unsigned long long)f->node_cu_du_id);
      return (byte_array_t){0};
    }

    in.format = LIBE3_E2SM_DAPP_IND_HDR_FORMAT_2;
    in.frmt_2 = (libe3_e2sm_dapp_ind_hdr_format2_t){.node_type = f->node_type,
                                                    .node_nb_id = f->node_nb_id,
                                                    .has_node_cu_du_id = f->node_cu_du_id_present,
                                                    .node_cu_du_id = (uint32_t)f->node_cu_du_id,
                                                    .has_timestamp = f->timestamp_ns != 0,
                                                    .timestamp = f->timestamp_ns,
                                                    .has_sequence_id = f->sequence_id != 0,
                                                    .sequence_id = f->sequence_id};
    memcpy(in.frmt_2.node_plmn_id, f->node_plmn_id, 3);
  } else {
    printf("[E2SM-DAPP]: IndicationHeader with unknown format %d, dropping\n", (int)src->format);
    return (byte_array_t){0};
  }

  libe3_e2sm_dapp_bytes_t out = {0};
  e3_error_t rc = libe3_e2sm_dapp_encode_ind_hdr(&in, &out);
  if (rc != E3_SUCCESS) {
    printf("[E2SM-DAPP]: IndicationHeader could not be encoded (error %d), dropping\n", (int)rc);
    return (byte_array_t){0};
  }

  return (byte_array_t){.len = out.len, .buf = out.data};
}

/* An IndicationMessage this function will not encode. Rejecting one is not a bug
 * in the E2 node: the struct arrives from the RAN, is reached through a union
 * discriminated by format, and is encoded on the RAN's own threads, so every
 * reason to refuse has to be a return value rather than an assert. */
static bool valid_ind_msg(e2sm_dapp_ind_msg_t const* src)
{
  if (src->format == FORMAT_1_E2SM_DAPP_IND_MSG) {
    /* FORMAT_1_E2SM_DAPP_IND_MSG is 0, so a zeroed or mis-typed struct lands
     * here and the payload check is what actually rejects it. */
    if (src->frmt_1.data == NULL || src->frmt_1.data_size == 0) {
      printf("[E2SM-DAPP]: IndicationMessage Format 1 carries no payload, dropping\n");
      return false;
    }
    if (src->frmt_1.data_size > DAPP_MAX_IND_MSG_SIZE) {
      printf("[E2SM-DAPP]: IndicationMessage payload of %zu bytes exceeds the %d the grammar allows, dropping\n",
             src->frmt_1.data_size,
             DAPP_MAX_IND_MSG_SIZE);
      return false;
    }
    return true;
  }

  if (src->format == FORMAT_2_E2SM_DAPP_IND_MSG)
    return true;

  printf("[E2SM-DAPP]: IndicationMessage with unknown format %d, dropping\n", (int)src->format);
  return false;
}

byte_array_t dapp_enc_ind_msg_asn(e2sm_dapp_ind_msg_t const* src)
{
  assert(src != NULL);

  // An empty byte_array_t is the failure signal; the caller drops the indication.
  if (valid_ind_msg(src) == false)
    return (byte_array_t){0};

  libe3_e2sm_dapp_ind_msg_t in = {0};

  if (src->format == FORMAT_1_E2SM_DAPP_IND_MSG) {
    in.format = LIBE3_E2SM_DAPP_IND_MSG_FORMAT_1;
    in.frmt_1.data = (libe3_e2sm_dapp_bytes_t){.data = src->frmt_1.data, .len = src->frmt_1.data_size};
  } else {
    in.format = LIBE3_E2SM_DAPP_IND_MSG_FORMAT_2;
    in.frmt_2.dapp_e3_subscriptions = borrow_subs(&src->frmt_2.dapp_e3_subs);
  }

  libe3_e2sm_dapp_bytes_t out = {0};
  e3_error_t rc = libe3_e2sm_dapp_encode_ind_msg(&in, &out);
  free(in.frmt_2.dapp_e3_subscriptions.items);

  /* A message that does not encode is one we cannot send, not a reason to take the
   * RAN down with us. */
  if (rc != E3_SUCCESS) {
    printf("[E2SM-DAPP]: IndicationMessage could not be encoded (error %d), dropping\n", (int)rc);
    return (byte_array_t){0};
  }

  return (byte_array_t){.len = out.len, .buf = out.data};
}

byte_array_t dapp_enc_ctrl_hdr_asn(e2sm_dapp_ctrl_hdr_t const* src)
{
  assert(src != NULL);
  assert(src->format == FORMAT_1_E2SM_DAPP_CTRL_HDR && "Unknown format");

  e2sm_dapp_ctrl_hdr_frmt_1_t const* f = &src->frmt_1;
  libe3_e2sm_dapp_ctrl_hdr_t in = {.format = LIBE3_E2SM_DAPP_CTRL_HDR_FORMAT_1,
                                   .ran_function_id = f->ran_function_id,
                                   .dapp_id = f->dapp_id,
                                   /* 0 means absent, see dapp_enc_ind_hdr_asn. */
                                   .has_timestamp = f->timestamp_ns != 0,
                                   .timestamp = f->timestamp_ns,
                                   .has_sequence_id = f->sequence_id != 0,
                                   .sequence_id = f->sequence_id};

  libe3_e2sm_dapp_bytes_t out = {0};
  e3_error_t rc = libe3_e2sm_dapp_encode_ctrl_hdr(&in, &out);
  return take_encoded(rc, out);
}

byte_array_t dapp_enc_ctrl_msg_asn(e2sm_dapp_ctrl_msg_t const* src)
{
  assert(src != NULL);
  assert(src->format == FORMAT_1_E2SM_DAPP_CTRL_MSG && "Unknown format type");
  assert(src->frmt_1.data_size > 0);
  assert(src->frmt_1.data != NULL);

  libe3_e2sm_dapp_ctrl_msg_t in = {.format = LIBE3_E2SM_DAPP_CTRL_MSG_FORMAT_1,
                                   .data = {.data = src->frmt_1.data, .len = src->frmt_1.data_size}};

  libe3_e2sm_dapp_bytes_t out = {0};
  e3_error_t rc = libe3_e2sm_dapp_encode_ctrl_msg(&in, &out);
  return take_encoded(rc, out);
}

byte_array_t dapp_enc_ctrl_out_asn(e2sm_dapp_ctrl_out_t const* src)
{
  assert(src != NULL);
  assert(src->format == FORMAT_1_E2SM_DAPP_CTRL_OUT && "Unknown format");

  e2sm_dapp_ctrl_out_frmt_1_t const* f = &src->frmt_1;
  libe3_e2sm_dapp_ctrl_outcome_t in = {.format = LIBE3_E2SM_DAPP_CTRL_OUTCOME_FORMAT_1,
                                       /* 0 means absent, see dapp_enc_ind_hdr_asn. */
                                       .has_timestamp = f->timestamp_ns != 0,
                                       .timestamp = f->timestamp_ns,
                                       .has_sequence_id = f->sequence_id != 0,
                                       .sequence_id = f->sequence_id};

  /* OPTIONAL: absent when the outcome only acknowledges the control. */
  if (f->e3_control_outcome != NULL && f->e3_control_outcome_size > 0) {
    in.has_e3_control_outcome = true;
    in.e3_control_outcome = (libe3_e2sm_dapp_bytes_t){.data = f->e3_control_outcome, .len = f->e3_control_outcome_size};
  }

  libe3_e2sm_dapp_bytes_t out = {0};
  e3_error_t rc = libe3_e2sm_dapp_encode_ctrl_outcome(&in, &out);
  return take_encoded(rc, out);
}

byte_array_t dapp_enc_func_def_asn(e2sm_dapp_func_def_t const* src)
{
  assert(src != NULL);

  libe3_e2sm_dapp_ran_function_definition_t in = {0};

  in.name.short_name = borrow_ba(src->name.name);
  in.name.e2sm_oid = borrow_ba(src->name.oid);
  in.name.description = borrow_ba(src->name.description);
  if (src->name.instance != NULL) {
    in.name.has_instance = true;
    in.name.instance = *src->name.instance;
  }

  in.has_event_trigger = src->ev_trig != NULL;

  if (src->report != NULL) {
    in.has_report = true;
    in.n_report_styles = src->report->sz_seq_report_sty;
    if (in.n_report_styles > 0) {
      in.report_styles = calloc(in.n_report_styles, sizeof(*in.report_styles));
      assert(in.report_styles != NULL && "Memory exhausted");
    }

    for (size_t i = 0; i < in.n_report_styles; ++i) {
      seq_report_sty_dapp_sm_t const* ir = &src->report->seq_report_sty[i];
      in.report_styles[i] = (libe3_e2sm_dapp_report_style_t){.report_style_type = ir->report_type,
                                                             .name = borrow_ba(ir->name),
                                                             .ind_hdr_format_type = ir->ind_hdr_type,
                                                             .ind_msg_format_type = ir->ind_msg_type};
      if (ir->dapp_e3_subs != NULL) {
        in.report_styles[i].has_dapp_e3_subscriptions = true;
        in.report_styles[i].dapp_e3_subscriptions = borrow_subs(ir->dapp_e3_subs);
      }
    }
  }

  if (src->ctrl != NULL) {
    in.has_ctrl = true;
    in.n_ctrl_styles = src->ctrl->sz_seq_ctrl_style;
    if (in.n_ctrl_styles > 0) {
      in.ctrl_styles = calloc(in.n_ctrl_styles, sizeof(*in.ctrl_styles));
      assert(in.ctrl_styles != NULL && "Memory exhausted");
    }

    for (size_t i = 0; i < in.n_ctrl_styles; ++i) {
      seq_ctrl_style_dapp_sm_t const* ir = &src->ctrl->seq_ctrl_style[i];
      in.ctrl_styles[i] = (libe3_e2sm_dapp_ctrl_style_t){.ctrl_style_type = ir->style_type,
                                                         .name = borrow_ba(ir->name),
                                                         .ctrl_hdr_format_type = ir->hdr,
                                                         .ctrl_msg_format_type = ir->msg,
                                                         .ctrl_outcome_format_type = ir->out_frmt};
      if (ir->dapp_e3_subs != NULL) {
        in.ctrl_styles[i].has_dapp_e3_subscriptions = true;
        in.ctrl_styles[i].dapp_e3_subscriptions = borrow_subs(ir->dapp_e3_subs);
      }
    }
  }

  libe3_e2sm_dapp_bytes_t out = {0};
  e3_error_t rc = libe3_e2sm_dapp_encode_ran_function_definition(&in, &out);

  for (size_t i = 0; i < in.n_report_styles; ++i)
    free(in.report_styles[i].dapp_e3_subscriptions.items);
  free(in.report_styles);
  for (size_t i = 0; i < in.n_ctrl_styles; ++i)
    free(in.ctrl_styles[i].dapp_e3_subscriptions.items);
  free(in.ctrl_styles);

  return take_encoded(rc, out);
}

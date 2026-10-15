#include "dapp_dec_asn.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libe3/e2sm_dapp_c.h>

/* libe3 owns the ASN.1 and hands back malloc'ed memory. Each decoder moves what
 * the IR keeps into IR structs (the pointers are plain malloc, so the IR's free_
 * helpers release them), nulls it in libe3's struct, and lets libe3's free_
 * release the rest. */

/* Move the bytes out of `b`; the result is released with free_byte_array(). */
static byte_array_t take_bytes(libe3_e2sm_dapp_bytes_t* b)
{
  byte_array_t ba = {.len = b->len, .buf = b->data};
  b->data = NULL;
  b->len = 0;
  return ba;
}

static dapp_e3_subscription_list_t take_subs(libe3_e2sm_dapp_sub_list_t* src)
{
  dapp_e3_subscription_list_t dst = {.sz_dapp_e3_subscriptions = src->n_items};

  if (src->n_items == 0)
    return dst;

  dst.dapp_e3_subscriptions = calloc(src->n_items, sizeof(*dst.dapp_e3_subscriptions));
  assert(dst.dapp_e3_subscriptions != NULL && "Memory exhausted");

  for (size_t i = 0; i < src->n_items; ++i) {
    libe3_e2sm_dapp_sub_item_t* item = &src->items[i];
    dst.dapp_e3_subscriptions[i] = (dapp_e3_subscription_item_t){.dapp_id = item->dapp_id,
                                                                 .sz_subscribed_e3_ran_functions = item->n_ran_functions,
                                                                 .subscribed_e3_ran_functions = item->ran_functions};
    item->ran_functions = NULL;
  }

  return dst;
}

/* A failed decode is a bug in the peer or in us, as it has always been here. */
static void check_decoded(e3_error_t rc)
{
  assert(rc == E3_SUCCESS && "E2SM-DAPP decode failed");
  (void)rc;
}

e2sm_dapp_event_trigger_t dapp_dec_event_trigger_asn(size_t len, uint8_t const buf[len])
{
  assert(buf != NULL);
  assert(len != 0);

  libe3_e2sm_dapp_event_trigger_t src = {0};
  check_decoded(libe3_e2sm_dapp_decode_event_trigger(buf, len, &src));
  assert(src.format == LIBE3_E2SM_DAPP_EVENT_TRIGGER_FORMAT_1 && "Unknown format");

  e2sm_dapp_event_trigger_t dst = {.format = FORMAT_1_E2SM_DAPP_EV_TRIGGER_FORMAT};
  libe3_e2sm_dapp_free_event_trigger(&src);
  return dst;
}

e2sm_dapp_action_def_t dapp_dec_action_def_asn(size_t len, uint8_t const buf[len])
{
  assert(buf != NULL);
  assert(len != 0);

  libe3_e2sm_dapp_action_definition_t src = {0};
  check_decoded(libe3_e2sm_dapp_decode_action_definition(buf, len, &src));
  assert(src.format == LIBE3_E2SM_DAPP_ACTION_DEFINITION_FORMAT_1 && "Unknown action definition format");

  e2sm_dapp_action_def_t dst = {.format = FORMAT_1_E2SM_DAPP_ACTION_DEF, .ric_style_type = (uint32_t)src.ric_style_type};
  libe3_e2sm_dapp_free_action_definition(&src);
  return dst;
}

e2sm_dapp_ind_hdr_t dapp_dec_ind_hdr_asn(size_t len, uint8_t const ind_hdr[len])
{
  assert(ind_hdr != NULL);
  assert(len != 0);

  libe3_e2sm_dapp_ind_hdr_t src = {0};
  check_decoded(libe3_e2sm_dapp_decode_ind_hdr(ind_hdr, len, &src));

  /* Absent timestamp and sequence id are 0 in the IR, as is a node_cu_du_id that is not present. */
  e2sm_dapp_ind_hdr_t dst = {0};

  if (src.format == LIBE3_E2SM_DAPP_IND_HDR_FORMAT_1) {
    libe3_e2sm_dapp_ind_hdr_format1_t const* f = &src.frmt_1;
    dst.format = FORMAT_1_E2SM_DAPP_IND_HDR;
    dst.frmt_1 = (e2sm_dapp_ind_hdr_frmt_1_t){.ran_function_id = f->ran_function_id,
                                              .dapp_id = f->dapp_id,
                                              .node_type = f->node_type,
                                              .node_nb_id = f->node_nb_id,
                                              .node_cu_du_id_present = f->has_node_cu_du_id,
                                              .node_cu_du_id = f->has_node_cu_du_id ? f->node_cu_du_id : 0,
                                              .timestamp_ns = f->has_timestamp ? f->timestamp : 0,
                                              .sequence_id = f->has_sequence_id ? f->sequence_id : 0};
    memcpy(dst.frmt_1.node_plmn_id, f->node_plmn_id, 3);
  } else if (src.format == LIBE3_E2SM_DAPP_IND_HDR_FORMAT_2) {
    libe3_e2sm_dapp_ind_hdr_format2_t const* f = &src.frmt_2;
    dst.format = FORMAT_2_E2SM_DAPP_IND_HDR;
    dst.frmt_2 = (e2sm_dapp_ind_hdr_frmt_2_t){.node_type = f->node_type,
                                              .node_nb_id = f->node_nb_id,
                                              .node_cu_du_id_present = f->has_node_cu_du_id,
                                              .node_cu_du_id = f->has_node_cu_du_id ? f->node_cu_du_id : 0,
                                              .timestamp_ns = f->has_timestamp ? f->timestamp : 0,
                                              .sequence_id = f->has_sequence_id ? f->sequence_id : 0};
    memcpy(dst.frmt_2.node_plmn_id, f->node_plmn_id, 3);
  } else {
    assert(0 != 0 && "Unknown format");
  }

  libe3_e2sm_dapp_free_ind_hdr(&src);
  return dst;
}

e2sm_dapp_ind_msg_t dapp_dec_ind_msg_asn(size_t len, uint8_t const ind_msg[len])
{
  assert(ind_msg != NULL);
  assert(len != 0);

  libe3_e2sm_dapp_ind_msg_t src = {0};
  check_decoded(libe3_e2sm_dapp_decode_ind_msg(ind_msg, len, &src));

  e2sm_dapp_ind_msg_t dst = {0};

  if (src.format == LIBE3_E2SM_DAPP_IND_MSG_FORMAT_1) {
    dst.format = FORMAT_1_E2SM_DAPP_IND_MSG;
    dst.frmt_1.data_size = src.frmt_1.data.len;
    dst.frmt_1.data = take_bytes(&src.frmt_1.data).buf;
  } else if (src.format == LIBE3_E2SM_DAPP_IND_MSG_FORMAT_2) {
    dst.format = FORMAT_2_E2SM_DAPP_IND_MSG;
    dst.frmt_2.dapp_e3_subs = take_subs(&src.frmt_2.dapp_e3_subscriptions);
  } else {
    assert(0 != 0 && "Unknown format type");
  }

  libe3_e2sm_dapp_free_ind_msg(&src);
  return dst;
}

e2sm_dapp_ctrl_hdr_t dapp_dec_ctrl_hdr_asn(size_t len, uint8_t const ctrl_hdr[len])
{
  assert(ctrl_hdr != NULL);

  libe3_e2sm_dapp_ctrl_hdr_t src = {0};
  check_decoded(libe3_e2sm_dapp_decode_ctrl_hdr(ctrl_hdr, len, &src));
  assert(src.format == LIBE3_E2SM_DAPP_CTRL_HDR_FORMAT_1 && "Unknown format type");

  e2sm_dapp_ctrl_hdr_t dst = {.format = FORMAT_1_E2SM_DAPP_CTRL_HDR,
                              .frmt_1 = {.ran_function_id = src.ran_function_id,
                                         .dapp_id = src.dapp_id,
                                         .timestamp_ns = src.has_timestamp ? src.timestamp : 0,
                                         .sequence_id = src.has_sequence_id ? src.sequence_id : 0}};
  libe3_e2sm_dapp_free_ctrl_hdr(&src);
  return dst;
}

e2sm_dapp_ctrl_msg_t dapp_dec_ctrl_msg_asn(size_t len, uint8_t const ctrl_msg[len])
{
  assert(ctrl_msg != NULL);
  assert(len > 0);

  libe3_e2sm_dapp_ctrl_msg_t src = {0};
  check_decoded(libe3_e2sm_dapp_decode_ctrl_msg(ctrl_msg, len, &src));
  assert(src.format == LIBE3_E2SM_DAPP_CTRL_MSG_FORMAT_1 && "Unknown format");

  e2sm_dapp_ctrl_msg_t dst = {.format = FORMAT_1_E2SM_DAPP_CTRL_MSG};
  dst.frmt_1.data_size = (uint32_t)src.data.len;
  dst.frmt_1.data = take_bytes(&src.data).buf;

  libe3_e2sm_dapp_free_ctrl_msg(&src);
  return dst;
}

e2sm_dapp_ctrl_out_t dapp_dec_ctrl_out_asn(size_t len, uint8_t const ctrl_out[len])
{
  assert(ctrl_out != NULL);
  assert(len > 0);

  libe3_e2sm_dapp_ctrl_outcome_t src = {0};
  check_decoded(libe3_e2sm_dapp_decode_ctrl_outcome(ctrl_out, len, &src));
  assert(src.format == LIBE3_E2SM_DAPP_CTRL_OUTCOME_FORMAT_1 && "Unknown format");

  e2sm_dapp_ctrl_out_t dst = {.format = FORMAT_1_E2SM_DAPP_CTRL_OUT,
                              .frmt_1 = {.timestamp_ns = src.has_timestamp ? src.timestamp : 0,
                                         .sequence_id = src.has_sequence_id ? src.sequence_id : 0}};

  /* An empty outcome is the same as an absent one: NULL with size 0. */
  if (src.has_e3_control_outcome && src.e3_control_outcome.len > 0) {
    dst.frmt_1.e3_control_outcome_size = (uint32_t)src.e3_control_outcome.len;
    dst.frmt_1.e3_control_outcome = take_bytes(&src.e3_control_outcome).buf;
  }

  libe3_e2sm_dapp_free_ctrl_outcome(&src);
  return dst;
}

e2sm_dapp_func_def_t dapp_dec_func_def_asn(size_t len, uint8_t const func_def[len])
{
  assert(func_def != NULL);
  assert(len > 0);

  libe3_e2sm_dapp_ran_function_definition_t src = {0};
  check_decoded(libe3_e2sm_dapp_decode_ran_function_definition(func_def, len, &src));

  e2sm_dapp_func_def_t dst = {0};

  dst.name.name = take_bytes(&src.name.short_name);
  dst.name.oid = take_bytes(&src.name.e2sm_oid);
  dst.name.description = take_bytes(&src.name.description);
  /* The instance is dropped: cp_ran_function_name() still asserts it is NULL. */

  if (src.has_event_trigger) {
    dst.ev_trig = calloc(1, sizeof(ran_func_def_ev_trig_dapp_sm_t));
    assert(dst.ev_trig != NULL && "Memory exhausted");
  }

  if (src.has_report) {
    dst.report = calloc(1, sizeof(ran_func_def_report_dapp_sm_t));
    assert(dst.report != NULL && "Memory exhausted");
    dst.report->sz_seq_report_sty = src.n_report_styles;

    if (src.n_report_styles > 0) {
      dst.report->seq_report_sty = calloc(src.n_report_styles, sizeof(seq_report_sty_dapp_sm_t));
      assert(dst.report->seq_report_sty != NULL && "Memory exhausted");
    }

    for (size_t i = 0; i < src.n_report_styles; ++i) {
      libe3_e2sm_dapp_report_style_t* s = &src.report_styles[i];
      seq_report_sty_dapp_sm_t* out = &dst.report->seq_report_sty[i];

      out->report_type = (uint32_t)s->report_style_type;
      out->name = take_bytes(&s->name);
      out->ind_hdr_type = (uint32_t)s->ind_hdr_format_type;
      out->ind_msg_type = (uint32_t)s->ind_msg_format_type;

      if (s->has_dapp_e3_subscriptions) {
        out->dapp_e3_subs = calloc(1, sizeof(dapp_e3_subscription_list_t));
        assert(out->dapp_e3_subs != NULL && "Memory exhausted");
        *out->dapp_e3_subs = take_subs(&s->dapp_e3_subscriptions);
      }
    }
  }

  if (src.has_ctrl) {
    dst.ctrl = calloc(1, sizeof(ran_func_def_ctrl_dapp_sm_t));
    assert(dst.ctrl != NULL && "Memory exhausted");
    dst.ctrl->sz_seq_ctrl_style = src.n_ctrl_styles;

    if (src.n_ctrl_styles > 0) {
      dst.ctrl->seq_ctrl_style = calloc(src.n_ctrl_styles, sizeof(seq_ctrl_style_dapp_sm_t));
      assert(dst.ctrl->seq_ctrl_style != NULL && "Memory exhausted");
    }

    for (size_t i = 0; i < src.n_ctrl_styles; ++i) {
      libe3_e2sm_dapp_ctrl_style_t* s = &src.ctrl_styles[i];
      seq_ctrl_style_dapp_sm_t* out = &dst.ctrl->seq_ctrl_style[i];

      out->style_type = (uint32_t)s->ctrl_style_type;
      out->name = take_bytes(&s->name);
      out->hdr = (uint32_t)s->ctrl_hdr_format_type;
      out->msg = (uint32_t)s->ctrl_msg_format_type;
      out->out_frmt = (uint32_t)s->ctrl_outcome_format_type;

      if (s->has_dapp_e3_subscriptions) {
        out->dapp_e3_subs = calloc(1, sizeof(dapp_e3_subscription_list_t));
        assert(out->dapp_e3_subs != NULL && "Memory exhausted");
        *out->dapp_e3_subs = take_subs(&s->dapp_e3_subscriptions);
      }
    }
  }

  libe3_e2sm_dapp_free_ran_function_definition(&src);
  return dst;
}

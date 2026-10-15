/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "dapp_sm_ric.h"
#include "dapp_sm_id.h"

#include <assert.h>
#include <stdlib.h>

#include "ie/dapp_data_ie.h"
#include "e3/dapp_dec_e3.h"
#include "e3/dapp_enc_e3.h"

#include "enc/dapp_enc_generic.h"
#include "dec/dapp_dec_generic.h"

typedef struct {
  sm_ric_t base;

#ifdef ASN
  dapp_enc_asn_t enc;
#elif FLATBUFFERS
  dapp_enc_fb_t enc;
#elif PLAIN
  dapp_enc_plain_t enc;
#else
  static_assert(false, "No encoding type selected");
#endif
} sm_dapp_ric_t;

/**
 * @brief Build an E2 subscription for the DAPP SM on the RIC.
 *
 * Takes a high-level dapp_sub_data_t containing:
 *   - Event trigger (common to all styles, Format 1)
 *   - Action definition (optional, carries the report style type)
 *
 * Encodes both into byte arrays and returns an sm_subs_data_t ready
 * to be sent over E2AP in a RIC SUBSCRIPTION REQUEST.
 *
 * The action definition's ric_style_type determines what indications
 * the agent will send:
 *   - Style 1: Format 1 only (E3 data reports)
 *   - Style 2: Format 2 only (E3 subscription map)
 *
 * @param sm_ric  The RIC-side DAPP SM instance (codec context).
 * @param cmd     Pointer to a dapp_sub_data_t with event trigger and
 *                action definition.
 * @return        Encoded subscription data (event trigger + action definition).
 */
static sm_subs_data_t on_subscription_dapp_sm_ric(sm_ric_t const* sm_ric, void* cmd)
{
  assert(sm_ric != NULL);
  assert(cmd != NULL);
  dapp_sub_data_t const* src = cmd;
  sm_dapp_ric_t* sm = (sm_dapp_ric_t*)sm_ric;
  sm_subs_data_t dst = {0};

  /* Encode E2SM-DAPP event trigger */
  const byte_array_t ba_et = dapp_enc_event_trigger(&sm->enc, &src->et);
  dst.event_trigger = ba_et.buf;
  dst.len_et = ba_et.len;

  /* Encode E2SM-DAPP action definition */
  if (src->action_def != NULL) {
    const byte_array_t ba_ad = dapp_enc_action_def(&sm->enc, src->action_def);
    dst.action_def = ba_ad.buf;
    dst.len_ad = ba_ad.len;
  }

  return dst;
}

/**
 * @brief RIC-side indication handler for the DAPP service model.
 *
 * Called when the RIC receives an indication message for the DAPP SM
 * from the E2 agent. Performs a two-stage decode:
 *
 *  1) E2SM-DAPP layer:
 *     - Decodes the indication header and message using the DAPP codec.
 *     - Fills dst.dapp.ind.hdr and dst.dapp.ind.msg.
 *
 *  2) Inner payload (format-dependent):
 *     - Format 1 (E3 data report):
 *       Extracts the RAN function ID from the header. If the message
 *       carries a non-empty data buffer, calls dapp_dec_e3_indication()
 *       to decode the inner E3 payload into dst.dapp.ind.e3.
 *     - Format 2 (subscription map):
 *       The subscription list is already fully decoded by the E2SM-DAPP
 *       message decoder. Sets dst.dapp.ind.e3.type = DAPP_E3_SM_NONE
 *       (no inner E3 payload).
 *
 * @param sm_ric  The RIC-side DAPP SM instance (codec context).
 * @param src     Raw indication data received from the E2 agent.
 * @return        Decoded indication container (type DAPP_STATS_V0).
 */
static sm_ag_if_rd_ind_t on_indication_dapp_sm_ric(sm_ric_t const* sm_ric, sm_ind_data_t const* src)
{
  assert(sm_ric != NULL);
  assert(src != NULL);
  sm_dapp_ric_t* sm = (sm_dapp_ric_t*)sm_ric;

  sm_ag_if_rd_ind_t dst = {.type = DAPP_STATS_V0};

  /* Decode the E2SM-DAPP indication header, message and, for format 1, the
   * inner E3 payload the message carries. */
  dst.dapp.ind.hdr = dapp_dec_ind_hdr(&sm->enc, src->len_hdr, src->ind_hdr);
  dst.dapp.ind.msg = dapp_dec_ind_msg(&sm->enc, src->len_msg, src->ind_msg);
  dst.dapp.ric_id = 0;

  if (dst.dapp.ind.msg.format == FORMAT_1_E2SM_DAPP_IND_MSG) {
    /* Format 1: decode inner E3 payload from raw bytes */
    assert(dst.dapp.ind.hdr.format == FORMAT_1_E2SM_DAPP_IND_HDR);

    e2sm_dapp_ind_msg_frmt_1_t* m1 = &dst.dapp.ind.msg.frmt_1;
    uint32_t ran_function_id = dst.dapp.ind.hdr.frmt_1.ran_function_id;

    if (m1->data_size > 0 && m1->data != NULL) {
      bool ok = dapp_dec_e3_indication(ran_function_id, m1->data, m1->data_size, &dst.dapp.ind.e3);
      if (!ok) {
        assert(0 && "Failed to decode inner E3 payload");
      }
    } else {
      dst.dapp.ind.e3.type = DAPP_E3_SM_NONE;
    }
  } else if (dst.dapp.ind.msg.format == FORMAT_2_E2SM_DAPP_IND_MSG) {
    /* Format 2: subscription map — no inner E3 payload to decode */
    assert(dst.dapp.ind.hdr.format == FORMAT_2_E2SM_DAPP_IND_HDR);
    dst.dapp.ind.e3.type = DAPP_E3_SM_NONE;
  } else {
    assert(0 != 0 && "Unknown indication message format");
  }

  return dst;
}

/**
 * @brief RIC-side handler for DAPP SM control requests.
 *
 * This function is called when the RIC plugin receives a control request
 * for the DAPP SM from the xApp side.
 *
 * High-level behavior:
 *  - Interprets @p ctrl as a dapp_ctrl_req_data_t (E2SM-DAPP control IR).
 *  - If the request carries a non-empty E3 control payload (tmp.e3.type !=
 *    DAPP_E3_SM_NONE) and the E2SM-DAPP ControlMessage (format 1) currently
 *    has no inner data (data == NULL, data_size == 0), it:
 *      1) Encodes the E3 control payload into a binary buffer using
 *         dapp_enc_e3_control(), based on the ran_function_id in the header.
 *      2) Injects the encoded E3 buffer into tmp.msg.frmt_1.data / data_size.
 *      3) Encodes the DAPP ControlHeader and ControlMessage into byte arrays
 *         using dapp_enc_ctrl_hdr() and dapp_enc_ctrl_msg().
 *      4) Releases the temporary E3 buffer once dapp_enc_ctrl_msg() has
 *         copied its bytes into the outer ControlMessage encoding.
 *  - Fills a sm_ctrl_req_data_t structure with:
 *      * ctrl_hdr / len_hdr: encoded E2SM-DAPP ControlHeader
 *      * ctrl_msg / len_msg: encoded E2SM-DAPP ControlMessage
 *  - Returns the filled sm_ctrl_req_data_t, which the FlexRIC core then
 *    forwards over the E2 interface.
 *
 * @param sm_ric  Pointer to the RIC-side DAPP SM instance (codec context).
 * @param ctrl    Pointer to a dapp_ctrl_req_data_t describing the control.
 *
 * @return Encoded control header and message ready to be sent over E2.
 */
static sm_ctrl_req_data_t ric_on_control_req_dapp_sm_ric(sm_ric_t const* sm_ric, void* ctrl)
{
  assert(sm_ric != NULL);
  assert(ctrl != NULL);

  sm_dapp_ric_t* sm = (sm_dapp_ric_t*)sm_ric;

  dapp_ctrl_req_data_t const* req = (dapp_ctrl_req_data_t const*)ctrl;
  sm_ctrl_req_data_t dst = (sm_ctrl_req_data_t){0};

  dapp_ctrl_req_data_t tmp = *req;

  if (tmp.e3.type != DAPP_E3_SM_NONE && tmp.msg.format == FORMAT_1_E2SM_DAPP_CTRL_MSG && tmp.msg.frmt_1.data == NULL
      && tmp.msg.frmt_1.data_size == 0) {
    uint8_t* e3_buf = NULL;
    size_t e3_len = 0;

    bool ok = dapp_enc_e3_control(tmp.hdr.frmt_1.ran_function_id, &tmp.e3, &e3_buf, &e3_len);
    if (!ok) {
      fprintf(stderr, "[DAPP][RIC] Failed to encode E3 control payload\n");
      assert(0 && "E3 control encoding failed");
    }

    tmp.msg.frmt_1.data = e3_buf;
    tmp.msg.frmt_1.data_size = (uint32_t)e3_len;

    byte_array_t ba_hdr = dapp_enc_ctrl_hdr(&sm->enc, &tmp.hdr);
    dst.ctrl_hdr = ba_hdr.buf;
    dst.len_hdr = ba_hdr.len;

    byte_array_t ba_msg = dapp_enc_ctrl_msg(&sm->enc, &tmp.msg);
    dst.ctrl_msg = ba_msg.buf;
    dst.len_msg = ba_msg.len;

    free(e3_buf);
  }

  return dst;
}

/**
 * @brief Decode a DAPP control outcome received at the RIC.
 *
 * - Takes raw control outcome bytes from the agent.
 * - Decodes them into the dapp-specific control outcome structure.
 */
static sm_ag_if_ans_ctrl_t ric_on_control_out_dapp_sm_ric(sm_ric_t const* sm_ric, const sm_ctrl_out_data_t* src)
{
  assert(sm_ric != NULL);
  assert(src != NULL);
  sm_dapp_ric_t* sm = (sm_dapp_ric_t*)sm_ric;

  /* Decode E2SM-DAPP control outcome */
  sm_ag_if_ans_ctrl_t dst = {.type = DAPP_AGENT_IF_CTRL_ANS_V0};
  dst.dapp = dapp_dec_ctrl_out(&sm->enc, src->len_out, src->ctrl_out);

  return dst;
}

/**
 * @brief Decode the E2 Setup Response for the DAPP SM at the RIC.
 *
 * - Parses the RAN function definition from the E2 setup data.
 * - Wraps it into an sm_ag_if_rd_e2setup_t with type DAPP_AGENT_IF_E2_SETUP_ANS_V0.
 */
static sm_ag_if_rd_e2setup_t ric_on_e2_setup_dapp_sm_ric(sm_ric_t const* sm_ric, sm_e2_setup_data_t const* src)
{
  assert(sm_ric != NULL);
  assert(src != NULL);
  sm_dapp_ric_t* sm = (sm_dapp_ric_t*)sm_ric;

  sm_ag_if_rd_e2setup_t dst = {.type = DAPP_AGENT_IF_E2_SETUP_ANS_V0};

  dst.dapp.ran_func_def = dapp_dec_func_def(&sm->enc, src->len_rfd, src->ran_fun_def);

  return dst;
}
/**
 * @brief Handle RIC Service Update messages for the DAPP SM.
 *
 * Currently a placeholder that returns an empty sm_ag_if_rd_rsu_t.
 */
static sm_ag_if_rd_rsu_t on_ric_service_update_dapp_sm_ric(sm_ric_t const* sm_ric, sm_ric_service_update_data_t const* src)
{
  assert(sm_ric != NULL);
  assert(src != NULL);
  sm_ag_if_rd_rsu_t dst = {0};
  return dst;
}

/**
 * @brief Free the DAPP SM RIC instance allocated by make_dapp_sm_ric().
 *
 * Releases the sm_dapp_ric_t container itself; the framework's plugin
 * loader stores no extra heap state beyond it.
 */
static void free_dapp_sm_ric(sm_ric_t* sm_ric)
{
  assert(sm_ric != NULL);
  sm_dapp_ric_t* sm = (sm_dapp_ric_t*)sm_ric;
  free(sm);
}

/*
 * The six alloc.free_* callbacks below are the duals of the on_* procedures
 * in sm_e2ap_procedures_ric_t — each one releases the heap memory the
 * matching procedure returned. The framework currently invokes
 * free_ind_data after every on_indication (msg_handler_ric.c); the others
 * are wired in for forward-compatibility.
 */

/**
 * @brief Free the sm_subs_data_t produced by on_subscription_dapp_sm_ric.
 *
 * Releases the encoded event_trigger and action_def byte buffers.
 */
static void free_subs_data_dapp_sm_ric(void* msg)
{
  assert(msg != NULL);
  free_sm_subs_data((sm_subs_data_t*)msg);
}

/**
 * @brief Free the sm_ag_if_rd_ind_t produced by on_indication_dapp_sm_ric.
 *
 * The framework calls this after publishing the indication. Releases the
 * decoded DAPP indication header, message, and inner E3 payload.
 */
static void free_ind_data_dapp_sm_ric(void* msg)
{
  assert(msg != NULL);
  sm_ag_if_rd_ind_t* rd_ind = (sm_ag_if_rd_ind_t*)msg;
  assert(rd_ind->type == DAPP_STATS_V0);
  free_dapp_ind_data(&rd_ind->dapp.ind);
}

/**
 * @brief Free the sm_ctrl_req_data_t produced by ric_on_control_req_dapp_sm_ric.
 *
 * Releases the encoded ctrl_hdr and ctrl_msg byte buffers.
 */
static void free_ctrl_req_data_dapp_sm_ric(void* msg)
{
  assert(msg != NULL);
  free_sm_ctrl_req_data((sm_ctrl_req_data_t*)msg);
}

/**
 * @brief Free the sm_ag_if_ans_ctrl_t produced by ric_on_control_out_dapp_sm_ric.
 *
 * Releases the decoded DAPP control outcome in the .dapp union member.
 */
static void free_ctrl_out_data_dapp_sm_ric(void* msg)
{
  assert(msg != NULL);
  sm_ag_if_ans_ctrl_t* ans = (sm_ag_if_ans_ctrl_t*)msg;
  assert(ans->type == DAPP_AGENT_IF_CTRL_ANS_V0);
  free_e2sm_dapp_ctrl_out(&ans->dapp);
}

/**
 * @brief Free the sm_ag_if_rd_e2setup_t produced by ric_on_e2_setup_dapp_sm_ric.
 *
 * Releases the decoded RAN function definition in the .dapp union member.
 */
static void free_e2_setup_dapp_sm_ric(void* msg)
{
  assert(msg != NULL);
  sm_ag_if_rd_e2setup_t* e2 = (sm_ag_if_rd_e2setup_t*)msg;
  assert(e2->type == DAPP_AGENT_IF_E2_SETUP_ANS_V0);
  free_e2sm_dapp_func_def(&e2->dapp.ran_func_def);
}

/**
 * @brief Free the sm_ag_if_rd_rsu_t produced by on_ric_service_update_dapp_sm_ric.
 *
 * The matching procedure is currently a stub that returns a zero-initialized
 * structure, so there is nothing to release; the function exists only to
 * keep the alloc.free_* table fully populated.
 */
static void free_ric_service_update_dapp_sm_ric(void* msg)
{
  assert(msg != NULL);
  (void)msg;
}

/**
 * @brief Allocate and initialize the DAPP Service Model instance at the RIC.
 *
 * - Allocates sm_dapp_ric_t.
 * - Sets the RAN function ID, name, and all function pointers for
 *   subscription, indication, control, E2 setup, and service update handlers.
 */
sm_ric_t* make_dapp_sm_ric(void)
{
  sm_dapp_ric_t* sm = calloc(1, sizeof(sm_dapp_ric_t));
  assert(sm != NULL && "Memory exhausted");

  *((uint16_t*)&sm->base.ran_func_id) = SM_DAPP_ID;

  sm->base.free_sm = free_dapp_sm_ric;

  sm->base.alloc.free_subs_data_msg = free_subs_data_dapp_sm_ric;
  sm->base.alloc.free_ind_data = free_ind_data_dapp_sm_ric;
  sm->base.alloc.free_ctrl_req_data = free_ctrl_req_data_dapp_sm_ric;
  sm->base.alloc.free_ctrl_out_data = free_ctrl_out_data_dapp_sm_ric;

  sm->base.alloc.free_e2_setup = free_e2_setup_dapp_sm_ric;
  sm->base.alloc.free_ric_service_update = free_ric_service_update_dapp_sm_ric;

  sm->base.proc.on_subscription = on_subscription_dapp_sm_ric;
  sm->base.proc.on_indication = on_indication_dapp_sm_ric;

  sm->base.proc.on_control_req = ric_on_control_req_dapp_sm_ric;
  sm->base.proc.on_control_out = ric_on_control_out_dapp_sm_ric;

  sm->base.proc.on_e2_setup = ric_on_e2_setup_dapp_sm_ric;
  sm->base.proc.on_ric_service_update = on_ric_service_update_dapp_sm_ric;
  sm->base.handle = NULL;

  assert(strlen(SM_DAPP_SHORT_NAME) < sizeof(sm->base.ran_func_name));
  memcpy(sm->base.ran_func_name, SM_DAPP_SHORT_NAME, strlen(SM_DAPP_SHORT_NAME));

  return &sm->base;
}

/**
 * @brief Return the RAN function ID for the given DAPP SM RIC instance.
 */
uint16_t id_sm_dapp_ric(sm_ric_t const* sm_ric)
{
  assert(sm_ric != NULL);
  sm_dapp_ric_t* sm = (sm_dapp_ric_t*)sm_ric;
  return sm->base.ran_func_id;
}

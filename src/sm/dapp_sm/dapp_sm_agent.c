/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "dapp_sm_agent.h"

#include "../../util/alg_ds/alg/defer.h"
#include "dapp_sm_id.h"
#include "enc/dapp_enc_generic.h"
#include "dec/dapp_dec_generic.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  sm_agent_t base;

#ifdef ASN
  dapp_enc_asn_t enc;
#elif FLATBUFFERS
  dapp_enc_fb_t enc;
#elif PLAIN
  dapp_enc_plain_t enc;
#else
  static_assert(false, "No encoding type selected");
#endif

} sm_dapp_agent_t;

/**
 * @brief Handle a new subscription for the DAPP service model on the agent.
 *
 * Decodes the E2SM-DAPP event trigger and action definition from the
 * incoming subscription data, wraps them in a wr_dapp_sub_data_t, and
 * forwards the subscription to the RAN via sm->base.io.write_subs().
 *
 * The action definition carries the report style type, which determines
 * what indication formats the agent will send to this subscriber:
 *   - Style 1: Format 1 only (E3 data reports)
 *   - Style 2: Format 2 only (E3 subscription map)
 *
 * @param sm_agent  The DAPP SM agent instance.
 * @param data      Raw subscription data from the E2AP layer (event trigger
 *                  and action definition as encoded byte arrays).
 * @return          Subscription outcome (APERIODIC_SUBSCRIPTION_FLRC).
 */
static sm_ag_if_ans_subs_t on_subscription_dapp_sm_ag(sm_agent_t const* sm_agent, const sm_subs_data_t* data)
{
  assert(sm_agent != NULL);
  sm_dapp_agent_t* sm = (sm_dapp_agent_t*)sm_agent;
  assert(data != NULL);

  wr_dapp_sub_data_t wr_dapp = {0};
  wr_dapp.ric_req_id = data->ric_req_id;

  /* Decode E2SM-DAPP event trigger */
  wr_dapp.dapp.et = dapp_dec_event_trigger(&sm->enc, data->len_et, data->event_trigger);
  defer({ free_e2sm_dapp_event_trigger(&wr_dapp.dapp.et); });

  /* Decode E2SM-DAPP action definition (if present) */
  if (data->action_def != NULL && data->len_ad > 0) {
    wr_dapp.dapp.action_def = calloc(1, sizeof(e2sm_dapp_action_def_t));
    assert(wr_dapp.dapp.action_def != NULL && "Memory exhausted");
    *wr_dapp.dapp.action_def = dapp_dec_action_def(&sm->enc, data->len_ad, data->action_def);
  }
  defer({
    if (wr_dapp.dapp.action_def != NULL) {
      free_e2sm_dapp_action_def(wr_dapp.dapp.action_def);
      free(wr_dapp.dapp.action_def);
    }
  });

  /* Forward subscription to the RAN via the SM I/O interface */
  sm_ag_if_ans_t subs = sm->base.io.write_subs(&wr_dapp);
  assert(subs.type == SUBS_OUTCOME_SM_AG_IF_ANS_V0);
  assert(subs.subs_out.type == APERIODIC_SUBSCRIPTION_FLRC);

  return subs.subs_out;
}

/**
 * @brief Encode and export an indication for the DAPP service model.
 *
 * Takes an internal dapp_ind_data_t (header + message + E3 payload),
 * encodes the indication header and message using the E2SM-DAPP codec,
 * and returns an exp_ind_data_t containing the raw encoded buffers.
 *
 * Supports both indication formats:
 *   - Format 1: E3 data report (header with ran_function_id/dapp_id)
 *   - Format 2: E3 subscription map (header with node identity only)
 *
 * The encoded buffers are sent over E2AP as OCTET STRINGs in the
 * RIC INDICATION message.
 *
 * @param sm_agent  The DAPP SM agent instance.
 * @param ind_data  Pointer to a heap-allocated dapp_ind_data_t. Ownership
 *                  is transferred — the function frees it before returning.
 * @return          Encoded indication header and message buffers.
 */
static exp_ind_data_t on_indication_dapp_sm_ag(sm_agent_t const* sm_agent, void* ind_data)
{
  assert(sm_agent != NULL);
  assert(ind_data != NULL && "Indication data needed for this SM");
  sm_dapp_agent_t* sm = (sm_dapp_agent_t*)sm_agent;

  exp_ind_data_t ret = {.has_value = true};

  dapp_ind_data_t* ind = (dapp_ind_data_t*)ind_data;
  defer({
    free_dapp_ind_data(ind);
    free(ind);
  });

  /* Encode indication header */
  byte_array_t ba_hdr = dapp_enc_ind_hdr(&sm->enc, &ind->hdr);
  assert(ba_hdr.len < 16 * 1024 && "Are you really encoding so much info?");
  ret.data.ind_hdr = ba_hdr.buf;
  ret.data.len_hdr = ba_hdr.len;

  /* Encode indication message */
  byte_array_t ba_msg = dapp_enc_ind_msg(&sm->enc, &ind->msg);
  assert(ba_msg.len < 16 * 1024 && "Are you really encoding so much info?");
  ret.data.ind_msg = ba_msg.buf;
  ret.data.len_msg = ba_msg.len;

  return ret;
}

/**
 * @brief Handle a control request for the DAPP service model on the agent.
 *
 * Decodes the E2SM-DAPP control header and message from the incoming
 * request, forwards them to the RAN via sm->base.io.write_ctrl(), and
 * encodes the resulting control outcome back into a byte array.
 *
 * The RAN-side implementation of write_ctrl is what forwards the control on
 * to its destination; this service model only relays it.
 *
 * @param sm_agent  The DAPP SM agent instance.
 * @param data      Encoded control header and message from the RIC.
 * @return          Encoded control outcome to send back to the RIC.
 */
static sm_ctrl_out_data_t on_control_dapp_sm_ag(sm_agent_t const* sm_agent, sm_ctrl_req_data_t const* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);
  sm_dapp_agent_t* sm = (sm_dapp_agent_t*)sm_agent;

  /* Decode the E2SM-DAPP control request */
  dapp_ctrl_req_data_t dapp_ctrl = {0};
  dapp_ctrl.hdr = dapp_dec_ctrl_hdr(&sm->enc, data->len_hdr, data->ctrl_hdr);
  dapp_ctrl.msg = dapp_dec_ctrl_msg(&sm->enc, data->len_msg, data->ctrl_msg);
  defer({ free_dapp_ctrl_req_data(&dapp_ctrl); });

  /* Forward control to the RAN via the SM I/O interface */
  sm_ag_if_ans_t ret = sm->base.io.write_ctrl(&dapp_ctrl);
  assert(ret.type == CTRL_OUTCOME_SM_AG_IF_ANS_V0);
  assert(ret.ctrl_out.type == DAPP_AGENT_IF_CTRL_ANS_V0);
  defer({ free_e2sm_dapp_ctrl_out(&ret.ctrl_out.dapp); });

  /* Encode the E2SM-DAPP control outcome */
  byte_array_t ba = dapp_enc_ctrl_out(&sm->enc, &ret.ctrl_out.dapp);
  sm_ctrl_out_data_t ans = {.ctrl_out = ba.buf, .len_out = ba.len};

  return ans;
}

/**
 * @brief Build the RAN function name, OID, and description for the DAPP SM.
 *
 * Uses the constants defined in dapp_sm_id.h (SM_DAPP_SHORT_NAME,
 * SM_DAPP_OID, SM_DAPP_DESCRIPTION).
 *
 * @return  Populated ran_function_name_t (caller owns the byte arrays).
 */
static ran_function_name_t fill_ran_func_name(void)
{
  ran_function_name_t dst = {0};

  // RAN Function Short Name
  dst.name = cp_str_to_ba(SM_DAPP_SHORT_NAME);

  // RAN Function Service Model OID
  dst.oid = cp_str_to_ba(SM_DAPP_OID);

  // RAN Function Description
  dst.description = cp_str_to_ba(SM_DAPP_DESCRIPTION);

  return dst;
}

/**
 * @brief Build the E2 Setup data for the DAPP SM on the agent.
 *
 * Called during E2 setup and RIC service update. Retrieves the DAPP
 * RAN function definition from the RAN (which includes report styles,
 * control styles, and the current dApp E3 subscription map), sets the
 * RAN function name/OID/description, and encodes the full definition
 * into a byte array for inclusion in the E2 SETUP REQUEST or
 * RIC SERVICE UPDATE message.
 *
 * @param sm_agent  The DAPP SM agent instance.
 * @return          Encoded RAN function definition.
 */
static sm_e2_setup_data_t on_e2_setup_dapp_sm_ag(sm_agent_t const* sm_agent)
{
  assert(sm_agent != NULL);

  sm_dapp_agent_t* sm = (sm_dapp_agent_t*)sm_agent;

  dapp_e2_setup_t dapp = {0};
  /* An agent that only emulates the RAN side leaves read_setup NULL; the RAN
     function definition below is then built from the SM's own constants. */
  if (sm->base.io.read_setup)
    sm->base.io.read_setup(&dapp);

  free_ran_function_name(&dapp.ran_func_def.name);
  dapp.ran_func_def.name = fill_ran_func_name();

  assert(dapp.ran_func_def.name.name.buf != NULL);
  assert(dapp.ran_func_def.name.name.len > 0);

  e2sm_dapp_func_def_t* ran_func = &dapp.ran_func_def;

  byte_array_t ba = dapp_enc_func_def(&sm->enc, ran_func);

  defer({ free_e2sm_dapp_func_def(ran_func); });

  sm_e2_setup_data_t setup = {0};
  setup.len_rfd = ba.len;
  setup.ran_fun_def = ba.buf;

  return setup;
}

/**
 * @brief Handle RIC Service Update for the DAPP SM on the agent.
 *
 * Currently a stub that returns an empty sm_ric_service_update_data_t,
 * as RIC Service Update is not used for this service model.
 */
static sm_ric_service_update_data_t on_ric_service_update_dapp_sm_ag(sm_agent_t const* sm_agent)
{
  sm_ric_service_update_data_t dst = {0};
  return dst;
}

/**
 * @brief Free the DAPP service model agent instance.
 *
 * Releases the memory allocated for sm_dapp_agent_t.
 */
static void free_dapp_sm_ag(sm_agent_t* sm_agent)
{
  assert(sm_agent != NULL);
  sm_dapp_agent_t* sm = (sm_dapp_agent_t*)sm_agent;
  free(sm);
}

/**
 * @brief Return the human-readable definition (short name) of the DAPP SM.
 */
static char const* def_dapp_sm_ag(void)
{
  return SM_DAPP_SHORT_NAME;
}

/**
 * @brief Return the RAN function ID of the DAPP service model.
 */
static uint16_t id_dapp_sm_ag(void)
{
  return SM_DAPP_ID;
}

/**
 * @brief Return the revision number of the DAPP service model.
 */
static uint16_t rev_dapp_sm_ag(void)
{
  return SM_DAPP_REV;
}

/**
 * @brief Return the OID string of the DAPP service model.
 */
static char const* oid_dapp_sm_ag(void)
{
  return SM_DAPP_OID;
}

/**
 * @brief Allocate and initialize the DAPP service model agent.
 *
 * Creates the internal sm_dapp_agent_t, wires the I/O callbacks from
 * the sm_io_ag_ran_t table, and registers all SM procedure handlers:
 *   - on_subscription: decodes event trigger + action definition,
 *     forwards to write_subs which routes by report style type
 *   - on_indication: encodes Format 1 or Format 2 indications
 *   - on_control: decodes/forwards/encodes control requests
 *   - on_e2_setup: builds the RAN function definition
 *   - on_ric_service_update: stub (updates triggered externally)
 *
 * @param io  I/O callback table connecting the SM to the RAN functions.
 * @return    Pointer to the initialized SM agent (caller owns).
 */
sm_agent_t* make_dapp_sm_agent(sm_io_ag_ran_t io)
{
  sm_dapp_agent_t* sm = calloc(1, sizeof(sm_dapp_agent_t));
  assert(sm != NULL && "Memory exhausted!!!");

  sm->base.io.read_ind = io.read_ind_tbl[DAPP_STATS_V0];
  sm->base.io.read_setup = io.read_setup_tbl[DAPP_AGENT_IF_E2_SETUP_ANS_V0];

  sm->base.io.write_ctrl = io.write_ctrl_tbl[DAPP_CTRL_V0];
  sm->base.io.write_subs = io.write_subs_tbl[DAPP_SUBS_V0];

  sm->base.free_sm = free_dapp_sm_ag;
  sm->base.free_act_def = NULL;

  sm->base.proc.on_subscription = on_subscription_dapp_sm_ag;
  sm->base.proc.on_indication = on_indication_dapp_sm_ag;
  sm->base.proc.on_control = on_control_dapp_sm_ag;
  sm->base.proc.on_ric_service_update = on_ric_service_update_dapp_sm_ag;
  sm->base.proc.on_e2_setup = on_e2_setup_dapp_sm_ag;
  sm->base.handle = NULL;

  sm->base.info.def = def_dapp_sm_ag;
  sm->base.info.id = id_dapp_sm_ag;
  sm->base.info.rev = rev_dapp_sm_ag;
  sm->base.info.oid = oid_dapp_sm_ag;

  return &sm->base;
}

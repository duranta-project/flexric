/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

/**
 * @file xapp_spectrum.c
 * @brief Demo xApp for the E2SM-DAPP service model.
 *
 * This xApp shows how to handle subscription, indications, and controls
 * for the E2SM-DAPP Service Model over the E2 interface via FlexRIC.
 *
 * Subscription behavior is controlled by the report style type:
 *   - Style 1 (DAPP_STYLE_E3_DATA): receives only E3 data reports.
 *   - Style 2 (DAPP_STYLE_SUBSCRIPTION_MAP): receives only subscription
 *     map updates.
 *
 * This version creates two independent subscriptions per E2 node:
 *   - Subscription A: Style 1 (E3 data only)        → sm_cb_dapp_frmt1
 *   - Subscription B: Style 2 (subscription map only) → sm_cb_dapp_frmt2
 *
 * Each subscription receives a distinct ric_req_id, so both the xApp-side
 * active-procedure registry and the agent-side bimap track them
 * independently. Deletion is safe: each handle is removed by its own
 * ric_req_id without affecting the other.
 *
 * Indication callbacks:
 *   - sm_cb_dapp_frmt1: handles Format 1 (E3 data, e.g. spectrum PRBs)
 *   - sm_cb_dapp_frmt2: handles Format 2 (dApp E3 subscription map)
 *
 * Control: predefined spectrum control variants can be sent periodically
 * to all E2 nodes.
 */

#include "../../../../src/util/alg_ds/ds/lock_guard/lock_guard.h"
#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/util/e.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>
#include <signal.h>
#include <pthread.h>

/** @brief DAPP SM identifier (RAN Function ID 255). */
int const DAPP_SM_ID = 255;

/** @brief E3 RAN function IDs used by inner service models. */
enum {
  E3_RAN_FUNC_ID_SPECTRUM = 1,
};

/**
 * @brief E2SM-DAPP report style types.
 *
 * These correspond to the styles advertised in the RAN Function Definition:
 *   - Style 1: E3 data reports only (indication format 1)
 *   - Style 2: E3 subscription map only (indication format 2)
 */
enum {
  DAPP_STYLE_E3_DATA = 1,
  DAPP_STYLE_SUBSCRIPTION_MAP = 2,
};

/** @brief Global termination flag, set by the signal handler thread. */
static volatile sig_atomic_t g_terminate = 0;

/**
 * @brief Dedicated thread that waits for SIGINT or SIGTERM.
 *
 * Uses sigwait() on a pre-blocked signal set so that the main thread
 * and all other threads are not interrupted. Sets g_terminate = 1
 * when a signal is received.
 */
static void* signal_thread(void* arg)
{
  sigset_t* set = (sigset_t*)arg;
  int sig;

  if (sigwait(set, &sig) == 0) {
    g_terminate = 1;
  }
  return NULL;
}

/* ------------------------------------------------------------------ */
/* Predefined control message variants                                 */
/* ------------------------------------------------------------------ */

/** @brief A list of PRB indices to be blocked. */
typedef struct {
  size_t count;
  uint16_t prbs[273];
} prb_list_t;


/**
 * @brief Next value of this xApp's control-request correlation counter.
 *
 * The correlation id is part of the E2SM-DAPP wire format, so it is produced
 * unconditionally:
 * wire-visible correlation id, not a benchmarking-only one, so it must not
 * depend on that build flag. Starts at 1 -- 0 is the E2SM-DAPP "no id
 * assigned" sentinel (see e2sm_dapp_v0_standard.asn's SequenceId comment).
 */
static uint32_t next_sequence_id(void) {
  static uint32_t counter;
  return ++counter;
}

/**
 * @brief Predefined spectrum control variants for testing.
 */
static const prb_list_t ctrl_variants[] = {
    {.count = 4, .prbs = {75, 80, 85, 90}},
    {.count = 5, .prbs = {100, 101, 102, 103, 104, 105}},
    {.count = 6, .prbs = {75, 76, 77, 103, 104, 105}},
    {.count = 3, .prbs = {80, 90, 100}},
    {.count = 0, .prbs = {0}},
};

#define NUM_VARIANTS (sizeof(ctrl_variants) / sizeof(ctrl_variants[0]))

/**
 * @brief Print E2 node identity fields common to both header formats.
 *
 * @param node_type     ngran_node_t cast to uint8_t.
 * @param plmn_id       3-byte packed PLMN identity.
 * @param node_nb_id    gNB / eNB numeric identity.
 * @param cu_du_present Whether the CU-DU ID is valid.
 * @param cu_du_id      CU-DU ID (only valid when cu_du_present is true).
 */
static void print_node_identity(uint8_t node_type,
                                const uint8_t plmn_id[3],
                                uint32_t node_nb_id,
                                bool cu_du_present,
                                uint64_t cu_du_id)
{
  printf("[DAPP][SM_CB]   node_type=%u, plmn_id=%02x%02x%02x, node_nb_id=%u\n",
         node_type,
         plmn_id[0],
         plmn_id[1],
         plmn_id[2],
         node_nb_id);
  if (cu_du_present) {
    printf("[DAPP][SM_CB]   node_cu_du_id=%lu\n", (unsigned long)cu_du_id);
  }
}

/* ------------------------------------------------------------------ */
/* Callback: Format 1 — E3 data reports                               */
/* ------------------------------------------------------------------ */

/**
 * @brief Indication callback for Format 1 (E3 data report) indications.
 *
 * Expects indication header format 1 (ran_function_id, dapp_id, node
 * identity) and indication message format 1 (raw E3 data payload).
 * Dispatches on the E3 SM type to decode and print the inner payload.
 *
 * Currently supports:
 *   - DAPP_E3_SM_SPECTRUM: prints the list of PRBs.
 */
static void sm_cb_dapp_frmt1(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  /* The SM callback runs on the dispatcher thread, not the E2 receive loop, so
   * it has neither that thread's ring nor its key: open one here (idempotent)
   * and stamp a point event to pair with the E2AP receive loop's stamps by
   * time. lat_rep also keys XAPP_PROCESS_DONE below, marking B4's exit --
   * this function *is* the xApp's decision, so its return is the decision. */

  const e2sm_dapp_ind_hdr_t* hdr = &rd->ind.dapp.ind.hdr;
  const e2sm_dapp_ind_msg_t* msg = &rd->ind.dapp.ind.msg;
  const dapp_e3_ind_payload_t* e3 = &rd->ind.dapp.ind.e3;

  assert(hdr->format == FORMAT_1_E2SM_DAPP_IND_HDR);
  assert(msg->format == FORMAT_1_E2SM_DAPP_IND_MSG);

  const e2sm_dapp_ind_hdr_frmt_1_t* h1 = &hdr->frmt_1;
  printf("[DAPP][FMT1] Indication: ran_function_id=%u, dapp_id=%u, age=%" PRId64
         " us, sequence_id=%" PRId64 "\n",
         h1->ran_function_id, h1->dapp_id,
         (time_now_ns() - h1->timestamp_ns) / 1000, h1->sequence_id);
  print_node_identity(h1->node_type, h1->node_plmn_id, h1->node_nb_id, h1->node_cu_du_id_present, h1->node_cu_du_id);

  const e2sm_dapp_ind_msg_frmt_1_t* m1 = &msg->frmt_1;

  if (m1->data == NULL || m1->data_size == 0) {
    printf("[DAPP][FMT1] Payload: <empty>\n");
    return;
  }

  printf("[DAPP][FMT1]   data_size=%u\n", (unsigned)m1->data_size);

  if (e3->type == DAPP_E3_SM_SPECTRUM) {
    const spectrum_sm_report_t* rep = &e3->u.spectrum;
    printf("[DAPP][FMT1] Spectrum E3 payload: prb_count=%ld, dApp-to-xApp=%" PRId64 " us\n",
           rep->prb_count, (time_now_ns() - rep->timestamp_ns) / 1000);
    printf("[DAPP][FMT1] %ld PRBs:", rep->prb_count);
    for (long i = 0; i < rep->prb_count; ++i) {
      printf(" %u", rep->prbs[i]);
    }
    printf("\n");
  }
}

/* ------------------------------------------------------------------ */
/* Callback: Format 2 — dApp E3 subscription map                      */
/* ------------------------------------------------------------------ */

/**
 * @brief Indication callback for Format 2 (subscription map) indications.
 *
 * Expects indication header format 2 (node identity only) and indication
 * message format 2 (dApp E3 subscription list).
 *
 * Prints the current subscription map: which dApps are connected and
 * which E3 RAN functions each is subscribed to. If no dApps are
 * registered, prints an explicit "no dApps" message.
 */
static void sm_cb_dapp_frmt2(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);

  const e2sm_dapp_ind_hdr_t* hdr = &rd->ind.dapp.ind.hdr;
  const e2sm_dapp_ind_msg_t* msg = &rd->ind.dapp.ind.msg;

  assert(hdr->format == FORMAT_2_E2SM_DAPP_IND_HDR);
  assert(msg->format == FORMAT_2_E2SM_DAPP_IND_MSG);

  const e2sm_dapp_ind_hdr_frmt_2_t* h2 = &hdr->frmt_2;
  printf("[DAPP][FMT2] Indication (subscription map): age=%" PRId64
         " us, sequence_id=%" PRId64 "\n",
         (time_now_ns() - h2->timestamp_ns) / 1000, h2->sequence_id);
  print_node_identity(h2->node_type, h2->node_plmn_id, h2->node_nb_id, h2->node_cu_du_id_present, h2->node_cu_du_id);

  const e2sm_dapp_ind_msg_frmt_2_t* m2 = &msg->frmt_2;
  const dapp_e3_subscription_list_t* subs = &m2->dapp_e3_subs;

  if (subs->sz_dapp_e3_subscriptions == 0 || subs->dapp_e3_subscriptions == NULL) {
    printf("[DAPP][FMT2] No dApps currently registered on this E2 node\n");
    return;
  }

  printf("[DAPP][FMT2] dApp E3 subscriptions: %zu dApp(s)\n", subs->sz_dapp_e3_subscriptions);

  for (size_t i = 0; i < subs->sz_dapp_e3_subscriptions; ++i) {
    const dapp_e3_subscription_item_t* item = &subs->dapp_e3_subscriptions[i];
    printf("[DAPP][FMT2]   dapp_id=%u, e3_ran_functions(%zu):", item->dapp_id, item->sz_subscribed_e3_ran_functions);
    for (size_t j = 0; j < item->sz_subscribed_e3_ran_functions; ++j) {
      printf(" %u", item->subscribed_e3_ran_functions[j]);
    }
    printf("\n");
  }
}

/* ------------------------------------------------------------------ */
/* Control helpers                                                     */
/* ------------------------------------------------------------------ */

/**
 * @brief Send a DAPP control message to all connected E2 nodes.
 *
 * Builds a dapp_ctrl_req_data_t with the given RAN function ID, dApp ID,
 * and E3 control payload, then sends it to each E2 node via
 * control_sm_xapp_api().
 *
 * @param nodes             Connected E2 nodes.
 * @param SM_ID             SM identifier (DAPP_SM_ID).
 * @param ran_function_id   Target E3 RAN function ID.
 * @param dapp_id           Target dApp ID.
 * @param e3_payload        E3 control payload to forward.
 */
static void send_control_message(e2_node_arr_xapp_t nodes,
                                 int SM_ID,
                                 uint32_t ran_function_id,
                                 uint32_t dapp_id,
                                 const dapp_e3_ctrl_payload_t* e3_payload)
{
  dapp_ctrl_req_data_t ctrl = (dapp_ctrl_req_data_t){0};

  ctrl.hdr.format = FORMAT_1_E2SM_DAPP_CTRL_HDR;
  ctrl.hdr.frmt_1.ran_function_id = ran_function_id;
  ctrl.hdr.frmt_1.dapp_id = dapp_id;
  ctrl.hdr.frmt_1.timestamp_ns = time_now_ns();
  ctrl.hdr.frmt_1.sequence_id = next_sequence_id();

  ctrl.msg.format = FORMAT_1_E2SM_DAPP_CTRL_MSG;
  ctrl.msg.frmt_1.data = NULL;
  ctrl.msg.frmt_1.data_size = 0;

  ctrl.e3 = *e3_payload;

  /* Stamped before the call: control_sm_xapp_api() is synchronous, so a stamp
   * after it records the completed round trip, not the send. The key is
   * published for the library stages further down the same thread
   * (ENCODE_E2SM_BEGIN/DONE, then ENCODE_E2AP_BEGIN/DONE). */
  for (int i = 0; i < nodes.len; i++) {
    sm_ans_xapp_t ans = control_sm_xapp_api(&nodes.n[i].id, SM_ID, &ctrl);
    assert(ans.success == true);
  }

  free_dapp_ctrl_req_data(&ctrl);
}

/**
 * @brief Build and send a test Spectrum SM control message.
 *
 * Selects a predefined PRB blacklist variant (cycling through
 * ctrl_variants[]) and sends it to all E2 nodes as a spectrum
 * control payload.
 *
 * @param nodes        Connected E2 nodes.
 * @param variant_idx  Index into ctrl_variants (wraps with modulo).
 */
static void generate_control_message_spectrum(e2_node_arr_xapp_t nodes, unsigned variant_idx)
{
  const prb_list_t* v = &ctrl_variants[variant_idx % NUM_VARIANTS];

  spectrum_sm_control_t ctrl = (spectrum_sm_control_t){0};
  ctrl.timestamp_ns = time_now_ns();
  ctrl.prb_count = (long)v->count;

  if (ctrl.prb_count > 0) {
    ctrl.blockedPRBs = calloc(ctrl.prb_count, sizeof(uint16_t));
    assert(ctrl.blockedPRBs != NULL && "Failed to allocate PRB list");
    for (size_t i = 0; i < v->count; ++i)
      ctrl.blockedPRBs[i] = v->prbs[i];
  } else {
    ctrl.blockedPRBs = NULL;
  }

  printf("[DAPP RC]: Sending control variant %u — %ld blocked PRBs:", variant_idx % (unsigned)NUM_VARIANTS, ctrl.prb_count);
  for (long i = 0; i < ctrl.prb_count; ++i)
    printf(" %u", ctrl.blockedPRBs[i]);
  printf("\n");

  dapp_e3_ctrl_payload_t payload = (dapp_e3_ctrl_payload_t){0};
  payload.type = DAPP_E3_SM_SPECTRUM;
  payload.u.spectrum = ctrl;

  send_control_message(nodes, DAPP_SM_ID, E3_RAN_FUNC_ID_SPECTRUM, /*dapp_id=*/1, &payload);
}

/* ------------------------------------------------------------------ */
/* Subscription generation                                             */
/* ------------------------------------------------------------------ */

/**
 * @brief Generate a DAPP subscription with the given report style.
 *
 * Builds a dapp_sub_data_t containing:
 *   - Event trigger: Format 1 (common to all styles)
 *   - Action definition: Format 1 with the requested ric_style_type
 *
 * The style type determines what indications the agent will send:
 *   1 = E3 data only
 *   2 = subscription map only
 *
 * @param ran_func    The RAN function definition.
 * @param style_type  Report style type (1 or 2).
 * @return            Populated subscription data (caller must free via
 *                    free_dapp_sub_data()).
 */
static dapp_sub_data_t gen_dapp_subs(e2sm_dapp_func_def_t const* ran_func, uint32_t style_type)
{
  assert(ran_func != NULL);
  dapp_sub_data_t dapp_sub = {0};

  dapp_sub.et.format = FORMAT_1_E2SM_DAPP_EV_TRIGGER_FORMAT;
  dapp_sub.et.frmt_1 = (e2sm_dapp_ev_trg_frmt_1_t){};

  dapp_sub.action_def = calloc(1, sizeof(e2sm_dapp_action_def_t));
  assert(dapp_sub.action_def != NULL);
  dapp_sub.action_def->format = FORMAT_1_E2SM_DAPP_ACTION_DEF;
  dapp_sub.action_def->ric_style_type = style_type;
  dapp_sub.action_def->frmt_1 = (e2sm_dapp_action_def_frmt_1_t){};

  return dapp_sub;
}

/**
 * @brief Find the index of a RAN function by SM ID in a RAN function array.
 *
 * @param rf  Array of RAN functions.
 * @param sz  Size of the array.
 * @param f   Predicate function: returns true if the element matches.
 * @param id  The SM ID to search for.
 * @return    Index of the matching element. Asserts if not found.
 */
static size_t find_sm_idx(sm_ran_function_t* rf, size_t sz, bool (*f)(sm_ran_function_t const*, int const), int const id)
{
  for (size_t i = 0; i < sz; i++) {
    if (f(&rf[i], id))
      return i;
  }

  assert(0 != 0 && "SM ID could not be found in the RAN Function List");
}

/**
 * @brief Predicate: returns true if elem->id matches the given SM ID.
 */
static bool eq_sm(sm_ran_function_t const* elem, int const id)
{
  if (elem->id == id)
    return true;

  return false;
}

/**
 * @brief Print dApp E3 subscription list with configurable indentation.
 *
 * Prints which dApps are subscribed and which E3 RAN functions each
 * is registered for. If the list is NULL or empty, prints "none".
 *
 * @param subs    Pointer to the subscription list (may be NULL).
 * @param indent  Prefix string for each output line (e.g. "      ").
 */
static void print_dapp_e3_subs(const dapp_e3_subscription_list_t* subs, const char* indent)
{
  if (subs == NULL || subs->sz_dapp_e3_subscriptions == 0 || subs->dapp_e3_subscriptions == NULL) {
    printf("%sdApp E3 subscriptions: none\n", indent);
    return;
  }

  printf("%sdApp E3 subscriptions (%zu):\n", indent, subs->sz_dapp_e3_subscriptions);
  for (size_t i = 0; i < subs->sz_dapp_e3_subscriptions; ++i) {
    const dapp_e3_subscription_item_t* sub = &subs->dapp_e3_subscriptions[i];
    printf("%s  - dApp %u: subscribed to %zu E3 RAN function(s):", indent, sub->dapp_id, sub->sz_subscribed_e3_ran_functions);
    for (size_t j = 0; j < sub->sz_subscribed_e3_ran_functions; ++j) {
      printf(" %u", sub->subscribed_e3_ran_functions[j]);
    }
    printf("\n");
  }
}

/**
 * @brief Print detailed information about a DAPP RAN function definition.
 *
 * Displays SM ID, name, description, OID, event trigger presence,
 * all report styles (with per-style dApp E3 subscriptions), and
 * all control styles (with per-style dApp E3 subscriptions).
 *
 * @param idx  Index of the RAN function in the node's RF array.
 * @param n    The E2 node whose RAN function to print.
 */
static void print_dapp_sm_info(size_t idx, e2_node_connected_xapp_t const* n)
{
  assert(n != NULL);
  assert(idx < n->len_rf);

  const sm_ran_function_t* rf = &n->rf[idx];

  assert(rf->defn.type == DAPP_RAN_FUNC_DEF_E);

  const e2sm_dapp_func_def_t* d = &rf->defn.dapp;

  printf("[DAPP xApp] RAN function @idx=%zu\n", idx);
  printf("  SM ID: %d\n", rf->id);

  if (d->name.name.buf && d->name.name.len > 0) {
    printf("  Name: %.*s\n", (int)d->name.name.len, (const char*)d->name.name.buf);
  }

  if (d->name.description.buf && d->name.description.len > 0) {
    printf("  Description: %.*s\n", (int)d->name.description.len, (const char*)d->name.description.buf);
  }

  if (d->name.oid.buf && d->name.oid.len > 0) {
    printf("  OID: %.*s\n", (int)d->name.oid.len, (const char*)d->name.oid.buf);
  }

  if (d->ev_trig == NULL) {
    printf("  Event trigger: none\n");
  } else {
    printf("  Event trigger: present (details omitted)\n");
  }

  if (d->report != NULL && d->report->sz_seq_report_sty > 0 && d->report->seq_report_sty != NULL) {
    printf("  Report styles (%zu):\n", d->report->sz_seq_report_sty);

    for (size_t i = 0; i < d->report->sz_seq_report_sty; ++i) {
      const seq_report_sty_dapp_sm_t* s = &d->report->seq_report_sty[i];

      printf(
          "    - report_type=%u, name=%.*s, "
          "ind_hdr_type=%u, ind_msg_type=%u\n",
          s->report_type,
          (int)s->name.len,
          (const char*)s->name.buf,
          s->ind_hdr_type,
          s->ind_msg_type);

      print_dapp_e3_subs(s->dapp_e3_subs, "      ");
    }
  } else {
    printf("  Report styles: none\n");
  }

  if (d->ctrl != NULL && d->ctrl->sz_seq_ctrl_style > 0 && d->ctrl->seq_ctrl_style != NULL) {
    printf("  Control styles (%zu):\n", d->ctrl->sz_seq_ctrl_style);

    for (size_t i = 0; i < d->ctrl->sz_seq_ctrl_style; ++i) {
      const seq_ctrl_style_dapp_sm_t* s = &d->ctrl->seq_ctrl_style[i];

      printf(
          "    - style_type=%u, name=%.*s, "
          "hdr_type=%u, msg_type=%u, out_frmt_type=%u\n",
          s->style_type,
          (int)s->name.len,
          (const char*)s->name.buf,
          s->hdr,
          s->msg,
          s->out_frmt);

      print_dapp_e3_subs(s->dapp_e3_subs, "      ");
    }
  } else {
    printf("  Control styles: none\n");
  }
}

/* ------------------------------------------------------------------ */
/* Subscription management                                             */
/* ------------------------------------------------------------------ */

/**
 * @brief Subscribe to the DAPP SM on all connected E2 nodes.
 *
 * For each E2 node:
 *   - Locates the DAPP RAN function in the node's RF list.
 *   - Prints the RAN function definition for debugging.
 *   - Builds a subscription with the requested report style.
 *   - Installs the subscription via report_sm_xapp_api().
 *   - Stores the subscription handle in hndl[i].
 *
 * @param nodes       Connected E2 nodes.
 * @param hndl        Output array for subscription handles (one per node).
 * @param style_type  Report style (1 = E3 data, 2 = sub map).
 * @param cb          Indication callback function.
 * @param label       Human-readable label for log messages.
 */
static void subscribe_sm(e2_node_arr_xapp_t nodes, sm_ans_xapp_t* hndl, uint32_t style_type, sm_cb cb, const char* label)
{
  for (size_t i = 0; i < nodes.len; ++i) {
    e2_node_connected_xapp_t* n = &nodes.n[i];

    size_t const idx = find_sm_idx(n->rf, n->len_rf, eq_sm, DAPP_SM_ID);
    assert(n->rf[idx].defn.type == DAPP_RAN_FUNC_DEF_E && "DAPP is not the received RAN Function");

    if (n->rf[idx].defn.dapp.name.name.buf != NULL) {
      print_dapp_sm_info(idx, n);

      dapp_sub_data_t dapp_sub = gen_dapp_subs(&n->rf[idx].defn.dapp, style_type);

      printf("[DAPP RC]: Installing %s (style %u) subscription on E2 node %zu\n", label, style_type, i);
      hndl[i] = report_sm_xapp_api(&n->id, DAPP_SM_ID, &dapp_sub, cb);
      assert(hndl[i].success == true);

      free_dapp_sub_data(&dapp_sub);
    }
  }
}

/**
 * @brief Remove DAPP SM subscriptions from all E2 nodes.
 *
 * Iterates over the handle array and calls rm_report_sm_xapp_api()
 * for each successful subscription.
 *
 * @param hndl   Array of subscription handles.
 * @param nodes  Connected E2 nodes (used for array length).
 */
static void delete_subscriptions(sm_ans_xapp_t* hndl, e2_node_arr_xapp_t nodes)
{
  for (int i = 0; i < nodes.len; ++i) {
    if (hndl[i].success == true) {
      rm_report_sm_xapp_api(hndl[i].u.handle);
    }
  }
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

/**
 * @brief Entry point for the DAPP xApp.
 *
 * Workflow:
 *   1. Parse command-line args and initialize the xApp API.
 *   2. Set up a dedicated signal-handling thread for clean shutdown.
 *   3. Retrieve connected E2 nodes.
 *   4. Subscribe to the DAPP SM twice (E3 data + subscription map).
 *   5. Enter a periodic loop (indications are handled asynchronously
 *      by the registered callbacks).
 *   6. On SIGINT/SIGTERM or timeout: unsubscribe both, stop the API,
 *      and exit.
 */
int main(int argc, char* argv[])
{
  fr_args_t args = init_fr_args(argc, argv);

  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTERM);

  int rc = pthread_sigmask(SIG_BLOCK, &set, NULL);
  assert(rc == 0);

  pthread_t sig_thr;
  rc = pthread_create(&sig_thr, NULL, signal_thread, &set);
  assert(rc == 0);

  init_xapp_api(&args);
  sleep(1);

  e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
  assert(nodes.len > 0);

  printf("[DAPP RC]: Connected E2 nodes = %d\n", nodes.len);

  /* Allocate two independent handle arrays — one per subscription */
  sm_ans_xapp_t* hndl_e3_data = calloc(nodes.len, sizeof(sm_ans_xapp_t));
  assert(hndl_e3_data != NULL);

  sm_ans_xapp_t* hndl_sub_map = calloc(nodes.len, sizeof(sm_ans_xapp_t));
  assert(hndl_sub_map != NULL);

  /*
   * Two subscriptions per E2 node, each with its own ric_req_id:
   *
   *   Subscription A — Style 1 (E3 data only):
   *     Receives Format 1 indications (E3 data reports).
   *     Callback: sm_cb_dapp_frmt1
   *
   *   Subscription B — Style 2 (subscription map only):
   *     Receives Format 2 indications (dApp E3 subscription map).
   *     Callback: sm_cb_dapp_frmt2
   *
   * Both subscriptions target the same ran_func_id (DAPP_SM_ID = 255)
   * but receive distinct ric_req_ids, so the xApp active-procedure
   * registry and the agent bimap track them independently.
   */
  subscribe_sm(nodes, hndl_e3_data, DAPP_STYLE_E3_DATA, sm_cb_dapp_frmt1, "DAPP (E3 data)");
  subscribe_sm(nodes, hndl_sub_map, DAPP_STYLE_SUBSCRIPTION_MAP, sm_cb_dapp_frmt2, "DAPP (subscription map)");

  /* ---- Periodic loop  ---- */
  const unsigned ctrl_period_sec = 5;
  const unsigned max_runtime_sec = 600;
  unsigned elapsed = 0;
  unsigned variant = 0;

  while (!g_terminate && elapsed < max_runtime_sec) {
    generate_control_message_spectrum(nodes, variant);
    variant++;

    for (unsigned s = 0; s < ctrl_period_sec && !g_terminate; ++s) {
      sleep(1);
      ++elapsed;
    }
  }

  if (g_terminate) {
    printf("[DAPP RC]: Termination signal received, shutting down...\n");
  } else {
    printf("[DAPP RC]: Max runtime reached, shutting down...\n");
  }

  /* Delete both subscriptions independently */

  delete_subscriptions(hndl_e3_data, nodes);
  delete_subscriptions(hndl_sub_map, nodes);

  free(hndl_e3_data);
  free(hndl_sub_map);

  sleep(1);

  while (try_stop_xapp_api() == false)
    usleep(1000);

  free_e2_node_arr_xapp(&nodes);

  printf("[DAPP RC]: Test xApp run SUCCESSFULLY\n");
}

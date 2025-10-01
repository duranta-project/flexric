/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BAS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/util/alg_ds/alg/defer.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/util/alg_ds/ds/lock_guard/lock_guard.h"
#include "../../../../src/util/e.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

static
uint64_t const period_ms = 1000;

static
pthread_mutex_t mtx;

static
void log_gnb_ue_id(ue_id_e2sm_t ue_id)
{
  if (ue_id.gnb.gnb_cu_ue_f1ap_lst != NULL) {
    for (size_t i = 0; i < ue_id.gnb.gnb_cu_ue_f1ap_lst_len; i++) {
      printf("UE ID type = gNB-CU, gnb_cu_ue_f1ap = %u\n", ue_id.gnb.gnb_cu_ue_f1ap_lst[i]);
    }
  } else {
    printf("UE ID type = gNB, amf_ue_ngap_id = %lu\n", ue_id.gnb.amf_ue_ngap_id);
  }
  if (ue_id.gnb.ran_ue_id != NULL) {
    printf("ran_ue_id = %lx\n", *ue_id.gnb.ran_ue_id); // RAN UE NGAP ID
  }
}

static
void log_du_ue_id(ue_id_e2sm_t ue_id)
{
  printf("UE ID type = gNB-DU, gnb_cu_ue_f1ap = %u\n", ue_id.gnb_du.gnb_cu_ue_f1ap);
  if (ue_id.gnb_du.ran_ue_id != NULL) {
    printf("ran_ue_id = %lx\n", *ue_id.gnb_du.ran_ue_id); // RAN UE NGAP ID
  }
}

static
void log_cuup_ue_id(ue_id_e2sm_t ue_id)
{
  printf("UE ID type = gNB-CU-UP, gnb_cu_cp_ue_e1ap = %u\n", ue_id.gnb_cu_up.gnb_cu_cp_ue_e1ap);
  if (ue_id.gnb_cu_up.ran_ue_id != NULL) {
    printf("ran_ue_id = %lx\n", *ue_id.gnb_cu_up.ran_ue_id); // RAN UE NGAP ID
  }
}

typedef void (*log_ue_id)(ue_id_e2sm_t ue_id);

static
log_ue_id log_ue_id_e2sm[END_UE_ID_E2SM] = {
    log_gnb_ue_id, // common for gNB-mono, CU and CU-CP
    log_du_ue_id,
    log_cuup_ue_id,
    NULL,
    NULL,
    NULL,
    NULL,
};

static
void log_int_value(byte_array_t name, meas_record_lst_t meas_record)
{
  char *name_str = cp_ba_to_str(name);
  printf("%s = %d\n", name_str, meas_record.int_val);
  free(name_str);
}

static
void log_real_value(byte_array_t name, meas_record_lst_t meas_record)
{
  char *name_str = cp_ba_to_str(name);
  printf("%s = %.2f\n", name_str, meas_record.real_val);
  free(name_str);
}

typedef void (*log_meas_value)(byte_array_t name, meas_record_lst_t meas_record);

static
log_meas_value get_meas_value[END_MEAS_VALUE] = {
    log_int_value,
    log_real_value,
    NULL,
};

static
void match_meas_name_type(meas_type_t meas_type, meas_record_lst_t meas_record)
{
  // Get the value of the Measurement
  get_meas_value[meas_record.value](meas_type.name, meas_record);
}

static
void match_id_meas_type(meas_type_t meas_type, meas_record_lst_t meas_record)
{
  (void)meas_type;
  (void)meas_record;
  assert(false && "ID Measurement Type not yet supported");
}

typedef void (*check_meas_type)(meas_type_t meas_type, meas_record_lst_t meas_record);

static
check_meas_type match_meas_type[END_MEAS_TYPE] = {
    match_meas_name_type,
    match_id_meas_type,
};

static
void log_ind_frm_2(const kpm_ind_msg_format_2_t *frm_2)
{
  assert(frm_2->meas_data_lst_len > 0 && frm_2->meas_info_cond_ue_lst_len > 0 && "Cannot correctly print measurements");

  // [1, 65535]
  for (size_t i = 0; i < frm_2->meas_data_lst_len; i++) {
    const meas_data_lst_t *data_item = &frm_2->meas_data_lst[i];
    assert((data_item->meas_record_len % frm_2->meas_info_cond_ue_lst_len) == 0 && "Expected number of parameters to be multiplication of number of UEs");
    const size_t sz_ue_cond = frm_2->meas_info_cond_ue_lst_len;

    for (size_t z = 0; z < sz_ue_cond; z++) {
      const meas_info_cond_ue_lst_t *ue_info = &frm_2->meas_info_cond_ue_lst[z];

      // [1, 32768]
      for (size_t y = 0; y < ue_info->matching_cond_lst_len; y++) {
        if (ue_info->matching_cond_lst[y].cond_type != LABEL_INFO) {
          printf("Expected Measurement Label information\n");
          continue;
        }
      }

      // print UE ID
      // [0, 65535]
      for (size_t y = 0; y < ue_info->ue_id_matched_lst_len; y++) {
        const ue_id_e2sm_t ue_id_e2sm = ue_info->ue_id_matched_lst[y];
        log_ue_id_e2sm[ue_id_e2sm.type](ue_id_e2sm);

        // print parameter names and values
        const meas_type_t meas_type = ue_info->meas_type;
	size_t j = z*ue_info->ue_id_matched_lst_len + y;
        const meas_record_lst_t record_item = data_item->meas_record_lst[j];
        match_meas_type[meas_type.type](meas_type, record_item);

        if (data_item->incomplete_flag && *data_item->incomplete_flag == TRUE_ENUM_VALUE)
          printf("Measurement Record not reliable");
      }
    }
  }
}

static
void sm_cb_kpm(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == KPM_STATS_V3_0);

  kpm_ind_data_t const* ind = &rd->ind.kpm.ind;
  //kpm_ric_ind_hdr_format_1_t const* hdr_frm_1 = &ind->hdr.kpm_ric_ind_hdr_format_1;

  //int64_t const now = time_now_us();
  static int counter = 1;
  {
    lock_guard(&mtx);

    printf("\n%7d KPM IND MSG\n", counter);

    if (ind->msg.type == FORMAT_2_INDICATION_MESSAGE)
      log_ind_frm_2(&ind->msg.frm_2);

    counter++;
  }
}

static
kpm_act_def_t fill_report_style_3(global_e2_node_id_t const* global_e2_node, ric_report_style_item_t const* report_item)
{
  assert(report_item != NULL);
  assert(report_item->act_def_format_type == FORMAT_3_ACTION_DEFINITION);

  kpm_act_def_t act_def = {.type = FORMAT_3_ACTION_DEFINITION};

  size_t const sz = report_item->meas_info_for_action_lst_len;

  // [1, 65535]
  act_def.frm_3.meas_info_lst_len = sz;
  act_def.frm_3.meas_info_lst = calloc(sz, sizeof(meas_info_format_3_lst_t));
  assert(act_def.frm_3.meas_info_lst != NULL && "Memory exhausted");

  for (size_t i = 0; i < sz; i++) {
    meas_info_format_3_lst_t* meas_item = &act_def.frm_3.meas_info_lst[i];
    // 8.3.9
    // Measurement Name
    meas_item->meas_type.type = NAME_MEAS_TYPE;
    meas_item->meas_type.name = copy_byte_array(report_item->meas_info_for_action_lst[i].name);

    // [1, 32768]
    meas_item->matching_cond_lst_len = 1;
    meas_item->matching_cond_lst = calloc(meas_item->matching_cond_lst_len, sizeof(matching_condition_format_3_lst_t));
    assert(meas_item->matching_cond_lst != NULL && "Memory exhausted");
    meas_item->matching_cond_lst[0].cond_type = LABEL_INFO;
    meas_item->matching_cond_lst[0].label_info_lst.noLabel = ecalloc(1, sizeof(enum_value_e));
    *meas_item->matching_cond_lst[0].label_info_lst.noLabel = TRUE_ENUM_VALUE;

    // 8.3.26  -  OPTIONAL
    meas_item->bin_range_def = NULL;
  }

  // 8.3.8 [0, 4294967295]
  act_def.frm_3.gran_period_ms = period_ms;

  // 8.3.20 - OPTIONAL
  act_def.frm_3.cell_global_id = calloc(1, sizeof(cell_global_id_t));
  cell_global_id_t* cell_id = &act_def.frm_3.cell_global_id[0];
  cell_id->type = NR_CGI_RAT_TYPE;
  cell_id->nr_cgi.nr_cell_id = 0x100563412;
  cell_id->nr_cgi.plmn_id.mcc = global_e2_node->plmn.mcc;
  cell_id->nr_cgi.plmn_id.mnc = global_e2_node->plmn.mnc;
  cell_id->nr_cgi.plmn_id.mnc_digit_len = global_e2_node->plmn.mnc_digit_len;

  return act_def;
}

typedef kpm_act_def_t (*fill_kpm_act_def)(global_e2_node_id_t const* global_e2_node, ric_report_style_item_t const* report_item);

static
fill_kpm_act_def get_kpm_act_def[END_RIC_SERVICE_REPORT] = {
    NULL,
    NULL,
    fill_report_style_3,
    NULL,
    NULL,
};

static
kpm_sub_data_t gen_kpm_subs(global_e2_node_id_t const* global_e2_node, kpm_ran_function_def_t const* ran_func)
{
  assert(ran_func != NULL);
  assert(ran_func->ric_event_trigger_style_list != NULL);

  kpm_sub_data_t kpm_sub = {0};

  // Generate Event Trigger
  assert(ran_func->ric_event_trigger_style_list[0].format_type == FORMAT_1_RIC_EVENT_TRIGGER);
  kpm_sub.ev_trg_def.type = FORMAT_1_RIC_EVENT_TRIGGER;
  kpm_sub.ev_trg_def.kpm_ric_event_trigger_format_1.report_period_ms = period_ms;

  // Generate Action Definition
  kpm_sub.sz_ad = 1;
  kpm_sub.ad = calloc(kpm_sub.sz_ad, sizeof(kpm_act_def_t));
  assert(kpm_sub.ad != NULL && "Memory exhausted");

  // Multiple Action Definitions in one SUBSCRIPTION message is not supported in this project
  // Multiple REPORT Styles = Multiple Action Definition = Multiple SUBSCRIPTION messages
  for (size_t i = 0; i < ran_func->sz_ric_report_style_list; i++) {
    ric_report_style_item_t* const report_item = &ran_func->ric_report_style_list[i];
    if (report_item->report_style_type == STYLE_3_RIC_SERVICE_REPORT)
      *kpm_sub.ad = get_kpm_act_def[STYLE_3_RIC_SERVICE_REPORT](global_e2_node, report_item);
  }

  return kpm_sub;
}

static
bool eq_sm(sm_ran_function_t const* elem, int const id)
{
  if (elem->id == id)
    return true;

  return false;
}

static
size_t find_sm_idx(sm_ran_function_t* rf, size_t sz, bool (*f)(sm_ran_function_t const*, int const), int const id)
{
  for (size_t i = 0; i < sz; i++) {
    if (f(&rf[i], id))
      return i;
  }

  assert(0 != 0 && "SM ID could not be found in the RAN Function List");
}

int main(int argc, char* argv[])
{
  fr_args_t args = init_fr_args(argc, argv);

  // Init the xApp
  init_xapp_api(&args);
  sleep(1);

  e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
  defer({ free_e2_node_arr_xapp(&nodes); });

  assert(nodes.len > 0);

  printf("Connected E2 nodes = %d\n", nodes.len);

  pthread_mutexattr_t attr = {0};
  int rc = pthread_mutex_init(&mtx, &attr);
  assert(rc == 0);

  sm_ans_xapp_t* hndl = calloc(nodes.len, sizeof(sm_ans_xapp_t));
  assert(hndl != NULL);

  ////////////
  // START KPM
  ////////////
  int const KPM_ran_function = 2;

  for (size_t i = 0; i < nodes.len; ++i) {
    e2_node_connected_xapp_t* n = &nodes.n[i];

    size_t const idx = find_sm_idx(n->rf, n->len_rf, eq_sm, KPM_ran_function);
    assert(n->rf[idx].defn.type == KPM_RAN_FUNC_DEF_E && "KPM is not the received RAN Function");
    // if REPORT Service is supported by E2 node, send SUBSCRIPTION
    // e.g. OAI CU-CP
    if (n->rf[idx].defn.kpm.ric_report_style_list != NULL) {
      // Generate KPM SUBSCRIPTION message
      kpm_sub_data_t kpm_sub = gen_kpm_subs(&n->id, &n->rf[idx].defn.kpm);

      hndl[i] = report_sm_xapp_api(&n->id, KPM_ran_function, &kpm_sub, sm_cb_kpm);
      assert(hndl[i].success == true);

      free_kpm_sub_data(&kpm_sub);
    }
  }
  ////////////
  // END KPM
  ////////////

  xapp_wait_end_api();

  for (int i = 0; i < nodes.len; ++i) {
    // Remove the handle previously returned
    if (hndl[i].success == true)
      rm_report_sm_xapp_api(hndl[i].u.handle);
  }
  free(hndl);

  // Stop the xApp
  while (try_stop_xapp_api() == false)
    usleep(1000);

  printf("Test xApp run SUCCESSFULLY\n");
}


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
#include "../../../../src/sm/rc_sm/rc_sm_id.h"
#include "../../../../src/sm/rc_sm/ie/ir/ran_param_list.h"
#include "../../../../src/sm/rc_sm/ie/ir/ran_param_struct.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

static
pthread_mutex_t mtx;

typedef struct {
  ue_id_e2sm_t ue_id;
  uint8_t sst;
} ue_slice_pair_t;

typedef struct {
  size_t len;
  ue_slice_pair_t ue_slice_pair[16];
} ue_slice_pair_lst_t;

static
ue_slice_pair_lst_t ue_slice_pair_lst = {0};

////////////
// START KPM
////////////
static
uint64_t const period_ms = 1000;

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
      ue_slice_pair_lst.len = ue_info->ue_id_matched_lst_len;
      for (size_t y = 0; y < ue_info->ue_id_matched_lst_len; y++) {
        const ue_id_e2sm_t ue_id_e2sm = ue_info->ue_id_matched_lst[y];
        log_ue_id_e2sm[ue_id_e2sm.type](ue_id_e2sm);
	ue_slice_pair_lst.ue_slice_pair[y].ue_id = cp_ue_id_e2sm(&ue_id_e2sm);

        // print parameter names and values
        const meas_type_t meas_type = ue_info->meas_type;
	size_t j = z*ue_info->ue_id_matched_lst_len + y;
        const meas_record_lst_t record_item = data_item->meas_record_lst[j];
        match_meas_type[meas_type.type](meas_type, record_item);

	if (cmp_str_ba("Viavi.Slice.id", meas_type.name) == 0) {
	  printf("passes and int_val = %d\n", record_item.int_val);
	  ue_slice_pair_lst.ue_slice_pair[y].sst = record_item.int_val;
	}
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
////////////
// END KPM
////////////

////////////
// START RC
////////////
static
e2sm_rc_ctrl_hdr_frmt_1_t gen_rc_ctrl_hdr_1(const size_t pair_id)
{
  e2sm_rc_ctrl_hdr_frmt_1_t hdr = {0};

  // UE ID
  // Mandatory
  // 9.3.10
  // 6.2.2.6
  hdr.ue_id = cp_ue_id_e2sm(&ue_slice_pair_lst.ue_slice_pair[pair_id].ue_id);

  // RIC Style Type
  // Mandatory
  // 9.3.3
  // 6.2.2.2.
  // INTEGER
  hdr.ric_style_type = 2; // Radio Resource Allocation Control

  // Control Action ID
  // Mandatory
  // 9.3.6
  // [1- 65535]
  hdr.ctrl_act_id = 6; // Slice-level PRB quota

  // RIC Control decision
  // Optional
  hdr.ric_ctrl_decision = NULL;

  return hdr;
}

typedef enum {
  RRM_Policy_Ratio_List_8_4_3_6 = 1,
  RRM_Policy_Ratio_Group_8_4_3_6 = 2,
  RRM_Policy_8_4_3_6 = 3,
  RRM_Policy_Member_List_8_4_3_6 = 5,
  RRM_Policy_Member_8_4_3_6 = 6,
  PLMN_Identity_8_4_3_6 = 7,
  S_NSSAI_8_4_3_6 = 8,
  SST_8_4_3_6 = 9,
  SD_8_4_3_6 = 10,
  Min_PRB_Policy_Ratio_8_4_3_6 = 11,
  Max_PRB_Policy_Ratio_8_4_3_6 = 12,
  Dedicated_PRB_Policy_Ratio_8_4_3_6 = 13,
} slice_level_PRB_quota_param_id_e;

static
ran_param_struct_t* fill_snssai(const size_t pair_id)
{
  ran_param_struct_t* nssai = ecalloc(1, sizeof(ran_param_struct_t));

  nssai->sz_ran_param_struct = 1; // 2;
  nssai->ran_param_struct = ecalloc(nssai->sz_ran_param_struct, sizeof(seq_ran_param_t));

  // filling only sST, and sD remains NULL
  printf("Hardcoding sST. Once the fix is resolved in RSG, KPM and RC can be merged in one xApp, and no more hardcoding.\n");
  nssai->ran_param_struct[0].ran_param_id = SST_8_4_3_6;
  nssai->ran_param_struct[0].ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  nssai->ran_param_struct[0].ran_param_val.flag_false = ecalloc(1, sizeof(ran_parameter_value_t));
  nssai->ran_param_struct[0].ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  const size_t sst_len = 1;
  nssai->ran_param_struct[0].ran_param_val.flag_false->octet_str_ran.len = sst_len;
  nssai->ran_param_struct[0].ran_param_val.flag_false->octet_str_ran.buf = ecalloc(sst_len, sizeof(uint8_t));
  nssai->ran_param_struct[0].ran_param_val.flag_false->octet_str_ran.buf[0] = ue_slice_pair_lst.ue_slice_pair[pair_id].sst;

/*  nssai->ran_param_struct[1].ran_param_id = SD_8_4_3_6;
  nssai->ran_param_struct[1].ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  nssai->ran_param_struct[1].ran_param_val.flag_false = ecalloc(1, sizeof(ran_parameter_value_t));
  nssai->ran_param_struct[1].ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  const size_t sd_len = 3;
  nssai->ran_param_struct[1].ran_param_val.flag_false->octet_str_ran.len = sd_len;
  nssai->ran_param_struct[1].ran_param_val.flag_false->octet_str_ran.buf = ecalloc(sd_len, sizeof(uint8_t));
  nssai->ran_param_struct[1].ran_param_val.flag_false->octet_str_ran.buf[0] = 1;
  nssai->ran_param_struct[1].ran_param_val.flag_false->octet_str_ran.buf[1] = 0;
  nssai->ran_param_struct[1].ran_param_val.flag_false->octet_str_ran.buf[2] = 0;
*/
  return nssai;
}

static
byte_array_t enc_plmn_id(const e2ap_plmn_t *plmn_id)
{
  byte_array_t ba = {.len = 3};
  ba.buf = ecalloc(ba.len, sizeof(uint8_t));
  ba.buf[0] = ((((plmn_id->mcc) / 10) % 10) << 4) | ((plmn_id->mcc) / 100);
  ba.buf[1] = ((plmn_id->mnc_digit_len == 2 ? 15 : (plmn_id->mnc) / 100) << 4) | ((plmn_id->mcc) % 10);
  ba.buf[2] = (((plmn_id->mnc) % 10) << 4) | (((plmn_id->mnc) / 10) % 10);

  return ba;
}

static
ran_param_struct_t* fill_rrm_policy_member_list_params(const e2ap_plmn_t *plmn_id, const size_t pair_id)
{
  ran_param_struct_t *strct = ecalloc(1, sizeof(ran_param_struct_t));

  strct->sz_ran_param_struct = 2;
  strct->ran_param_struct = ecalloc(strct->sz_ran_param_struct, sizeof(seq_ran_param_t));

  strct->ran_param_struct[0].ran_param_id = PLMN_Identity_8_4_3_6;
  strct->ran_param_struct[0].ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  strct->ran_param_struct[0].ran_param_val.flag_false = ecalloc(1, sizeof(ran_parameter_value_t));
  strct->ran_param_struct[0].ran_param_val.flag_false[0].type = OCTET_STRING_RAN_PARAMETER_VALUE;
  strct->ran_param_struct[0].ran_param_val.flag_false[0].octet_str_ran = enc_plmn_id(plmn_id);

  strct->ran_param_struct[1].ran_param_id = S_NSSAI_8_4_3_6;
  strct->ran_param_struct[1].ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  strct->ran_param_struct[1].ran_param_val.strct = fill_snssai(pair_id);

  return strct;
}

static
lst_ran_param_t* fill_rrm_policy_member_list(const e2ap_plmn_t *plmn_id, const size_t pair_id)
{
  lst_ran_param_t* lst = ecalloc(1, sizeof(lst_ran_param_t));

  lst->ran_param_struct.sz_ran_param_struct = 1;
  lst->ran_param_struct.ran_param_struct = ecalloc(lst->ran_param_struct.sz_ran_param_struct, sizeof(seq_ran_param_t));

  lst->ran_param_struct.ran_param_struct[0].ran_param_id = RRM_Policy_Member_8_4_3_6;
  lst->ran_param_struct.ran_param_struct[0].ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  lst->ran_param_struct.ran_param_struct[0].ran_param_val.strct = fill_rrm_policy_member_list_params(plmn_id, pair_id);

  return lst;
}

static
ran_param_struct_t* fill_rrm_policy(const e2ap_plmn_t *plmn_id, const size_t pair_id)
{
  ran_param_struct_t *rrm_policy = ecalloc(1, sizeof(ran_param_struct_t));

  rrm_policy->sz_ran_param_struct = 1;
  rrm_policy->ran_param_struct = ecalloc(rrm_policy->sz_ran_param_struct, sizeof(seq_ran_param_t));
  rrm_policy->ran_param_struct[0].ran_param_id = RRM_Policy_Member_List_8_4_3_6;
  rrm_policy->ran_param_struct[0].ran_param_val.type = LIST_RAN_PARAMETER_VAL_TYPE;
  rrm_policy->ran_param_struct[0].ran_param_val.lst = ecalloc(1, sizeof(ran_param_list_t));
  rrm_policy->ran_param_struct[0].ran_param_val.lst[0].sz_lst_ran_param = 1;
  rrm_policy->ran_param_struct[0].ran_param_val.lst[0].lst_ran_param = fill_rrm_policy_member_list(plmn_id, pair_id);

  return rrm_policy;
}

static
int get_prb_ratio_per_slice(const size_t pair_id)
{
  const uint8_t sst = ue_slice_pair_lst.ue_slice_pair[pair_id].sst;
  if (sst == 1) {
    return 5;
  } else if (sst == 2) {
    return 20;
  } else if (sst == 3) {
    return 75;
  }
  return (100 / ue_slice_pair_lst.len);
}

static
ran_param_struct_t* fill_rrm_ratio_policy_group_params(const e2ap_plmn_t *plmn_id, const size_t pair_id)
{
  ran_param_struct_t *strct = ecalloc(1, sizeof(ran_param_struct_t));

  strct->sz_ran_param_struct = 4;
  strct->ran_param_struct = ecalloc(strct->sz_ran_param_struct, sizeof(seq_ran_param_t));

  strct->ran_param_struct[0].ran_param_id = RRM_Policy_8_4_3_6;
  strct->ran_param_struct[0].ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  strct->ran_param_struct[0].ran_param_val.strct = fill_rrm_policy(plmn_id, pair_id);

  const int prb_ratio = get_prb_ratio_per_slice(pair_id);

  strct->ran_param_struct[1].ran_param_id = Min_PRB_Policy_Ratio_8_4_3_6;
  strct->ran_param_struct[1].ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  strct->ran_param_struct[1].ran_param_val.flag_false = ecalloc(1, sizeof(ran_parameter_value_t));
  strct->ran_param_struct[1].ran_param_val.flag_false[0].type = INTEGER_RAN_PARAMETER_VALUE;
  strct->ran_param_struct[1].ran_param_val.flag_false[0].int_ran = prb_ratio;

  strct->ran_param_struct[2].ran_param_id = Max_PRB_Policy_Ratio_8_4_3_6;
  strct->ran_param_struct[2].ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  strct->ran_param_struct[2].ran_param_val.flag_false = ecalloc(1, sizeof(ran_parameter_value_t));
  strct->ran_param_struct[2].ran_param_val.flag_false[0].type = INTEGER_RAN_PARAMETER_VALUE;
  strct->ran_param_struct[2].ran_param_val.flag_false[0].int_ran = prb_ratio;

  strct->ran_param_struct[3].ran_param_id = Dedicated_PRB_Policy_Ratio_8_4_3_6;
  strct->ran_param_struct[3].ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  strct->ran_param_struct[3].ran_param_val.flag_false = ecalloc(1, sizeof(ran_parameter_value_t));
  strct->ran_param_struct[3].ran_param_val.flag_false[0].type = INTEGER_RAN_PARAMETER_VALUE;
  strct->ran_param_struct[3].ran_param_val.flag_false[0].int_ran = prb_ratio;

  return strct;
}

static
lst_ran_param_t* fill_rrm_ratio_policy_group(const e2ap_plmn_t *plmn_id, const size_t pair_id)
{
  lst_ran_param_t *lst = ecalloc(1, sizeof(lst_ran_param_t));

  lst->ran_param_struct.sz_ran_param_struct = 1;
  lst->ran_param_struct.ran_param_struct = ecalloc(lst->ran_param_struct.sz_ran_param_struct, sizeof(seq_ran_param_t));

  lst->ran_param_struct.ran_param_struct[0].ran_param_id = RRM_Policy_Ratio_Group_8_4_3_6;
  lst->ran_param_struct.ran_param_struct[0].ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  lst->ran_param_struct.ran_param_struct[0].ran_param_val.strct = fill_rrm_ratio_policy_group_params(plmn_id, pair_id);

  return lst;
}

static
e2sm_rc_ctrl_msg_frmt_1_t gen_rc_ctrl_msg_1(const e2ap_plmn_t *plmn_id, const size_t pair_id)
{
  e2sm_rc_ctrl_msg_frmt_1_t msg = {0};

  // List of RAN parameters
  // [0- 65535]
  msg.sz_ran_param = 1;
  msg.ran_param = ecalloc(msg.sz_ran_param, sizeof(seq_ran_param_t));
  msg.ran_param[0].ran_param_id = RRM_Policy_Ratio_List_8_4_3_6;
  msg.ran_param[0].ran_param_val.type = LIST_RAN_PARAMETER_VAL_TYPE;
  msg.ran_param[0].ran_param_val.lst = ecalloc(1, sizeof(ran_param_list_t));
  msg.ran_param[0].ran_param_val.lst[0].sz_lst_ran_param = 1;
  msg.ran_param[0].ran_param_val.lst[0].lst_ran_param = fill_rrm_ratio_policy_group(plmn_id, pair_id);

  return msg;
}

static
rc_ctrl_req_data_t gen_rc_ctrl_style_2(const seq_ctrl_style_t *ctrl_style, const e2ap_plmn_t *plmn_id, const size_t pair_id)
{
  rc_ctrl_req_data_t rc_ctrl = {0};

  // RIC Control Style Name
  // Mandatory
  // 9.3.4
  // [1 -150]
  assert(cmp_str_ba("Radio Resource Allocation Control", ctrl_style->name) == 0 && "Unexpected name for Style Type 2.\n");

  // RIC Control Header Format Type
  // Mandatory
  // 9.3.5
  assert(ctrl_style->hdr == 1 && "Unexpected control header format.\n");

  // RIC Control Message Format Type
  // Mandatory
  // 9.3.5
  assert(ctrl_style->msg == 1 && "Unexpected control message format.\n");

  // Sequence of Control Actions
  // [0-65535]
  for (size_t i = 0; i < ctrl_style->sz_seq_ctrl_act; i++) {
    if (ctrl_style->seq_ctrl_act[i].id == 6) {
      rc_ctrl.hdr.format = FORMAT_1_E2SM_RC_CTRL_HDR,
      rc_ctrl.hdr.frmt_1 = gen_rc_ctrl_hdr_1(pair_id),
      rc_ctrl.msg.format = FORMAT_1_E2SM_RC_CTRL_MSG,
      rc_ctrl.msg.frmt_1 = gen_rc_ctrl_msg_1(plmn_id, pair_id);
      return rc_ctrl;
    }
  }
  assert(false && "E2 node does not support Control Action ID 6 (Slice-level PRB quota).\n");

  return rc_ctrl;
}
////////////
// END RC
////////////

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

static
ran_func_def_ctrl_t *get_rc_ctrl_cap(const e2_node_connected_xapp_t *n, const int RC_ran_function)
{
  size_t const idx = find_sm_idx(n->rf, n->len_rf, eq_sm, RC_ran_function);
  if (n->rf[idx].defn.type != RC_RAN_FUNC_DEF_E) {
    printf("E2 node does not support RAN Control SM.\n");
    return NULL;
  }

  return n->rf[idx].defn.rc.ctrl;
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

  sleep(3);

  ////////////
  // START RC
  ////////////
  int const RC_ran_function = 3;

  for (int i = 0; i < nodes.len; i++) {
    e2_node_connected_xapp_t* n = &nodes.n[i];
    ran_func_def_ctrl_t *ran_ctrl = get_rc_ctrl_cap(n, RC_ran_function);
    // if CONTROL Service is supported by E2 node, send CONTROL message
    if (ran_ctrl != NULL) {
      // Generate RC CONTROL messages
      for (size_t j = 0; j < ran_ctrl->sz_seq_ctrl_style; j++) {
        // RIC Style Type 2 = Radio Resource Allocation Control
        // Control Action ID 6 = Slice-level PRB quota (8.4.3.6)
        // RIC Control Header IE Format 1 (9.2.1.6.1)
        // RIC Control Message IE Format 1 (9.2.1.7.1)
        if (ran_ctrl->seq_ctrl_style[j].style_type == 2) {
	  for (size_t z = 0; z < ue_slice_pair_lst.len; z++) {
            rc_ctrl_req_data_t rc_ctrl = gen_rc_ctrl_style_2(&ran_ctrl->seq_ctrl_style[j], &n->id.plmn, z);
            control_sm_xapp_api(&n->id, SM_RC_ID, &rc_ctrl);
	  }
	}
      }
    }
  }
  ////////////
  // END RC
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


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
#include "../../../../src/util/e.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/util/alg_ds/ds/lock_guard/lock_guard.h"
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

static
ue_id_e2sm_t *ue_id_lst = NULL;

static
ue_id_e2sm_t gen_rc_ue_id(void)
{
  ue_id_e2sm_t ue_id = {0};
  ue_id.type = GNB_DU_UE_ID_E2SM;
  ue_id.gnb_du.gnb_cu_ue_f1ap = 1;
  ue_id.gnb_du.ran_ue_id = NULL;

  return ue_id;
}

static
e2sm_rc_ctrl_hdr_frmt_1_t gen_rc_ctrl_hdr_1(void)
{
  e2sm_rc_ctrl_hdr_frmt_1_t hdr = {0};

  // UE ID
  // Mandatory
  // 9.3.10
  // 6.2.2.6
  if (ue_id_lst == NULL) {
    printf("Hardcoding UE ID. Once the fix is resolved in RSG, KPM and RC can be merged in one xApp, and no more hardcoding.\n");
    hdr.ue_id = gen_rc_ue_id(); //cp_ue_id_e2sm(ue_id_lst);
  }

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
ran_param_struct_t* fill_snssai(void)
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
  nssai->ran_param_struct[0].ran_param_val.flag_false->octet_str_ran.buf[0] = 1;

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
ran_param_struct_t* fill_rrm_policy_member_list_params(const e2ap_plmn_t *plmn_id)
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
  strct->ran_param_struct[1].ran_param_val.strct = fill_snssai();

  return strct;
}

static
lst_ran_param_t* fill_rrm_policy_member_list(const e2ap_plmn_t *plmn_id)
{
  lst_ran_param_t* lst = ecalloc(1, sizeof(lst_ran_param_t));

  lst->ran_param_struct.sz_ran_param_struct = 1;
  lst->ran_param_struct.ran_param_struct = ecalloc(lst->ran_param_struct.sz_ran_param_struct, sizeof(seq_ran_param_t));

  lst->ran_param_struct.ran_param_struct[0].ran_param_id = RRM_Policy_Member_8_4_3_6;
  lst->ran_param_struct.ran_param_struct[0].ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  lst->ran_param_struct.ran_param_struct[0].ran_param_val.strct = fill_rrm_policy_member_list_params(plmn_id);

  return lst;
}

static
ran_param_struct_t* fill_rrm_policy(const e2ap_plmn_t *plmn_id)
{
  ran_param_struct_t *rrm_policy = ecalloc(1, sizeof(ran_param_struct_t));

  rrm_policy->sz_ran_param_struct = 1;
  rrm_policy->ran_param_struct = ecalloc(rrm_policy->sz_ran_param_struct, sizeof(seq_ran_param_t));
  rrm_policy->ran_param_struct[0].ran_param_id = RRM_Policy_Member_List_8_4_3_6;
  rrm_policy->ran_param_struct[0].ran_param_val.type = LIST_RAN_PARAMETER_VAL_TYPE;
  rrm_policy->ran_param_struct[0].ran_param_val.lst = ecalloc(1, sizeof(ran_param_list_t));
  rrm_policy->ran_param_struct[0].ran_param_val.lst[0].sz_lst_ran_param = 1;
  rrm_policy->ran_param_struct[0].ran_param_val.lst[0].lst_ran_param = fill_rrm_policy_member_list(plmn_id);

  return rrm_policy;
}

static
ran_param_struct_t* fill_rrm_ratio_policy_group_params(const e2ap_plmn_t *plmn_id)
{
  ran_param_struct_t *strct = ecalloc(1, sizeof(ran_param_struct_t));

  strct->sz_ran_param_struct = 4;
  strct->ran_param_struct = ecalloc(strct->sz_ran_param_struct, sizeof(seq_ran_param_t));

  strct->ran_param_struct[0].ran_param_id = RRM_Policy_8_4_3_6;
  strct->ran_param_struct[0].ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  strct->ran_param_struct[0].ran_param_val.strct = fill_rrm_policy(plmn_id);

  strct->ran_param_struct[1].ran_param_id = Min_PRB_Policy_Ratio_8_4_3_6;
  strct->ran_param_struct[1].ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  strct->ran_param_struct[1].ran_param_val.flag_false = ecalloc(1, sizeof(ran_parameter_value_t));
  strct->ran_param_struct[1].ran_param_val.flag_false[0].type = INTEGER_RAN_PARAMETER_VALUE;
  strct->ran_param_struct[1].ran_param_val.flag_false[0].int_ran = 30;

  strct->ran_param_struct[2].ran_param_id = Max_PRB_Policy_Ratio_8_4_3_6;
  strct->ran_param_struct[2].ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  strct->ran_param_struct[2].ran_param_val.flag_false = ecalloc(1, sizeof(ran_parameter_value_t));
  strct->ran_param_struct[2].ran_param_val.flag_false[0].type = INTEGER_RAN_PARAMETER_VALUE;
  strct->ran_param_struct[2].ran_param_val.flag_false[0].int_ran = 30;

  strct->ran_param_struct[3].ran_param_id = Dedicated_PRB_Policy_Ratio_8_4_3_6;
  strct->ran_param_struct[3].ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  strct->ran_param_struct[3].ran_param_val.flag_false = ecalloc(1, sizeof(ran_parameter_value_t));
  strct->ran_param_struct[3].ran_param_val.flag_false[0].type = INTEGER_RAN_PARAMETER_VALUE;
  strct->ran_param_struct[3].ran_param_val.flag_false[0].int_ran = 30;

  return strct;
}

static
lst_ran_param_t* fill_rrm_ratio_policy_group(const e2ap_plmn_t *plmn_id)
{
  lst_ran_param_t *lst = ecalloc(1, sizeof(lst_ran_param_t));

  lst->ran_param_struct.sz_ran_param_struct = 1;
  lst->ran_param_struct.ran_param_struct = ecalloc(lst->ran_param_struct.sz_ran_param_struct, sizeof(seq_ran_param_t));

  lst->ran_param_struct.ran_param_struct[0].ran_param_id = RRM_Policy_Ratio_Group_8_4_3_6;
  lst->ran_param_struct.ran_param_struct[0].ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  lst->ran_param_struct.ran_param_struct[0].ran_param_val.strct = fill_rrm_ratio_policy_group_params(plmn_id);

  return lst;
}

static
e2sm_rc_ctrl_msg_frmt_1_t gen_rc_ctrl_msg_1(const e2ap_plmn_t *plmn_id)
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
  msg.ran_param[0].ran_param_val.lst[0].lst_ran_param = fill_rrm_ratio_policy_group(plmn_id);

  return msg;
}

static
rc_ctrl_req_data_t gen_rc_ctrl_style_2(const seq_ctrl_style_t *ctrl_style, const e2ap_plmn_t *plmn_id)
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
      rc_ctrl.hdr.frmt_1 = gen_rc_ctrl_hdr_1(),
      rc_ctrl.msg.format = FORMAT_1_E2SM_RC_CTRL_MSG,
      rc_ctrl.msg.frmt_1 = gen_rc_ctrl_msg_1(plmn_id);
      return rc_ctrl;
    }
  }
  assert(false && "E2 node does not support Control Action ID 6 (Slice-level PRB quota).\n");

  return rc_ctrl;
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
          rc_ctrl_req_data_t rc_ctrl = gen_rc_ctrl_style_2(&ran_ctrl->seq_ctrl_style[j], &n->id.plmn);
          control_sm_xapp_api(&n->id, SM_RC_ID, &rc_ctrl);
        }
      }
    }
  }
  ////////////
  // END RC
  ////////////

  xapp_wait_end_api();

  // Stop the xApp
  while (try_stop_xapp_api() == false)
    usleep(1000);

  printf("Test xApp run SUCCESSFULLY\n");
}


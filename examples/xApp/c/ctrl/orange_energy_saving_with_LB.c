
/*
 * Copyright (c) 2024 Orange Innovation Egypt
 * Developed by Orange Innovation Egypt, the xApp is presented as a significant
 * contribution to the advancement of the O-RAN ecosystem. It is integrated as a key element 
 * within the RIC Testing as a Platform (RIC-TaaP). 
 * The platform is established as an open-source initiative, 
 * designed to provide researchers and contributors with tools for exploring
 * the O-RAN ecosystem. Developments are tested, and the capabilities of 
 * the RAN Intelligent Controller (RIC) are analyzed through this framework.
 * 
 *       https://github.com/Orange-OpenSource/ns-O-RAN-flexric
 * 
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
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * Author:
 *  Abdelrhman Soliman <abdelrhman.soliman.ext@orange.com>
 */ 

//Energy saving xapp with load balancing 
#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/sm/rc_sm/ie/ir/ran_param_struct.h"
#include "../../../../src/sm/rc_sm/ie/ir/ran_param_list.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/util/alg_ds/ds/lock_guard/lock_guard.h"
#include "../../../../src/sm/rc_sm/rc_sm_id.h"
#include "../../../../src/util/e.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#define CURRENT_CELL '2'
#define CELL_OFF '0'



static
ue_id_e2sm_t ue_id;

static
uint64_t const period_ms = 100;

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
  if (cmp_str_ba("RRU.PrbTotDl", name) == 0) {
    printf("RRU.PrbTotDl = %d [PRBs]\n", meas_record.int_val);
  } else if (cmp_str_ba("RRU.PrbTotUl", name) == 0) {
    printf("RRU.PrbTotUl = %d [PRBs]\n", meas_record.int_val);
  } else if (cmp_str_ba("DRB.PdcpSduVolumeDL", name) == 0) {
    printf("DRB.PdcpSduVolumeDL = %d [kb]\n", meas_record.int_val);
  } else if (cmp_str_ba("DRB.PdcpSduVolumeUL", name) == 0) {
    printf("DRB.PdcpSduVolumeUL = %d [kb]\n", meas_record.int_val);
  } else {
    printf("Measurement Name not yet supported\n");
  }
}

static
void log_real_value(byte_array_t name, meas_record_lst_t meas_record)
{
  if (cmp_str_ba("DRB.RlcSduDelayDl", name) == 0) {
    printf("DRB.RlcSduDelayDl = %.2f [μs]\n", meas_record.real_val);
  } else if (cmp_str_ba("DRB.UEThpDl", name) == 0) {
    printf("DRB.UEThpDl = %.2f [kbps]\n", meas_record.real_val);
  } else if (cmp_str_ba("DRB.UEThpUl", name) == 0) {
    printf("DRB.UEThpUl = %.2f [kbps]\n", meas_record.real_val);
  } else {
    printf("Measurement Name not yet supported\n");
  }
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
void log_kpm_measurements(kpm_ind_msg_format_1_t const* msg_frm_1)
{
  assert(msg_frm_1->meas_info_lst_len > 0 && "Cannot correctly print measurements");

  // UE Measurements per granularity period
  for (size_t j = 0; j < msg_frm_1->meas_data_lst_len; j++) {
    meas_data_lst_t const data_item = msg_frm_1->meas_data_lst[j];

    for (size_t z = 0; z < data_item.meas_record_len; z++) {
      meas_type_t const meas_type = msg_frm_1->meas_info_lst[z].meas_type;
      meas_record_lst_t const record_item = data_item.meas_record_lst[z];

      match_meas_type[meas_type.type](meas_type, record_item);

      if (data_item.incomplete_flag && *data_item.incomplete_flag == TRUE_ENUM_VALUE)
        printf("Measurement Record not reliable");
    }
  }

}

static
void sm_cb_kpm(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == KPM_STATS_V3_0);

  // Reading Indication Message Format 3
  kpm_ind_data_t const* ind = &rd->ind.kpm.ind;
  kpm_ric_ind_hdr_format_1_t const* hdr_frm_1 = &ind->hdr.kpm_ric_ind_hdr_format_1;
  kpm_ind_msg_format_3_t const* msg_frm_3 = &ind->msg.frm_3;

  uint64_t const now = time_now_us();
  static int counter = 1;
  {
    lock_guard(&mtx);
    printf("\n time now = %ld \n",now); 
    printf("\n time from simulator = %ld \n", hdr_frm_1->collectStartTime); //ntohll(hdr_frm_1->collectStartTime)
    printf("\n%7d KPM ind_msg latency = %ld [μs]\n", counter, now -  hdr_frm_1->collectStartTime); // xApp <-> E2 Node

    // Reported list of measurements per UE
    for (size_t i = 0; i < msg_frm_3->ue_meas_report_lst_len; i++) {
      // log UE ID
      ue_id_e2sm_t const ue_id_e2sm = msg_frm_3->meas_report_per_ue[i].ue_meas_report_lst;
      ue_id_e2sm_e const type = ue_id_e2sm.type;
      log_ue_id_e2sm[type](ue_id_e2sm);
      // Save UE ID for filling RC Control message
      free_ue_id_e2sm(&ue_id);
      ue_id = cp_ue_id_e2sm(&ue_id_e2sm);

      // log measurements
      log_kpm_measurements(&msg_frm_3->meas_report_per_ue[i].ind_msg_format_1);
      
    }
    counter++;
  }
}

typedef enum {
  DRB_QoS_Configuration_7_6_2_1 = 1,
  QoS_flow_mapping_configuration_7_6_2_1 = 2,
  Logical_channel_configuration_7_6_2_1 = 3,
  Radio_admission_control_7_6_2_1 = 4,
  DRB_termination_control_7_6_2_1 = 5,
  DRB_split_ratio_control_7_6_2_1 = 6,
  PDCP_Duplication_control_7_6_2_1 = 7,
} rc_ctrl_service_style_1_e;

typedef enum {
  DRB_ID_8_4_2_2 = 1,
  LIST_OF_QOS_FLOWS_MOD_IN_DRB_8_4_2_2 = 2,
  QOS_FLOW_ITEM_8_4_2_2 = 3,
  QOS_FLOW_ID_8_4_2_2 = 4,
  QOS_FLOW_MAPPING_IND_8_4_2_2 = 5,
} qos_flow_mapping_conf_e;

static
test_info_lst_t filter_predicate(test_cond_type_e type, test_cond_e cond, int value)
{
  test_info_lst_t dst = {0};

  dst.test_cond_type = type;
  // It can only be TRUE_TEST_COND_TYPE so it does not matter the type
  // but ugly ugly...
  dst.S_NSSAI = TRUE_TEST_COND_TYPE;

  dst.test_cond = calloc(1, sizeof(test_cond_e));
  assert(dst.test_cond != NULL && "Memory exhausted");
  *dst.test_cond = cond;

  dst.test_cond_value = calloc(1, sizeof(test_cond_value_t));
  assert(dst.test_cond_value != NULL && "Memory exhausted");
  dst.test_cond_value->type = OCTET_STRING_TEST_COND_VALUE;

  dst.test_cond_value->octet_string_value = calloc(1, sizeof(byte_array_t));
  assert(dst.test_cond_value->octet_string_value != NULL && "Memory exhausted");
  const size_t len_nssai = 1;
  dst.test_cond_value->octet_string_value->len = len_nssai;
  dst.test_cond_value->octet_string_value->buf = calloc(len_nssai, sizeof(uint8_t));
  assert(dst.test_cond_value->octet_string_value->buf != NULL && "Memory exhausted");
  dst.test_cond_value->octet_string_value->buf[0] = value;

  return dst;
}

static
label_info_lst_t fill_kpm_label(void)
{
  label_info_lst_t label_item = {0};

  label_item.noLabel = ecalloc(1, sizeof(enum_value_e));
  *label_item.noLabel = TRUE_ENUM_VALUE;

  return label_item;
}

static
kpm_act_def_format_1_t fill_act_def_frm_1(ric_report_style_item_t const* report_item)
{
  assert(report_item != NULL);

  kpm_act_def_format_1_t ad_frm_1 = {0};

  size_t const sz = report_item->meas_info_for_action_lst_len;

  // [1, 65535]
  ad_frm_1.meas_info_lst_len = sz;
  ad_frm_1.meas_info_lst = calloc(sz, sizeof(meas_info_format_1_lst_t));
  assert(ad_frm_1.meas_info_lst != NULL && "Memory exhausted");

  for (size_t i = 0; i < sz; i++) {
    meas_info_format_1_lst_t* meas_item = &ad_frm_1.meas_info_lst[i];
    // 8.3.9
    // Measurement Name
    meas_item->meas_type.type = NAME_MEAS_TYPE;
    meas_item->meas_type.name = copy_byte_array(report_item->meas_info_for_action_lst[i].name);

    // [1, 2147483647]
    // 8.3.11
    meas_item->label_info_lst_len = 1;
    meas_item->label_info_lst = ecalloc(1, sizeof(label_info_lst_t));
    meas_item->label_info_lst[0] = fill_kpm_label();
  }

  // 8.3.8 [0, 4294967295]
  ad_frm_1.gran_period_ms = period_ms;

  // 8.3.20 - OPTIONAL
  ad_frm_1.cell_global_id = NULL;

#if defined KPM_V2_03 || defined KPM_V3_00
  // [0, 65535]
  ad_frm_1.meas_bin_range_info_lst_len = 0;
  ad_frm_1.meas_bin_info_lst = NULL;
#endif

  return ad_frm_1;
}

static
kpm_act_def_t fill_report_style_4(ric_report_style_item_t const* report_item)
{
  assert(report_item != NULL);
  assert(report_item->act_def_format_type == FORMAT_4_ACTION_DEFINITION);

  kpm_act_def_t act_def = {.type = FORMAT_4_ACTION_DEFINITION};

  // Fill matching condition
  // [1, 32768]
  act_def.frm_4.matching_cond_lst_len = 1;
  act_def.frm_4.matching_cond_lst = calloc(act_def.frm_4.matching_cond_lst_len, sizeof(matching_condition_format_4_lst_t));
  assert(act_def.frm_4.matching_cond_lst != NULL && "Memory exhausted");
  // Filter connected UEs by S-NSSAI criteria
  test_cond_type_e const type = S_NSSAI_TEST_COND_TYPE; // CQI_TEST_COND_TYPE
  test_cond_e const condition = EQUAL_TEST_COND; // GREATERTHAN_TEST_COND
  int const value = 1;
  act_def.frm_4.matching_cond_lst[0].test_info_lst = filter_predicate(type, condition, value);

  // Fill Action Definition Format 1
  // 8.2.1.2.1
  act_def.frm_4.action_def_format_1 = fill_act_def_frm_1(report_item);

  return act_def;
}

typedef kpm_act_def_t (*fill_kpm_act_def)(ric_report_style_item_t const* report_item);

static
fill_kpm_act_def get_kpm_act_def[END_RIC_SERVICE_REPORT] = {
    NULL,
    NULL,
    NULL,
    fill_report_style_4,
    NULL,
};

static
kpm_sub_data_t gen_kpm_subs(kpm_ran_function_def_t const* ran_func)
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
  ric_report_style_item_t* const report_item = &ran_func->ric_report_style_list[0];
  ric_service_report_e const report_style_type = report_item->report_style_type;
  *kpm_sub.ad = get_kpm_act_def[report_style_type](report_item);

  return kpm_sub;
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

//*****************************************************************//

typedef enum{
    Connected_mode_mobility =3,Energy_state =300
} rc_ctrl_service_style_id_e;


static
e2sm_rc_ctrl_hdr_frmt_1_t gen_rc_ctrl_hdr_frmt_1(ue_id_e2sm_t ue_id, uint32_t ric_style_type, uint16_t ctrl_act_id)
{
  e2sm_rc_ctrl_hdr_frmt_1_t dst = {0};

  // 6.2.2.6
  dst.ue_id = cp_ue_id_e2sm(&ue_id);

  dst.ric_style_type = ric_style_type;
  dst.ctrl_act_id = ctrl_act_id;

  return dst;
}


static
e2sm_rc_ctrl_hdr_t gen_rc_ctrl_hdr(e2sm_rc_ctrl_hdr_e hdr_frmt, ue_id_e2sm_t ue_id, uint32_t ric_style_type, uint16_t ctrl_act_id)
{
  e2sm_rc_ctrl_hdr_t dst = {0};

  if (hdr_frmt == FORMAT_1_E2SM_RC_CTRL_HDR) {
    dst.format = FORMAT_1_E2SM_RC_CTRL_HDR;
    dst.frmt_1 = gen_rc_ctrl_hdr_frmt_1(ue_id, ric_style_type, ctrl_act_id);
  } else {
    assert(0!=0 && "not implemented the fill func for this ctrl hdr frmt");
  }

  return dst;
}

static
void gen_Target_Primary_Cell_ID (seq_ran_param_t* Target_Primary_Cell_ID,char targetcell)
{ 
    // Target Primary Cell ID, STRUCTURE (len 1)

  Target_Primary_Cell_ID->ran_param_id = TARGET_PRIMARY_CELL_ID_8_4_4_1;
  Target_Primary_Cell_ID->ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  Target_Primary_Cell_ID->ran_param_val.strct = calloc(1, sizeof(ran_param_struct_t));
  assert(Target_Primary_Cell_ID->ran_param_val.strct != NULL && "Memory exhausted");
  Target_Primary_Cell_ID->ran_param_val.strct->sz_ran_param_struct = 1;
  Target_Primary_Cell_ID->ran_param_val.strct->ran_param_struct = calloc(1, sizeof(seq_ran_param_t));
  assert(Target_Primary_Cell_ID->ran_param_val.strct->ran_param_struct != NULL && "Memory exhausted");
 
   // > CHOICE Target Cell, STRUCTURE (len 2)
  seq_ran_param_t* CHOICE_Target_Cell = &Target_Primary_Cell_ID->ran_param_val.strct->ran_param_struct[0];
  CHOICE_Target_Cell->ran_param_id = CHOICE_TARGET_CELL_8_4_4_1;
  CHOICE_Target_Cell->ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  CHOICE_Target_Cell->ran_param_val.strct = calloc(1, sizeof(ran_param_struct_t));
  assert(CHOICE_Target_Cell->ran_param_val.strct != NULL && "Memory exhausted");
  CHOICE_Target_Cell->ran_param_val.strct->sz_ran_param_struct = 2;
  CHOICE_Target_Cell->ran_param_val.strct->ran_param_struct = calloc(2, sizeof(seq_ran_param_t));
  assert(CHOICE_Target_Cell->ran_param_val.strct->ran_param_struct != NULL && "Memory exhausted");
 
  // >>  NR Cell, STRUCTURE (len 1))
  seq_ran_param_t* NR_Cell = &CHOICE_Target_Cell->ran_param_val.strct->ran_param_struct[0];
  NR_Cell->ran_param_id = NR_CELL_8_4_4_1;
  NR_Cell->ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  NR_Cell->ran_param_val.strct = calloc(1, sizeof(ran_param_struct_t));
  assert(NR_Cell->ran_param_val.strct != NULL && "Memory exhausted");
  NR_Cell->ran_param_val.strct->sz_ran_param_struct = 1;
  NR_Cell->ran_param_val.strct->ran_param_struct = calloc(1, sizeof(seq_ran_param_t));
  
   // >>> NR CGI, ELEMENT   NR CGI is usually written in the format: PLMN ID + NR Cell Identity.
  seq_ran_param_t* NR_CGI = &NR_Cell->ran_param_val.strct->ran_param_struct[0];
  NR_CGI->ran_param_id = NR_CGI_8_4_4_1;
  NR_CGI->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  NR_CGI->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(NR_CGI->ran_param_val.flag_false != NULL && "Memory exhausted");
  NR_CGI->ran_param_val.flag_false->type =   BIT_STRING_RAN_PARAMETER_VALUE;
  //NR_CGI->ran_param_val.flag_false->int_ran=  TARGET_CELL;
  char nr_cgi_str[1] = {targetcell};
  

  byte_array_t nr_cgi = cp_str_to_ba(nr_cgi_str); 
  NR_CGI->ran_param_val.flag_false->octet_str_ran.len = nr_cgi.len;
  NR_CGI->ran_param_val.flag_false->octet_str_ran.buf = nr_cgi.buf;

  // >>E-UTRA Cell, STRUCTURE (len 1)
  seq_ran_param_t* EUTRA_Cell = &CHOICE_Target_Cell->ran_param_val.strct->ran_param_struct[1];
  EUTRA_Cell->ran_param_id = EUTRA_CELL_8_4_4_1;
  EUTRA_Cell->ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  EUTRA_Cell->ran_param_val.strct = calloc(1, sizeof(ran_param_struct_t));
  assert(EUTRA_Cell->ran_param_val.strct != NULL && "Memory exhausted");
  EUTRA_Cell->ran_param_val.strct->sz_ran_param_struct = 1;
  EUTRA_Cell->ran_param_val.strct->ran_param_struct = calloc(1, sizeof(seq_ran_param_t));
  
  // >>>E-UTRA CGI, ELEMENT  The E-UTRA CGI is typically written in the format: PLMN ID + E-UTRAN Cell Identity.
  seq_ran_param_t* EUTRA_CGI = &EUTRA_Cell->ran_param_val.strct->ran_param_struct[0];
  EUTRA_CGI->ran_param_id = EUTRA_CGI_8_4_4_1;
  EUTRA_CGI->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  EUTRA_CGI->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(EUTRA_CGI->ran_param_val.flag_false != NULL && "Memory exhausted");
  EUTRA_CGI->ran_param_val.flag_false->type = BIT_STRING_RAN_PARAMETER_VALUE;
  char eUTRA_cgi_str [2] ;
  eUTRA_cgi_str[0] = targetcell;
  eUTRA_cgi_str [1] = '\0';

  byte_array_t eUTRA_cgi = cp_str_to_ba(eUTRA_cgi_str); 
  EUTRA_CGI->ran_param_val.flag_false->octet_str_ran.len = eUTRA_cgi.len;
  EUTRA_CGI->ran_param_val.flag_false->octet_str_ran.buf = eUTRA_cgi.buf;
  return;
}



static
void gen_List_of_PDU_sessions_for_handover(seq_ran_param_t* List_PDU_sessions_ho )
{
  int num_PDU_session = 1;
  // List of PDU sessions for handover, LIST (len 1)
  List_PDU_sessions_ho->ran_param_id = LIST_OF_PDU_SESSIONS_FOR_HANDOVER_8_4_4_1;
  List_PDU_sessions_ho->ran_param_val.type = LIST_RAN_PARAMETER_VAL_TYPE;
  List_PDU_sessions_ho->ran_param_val.lst = calloc(1, sizeof(ran_param_list_t));
  assert(List_PDU_sessions_ho->ran_param_val.lst != NULL && "Memory exhausted");
  List_PDU_sessions_ho->ran_param_val.lst->sz_lst_ran_param = num_PDU_session;
  List_PDU_sessions_ho->ran_param_val.lst->lst_ran_param = calloc(num_PDU_session, sizeof(lst_ran_param_t));
  assert(List_PDU_sessions_ho->ran_param_val.lst->lst_ran_param != NULL && "Memory exhausted");
  
  // >PDU session Item for handover, STRUCTURE (len 2) 
  lst_ran_param_t* PDU_session_item =  (lst_ran_param_t*)&List_PDU_sessions_ho->ran_param_val.strct->ran_param_struct[0];
  //PDU_session_item->ran_param_id = PDU_session_Item_for_handover_8_4_4_1;
  //PDU_session_item->ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  //PDU_session_item->ran_param_val.strct = calloc(1, sizeof(ran_param_struct_t));
  //assert(PDU_session_item->ran_param_val.strct != NULL && "Memory exhausted");
  PDU_session_item->ran_param_struct.sz_ran_param_struct = 2;
  PDU_session_item->ran_param_struct.ran_param_struct = calloc(2, sizeof(seq_ran_param_t));
  assert(PDU_session_item->ran_param_struct.ran_param_struct != NULL && "Memory exhausted");

  // >>PDU Session ID, ELEMENT

  seq_ran_param_t* PDU_Session_ID = &PDU_session_item->ran_param_struct.ran_param_struct[0];
  PDU_Session_ID->ran_param_id = PDU_SESSION_ID_8_4_4_1;
  PDU_Session_ID->ran_param_val.type = ELEMENT_KEY_FLAG_TRUE_RAN_PARAMETER_VAL_TYPE;
  PDU_Session_ID->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(PDU_Session_ID->ran_param_val.flag_false != NULL && "Memory exhausted");
  PDU_Session_ID->ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  char pduid_str []= "5";
  byte_array_t pduid  = cp_str_to_ba(pduid_str); 
  PDU_Session_ID->ran_param_val.flag_false->octet_str_ran.len = pduid.len;
  PDU_Session_ID->ran_param_val.flag_false->octet_str_ran.buf = pduid.buf;

  // >>List of QoS flows in the PDU session, LIST (len 1)

  seq_ran_param_t* List_of_QoS_flows = &PDU_session_item->ran_param_struct.ran_param_struct[1];
  List_of_QoS_flows->ran_param_id = LIST_OF_QOS_FLOWS_IN_THE_PDU_SESSION_8_4_4_1;
  List_of_QoS_flows->ran_param_val.type = LIST_RAN_PARAMETER_VAL_TYPE;
  List_of_QoS_flows->ran_param_val.lst = calloc(1, sizeof(ran_param_list_t));
  assert(List_of_QoS_flows->ran_param_val.lst != NULL && "Memory exhausted");
  List_of_QoS_flows->ran_param_val.lst->sz_lst_ran_param = 1;
  List_of_QoS_flows->ran_param_val.lst->lst_ran_param = calloc(1, sizeof(lst_ran_param_t));
  assert(List_of_QoS_flows->ran_param_val.lst->lst_ran_param != NULL && "Memory exhausted");

  // >>>QoS flow Item, STRUCTURE (len 1)
  lst_ran_param_t* QoS_flow_Item = &List_of_QoS_flows->ran_param_val.lst->lst_ran_param[0];
  
  QoS_flow_Item->ran_param_struct.sz_ran_param_struct = 1;
  QoS_flow_Item->ran_param_struct.ran_param_struct = calloc(1, sizeof(seq_ran_param_t));
  assert(QoS_flow_Item->ran_param_struct.ran_param_struct != NULL && "Memory exhausted");

  // >>>>QoS Flow Identifier, ELEMENT
  seq_ran_param_t* QoS_Flow_Id = &QoS_flow_Item->ran_param_struct.ran_param_struct[0];
  QoS_Flow_Id->ran_param_id = QOS_FLOW_IDENTIFIER_8_4_4_1;
  QoS_Flow_Id->ran_param_val.type = ELEMENT_KEY_FLAG_TRUE_RAN_PARAMETER_VAL_TYPE;
  QoS_Flow_Id->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(QoS_Flow_Id->ran_param_val.flag_false != NULL && "Memory exhausted");
  QoS_Flow_Id->ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  char QFI_str []= "1";
  byte_array_t QFI  = cp_str_to_ba(QFI_str); 
  QoS_Flow_Id->ran_param_val.flag_false->octet_str_ran.len = QFI.len;
  QoS_Flow_Id->ran_param_val.flag_false->octet_str_ran.buf = QFI.buf;


  return;
}

static
void gen_List_of_DRBs_for_handover(seq_ran_param_t* List_DRBs_ho )
{
  int num_DRBs = 1;
  // List of DRBs for handover, LIST (len 1)
  List_DRBs_ho->ran_param_id = LIST_OF_DRBS_FOR_HANDOVER_8_4_4_1;
  List_DRBs_ho->ran_param_val.type = LIST_RAN_PARAMETER_VAL_TYPE;
  List_DRBs_ho->ran_param_val.lst = calloc(1, sizeof(ran_param_list_t));
  assert(List_DRBs_ho->ran_param_val.lst != NULL && "Memory exhausted");
  List_DRBs_ho->ran_param_val.lst->sz_lst_ran_param = num_DRBs;
  List_DRBs_ho->ran_param_val.lst->lst_ran_param = calloc(num_DRBs, sizeof(lst_ran_param_t));
  assert(List_DRBs_ho->ran_param_val.lst->lst_ran_param != NULL && "Memory exhausted");

  // > DRB item for handover, STRUCTURE (len 2)
  lst_ran_param_t* DRB_item_ho =  (lst_ran_param_t*) &List_DRBs_ho->ran_param_val.strct->ran_param_struct[0];
  
  DRB_item_ho->ran_param_struct.sz_ran_param_struct = 2;
  DRB_item_ho->ran_param_struct.ran_param_struct = calloc(2, sizeof(seq_ran_param_t));
  assert(DRB_item_ho->ran_param_struct.ran_param_struct != NULL && "Memory exhausted");

  // >> DRB ID, ELEMENT

  seq_ran_param_t* DRB_ID = &DRB_item_ho->ran_param_struct.ran_param_struct[0];
  DRB_ID->ran_param_id = DRB_ID_8_4_4_1;
  DRB_ID->ran_param_val.type = ELEMENT_KEY_FLAG_TRUE_RAN_PARAMETER_VAL_TYPE;
  DRB_ID->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(DRB_ID->ran_param_val.flag_false != NULL && "Memory exhausted");
  DRB_ID->ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  char DRB_ID_str []= "3";
  byte_array_t drpID  = cp_str_to_ba(DRB_ID_str); 
  DRB_ID->ran_param_val.flag_false->octet_str_ran.len = drpID.len;
  DRB_ID->ran_param_val.flag_false->octet_str_ran.buf = drpID.buf;

  // >> List of QoS flows in the DRB, LIST (len 1)

  seq_ran_param_t* List_of_QoS_flows = &DRB_item_ho->ran_param_struct.ran_param_struct[1];  //////.....//////
  List_of_QoS_flows->ran_param_id = LIST_OF_QOS_FLOWS_IN_THE_DRB_8_4_4_1;
  List_of_QoS_flows->ran_param_val.type = LIST_RAN_PARAMETER_VAL_TYPE;
  List_of_QoS_flows->ran_param_val.lst = calloc(1, sizeof(ran_param_list_t));
  assert(List_of_QoS_flows->ran_param_val.lst != NULL && "Memory exhausted");
  List_of_QoS_flows->ran_param_val.lst->sz_lst_ran_param = 1;
  List_of_QoS_flows->ran_param_val.lst->lst_ran_param = calloc(1, sizeof(lst_ran_param_t));
  assert(List_of_QoS_flows->ran_param_val.lst->lst_ran_param != NULL && "Memory exhausted");

  // >>>QoS flow Item, STRUCTURE (len 1)
  lst_ran_param_t* QoS_flow_Item = &List_of_QoS_flows->ran_param_val.lst->lst_ran_param[0];
  QoS_flow_Item->ran_param_struct.sz_ran_param_struct = 1;
  QoS_flow_Item->ran_param_struct.ran_param_struct = calloc(1, sizeof(seq_ran_param_t));
  assert(QoS_flow_Item->ran_param_struct.ran_param_struct != NULL && "Memory exhausted");

  // >>>>QoS Flow Identifier, ELEMENT
  seq_ran_param_t* QoS_Flow_Id = &QoS_flow_Item->ran_param_struct.ran_param_struct[0];
  QoS_Flow_Id->ran_param_id = QOS_FLOW_IDENTIFIER_8_4_4_1;
  QoS_Flow_Id->ran_param_val.type = ELEMENT_KEY_FLAG_TRUE_RAN_PARAMETER_VAL_TYPE;
  QoS_Flow_Id->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(QoS_Flow_Id->ran_param_val.flag_false != NULL && "Memory exhausted");
  QoS_Flow_Id->ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  char QFI_str []= "10";
  byte_array_t QFI  = cp_str_to_ba(QFI_str); 
  QoS_Flow_Id->ran_param_val.flag_false->octet_str_ran.len = QFI.len;
  QoS_Flow_Id->ran_param_val.flag_false->octet_str_ran.buf = QFI.buf;

  return;

}

static
void gen_List_of_Secondary_cells_to_be_setup(seq_ran_param_t* List_num_2ndCells)
{

  int num_2ndCells= 1;
  // List of Secondary cells to be setup, LIST (len 1)
  List_num_2ndCells->ran_param_id = LIST_OF_SECONDARY_CELLS_TO_BE_SETUP_8_4_4_1;
  List_num_2ndCells->ran_param_val.type = LIST_RAN_PARAMETER_VAL_TYPE;
  List_num_2ndCells->ran_param_val.lst = calloc(1, sizeof(ran_param_list_t));
  assert(List_num_2ndCells->ran_param_val.lst != NULL && "Memory exhausted");
  List_num_2ndCells->ran_param_val.lst->sz_lst_ran_param = num_2ndCells;
  List_num_2ndCells->ran_param_val.lst->lst_ran_param = calloc(num_2ndCells, sizeof(lst_ran_param_t));
  assert(List_num_2ndCells->ran_param_val.lst->lst_ran_param != NULL && "Memory exhausted");

   // >Secondary cell Item to be setup, STRUCTURE (len 1)


  lst_ran_param_t* secCell_item =  (lst_ran_param_t*)&List_num_2ndCells->ran_param_val.strct->ran_param_struct[0];
  
  secCell_item->ran_param_struct.sz_ran_param_struct = 1;
  secCell_item->ran_param_struct.ran_param_struct = calloc(1, sizeof(seq_ran_param_t));
  assert(secCell_item->ran_param_struct.ran_param_struct != NULL && "Memory exhausted");

   // >>Secondary cell ID, ELEMENT
  seq_ran_param_t* secCell_Id = &secCell_item->ran_param_struct.ran_param_struct[0];
  secCell_Id->ran_param_id = SECONDARY_CELL_ID_8_4_4_1;
  secCell_Id->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  secCell_Id->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(secCell_Id->ran_param_val.flag_false != NULL && "Memory exhausted");
  secCell_Id->ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  char cellID_str []= "0";
  byte_array_t QFI  = cp_str_to_ba(cellID_str); 
  secCell_Id->ran_param_val.flag_false->octet_str_ran.len = QFI.len;
  secCell_Id->ran_param_val.flag_false->octet_str_ran.buf = QFI.buf;
  
  return ;
}

static
e2sm_rc_ctrl_msg_frmt_1_t gen_rc_ctrl_msg_frmt_1_Handover_Control(char targetcell)
{
  e2sm_rc_ctrl_msg_frmt_1_t dst = {0};
  // 8.4.4.1

  // Target Primary Cell ID, STRUCTURE (len 1)
  // > CHOICE Target Cell, STRUCTURE (len 2)
  // >>  NR Cell, STRUCTURE (len 1))
  // >>> NR CGI, ELEMENT
  // >>E-UTRA Cell, STRUCTURE (len 1)
  // >>>E-UTRA CGI, ELEMENT

  // List of PDU sessions for handover, LIST (len 1)
  // >PDU session Item for handover, STRUCTURE (len 2) 
  // >>PDU Session ID, ELEMENT
  // >>List of QoS flows in the PDU session, LIST (len 1)
  // >>>QoS flow Item, STRUCTURE (len 1)
  // >>>>QoS Flow Identifier, ELEMENT

  // List of DRBs for handover, LIST (len 1)
  // > DRB item for handover, STRUCTURE (len 2)
  // >> DRB ID, ELEMENT
  // >> List of QoS flows in the DRB, LIST (len 1)
  // >>> QoS flow Item, STRUCTURE (len 1)
  // >>>> QoS flow Identifier, ELEMENT

  // List of Secondary cells to be setup, LIST (len 1)
  // >Secondary cell Item to be setup, STRUCTURE (len 1)
  // >>Secondary cell ID, ELEMENT

  dst.sz_ran_param = 4;
  dst.ran_param = calloc(4, sizeof(seq_ran_param_t));
  assert(dst.ran_param != NULL && "Memory exhausted");

  gen_Target_Primary_Cell_ID(&dst.ran_param[0],targetcell);
  gen_List_of_PDU_sessions_for_handover(&dst.ran_param[1]);
  gen_List_of_DRBs_for_handover(&dst.ran_param[2]);
  gen_List_of_Secondary_cells_to_be_setup(&dst.ran_param[3]);

  return dst;
}

static
e2sm_rc_ctrl_msg_frmt_1_t gen_rc_ctrl_msg_frmt_1_cell_trigger(char targetcell)
{
  e2sm_rc_ctrl_msg_frmt_1_t dst = {0};
  // 8.4.4.1

  // Target Primary Cell ID, STRUCTURE (len 1)
  // > CHOICE Target Cell, STRUCTURE (len 2)
  // >>  NR Cell, STRUCTURE (len 1))
  // >>> NR CGI, ELEMENT
  // >>E-UTRA Cell, STRUCTURE (len 1)
  // >>>E-UTRA CGI, ELEMENT
  dst.sz_ran_param =1;
  dst.ran_param = calloc(4, sizeof(seq_ran_param_t));
  assert(dst.ran_param != NULL && "Memory exhausted");

  gen_Target_Primary_Cell_ID(&dst.ran_param[0],targetcell);
  return dst;
}


static
e2sm_rc_ctrl_msg_t gen_handover_rc_ctrl_msg(e2sm_rc_ctrl_msg_e msg_frmt,char targetcell)
{
  e2sm_rc_ctrl_msg_t dst = {0};

  if (msg_frmt == FORMAT_1_E2SM_RC_CTRL_MSG) {
    dst.format = msg_frmt;
    dst.frmt_1 = gen_rc_ctrl_msg_frmt_1_Handover_Control(targetcell);
  } else {
    assert(0!=0 && "not implemented the fill func for this ctrl msg frmt");
  }

  return dst;
}

static
e2sm_rc_ctrl_msg_t gen_cell_trigger_rc_ctrl_msg(e2sm_rc_ctrl_msg_e msg_frmt,char targetcell)
{
  e2sm_rc_ctrl_msg_t dst = {0};

  if (msg_frmt == FORMAT_1_E2SM_RC_CTRL_MSG) {
    dst.format = msg_frmt;
    dst.frmt_1 = gen_rc_ctrl_msg_frmt_1_cell_trigger(targetcell);
  } else {
    assert(0!=0 && "not implemented the fill func for this ctrl msg frmt");
  }

  return dst;
}

static
ue_id_e2sm_t gen_rc_ue_id(ue_id_e2sm_e type,int IMSI)
{
  ue_id_e2sm_t ue_id = {0};
  if (type == GNB_UE_ID_E2SM) {
    ue_id.type = GNB_UE_ID_E2SM;
    ue_id.gnb.ran_ue_id = (uint64_t *)malloc(sizeof(uint64_t));
     *(ue_id.gnb.ran_ue_id) = IMSI;
  } else {
    assert(0!=0 && "not supported UE ID type");
  }
  return ue_id;
}

static
bool eq_sm(sm_ran_function_t const* elem, int const id)
{
  if (elem->id == id)
    return true;

  return false;
}

int main(int argc, char *argv[])
{
  fr_args_t args = init_fr_args(argc, argv);

  //Init the xApp
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
      kpm_sub_data_t kpm_sub = gen_kpm_subs(&n->rf[idx].defn.kpm);
      hndl[i] = report_sm_xapp_api(&n->id, KPM_ran_function, &kpm_sub, sm_cb_kpm);
      assert(hndl[i].success == true);
      free_kpm_sub_data(&kpm_sub);
    }
  }
  ////////////
  // END KPM
  ////////////

    sleep(40);

  ////////////
  // START RC
  ////////////

  // RC Control
  // CONTROL Service Style 3: Connected mode mobility control
  // Action ID 1: Handover Control
  // E2SM-RC Control Header Format 1
  // E2SM-RC Control Message Format 1


  //move imsi 3 that have ranti 1 to cell 5  
  char targetcell = '5' ;
  int IMSI = 3;
  rc_ctrl_req_data_t rc_ctrl_1 = {0};
  ue_id_e2sm_t ue_id_1 = gen_rc_ue_id(GNB_UE_ID_E2SM,IMSI);

  rc_ctrl_1.hdr = gen_rc_ctrl_hdr(FORMAT_1_E2SM_RC_CTRL_HDR, ue_id_1, Connected_mode_mobility, HANDOVER_CONTROL_7_6_4_1);
  rc_ctrl_1.msg = gen_handover_rc_ctrl_msg(FORMAT_1_E2SM_RC_CTRL_MSG,targetcell);

  int64_t st = time_now_us();
  printf("[xApp]: Send Handover Control message to move IMSI %ld from cellId %c to target cellId %c \n",*(ue_id_1.gnb.ran_ue_id), CURRENT_CELL,targetcell);
  for(size_t i =0; i < nodes.len; ++i){
    control_sm_xapp_api(&nodes.n[i].id, SM_RC_ID, &rc_ctrl_1);
  }
  printf("[xApp]: Control Loop Latency for the first control message : %ld us\n", time_now_us() - st);
   sleep(5);
//  ***********************************************************************************************************
  
  //move imsi 8 that have ranti 2 to cell 4 
  targetcell = '4';
  rc_ctrl_req_data_t rc_ctrl_2 = {0};
  IMSI = 8;
  ue_id_e2sm_t ue_id_2 = gen_rc_ue_id(GNB_UE_ID_E2SM,IMSI);
  rc_ctrl_2.hdr = gen_rc_ctrl_hdr(FORMAT_1_E2SM_RC_CTRL_HDR, ue_id_2, Connected_mode_mobility, HANDOVER_CONTROL_7_6_4_1);
  rc_ctrl_2.msg = gen_handover_rc_ctrl_msg(FORMAT_1_E2SM_RC_CTRL_MSG,targetcell);

  int64_t st1 = time_now_us();
    printf("[xApp]: Send Handover Control message to move IMSI %ld from cellId %c to target cellId %c \n",*(ue_id_2.gnb.ran_ue_id), CURRENT_CELL,targetcell);
  for(size_t i =0; i < nodes.len; ++i){
    control_sm_xapp_api(&nodes.n[i].id, SM_RC_ID, &rc_ctrl_2);
  }
  printf("[xApp]: Control Loop Latency for the second control message : %ld us\n", time_now_us() - st1);
 
  
  //**********************************************************************************************************
  sleep(5);
    //move imsi 9 that have ranti 3 to cell 3  
  targetcell = '3';
  rc_ctrl_req_data_t rc_ctrl_3 = {0};
  IMSI = 9;
  ue_id_e2sm_t ue_id_3 = gen_rc_ue_id(GNB_UE_ID_E2SM,IMSI);
  rc_ctrl_3.hdr = gen_rc_ctrl_hdr(FORMAT_1_E2SM_RC_CTRL_HDR, ue_id_3, Connected_mode_mobility, HANDOVER_CONTROL_7_6_4_1);
  rc_ctrl_3.msg = gen_handover_rc_ctrl_msg(FORMAT_1_E2SM_RC_CTRL_MSG,targetcell);

  int64_t st2 = time_now_us();
    printf("[xApp]: Send Handover Control message to move IMSI %ld from cellId %c to target cellId %c \n",*(ue_id_3.gnb.ran_ue_id), CURRENT_CELL,targetcell);
  for(size_t i =0; i < nodes.len; ++i){
    control_sm_xapp_api(&nodes.n[i].id, SM_RC_ID, &rc_ctrl_3);
  }
  printf("[xApp]: Control Loop Latency for the second control message : %ld us\n", time_now_us() - st2);
  sleep(5);
  
  //**********************************************************************************************************
    //move imsi 10 that have ranti 4 to cell 5  
  targetcell= '5';
  rc_ctrl_req_data_t rc_ctrl_4 = {0};
  IMSI = 10;
  ue_id_e2sm_t ue_id_4 = gen_rc_ue_id(GNB_UE_ID_E2SM,IMSI);
  rc_ctrl_4.hdr = gen_rc_ctrl_hdr(FORMAT_1_E2SM_RC_CTRL_HDR, ue_id_4, Connected_mode_mobility, HANDOVER_CONTROL_7_6_4_1);
  rc_ctrl_4.msg = gen_handover_rc_ctrl_msg(FORMAT_1_E2SM_RC_CTRL_MSG,targetcell);
  
  int64_t st3 = time_now_us();
    printf("[xApp]: Send Handover Control message to move IMSI %ld from cellId %c to target cellId %c \n",*(ue_id_4.gnb.ran_ue_id), CURRENT_CELL,targetcell);
  for(size_t i =0; i < nodes.len; ++i){
    control_sm_xapp_api(&nodes.n[i].id, SM_RC_ID, &rc_ctrl_4);
  }
  printf("[xApp]: Control Loop Latency for the second control message : %ld us\n", time_now_us() - st3);

  //**********************************************************************************************************
   sleep(10);
  rc_ctrl_req_data_t rc_ctrl_6 = {0};
  rc_ctrl_6.hdr = gen_rc_ctrl_hdr(FORMAT_1_E2SM_RC_CTRL_HDR, ue_id_2, Energy_state, CELL_OFF);
  rc_ctrl_6.msg = gen_cell_trigger_rc_ctrl_msg(FORMAT_1_E2SM_RC_CTRL_MSG,CURRENT_CELL);
    printf("[xApp]: Send switch off Control message to switch off target cellId %c \n",CURRENT_CELL);
    control_sm_xapp_api(&nodes.n[0].id, SM_RC_ID, &rc_ctrl_6);


  free_rc_ctrl_req_data(&rc_ctrl_1);
  free_rc_ctrl_req_data(&rc_ctrl_2);
  free_rc_ctrl_req_data(&rc_ctrl_3);
  free_rc_ctrl_req_data(&rc_ctrl_4);
  free_rc_ctrl_req_data(&rc_ctrl_6);


  ////////////
  // END RC
  ////////////

  sleep(2000);

  for (int i = 0; i < nodes.len; ++i) {
    // Remove the handle previously returned
    if (hndl[i].success == true)
      rm_report_sm_xapp_api(hndl[i].u.handle);
  }
  free(hndl);

  //Stop the xApp
  while(try_stop_xapp_api() == false)
    usleep(1000);
  
    //free_e2_node_arr_xapp(&nodes);

  rc = pthread_mutex_destroy(&mtx);
  assert(rc == 0);

  printf("Test xApp run SUCCESSFULLY\n");


}


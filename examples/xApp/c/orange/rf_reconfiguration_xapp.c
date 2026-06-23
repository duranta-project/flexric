#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/sm/ccc_sm/ie/ccc_data_ie.h"
#include "../../../../src/sm/ccc_sm/ccc_sm_id.h"
#include "../../../../src/util/alg_ds/ds/lock_guard/lock_guard.h"
#include "../../../../src/util/e.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>


static
ue_id_e2sm_t ue_id;

static
uint64_t const period_ms = 100;

static
pthread_mutex_t mtx;
uint32_t old_ports_off_count = 0;
uint32_t old_ports_on_count = 0;
static e2_node_arr_xapp_t* g_nodes = NULL;  // Global reference to E2 nodes

// Forward declarations
char* gen_json_string_O_NESPolicy(const char* antenna_mask, const char* old_antenna_mask,
  uint16_t mcc, uint16_t mnc, uint8_t mnc_digit_len,
  uint64_t nr_cell_id);
static void send_antenna_control_message(const char* antenna_mask);
static bool eq_sm(sm_ran_function_t const* elem, int const id);
static size_t find_sm_idx(sm_ran_function_t* rf, size_t sz, bool (*f)(sm_ran_function_t const*, int const), int const id);
static ccc_ctrl_hdr_t gen_ccc_hdr(e2sm_ccc_ctrl_hdr_format_e hdr_frmt, uint32_t ric_style_type);
static ccc_ctrl_msg_t gen_ccc_ctrl_msg(e2sm_ccc_ctrl_msg_format_e msg_frmt, const char* ran_configuration_structure_name, const char* antenna_mask, const char* old_antenna_mask);

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


static void log_int_value(byte_array_t name, meas_record_lst_t meas_record) 
{
  if (cmp_str_ba("RRU.PrbTotDl", name) == 0) {
    printf("RRU.PrbTotDl = %d [PRBs]\n", meas_record.int_val);
    printf("RRU.PrbTotUl = %d [PRBs]\n", meas_record.int_val);
  } else if (cmp_str_ba("DRB.PdcpSduVolumeDL", name) == 0) {
    printf("DRB.PdcpSduVolumeDL = %d [kb]\n", meas_record.int_val);
  } else if (cmp_str_ba("DRB.PdcpSduVolumeUL", name) == 0) {
    printf("DRB.PdcpSduVolumeUL = %d [kb]\n", meas_record.int_val);
    // } else if (strncmp(name.buf, "L3neighSINRListOf_UEID_", strlen("L3neighSINRListOf_UEID_")) == 0) {
    //   printf("%s, Neighbour=%d \n", name.buf, meas_record.int_val);
  } else {
     printf("int.Name= %s, value= %d \n", name.buf, meas_record.int_val);

  }
}

static void log_real_value(byte_array_t name, meas_record_lst_t meas_record) 
{
  if (cmp_str_ba("DRB.RlcSduDelayDl", name) == 0) {
    printf("DRB.RlcSduDelayDl = %.2f [μs]\n", meas_record.real_val);
  } else if (cmp_str_ba("DRB.UEThpDl", name) == 0) {
    printf("DRB.UEThpDl = %.2f [kbps]\n", meas_record.real_val);
  } else if (cmp_str_ba("DLThroughput.UEID", name) == 0) {
    double throughput = meas_record.real_val;
    printf("%s, value = %.4f \n", name.buf, throughput);
    
    // Threshold-based antenna control logic
    if (throughput > 250.0 && old_ports_on_count == 4) {
      // Throughput is good, no action needed
      printf("  [Threshold] Throughput %.4f > 250 - No control message needed\n", throughput);
    } else if (throughput > 250 && old_ports_on_count == 2){
      printf("  [Threshold] Throughput %.4f > 250 and Old.PortsOn=%u == 3 - Setting antenna_mask to 1111\n", throughput, old_ports_on_count);
      send_antenna_control_message("1111");
    }
    else if (throughput > 120.0 && throughput <= 250.0) {
      // Throughput is medium: if old_ports_on_count == 4, set to "1100"
      if (old_ports_on_count == 4) {
        printf("  [Threshold] Throughput %.4f between 120-250 and Old.PortsOn=%u - Setting antenna_mask to 1100\n", 
               throughput, old_ports_on_count);
        send_antenna_control_message("1100");
      } else if (old_ports_on_count == 1){
        printf("  [Threshold] Throughput %.4f between 120-250 and Old.PortsOn=%u == 1 - Setting antenna_mask to 1100\n", 
               throughput, old_ports_on_count);
        send_antenna_control_message("1100");
      } else {
        printf("  [Threshold] Throughput %.4f between 120-250 but Old.PortsOn=%u != 4 - No action\n", 
               throughput, old_ports_on_count);
      }
    } else {
      // Throughput is low (< 120): set to "1000"
      printf("  [Threshold] Throughput %.4f < 120 - Setting antenna_mask to 1000\n", throughput);
      send_antenna_control_message("1000");
    }
  } else if (cmp_str_ba("Old.PortsOff", name) == 0) {
    printf("%s, value = %.4f \n", name.buf, meas_record.real_val);
    old_ports_off_count =  meas_record.real_val;
  } else if (cmp_str_ba("Old.PortsOn", name) == 0) {
    printf("%s, value = %.4f \n", name.buf, meas_record.real_val);
    old_ports_on_count =  meas_record.real_val;
    printf("old_ports_off_count = %d, old_ports_on_count = %d\n", old_ports_off_count, old_ports_on_count);
  }
}


static
char* GetAntennaMask(uint32_t onPorts)
{
    const int totalPorts = 4;  // Fixed total ports
    static char mask[5];  // Static buffer for return string (4 chars + null terminator)
    
    // Initialize all positions to '0'
    for (int i = 0; i < totalPorts; i++) {
        mask[i] = '0';
    }
    
    // Set first 'onPorts' positions to '1'
    for (uint32_t i = 0; i < onPorts && i < (uint32_t)totalPorts; i++) {
        mask[i] = '1';
    }
    
    mask[totalPorts] = '\0';  // Null terminator
    return mask;
}

static
void send_antenna_control_message(const char* antenna_mask)
{
  if (g_nodes == NULL || g_nodes->len == 0) {
    printf("WARNING: Cannot send control message - E2 nodes not available\n");
    return;
  }

  int const CCC_ran_function = 4;
  
  for (size_t i = 0; i < g_nodes->len; ++i) {
    e2_node_connected_xapp_t* n = &g_nodes->n[i];
    
    // Find CCC RAN function
    size_t const idx = find_sm_idx(n->rf, n->len_rf, eq_sm, CCC_ran_function);
    if (n->rf[idx].defn.type != CCC_RAN_FUNC_DEF_E) {
      printf("WARNING: CCC RAN function not found for node %zu\n", i);
      continue;
    }
    
    // Prepare control message
    ccc_ctrl_req_data_t ccc_ctrl_req = {0};
    char* old_antenna_mask = GetAntennaMask(old_ports_on_count);
    char* ran_configuration_structure_name = "O-NESPolicy";
    
    // Generate CCC CONTROL message
    ccc_ctrl_req.hdr = gen_ccc_hdr(FORMAT_1_E2SM_CCC_CTRL_HDR, CCC_CTRL_SERVICE_STYLE_TYPE_2);
    ccc_ctrl_req.msg = gen_ccc_ctrl_msg(FORMAT_2_E2SM_CCC_CTRL_MSG, ran_configuration_structure_name, antenna_mask, old_antenna_mask);
    
    printf("[xApp]: Sending RF Reconfiguration Control message - antenna mask: %s (old: %s)\n", antenna_mask, old_antenna_mask);
    
    control_sm_xapp_api(&n->id, SM_CCC_ID, &ccc_ctrl_req);
    
    free_ccc_ctrl_req_data(&ccc_ctrl_req);
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
      // Determine the correct measurement type index
      // Standard KPM Format 1: meas_info_lst[z] corresponds to meas_record_lst[z] in each data item
      // Non-standard (ns-3 bug): multiple data items, each with 1 record, should map sequentially
      // If we have multiple data items with single records, use data item index
      size_t meas_type_idx;
      if (msg_frm_1->meas_data_lst_len > 1 && data_item.meas_record_len == 1) {
        // Non-standard encoding: use data item index
        meas_type_idx = (j < msg_frm_1->meas_info_lst_len) ? j : msg_frm_1->meas_info_lst_len - 1;
      } else {
        // Standard encoding: use record index within data item
        meas_type_idx = (z < msg_frm_1->meas_info_lst_len) ? z : msg_frm_1->meas_info_lst_len - 1;
      }
      
      meas_type_t const meas_type = msg_frm_1->meas_info_lst[meas_type_idx].meas_type;
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

  int64_t const now = time_now_us();
  static int counter = 1;
  {
    lock_guard(&mtx);
    printf("\n%7d KPM ind_msg latency = %ld [μs]\n", counter, now - hdr_frm_1->collectStartTime); // xApp <-> E2 Node


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
  return 0;
}

static
ccc_ctrl_hdr_format1_t gen_ccc_ctrl_hdr_format1(uint32_t ric_style_type)
{
  ccc_ctrl_hdr_format1_t dst = {0};
  dst.ric_style_type = ric_style_type;  
  return dst;  // Return struct
}

static
ccc_ctrl_hdr_t gen_ccc_hdr(e2sm_ccc_ctrl_hdr_format_e hdr_frmt, uint32_t ric_style_type)
{
  ccc_ctrl_hdr_t dst = {0};
  dst.format = hdr_frmt;
  dst.format1 = gen_ccc_ctrl_hdr_format1(ric_style_type);
  return dst;
}


static
ccc_ctrl_msg_format2_t gen_ccc_ctrl_msg_format2(const char* ran_configuration_structure_name, const char* antenna_mask, const char* old_antenna_mask)
{
  ccc_ctrl_msg_format2_t dst = {0};
  //9.2.1.7.2   E2SM-CCC Control Message Format 2
  //List of Cells
  //>Cell Global ID
  //>List of Configuration Structures
  //>>RAN Configuration Structure Name  "O-NESPolicy"
  //>>Old Values of Attributes
  //>>New Values of Attributes
  switch (strcmp(ran_configuration_structure_name, "O-NESPolicy")) {
    case 0: {
        char* json = gen_json_string_O_NESPolicy(antenna_mask, old_antenna_mask, g_nodes->n[0].id.plmn.mcc, g_nodes->n[0].id.plmn.mnc, g_nodes->n[0].id.plmn.mnc_digit_len, g_nodes->n[0].id.nb_id.nb_id);
        dst.list_of_cells_controlled.data = json;
        dst.list_of_cells_controlled.len = strlen(json);
        break;
    }
    default:
      assert(0!=0 && "not implemented the fill func for this ctrl msg frmt");
  }
  return dst;
}

char* gen_json_string_O_NESPolicy(const char* antenna_mask, const char* old_antenna_mask,
  uint16_t mcc, uint16_t mnc, uint8_t mnc_digit_len,
  uint64_t nr_cell_id)
{

    //8.8.2.6   O-NESPolicy
  //>Policy Type 
  //>Antenna Mask Name 
  //>Antenna Mask 
  //>Sleep Mode 
  //>Data Dir
  //>Symbol Mask 
  //>Slot Mask 
  //>Valid Duration 
      // Build PLMN string: "00101" format
      char plmn_str[7];
      snprintf(plmn_str, sizeof(plmn_str), "%03d%0*d", mcc, mnc_digit_len, mnc);
      // Build nRCellIdentity as 18-digit zero-padded binary string (36 bits)
      char cell_id_str[37];
      snprintf(cell_id_str, sizeof(cell_id_str), "%036llu", (unsigned long long)nr_cell_id);
  
    // Allocate buffer for formatted JSON
    char* formatted_json = malloc(2048);  
    
    snprintf(formatted_json, 2048,
      "{\n"
      "  \"list_of_cells_controlled\": [\n"
      "    {\n"
      "      \"cell_global_id\": {\n"
      "        \"nR-CGI\": {\n"
      "          \"pLMNIdentity\": \"%s\",\n"      
      "          \"nRCellIdentity\": \"%s\"\n"
      "        }\n"
      "      },\n"
      "      \"list_of_configuration_structures\": [\n"
      "        {\n"
      "          \"ran_configuration_structure_name\": \"O-NESPolicy\",\n"
      "          \"old_values_of_attributes\": {\n"
      "            \"policyType\": \"TRX\",\n"
      // "            \"antennaMaskName\": \"TRX_MASK_1\",\n"
      "            \"antennaMask\": \"%s\"\n"
      // "            \"sleepMode\": \"NO_SLEEP\",\n"
      // "            \"dataDir\": \"DL\",\n"
      // "            \"symbolMask\": \"11111111111111\",\n"
      // "            \"slotMask\": \"1111111111\",\n"
      // "            \"validDuration\": 0\n"
      "          },\n"
      "          \"new_values_of_attributes\": {\n"
      "            \"policyType\": \"TRX\",\n"
      // "            \"antennaMaskName\": \"TRX_MASK_1\",\n"
      "            \"antennaMask\": \"%s\"\n"  
      // "            \"sleepMode\": \"NO_SLEEP\",\n"
      // "            \"dataDir\": \"DL\",\n"
      // "            \"symbolMask\": \"11111111111111\",\n"
      // "            \"slotMask\": \"1111111111\",\n"
      // "            \"validDuration\": 0\n"
      "          }\n"
      "        }\n"
      "      ]\n"
      "    }\n"
      "  ]\n"
      "}", plmn_str, cell_id_str, old_antenna_mask, antenna_mask);
    
  
    return formatted_json ;
  }

static
ccc_ctrl_msg_t gen_ccc_ctrl_msg(e2sm_ccc_ctrl_msg_format_e msg_frmt, const char* ran_configuration_structure_name, const char* antenna_mask, const char* old_antenna_mask)
  {
    ccc_ctrl_msg_t dst = {0};
  
    if (msg_frmt == FORMAT_2_E2SM_CCC_CTRL_MSG) {
      dst.format = msg_frmt;
      dst.format2 = gen_ccc_ctrl_msg_format2(ran_configuration_structure_name, antenna_mask, old_antenna_mask);   //8.8.2.6
    } else {
      assert(0!=0 && "not implemented the fill func for this ctrl msg frmt");
    }
    return dst;
}



int main(int argc, char* argv[])
{
  fr_args_t args = init_fr_args(argc, argv);

  // Init the xApp
  init_xapp_api(&args);
  sleep(4);

  e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
  assert(nodes.len > 0);
  g_nodes = &nodes;
  printf("[KPM CCC]: Connected E2 nodes = %d\n", nodes.len);
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

  sleep(1000);

  // ////////////
  // // START CCC
  // ////////////
  // int const CCC_ran_function = 4;

  // for (size_t i = 0; i < nodes.len; ++i) {
  //   e2_node_connected_xapp_t* n = &nodes.n[i];

  //   size_t const idx = find_sm_idx(n->rf, n->len_rf, eq_sm, CCC_ran_function);
  //   assert(n->rf[idx].defn.type == CCC_RAN_FUNC_DEF_E && "CCC is not the received RAN Function");
    
  //   // Send CONTROL message
  //   ccc_ctrl_req_data_t ccc_ctrl_req = {0};
  //   char* antenna_mask = "1010";//new antenna mask
  //   char* old_antenna_mask = GetAntennaMask(old_ports_on_count);
  //   char* ran_configuration_structure_name = "O-NESPolicy";
    
  //   // Generate CCC CONTROL message
  //   ccc_ctrl_req.hdr = gen_ccc_hdr(FORMAT_1_E2SM_CCC_CTRL_HDR, CCC_CTRL_SERVICE_STYLE_TYPE_2);
  //   ccc_ctrl_req.msg = gen_ccc_ctrl_msg(FORMAT_2_E2SM_CCC_CTRL_MSG, ran_configuration_structure_name, antenna_mask, old_antenna_mask);

  //   int64_t st = time_now_us();
  //   printf("[xApp]: Send RF Reconfiguration Control message to mask the antenna to %s for cell\n", antenna_mask);
    
  //   for(size_t j = 0; j < nodes.len; ++j){
  //     control_sm_xapp_api(&nodes.n[j].id, SM_CCC_ID, &ccc_ctrl_req);
  //   }
    
  //   printf("[xApp]: Control Loop Latency for the first control message: %ld us\n", time_now_us() - st);
  //   sleep(50);
  //   free_ccc_ctrl_req_data(&ccc_ctrl_req);
  // }
  // ////////////
  // // END CCC
  // ////////////
 
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
  g_nodes = NULL;
  free_e2_node_arr_xapp(&nodes);
  pthread_mutex_destroy(&mtx);
  printf("[CCC]: Test xApp run SUCCESSFULLY\n");
  return 0;
}

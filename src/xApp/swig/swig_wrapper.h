#ifndef SWIG_WRAPPER_H
#define SWIG_WRAPPER_H 

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "../../lib/e2ap/e2ap_global_node_id_wrapper.h"
#include "../../lib/e2ap/e2ap_plmn_wrapper.h"
#include "../../lib/e2ap/e2ap_ran_function_wrapper.h"

#include "../../sm/mac_sm/ie/mac_data_ie.h"
#include "../../sm/rlc_sm/ie/rlc_data_ie.h"
#include "../../sm/pdcp_sm/ie/pdcp_data_ie.h"
#include "../../sm/slice_sm/ie/slice_data_ie.h"
#include "../../sm/gtp_sm/ie/gtp_data_ie.h"

//////////////////////////////////////
// General    
/////////////////////////////////////

struct E2Node {
  global_e2_node_id_t id;
  std::vector<ran_function_t> ran_func;
};

void init(void); 

void xapp_wait(void);

bool try_stop(void);

std::vector<E2Node> conn_e2_nodes(void);

enum class Interval {
  ms_1,
  ms_2,
  ms_5,
  ms_10,
};

//////////////////////////////////////
// MAC SM   
/////////////////////////////////////

struct swig_mac_ind_msg_t{
  std::vector<mac_ue_stats_impl_t> ue_stats;
  int64_t tstamp;
};

struct mac_cb {
    virtual void handle(swig_mac_ind_msg_t* a) = 0;
    virtual ~mac_cb() {}
};

int report_mac_sm(global_e2_node_id_t* id, Interval inter, mac_cb* handler);

void rm_report_mac_sm(int);

//////////////////////////////////////
// RLC SM   
/////////////////////////////////////

struct swig_rlc_ind_msg_t{
  std::vector<rlc_radio_bearer_stats_t> rb_stats; 
  int64_t tstamp;
};

struct rlc_cb {
    virtual void handle(swig_rlc_ind_msg_t* a) = 0;
    virtual ~rlc_cb() {}
};

int report_rlc_sm(global_e2_node_id_t* id, Interval inter, rlc_cb* handler);

void rm_report_rlc_sm(int);

//////////////////////////////////////
// PDCP SM   
/////////////////////////////////////

struct swig_pdcp_ind_msg_t{
  std::vector<pdcp_radio_bearer_stats_t> rb_stats;
  int64_t tstamp;
};

struct pdcp_cb {
    virtual void handle(swig_pdcp_ind_msg_t* a) = 0;
    virtual ~pdcp_cb() {}
};

int report_pdcp_sm(global_e2_node_id_t* id, Interval inter, pdcp_cb* handler);

void rm_report_pdcp_sm(int);

//////////////////////////////////////
// SLICE SM   
/////////////////////////////////////

typedef struct{
    uint32_t id;

    uint32_t len_label;
    std::vector<std::string> label;

    uint32_t len_sched;
    std::vector<std::string> sched;

    slice_params_t params;
} swig_fr_slice_t ;

typedef struct{
    uint32_t len_slices;
    std::vector<swig_fr_slice_t> slices;

    uint32_t len_sched_name;
    std::vector<std::string> sched_name;
} swig_ul_dl_slice_conf_t ;

typedef struct{
    swig_ul_dl_slice_conf_t dl;
    swig_ul_dl_slice_conf_t ul;
} swig_slice_conf_t ;

typedef struct{
    uint32_t len_ue_slice;
    std::vector<ue_slice_assoc_t> ues;
} swig_ue_slice_conf_t;

struct swig_slice_ind_msg_t{
  swig_slice_conf_t slice_stats;
  swig_ue_slice_conf_t ue_slice_stats;
  int64_t tstamp;
};

struct slice_cb {
    virtual void handle(swig_slice_ind_msg_t* a) = 0;
    virtual ~slice_cb() {}
};

int report_slice_sm(global_e2_node_id_t* id, Interval inter, slice_cb* handler);

void rm_report_slice_sm(int);

void control_slice_sm(global_e2_node_id_t* id, slice_ctrl_msg_t* ctrl);

//////////////////////////////////////
// GTP SM   
/////////////////////////////////////

struct swig_gtp_ind_msg_t{
  std::vector<gtp_ngu_t_stats_t> gtp_stats; 
  int64_t tstamp;
};

struct gtp_cb {
    virtual void handle(swig_gtp_ind_msg_t* a) = 0;
    virtual ~gtp_cb() {}
};

int report_gtp_sm(global_e2_node_id_t* id, Interval inter, gtp_cb* handler);

void rm_report_gtp_sm(int);

//////////////////////////////////////
// DAPP SM
/////////////////////////////////////

struct swig_dapp_e3_data_ind_t {
  uint32_t ran_function_id;
  uint32_t dapp_id;
  uint8_t node_type;
  uint8_t plmn_0, plmn_1, plmn_2;
  uint32_t node_nb_id;
  bool node_cu_du_id_present;
  uint64_t node_cu_du_id;
  int64_t timestamp_ns;
  int64_t sequence_id;
  std::vector<int> prbs;
};

struct swig_dapp_sub_item_t {
  uint32_t dapp_id;
  std::vector<int> e3_ran_functions;
};

struct swig_dapp_sub_map_ind_t {
  uint8_t node_type;
  uint8_t plmn_0, plmn_1, plmn_2;
  uint32_t node_nb_id;
  bool node_cu_du_id_present;
  uint64_t node_cu_du_id;
  int64_t timestamp_ns;
  int64_t sequence_id;
  std::vector<swig_dapp_sub_item_t> dapp_subs;
};

struct dapp_e3_data_cb {
  virtual void handle(swig_dapp_e3_data_ind_t* a) = 0;
  virtual ~dapp_e3_data_cb()
  {
  }
};

struct dapp_sub_map_cb {
  virtual void handle(swig_dapp_sub_map_ind_t* a) = 0;
  virtual ~dapp_sub_map_cb()
  {
  }
};

void print_dapp_ran_func_info(global_e2_node_id_t* id);

int report_dapp_e3_data_sm(global_e2_node_id_t* id, dapp_e3_data_cb* handler);
void rm_report_dapp_e3_data_sm(int handle);

int report_dapp_sub_map_sm(global_e2_node_id_t* id, dapp_sub_map_cb* handler);
void rm_report_dapp_sub_map_sm(int handle);

void control_dapp_spectrum_sm(global_e2_node_id_t* id, uint32_t ran_func_id, uint32_t dapp_id, const std::vector<uint16_t>& blocked_prbs);

#endif

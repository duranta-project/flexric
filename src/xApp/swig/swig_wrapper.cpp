//#include "FlexRIC.h"
#include "swig_wrapper.h"

#ifdef XAPP_LANG_PYTHON
#include "Python.h"
#endif

#include "../e42_xapp_api.h"

#include "../../sm/mac_sm/mac_sm_id.h"
#include "../../sm/rlc_sm/rlc_sm_id.h"
#include "../../sm/pdcp_sm/pdcp_sm_id.h"
#include "../../sm/gtp_sm/gtp_sm_id.h"
#include "../../sm/slice_sm/slice_sm_id.h"
#include "../../util/conf_file.h"


#include <arpa/inet.h>
#include <cassert>
#include <ctime>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <iostream>

static
bool initialized = false;


static
const char* convert_period(Interval  inter_arg)
{
  if(inter_arg == Interval::ms_1 ){
    return "1_ms";
  } else if (inter_arg == Interval::ms_2) {
    return "2_ms";
  } else if(inter_arg == Interval::ms_5) {
    return "5_ms";
  } else if(inter_arg == Interval::ms_10) {
    return "10_ms";
  } else {
    assert(0 != 0 && "Unknown type");
  }

}

void init()
{
  assert(initialized == false && "Already initialized!");

  int const argc = 1;
  char** argv = NULL;
  fr_args_t args = init_fr_args(argc, argv);

  initialized = true; 

  init_xapp_api(&args);
}

void xapp_wait(void)
{
  return xapp_wait_end_api();
}

bool try_stop()
{
  return try_stop_xapp_api();
}

std::vector<E2Node> conn_e2_nodes(void)
{
  e2_node_arr_xapp_t arr = e2_nodes_xapp_api();

  std::vector<E2Node> x; //(arr.len);

  for(int i = 0; i < arr.len; ++i){

    E2Node tmp;

    e2_node_connected_xapp_t const* src = &arr.n[i];
    tmp.id = cp_global_e2_node_id(&src->id); 

    std::vector<ran_function_t> ran_func;//(src->len_rf);

    for(size_t j = 0; j < src->len_rf; ++j){
      ran_function_t rf = {.id = src->rf[i].id, .rev = src->rf[i].rev }; //cp_ran_function(&src->ack_rf[j]);
      ran_func.push_back(rf);// [j] = rf;
    }
    tmp.ran_func = ran_func;
    x.push_back(tmp);//[i] = tmp;
  }

  free_e2_node_arr_xapp(&arr);

  return x;
}

static 
mac_cb* hndlr_mac_cb; 

static
void sm_cb_mac(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == MAC_STATS_V0);
  assert(hndlr_mac_cb != NULL);

  mac_ind_data_t const* data = &rd->ind.mac; 

  swig_mac_ind_msg_t ind;
  ind.tstamp = data->msg.tstamp;

  for(uint32_t i = 0; i < data->msg.len_ue_stats; ++i){
      mac_ue_stats_impl_t tmp = cp_mac_ue_stats_impl(&data->msg.ue_stats[i]) ;
      ind.ue_stats.emplace_back(tmp);
  }

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

    hndlr_mac_cb->handle(&ind);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

}

int report_mac_sm(global_e2_node_id_t* id, Interval inter_arg, mac_cb* handler)
{
  assert(id != NULL);
  assert(handler != NULL);

  hndlr_mac_cb = handler;

  const char* period = convert_period(inter_arg);
  
  sm_ans_xapp_t ans = report_sm_xapp_api(id, SM_MAC_ID, (void*)period, sm_cb_mac);
  assert(ans.success == true); 
  return ans.u.handle;
}


void rm_report_mac_sm(int handle)
{

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

//  assert(hndlr_mac_ans.u.handle != 0);
  rm_report_sm_xapp_api(handle);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

}


//////////////////////////////////////
// RLC SM   
/////////////////////////////////////

//static
//pthread_t t_rlc;

static 
rlc_cb* hndlr_rlc_cb; 

static
void sm_cb_rlc(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == RLC_STATS_V0);
  assert(hndlr_rlc_cb != NULL);

  rlc_ind_data_t const* data = &rd->ind.rlc; 

  swig_rlc_ind_msg_t ind;
  ind.tstamp = data->msg.tstamp;

  for(uint32_t i = 0; i < data->msg.len; ++i){
    rlc_radio_bearer_stats_t tmp = data->msg.rb[i];
    ind.rb_stats.push_back(tmp);
  }

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

    hndlr_rlc_cb->handle(&ind);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

}

int report_rlc_sm(global_e2_node_id_t* id, Interval inter_arg, rlc_cb* handler)
{

  assert(id != NULL);
  assert(handler != NULL);

  hndlr_rlc_cb = handler;

  const char* period = convert_period(inter_arg);

  sm_ans_xapp_t ans = report_sm_xapp_api(id, SM_RLC_ID, (void*)period, sm_cb_rlc);
  assert(ans.success == true); 
  return ans.u.handle;
}

void rm_report_rlc_sm(int handler)
{

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

  rm_report_sm_xapp_api(handler);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

}



//////////////////////////////////////
// PDCP 
/////////////////////////////////////

static 
pdcp_cb* hndlr_pdcp_cb; 

static
void sm_cb_pdcp(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == PDCP_STATS_V0);
  assert(hndlr_pdcp_cb != NULL);

  pdcp_ind_data_t const* data = &rd->ind.pdcp; 

  swig_pdcp_ind_msg_t ind;
  ind.tstamp = data->msg.tstamp;

  for(uint32_t i = 0; i < data->msg.len; ++i){
    pdcp_radio_bearer_stats_t tmp = data->msg.rb[i];
    ind.rb_stats.push_back(tmp);
  }

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

    hndlr_pdcp_cb->handle(&ind);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

}

int report_pdcp_sm(global_e2_node_id_t* id, Interval inter_arg, pdcp_cb* handler)
{
  assert(id != NULL);
  assert(handler != NULL);

  hndlr_pdcp_cb = handler;

  const char* period = convert_period(inter_arg);
  sm_ans_xapp_t ans = report_sm_xapp_api(id , SM_PDCP_ID, (void*)period, sm_cb_pdcp);
  assert(ans.success == true); 
  return ans.u.handle;
}

void rm_report_pdcp_sm(int handler)
{

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

  rm_report_sm_xapp_api(handler);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

}

//////////////////////////////////////
// SLICE Indication & Control
/////////////////////////////////////

static
slice_cb* hndlr_slice_cb;

static
void sm_cb_slice(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == SLICE_STATS_V0);
  assert(hndlr_slice_cb != NULL);

  slice_ind_data_t const* data = &rd->ind.slice;

  swig_slice_ind_msg_t ind;
  ind.tstamp = data->msg.tstamp;


  ind.slice_stats.dl.len_slices = data->msg.slice_conf.dl.len_slices;
  ind.slice_stats.dl.sched_name.push_back(data->msg.slice_conf.dl.sched_name);
  for (size_t i = 0; i < ind.slice_stats.dl.len_slices; ++i) {
    swig_fr_slice_t tmp;
    tmp.id = data->msg.slice_conf.dl.slices[i].id;
    tmp.label.push_back(data->msg.slice_conf.dl.slices[i].label);
    tmp.sched.push_back(data->msg.slice_conf.dl.slices[i].sched);
    tmp.params = data->msg.slice_conf.dl.slices[i].params;
    ind.slice_stats.dl.slices.emplace_back(tmp);
  }

  ind.ue_slice_stats.len_ue_slice = data->msg.ue_slice_conf.len_ue_slice;
  for (size_t i = 0; i < ind.ue_slice_stats.len_ue_slice; ++i) {
    ue_slice_assoc_t tmp_ue;
    tmp_ue.rnti = data->msg.ue_slice_conf.ues[i].rnti;
    tmp_ue.dl_id = data->msg.ue_slice_conf.ues[i].dl_id;
    tmp_ue.ul_id = data->msg.ue_slice_conf.ues[i].ul_id;
    ind.ue_slice_stats.ues.emplace_back(tmp_ue);
  }

#ifdef XAPP_LANG_PYTHON
  PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

  hndlr_slice_cb->handle(&ind);

#ifdef XAPP_LANG_PYTHON
  PyGILState_Release(gstate);
#endif

}

int report_slice_sm(global_e2_node_id_t* id, Interval inter_arg, slice_cb* handler)
{
  assert( id != NULL);
  (void)inter_arg;
  assert(handler != NULL);

  hndlr_slice_cb = handler;

  const char* period = convert_period(inter_arg);
  sm_ans_xapp_t ans = report_sm_xapp_api(id, SM_SLICE_ID, (void*)period, sm_cb_slice);
  assert(ans.success == true);
  return ans.u.handle;
}

void rm_report_slice_sm(int handler)
{

#ifdef XAPP_LANG_PYTHON
  PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

  rm_report_sm_xapp_api(handler);

#ifdef XAPP_LANG_PYTHON
  PyGILState_Release(gstate);
#endif

}

void control_slice_sm(global_e2_node_id_t* id, slice_ctrl_msg_t* ctrl)
{
  assert(id != NULL);
  assert(ctrl != NULL);

  if(ctrl->type == SLICE_CTRL_SM_V0_ADD){
    slice_conf_t* s_conf = &ctrl->u.add_mod_slice;
    assert(s_conf->dl.sched_name != NULL);

    if (s_conf->dl.len_slices == 0)
      std::cout << "RESET DL SLICE, algo = NONE" << '\n';
    for(size_t i =0; i < s_conf->dl.len_slices; ++i) {
      fr_slice_t *s = &s_conf->dl.slices[i];
      assert(s->len_sched != 0);
      assert(s->sched != NULL);
      slice_params_t *p = &s->params;
      if (p->type == SLICE_ALG_SM_V0_STATIC) {
        static_slice_t *sta_sli = &p->u.sta;
        std::cout << "ADD STATIC DL SLICE: id " << s->id <<
                  ", label " << s->label <<
                  ", pos_low " << sta_sli->pos_low <<
                  ", pos_high " << sta_sli->pos_high << '\n';
      } else if (p->type == SLICE_ALG_SM_V0_NVS) {
        nvs_slice_t *nvs_sli = &p->u.nvs;
        if (nvs_sli->conf == SLICE_SM_NVS_V0_RATE)
          std::cout << "ADD NVS DL SLICE: id " << s->id <<
                    ", label " << s->label <<
                    ", conf " << nvs_sli->conf << "(rate)" <<
                    ", mbps_required " << nvs_sli->u.rate.u1.mbps_required <<
                    ", mbps_reference " << nvs_sli->u.rate.u2.mbps_reference << '\n';
        else if (nvs_sli->conf == SLICE_SM_NVS_V0_CAPACITY)
          std::cout << "ADD NVS DL SLICE: id " << s->id <<
                    ", label " << s->label <<
                    ", conf " << nvs_sli->conf << "(capacity)" <<
                    ", pct_reserved " << nvs_sli->u.capacity.u.pct_reserved << '\n';
        else assert(0 != 0 && "Unknow NVS conf");
      } else if (p->type == SLICE_ALG_SM_V0_EDF) {
        edf_slice_t *edf_sli = &p->u.edf;
        std::cout << "ADD EDF DL SLICE: id " << s->id <<
                  ", label " << s->label <<
                  ", deadline " << edf_sli->deadline <<
                  ", guaranteed_prbs " << edf_sli->guaranteed_prbs <<
                  ", max_replenish " << edf_sli->max_replenish << '\n';
      } else assert(0 != 0 && "Unknow slice algo type");
    }
  } else if(ctrl->type == SLICE_CTRL_SM_V0_UE_SLICE_ASSOC){
    for (size_t i = 0; i <  ctrl->u.ue_slice.len_ue_slice; ++i)
      std::cout << "ASSOC DL SLICE: rnti " << std::hex << ctrl->u.ue_slice.ues[i].rnti <<
                ", to slice id " << ctrl->u.ue_slice.ues[i].dl_id << '\n';
  } else if (ctrl->type == SLICE_CTRL_SM_V0_DEL) {
    del_slice_conf_t* d_conf = &ctrl->u.del_slice;
    for (size_t i = 0; i <  d_conf->len_dl; ++i)
      std::cout << "DEL DL SLICE: id " << d_conf->dl[i] << '\n';
    // TODO: UL

  } else {
    assert(0!=0 && "not foreseen case");
  }

  //sm_ag_if_wr_t wr;
  //wr.type = CONTROL_SM_AG_IF_WR;
  //wr.ctrl.type = SLICE_CTRL_REQ_V0;
  //wr.ctrl.slice_req_ctrl.msg = 
  slice_ctrl_req_data_t cp = {.msg = cp_slice_ctrl_msg(ctrl)};  
  control_sm_xapp_api(id, SM_SLICE_ID, &cp);
}

//////////////////////////////////////
// GTP SM   
/////////////////////////////////////

static 
gtp_cb* hndlr_gtp_cb; 

static
void sm_cb_gtp(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == GTP_STATS_V0);
  assert(hndlr_gtp_cb != NULL);

  gtp_ind_data_t const* data = &rd->ind.gtp; 

  swig_gtp_ind_msg_t ind;
  ind.tstamp = data->msg.tstamp;

  for(uint32_t i = 0; i < data->msg.len; ++i){
    gtp_ngu_t_stats_t tmp = data->msg.ngut[i];
    ind.gtp_stats.push_back(tmp);
  }

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

    hndlr_gtp_cb->handle(&ind);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif

}

int report_gtp_sm(global_e2_node_id_t* id, Interval inter_arg, gtp_cb* handler)
{
  assert(id != NULL);
  assert(handler != NULL);

  hndlr_gtp_cb = handler;

  const char* period = convert_period(inter_arg);
  sm_ans_xapp_t ans = report_sm_xapp_api(id, SM_GTP_ID, (void*)period, sm_cb_gtp);
  assert(ans.success == true); 
  return ans.u.handle;
}

void rm_report_gtp_sm(int handler)
{

#ifdef XAPP_LANG_PYTHON
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
#endif

  rm_report_sm_xapp_api(handler);

#ifdef XAPP_LANG_PYTHON
    PyGILState_Release(gstate);
#endif
}

//////////////////////////////////////
// DAPP SM (Spectrum)
/////////////////////////////////////

#include "../../sm/dapp_sm/dapp_sm_id.h"
#include "../../sm/dapp_sm/ie/dapp_data_ie.h"
#include "../../sm/dapp_sm/e3/service_models/spectrum_sm/ir/spectrum_report.h"
#include "../../sm/dapp_sm/e3/service_models/spectrum_sm/ir/spectrum_control.h"
#include "../../util/time_now_us.h"
#include "../e2_node_connected_xapp.h"
#include "../sm_ran_function_def.h"

// ---- print_dapp_ran_func_info: mirrors print_dapp_sm_info from xapp_spectrum.c ----

static void _print_dapp_e3_subs(const dapp_e3_subscription_list_t* subs, const char* indent)
{
  if (subs == NULL || subs->sz_dapp_e3_subscriptions == 0 || subs->dapp_e3_subscriptions == NULL) {
    printf("%sdApp E3 subscriptions: none\n", indent);
    return;
  }
  printf("%sdApp E3 subscriptions (%zu):\n", indent, subs->sz_dapp_e3_subscriptions);
  for (size_t i = 0; i < subs->sz_dapp_e3_subscriptions; ++i) {
    const dapp_e3_subscription_item_t* sub = &subs->dapp_e3_subscriptions[i];
    printf("%s  - dApp %u: subscribed to %zu E3 RAN function(s):", indent, sub->dapp_id, sub->sz_subscribed_e3_ran_functions);
    for (size_t j = 0; j < sub->sz_subscribed_e3_ran_functions; ++j)
      printf(" %u", sub->subscribed_e3_ran_functions[j]);
    printf("\n");
  }
}

void print_dapp_ran_func_info(global_e2_node_id_t* id)
{
  (void)id; // currently prints all nodes; future: filter by id
  e2_node_arr_xapp_t arr = e2_nodes_xapp_api();

  for (int i = 0; i < arr.len; ++i) {
    e2_node_connected_xapp_t* n = &arr.n[i];

    for (size_t j = 0; j < n->len_rf; ++j) {
      sm_ran_function_t* rf = &n->rf[j];
      if (rf->id != SM_DAPP_ID)
        continue;
      if (rf->defn.type != DAPP_RAN_FUNC_DEF_E)
        continue;

      const e2sm_dapp_func_def_t* d = &rf->defn.dapp;

      printf("[DAPP xApp] RAN function @node=%d idx=%zu\n", i, j);
      printf("  SM ID: %d\n", rf->id);

      if (d->name.name.buf && d->name.name.len > 0)
        printf("  Name: %.*s\n", (int)d->name.name.len, (const char*)d->name.name.buf);

      if (d->name.description.buf && d->name.description.len > 0)
        printf("  Description: %.*s\n", (int)d->name.description.len, (const char*)d->name.description.buf);

      if (d->name.oid.buf && d->name.oid.len > 0)
        printf("  OID: %.*s\n", (int)d->name.oid.len, (const char*)d->name.oid.buf);

      printf("  Event trigger: %s\n", d->ev_trig ? "present" : "none");

      if (d->report && d->report->sz_seq_report_sty > 0 && d->report->seq_report_sty) {
        printf("  Report styles (%zu):\n", d->report->sz_seq_report_sty);
        for (size_t k = 0; k < d->report->sz_seq_report_sty; ++k) {
          const seq_report_sty_dapp_sm_t* s = &d->report->seq_report_sty[k];
          printf("    - report_type=%u, name=%.*s, ind_hdr_type=%u, ind_msg_type=%u\n",
                 s->report_type,
                 (int)s->name.len,
                 (const char*)s->name.buf,
                 s->ind_hdr_type,
                 s->ind_msg_type);
          _print_dapp_e3_subs(s->dapp_e3_subs, "      ");
        }
      } else {
        printf("  Report styles: none\n");
      }

      if (d->ctrl && d->ctrl->sz_seq_ctrl_style > 0 && d->ctrl->seq_ctrl_style) {
        printf("  Control styles (%zu):\n", d->ctrl->sz_seq_ctrl_style);
        for (size_t k = 0; k < d->ctrl->sz_seq_ctrl_style; ++k) {
          const seq_ctrl_style_dapp_sm_t* s = &d->ctrl->seq_ctrl_style[k];
          printf("    - style_type=%u, name=%.*s, hdr_type=%u, msg_type=%u, out_frmt_type=%u\n",
                 s->style_type,
                 (int)s->name.len,
                 (const char*)s->name.buf,
                 s->hdr,
                 s->msg,
                 s->out_frmt);
          _print_dapp_e3_subs(s->dapp_e3_subs, "      ");
        }
      } else {
        printf("  Control styles: none\n");
      }
    }
  }

  free_e2_node_arr_xapp(&arr);
}

// ---- Style 1: E3 data report (Format 1 indication) ----

static dapp_e3_data_cb* hndlr_dapp_e3_data_cb;

static void sm_cb_dapp_frmt1(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == DAPP_STATS_V0);
  assert(hndlr_dapp_e3_data_cb != NULL);

  const e2sm_dapp_ind_hdr_t* hdr = &rd->ind.dapp.ind.hdr;
  const dapp_e3_ind_payload_t* e3 = &rd->ind.dapp.ind.e3;

  if (hdr->format != FORMAT_1_E2SM_DAPP_IND_HDR)
    return;

  swig_dapp_e3_data_ind_t ind;

  const e2sm_dapp_ind_hdr_frmt_1_t* h1 = &hdr->frmt_1;
  ind.ran_function_id = h1->ran_function_id;
  ind.dapp_id = h1->dapp_id;
  ind.node_type = h1->node_type;
  ind.plmn_0 = h1->node_plmn_id[0];
  ind.plmn_1 = h1->node_plmn_id[1];
  ind.plmn_2 = h1->node_plmn_id[2];
  ind.node_nb_id = h1->node_nb_id;
  ind.node_cu_du_id_present = h1->node_cu_du_id_present;
  ind.node_cu_du_id = h1->node_cu_du_id;
  ind.timestamp_ns = h1->timestamp_ns;
  ind.sequence_id = h1->sequence_id;

  if (e3->type == DAPP_E3_SM_SPECTRUM) {
    const spectrum_sm_report_t* rep = &e3->u.spectrum;
    for (long i = 0; i < rep->prb_count; ++i)
      ind.prbs.push_back((int)rep->prbs[i]);
  }

#ifdef XAPP_LANG_PYTHON
  PyGILState_STATE gstate = PyGILState_Ensure();
#endif

  hndlr_dapp_e3_data_cb->handle(&ind);

#ifdef XAPP_LANG_PYTHON
  PyGILState_Release(gstate);
#endif
}

int report_dapp_e3_data_sm(global_e2_node_id_t* id, dapp_e3_data_cb* handler)
{
  assert(id != NULL);
  assert(handler != NULL);

  hndlr_dapp_e3_data_cb = handler;

  dapp_sub_data_t sub = {};
  sub.et.format = FORMAT_1_E2SM_DAPP_EV_TRIGGER_FORMAT;
  sub.action_def = (e2sm_dapp_action_def_t*)calloc(1, sizeof(e2sm_dapp_action_def_t));
  assert(sub.action_def != NULL);
  sub.action_def->format = FORMAT_1_E2SM_DAPP_ACTION_DEF;
  sub.action_def->ric_style_type = 1; // DAPP_STYLE_E3_DATA

  sm_ans_xapp_t ans = report_sm_xapp_api(id, SM_DAPP_ID, &sub, sm_cb_dapp_frmt1);
  assert(ans.success == true);

  free_dapp_sub_data(&sub);

  return ans.u.handle;
}

void rm_report_dapp_e3_data_sm(int handle)
{
#ifdef XAPP_LANG_PYTHON
  PyGILState_STATE gstate = PyGILState_Ensure();
#endif

  rm_report_sm_xapp_api(handle);

#ifdef XAPP_LANG_PYTHON
  PyGILState_Release(gstate);
#endif
}

// ---- Style 2: subscription map (Format 2 indication) ----

static dapp_sub_map_cb* hndlr_dapp_sub_map_cb;

static void sm_cb_dapp_frmt2(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == DAPP_STATS_V0);
  assert(hndlr_dapp_sub_map_cb != NULL);

  const e2sm_dapp_ind_hdr_t* hdr = &rd->ind.dapp.ind.hdr;
  const e2sm_dapp_ind_msg_t* msg = &rd->ind.dapp.ind.msg;

  if (hdr->format != FORMAT_2_E2SM_DAPP_IND_HDR)
    return;

  swig_dapp_sub_map_ind_t ind;

  const e2sm_dapp_ind_hdr_frmt_2_t* h2 = &hdr->frmt_2;
  ind.node_type = h2->node_type;
  ind.plmn_0 = h2->node_plmn_id[0];
  ind.plmn_1 = h2->node_plmn_id[1];
  ind.plmn_2 = h2->node_plmn_id[2];
  ind.node_nb_id = h2->node_nb_id;
  ind.timestamp_ns = h2->timestamp_ns;
  ind.sequence_id = h2->sequence_id;
  ind.node_cu_du_id_present = h2->node_cu_du_id_present;
  ind.node_cu_du_id = h2->node_cu_du_id;

  const e2sm_dapp_ind_msg_frmt_2_t* m2 = &msg->frmt_2;
  const dapp_e3_subscription_list_t* subs = &m2->dapp_e3_subs;

  for (size_t i = 0; i < subs->sz_dapp_e3_subscriptions; ++i) {
    const dapp_e3_subscription_item_t* item = &subs->dapp_e3_subscriptions[i];
    swig_dapp_sub_item_t si;
    si.dapp_id = item->dapp_id;
    for (size_t j = 0; j < item->sz_subscribed_e3_ran_functions; ++j)
      si.e3_ran_functions.push_back((int)item->subscribed_e3_ran_functions[j]);
    ind.dapp_subs.push_back(si);
  }

#ifdef XAPP_LANG_PYTHON
  PyGILState_STATE gstate = PyGILState_Ensure();
#endif

  hndlr_dapp_sub_map_cb->handle(&ind);

#ifdef XAPP_LANG_PYTHON
  PyGILState_Release(gstate);
#endif
}

int report_dapp_sub_map_sm(global_e2_node_id_t* id, dapp_sub_map_cb* handler)
{
  assert(id != NULL);
  assert(handler != NULL);

  hndlr_dapp_sub_map_cb = handler;

  dapp_sub_data_t sub = {};
  sub.et.format = FORMAT_1_E2SM_DAPP_EV_TRIGGER_FORMAT;
  sub.action_def = (e2sm_dapp_action_def_t*)calloc(1, sizeof(e2sm_dapp_action_def_t));
  assert(sub.action_def != NULL);
  sub.action_def->format = FORMAT_1_E2SM_DAPP_ACTION_DEF;
  sub.action_def->ric_style_type = 2; // DAPP_STYLE_SUBSCRIPTION_MAP

  sm_ans_xapp_t ans = report_sm_xapp_api(id, SM_DAPP_ID, &sub, sm_cb_dapp_frmt2);
  assert(ans.success == true);

  free_dapp_sub_data(&sub);

  return ans.u.handle;
}

void rm_report_dapp_sub_map_sm(int handle)
{
#ifdef XAPP_LANG_PYTHON
  PyGILState_STATE gstate = PyGILState_Ensure();
#endif

  rm_report_sm_xapp_api(handle);

#ifdef XAPP_LANG_PYTHON
  PyGILState_Release(gstate);
#endif
}

// ---- Control ----

/* sequence_id is a wire-visible correlation id. Starts at 1 -- 0 is the
 * E2SM-DAPP "no id assigned" sentinel. Mirrors
 * examples/xApp/c/spectrum/xapp_spectrum.c's next_sequence_id(); this is the
 * Python-binding path's own counter, not shared with that example. */
static uint32_t next_sequence_id() {
  static uint32_t counter;
  return ++counter;
}

void control_dapp_spectrum_sm(global_e2_node_id_t* id, uint32_t ran_func_id, uint32_t dapp_id, const std::vector<uint16_t>& blocked_prbs)
{
  assert(id != NULL);

  dapp_ctrl_req_data_t ctrl = {};
  ctrl.hdr.format = FORMAT_1_E2SM_DAPP_CTRL_HDR;
  ctrl.hdr.frmt_1.ran_function_id = ran_func_id;
  ctrl.hdr.frmt_1.dapp_id = dapp_id;
  ctrl.hdr.frmt_1.timestamp_ns = time_now_ns();
  ctrl.hdr.frmt_1.sequence_id = next_sequence_id();

  ctrl.msg.format = FORMAT_1_E2SM_DAPP_CTRL_MSG;
  ctrl.msg.frmt_1.data = NULL;
  ctrl.msg.frmt_1.data_size = 0;

  ctrl.e3.type = DAPP_E3_SM_SPECTRUM;
  ctrl.e3.u.spectrum.timestamp_ns = time_now_ns();
  ctrl.e3.u.spectrum.prb_count = (long)blocked_prbs.size();
  if (!blocked_prbs.empty()) {
    ctrl.e3.u.spectrum.blockedPRBs = (uint16_t*)calloc(blocked_prbs.size(), sizeof(uint16_t));
    assert(ctrl.e3.u.spectrum.blockedPRBs != NULL);
    for (size_t i = 0; i < blocked_prbs.size(); ++i)
      ctrl.e3.u.spectrum.blockedPRBs[i] = blocked_prbs[i];
  }

  sm_ans_xapp_t ans = control_sm_xapp_api(id, SM_DAPP_ID, &ctrl);
  assert(ans.success == true);

  free_dapp_ctrl_req_data(&ctrl);
}

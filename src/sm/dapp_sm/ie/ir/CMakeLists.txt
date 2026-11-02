# SPDX-License-Identifier: LicenseRef-CSSL-1.0

set (IR_DAPP_SRC
  dapp_e3_subscription_item.c
  dapp_e3_subscription_list.c
  e2sm_dapp_ctrl_hdr_frmt_1.c
  e2sm_dapp_ctrl_msg_frmt_1.c
  e2sm_dapp_ctrl_out_frmt_1.c
  e2sm_dapp_ev_trg_frmt_1.c
  e2sm_dapp_action_def.c
  e2sm_dapp_ind_hdr_frmt_1.c
  e2sm_dapp_ind_hdr_frmt_2.c
  e2sm_dapp_ind_msg_frmt_1.c
  e2sm_dapp_ind_msg_frmt_2.c
  ran_func_def_ctrl.c
  ran_func_def_ev_trig.c
  ran_func_def_report.c
  seq_ctrl_style.c
  seq_report_sty.c
  )

add_library(e2sm_dapp_ir_obj OBJECT ${IR_DAPP_SRC})
target_include_directories(e2sm_dapp_ir_obj
                          PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                           )

target_compile_options(e2sm_dapp_ir_obj PRIVATE -Wno-missing-field-initializers -Wno-unused-parameter  -fPIC -fvisibility=hidden )



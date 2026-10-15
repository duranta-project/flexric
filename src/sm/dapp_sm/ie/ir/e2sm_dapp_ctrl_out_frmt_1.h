#ifndef E2SM_DAPP_CTRL_OUT_FRMT_1_H
#define E2SM_DAPP_CTRL_OUT_FRMT_1_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  /* Nanoseconds since the Unix epoch, from the realtime clock (time_now_ns). */
  int64_t timestamp_ns;

  /* Per-procedure correlation id assigned by the producer; 0 means none was
   * assigned, same absent convention as timestamp_ns. */
  int64_t sequence_id;

  /* The Service Model's own account of applying the control, encoded by the SM
   * named by the control's ran_function_id and opaque here. Owned by this
   * struct: freed by free_, deep-copied by cp_. NULL with size 0 when the
   * outcome only acknowledges. */
  uint8_t* e3_control_outcome;
  uint32_t e3_control_outcome_size;
} e2sm_dapp_ctrl_out_frmt_1_t;

void free_e2sm_dapp_ctrl_out_frmt_1(e2sm_dapp_ctrl_out_frmt_1_t* src);

e2sm_dapp_ctrl_out_frmt_1_t cp_e2sm_dapp_ctrl_out_frmt_1(e2sm_dapp_ctrl_out_frmt_1_t const* src);

bool eq_e2sm_dapp_ctrl_out_frmt_1(e2sm_dapp_ctrl_out_frmt_1_t const* m0, e2sm_dapp_ctrl_out_frmt_1_t const* m1);

#endif

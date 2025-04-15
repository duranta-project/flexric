#ifndef SM_SRS_READ_WRITE_AGENT_H
#define SM_SRS_READ_WRITE_AGENT_H

#include "../../../src/agent/e2_agent_api.h"

void init_srs_sm(void);

void free_srs_sm(void);

bool read_srs_sm(void*);

void read_srs_setup_sm(void*);

sm_ag_if_ans_t write_ctrl_srs_sm(void const*);

#endif


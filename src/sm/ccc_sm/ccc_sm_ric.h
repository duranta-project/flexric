/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */
#ifndef CCC_SERVICE_MODEL_RIC_H
#define CCC_SERVICE_MODEL_RIC_H

#include <stddef.h>
#include <stdint.h>

#include "../sm_ric.h"

__attribute__ ((visibility ("default"))) 
sm_ric_t* make_ccc_sm_ric(void);

#endif

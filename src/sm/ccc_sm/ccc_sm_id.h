/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */
#ifndef CCC_SERVICE_MODEL_ID_H
#define CCC_SERVICE_MODEL_ID_H 

#include <stdint.h>

static
const uint16_t SM_CCC_ID = 4; 

__attribute__((unused)) static
const char* SM_CCC_STR = "ORAN-E2SM-CCC"; 

static
const uint16_t SM_CCC_REV = 1; 

__attribute__((unused)) static
const char SM_CCC_SHORT_NAME[] = "E2SM-CCC";

//iso(0) identified-organization(0)
//dod(0) internet(0) private(0)
//enterprise(0) 53148 e2(0)
// version1 (1) e2sm(150) e2sm-CCC-
// IEs (0)

__attribute__((unused)) static
const char SM_CCC_OID[] = "0.0.0.0.0.0.0.0.1.150.0"; 

__attribute__((unused)) static
const char SM_CCC_DESCRIPTION[] = "CCC Service Model";

#endif

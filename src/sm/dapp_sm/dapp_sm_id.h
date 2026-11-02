/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef DAPP_SERVICE_MODEL_ID_H
#define DAPP_SERVICE_MODEL_ID_H

#include <stdint.h>

static const uint16_t SM_DAPP_ID = 255;

static const uint16_t SM_DAPP_REV = 1;

#define SM_DAPP_SHORT_NAME "E2SM-DAPP"
// iso(1) identified-organization(3)
// dod(6) internet(1) private(4)
// enterprise(1) 53148 e2(1)
//  version1 (1) e2sm(2) e2sm-DAPP-
//  IEs (3)

#define SM_DAPP_OID "1.3.6.1.4.1.53148.1.1.255.3"

#define SM_DAPP_DESCRIPTION "DAPP Service Model for E2/E3 bridge"

#endif

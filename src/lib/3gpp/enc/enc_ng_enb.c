/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include <assert.h>

#include "../../../util/conversions.h"
#include "enc_ng_enb.h"
#include "enc_global_ng_ran.h"
#include "enc_global_ng_enb.h"


// AMF UE NGAP ID: INTEGER (0..2^40-1). An INTEGER_t holds a big-endian two's
// complement value in the minimum number of octets, so values >= 2^39 need a
// leading 0x00 octet (6 octets)
static
INTEGER_t enc_amf_ue_ngap_id(uint64_t val)
{
    assert(val < (1UL << 40));

    uint8_t tmp[6] = {0};
    for (int i = 5; i >= 0; --i) {
        tmp[i] = val & 0xFF;
        val >>= 8;
    }

    // Skip leading 0x00 octets, unless needed to keep the value positive
    size_t start = 0;
    while (start < 5 && tmp[start] == 0 && (tmp[start + 1] & 0x80) == 0)
        ++start;

    INTEGER_t dst = {0};
    dst.size = 6 - start;
    dst.buf = calloc(dst.size, sizeof(uint8_t));
    assert(dst.buf != NULL && "Memory exhausted");
    memcpy(dst.buf, tmp + start, dst.size);
    return dst;
}

UEID_NG_ENB_t * enc_ng_eNB_UE_asn(const ng_enb_e2sm_t * ng_enb)
{
    UEID_NG_ENB_t * ng_enb_asn = calloc(1, sizeof(UEID_NG_ENB_t));
    assert(ng_enb_asn != NULL && "Memory exhausted");


    // AMF UE NGAP ID
    ng_enb_asn->amf_UE_NGAP_ID = enc_amf_ue_ngap_id(ng_enb->amf_ue_ngap_id);


    // GUAMI

    MCC_MNC_TO_PLMNID(ng_enb->guami.plmn_id.mcc, ng_enb->guami.plmn_id.mnc, ng_enb->guami.plmn_id.mnc_digit_len, &ng_enb_asn->guami.pLMNIdentity);

    ng_enb_asn->guami.aMFRegionID = cp_amf_region_id_to_bit_string(ng_enb->guami.amf_region_id);

    ng_enb_asn->guami.aMFSetID = cp_amf_set_id_to_bit_string(ng_enb->guami.amf_set_id);

    ng_enb_asn->guami.aMFPointer = cp_amf_ptr_to_bit_string(ng_enb->guami.amf_ptr);


    // C-if CU DU separated
    // ng-eNB-CU UE W1AP ID

    if (ng_enb->ng_enb_cu_ue_w1ap_id != NULL)
    {
        ng_enb_asn->ng_eNB_CU_UE_W1AP_ID = calloc(1, sizeof(*ng_enb_asn->ng_eNB_CU_UE_W1AP_ID));
        memcpy(ng_enb_asn->ng_eNB_CU_UE_W1AP_ID, ng_enb->ng_enb_cu_ue_w1ap_id, 4);
    }
    

    // C- ifDCSetup
    // M-NG-RAN node UE XnAP ID

    if (ng_enb->ng_ran_node_ue_xnap_id != NULL)
    {
        ng_enb_asn->m_NG_RAN_UE_XnAP_ID = calloc(1, sizeof(*ng_enb_asn->m_NG_RAN_UE_XnAP_ID));
        memcpy(ng_enb_asn->m_NG_RAN_UE_XnAP_ID, ng_enb->ng_ran_node_ue_xnap_id, 4);
    }


    // Global NG eNB
    // Optional
    if (ng_enb->global_ng_enb_id != NULL)
      ng_enb_asn->globalNgENB_ID = enc_global_ng_enb_asn(ng_enb->global_ng_enb_id);


    // Global NG-RAN Node ID
    // C-ifDCSetup
    if (ng_enb->global_ng_ran_node_id != NULL)
      ng_enb_asn->globalNG_RANNode_ID = enc_global_ng_ran_asn(ng_enb->global_ng_ran_node_id);


    return ng_enb_asn;
}

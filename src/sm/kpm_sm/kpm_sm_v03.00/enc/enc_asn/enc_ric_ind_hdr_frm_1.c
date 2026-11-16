/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "../../../../../util/conversions.h"
#include "../../../../../util/ntp_time.h"

#include "enc_ric_ind_hdr_frm_1.h"

E2SM_KPM_IndicationHeader_Format1_t * kpm_enc_ind_hdr_frm_1_asn(const kpm_ric_ind_hdr_format_1_t * ind_hdr)
{
    E2SM_KPM_IndicationHeader_Format1_t * ind_hdr_asn = calloc(1, sizeof(E2SM_KPM_IndicationHeader_Format1_t));
    assert(ind_hdr_asn != NULL && "Memory exhausted");


    // big-endian, as per RFC 5905
    const uint64_t ts = unix_us_to_ntp64(ind_hdr->collectStartTime);
    uint8_t ts_be[8];
    for (int i = 0; i < 8; ++i)
        ts_be[i] = (uint8_t)(ts >> (56 - 8 * i));
    int ret = OCTET_STRING_fromBuf(&ind_hdr_asn->colletStartTime, (char *)ts_be, sizeof(ts_be));
    assert(ret == 0);


    if (ind_hdr->fileformat_version != NULL){
        ind_hdr_asn->fileFormatversion = calloc(1, sizeof(*ind_hdr_asn->fileFormatversion));

        const size_t len = ind_hdr->fileformat_version->len;
        ret = OCTET_STRING_fromBuf(ind_hdr_asn->fileFormatversion, (char *)(ind_hdr->fileformat_version->buf), len);
        assert(ret == 0);
    }

    if (ind_hdr->sender_name != NULL){
        ind_hdr_asn->senderName = calloc(1, sizeof(*ind_hdr_asn->senderName));

        const size_t len = ind_hdr->sender_name->len;
        ret = OCTET_STRING_fromBuf(ind_hdr_asn->senderName, (char *)(ind_hdr->sender_name->buf), len);
        assert(ret == 0);
    }

    if (ind_hdr->sender_type != NULL){
        ind_hdr_asn->senderType = calloc(1, sizeof(*ind_hdr_asn->senderType));

        const size_t len = ind_hdr->sender_type->len;
        ret = OCTET_STRING_fromBuf(ind_hdr_asn->senderType, (char *)(ind_hdr->sender_type->buf), len);
        assert(ret == 0);
    }

	if (ind_hdr->vendor_name != NULL) {
        ind_hdr_asn->vendorName = calloc(1, sizeof(*ind_hdr_asn->vendorName));

        const size_t len = ind_hdr->vendor_name->len;
        ret = OCTET_STRING_fromBuf(ind_hdr_asn->vendorName, (char *)(ind_hdr->vendor_name->buf), len);
        assert(ret == 0);
    }

    return ind_hdr_asn;
}

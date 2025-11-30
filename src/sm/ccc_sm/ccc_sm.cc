/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */
#include "ccc_sm.h"
#include "e2sm_ccc.hpp"

#include <nlohmann/json.hpp>
#include <cstring>
#include <cstdlib>
#include <cstdio>

using json = nlohmann::json;

extern "C" {

/* ── Internal helper ───────────────────────────────────────────────────── */

static uint8_t* dup_str_to_buf(const std::string& s, size_t* out_len)
{
    uint8_t* buf = (uint8_t*)malloc(s.size() + 1);
    if (!buf) return nullptr;
    memcpy(buf, s.data(), s.size());
    buf[s.size()] = '\0';
    *out_len = s.size();
    return buf;
}

/* Parse JSON or fall back to verbatim copy. Returns canonical dump. */
static std::string parse_or_passthrough(const char* payload, size_t len,
                                         const char* tag)
{
    try {
        return json::parse(std::string(payload, len)).dump();
    } catch (const std::exception& e) {
        fprintf(stderr, "[CCC SM]: %s parse error – not valid JSON: %s\n",
                tag, e.what());
        return std::string(payload, len);
    }
}

/* ── Control Message ────────────────────────────────────────────────────── */
/*
 * Format 1 (node-level)  wire key: "listOfConfigurationStructures"
 * Format 2 (cell-level)  wire key: "listOfCellsControlled"
 */
uint8_t* ccc_sm_encode_ctrl_msg(const char* json_payload, size_t payload_len,
                                 size_t* out_len)
{
    if (!json_payload || payload_len == 0 || !out_len) return nullptr;
    std::string canonical = parse_or_passthrough(
            json_payload, payload_len, "ccc_sm_encode_ctrl_msg");
    return dup_str_to_buf(canonical, out_len);
}

char* ccc_sm_decode_ctrl_msg(const uint8_t* data, size_t len)
{
    if (!data || len == 0) return nullptr;
    std::string canonical = parse_or_passthrough(
            (const char*)data, len, "ccc_sm_decode_ctrl_msg");
    char* buf = (char*)malloc(canonical.size() + 1);
    if (!buf) return nullptr;
    memcpy(buf, canonical.c_str(), canonical.size() + 1);
    return buf;
}

/* ── Control Header ─────────────────────────────────────────────────────── */
/*
 * Wire key: "ricStyleType"  (from E2SmCccControlHeaderFormat1Properties)
 */
uint8_t* ccc_sm_encode_ctrl_hdr(int ric_style_type, size_t* out_len)
{
    if (!out_len) return nullptr;
    json j;
    j["ricStyleType"] = ric_style_type;
    return dup_str_to_buf(j.dump(), out_len);
}

char* ccc_sm_decode_ctrl_hdr(const uint8_t* data, size_t len)
{
    if (!data || len == 0) return nullptr;
    char* buf = (char*)malloc(len + 1);
    if (!buf) return nullptr;
    memcpy(buf, data, len);
    buf[len] = '\0';
    return buf;
}

/* ── Indication Message ─────────────────────────────────────────────────── */
/*
 * Format 1  wire key: "listOfConfigurationStructuresReported"
 * Format 2  wire key: "listOfCellsReported"
 */
uint8_t* ccc_sm_encode_ind_msg(const char* json_payload, size_t payload_len,
                                size_t* out_len)
{
    if (!json_payload || payload_len == 0 || !out_len) return nullptr;
    std::string canonical = parse_or_passthrough(
            json_payload, payload_len, "ccc_sm_encode_ind_msg");
    return dup_str_to_buf(canonical, out_len);
}

char* ccc_sm_decode_ind_msg(const uint8_t* data, size_t len)
{
    if (!data || len == 0) return nullptr;
    std::string canonical = parse_or_passthrough(
            (const char*)data, len, "ccc_sm_decode_ind_msg");
    char* buf = (char*)malloc(canonical.size() + 1);
    if (!buf) return nullptr;
    memcpy(buf, canonical.c_str(), canonical.size() + 1);
    return buf;
}

/* ── Indication Header ──────────────────────────────────────────────────── */
/*
 * Wire keys: "indicationReason", "eventTime"
 * (from E2SmCccIndicationHeaderFormat1Properties)
 */
uint8_t* ccc_sm_encode_ind_hdr(int64_t event_time, const char* indication_reason,
                                size_t* out_len)
{
    if (!out_len) return nullptr;
    json j;
    j["eventTime"] = event_time;
    if (indication_reason && indication_reason[0] != '\0')
        j["indicationReason"] = indication_reason;
    return dup_str_to_buf(j.dump(), out_len);
}

char* ccc_sm_decode_ind_hdr(const uint8_t* data, size_t len)
{
    if (!data || len == 0) return nullptr;
    char* buf = (char*)malloc(len + 1);
    if (!buf) return nullptr;
    memcpy(buf, data, len);
    buf[len] = '\0';
    return buf;
}

/* ── Legacy helpers (kept for API compatibility) ────────────────────────── */

void* ccc_sm_pack_control(void* ctrl_msg, size_t* len_out)
{
    using namespace quicktype;
    auto* msg = static_cast<E2SmCccControlMessageFormat1*>(ctrl_msg);
    json j = *msg;
    std::string payload = j.dump();
    uint8_t* buf = (uint8_t*)malloc(payload.size());
    if (!buf) { *len_out = 0; return nullptr; }
    memcpy(buf, payload.data(), payload.size());
    *len_out = payload.size();
    return buf;
}

void* ccc_sm_unpack_indication(const uint8_t* data, size_t len)
{
    using namespace quicktype;
    std::string s((const char*)data, len);
    auto msg = json::parse(s).get<E2SmCccIndicationMessageFormat1>();
    return new E2SmCccIndicationMessageFormat1(msg);
}

} // extern "C"

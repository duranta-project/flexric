#include "ccc_sm.h"
#include "e2sm_ccc.hpp"

#include <nlohmann/json.hpp>
#include <cstring>

using json = nlohmann::json;

extern "C" {

void* ccc_sm_pack_control(void* ctrl_msg, size_t* len_out)
{
    // ctrl_msg is a pointer to your CCC struct
    auto* msg = static_cast<quicktype::E2SmCccControlMessageFormat1*>(ctrl_msg);

    // Convert struct → JSON string
    json j = *msg;
    std::string payload = j.dump();  // compact JSON

    // allocate buffer for FlexRIC to send
    uint8_t* buf = (uint8_t*)malloc(payload.size());
    memcpy(buf, payload.data(), payload.size());

    *len_out = payload.size();
    return buf;  // this becomes RIC-ControlMessage OCTET STRING
}

void* ccc_sm_unpack_indication(const uint8_t* data, size_t len)
{
    // Convert raw bytes → string
    std::string s((char*)data, len);

    // JSON string → struct
    auto msg = json::parse(s)
        .get<quicktype::E2SmCccIndicationMessageFormat1>();

    // Return heap-allocated C++ struct
    return new quicktype::E2SmCccIndicationMessageFormat1(msg);
}

} // extern "C"

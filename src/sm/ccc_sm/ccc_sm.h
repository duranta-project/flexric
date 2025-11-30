#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Convert CCC Control struct → JSON -> raw buffer
void* ccc_sm_pack_control(void* ctrl_msg, size_t* len_out);

// Convert JSON -> CCC Indication struct
void* ccc_sm_unpack_indication(const uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif

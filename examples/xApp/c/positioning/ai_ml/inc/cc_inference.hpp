#ifndef CC_H
#define CC_H

#include "../../common.h"

int cc_inference(const char* torchscript_path, uint32_t cir_shifted[][N_SHIFT]);

#endif

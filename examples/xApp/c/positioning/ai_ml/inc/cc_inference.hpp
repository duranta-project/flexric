#ifndef CC_H
#define CC_H

#include "../../common.h"
#include <vector>
#include <torch/script.h>

torch::jit::script::Module load_torchscript_model(const char* torchscript_path);
int cc_inference(torch::jit::script::Module& module, uint32_t cir_shifted[][N_SHIFT], std::vector<float>& prediction);
int dummy_cc_inference(torch::jit::script::Module& module, std::vector<float>& prediction);

#endif

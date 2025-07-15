#include "../inc/cc_inference.hpp"
#include <vector>
#include <iostream>
#include <cstdio>
#include <cstdint>
#include <cstdlib>

#include <cmath>

#include <torch/torch.h>
#include <torch/script.h>
#include <memory>


#include <fstream>
#include <string>

torch::jit::script::Module load_torchscript_model(const char* torchscript_path)
{

  torch::jit::script::Module module;
  try {
    // Deserialize the ScriptModule from a file using torch::jit::load().
    module = torch::jit::load(torchscript_path);
  }
  catch (const c10::Error& e) {
    std::cerr << "error loading the model\n";
  }

  std::cout << "loading script ok\n";
  return module;
}

/* Loads Torchscript model, performs channel charting inference and writes predictions to a csv file*/
int cc_inference(torch::jit::script::Module& module, uint32_t cir_shifted[][N_SHIFT], std::vector<float>& prediction)
{
 /*
  torch::jit::script::Module module;
  try {
    // Deserialize the ScriptModule from a file using torch::jit::load().
    module = torch::jit::load(torchscript_path);
  }
  catch (const c10::Error& e) {
    std::cerr << "error loading the model\n";
    return -1;
  }

  std::cout << "loading script ok\n";
  */

  // inference, which datatype is the best, float seems to be the defacto type 
  // need to flatten first
  std::vector<float> input_cir;
  input_cir.reserve(N_rx * N_SHIFT);

  for (size_t ant = 0; ant < N_rx; ant++) {
    for (size_t i = 0; i < N_SHIFT; i++) {
      input_cir.push_back(static_cast<float>(cir_shifted[ant][i]));
    }
  }

  // create a tensor

  auto options = torch::TensorOptions().dtype(torch::kFloat32);
  torch::Tensor cir_tensor = torch::from_blob(input_cir.data(),{static_cast<long>(N_rx), static_cast<long>(N_SHIFT)}, options).clone();  // shape: [N_rx, N_SHIFT]

  cir_tensor = cir_tensor.unsqueeze(0); // add channel dimension to match (1, N_rx, N_SHIFT)
  cir_tensor = cir_tensor / NORM_FACTOR; // Normalize

  // Create binary mask
  torch::Tensor max_vals = std::get<0>(cir_tensor.max(2));
  torch::Tensor binary_mask = (max_vals > THRESHOLD).to(torch::kFloat32); // shape (1,N_rx)

  torch::Tensor masked_input = cir_tensor * binary_mask.unsqueeze(2);

  torch::Tensor input_tensor = masked_input.unsqueeze(0).to(torch::kFloat32);

  // no-grad scope
  {
    torch::NoGradGuard no_grad;  
    at::Tensor output = module.forward({input_tensor}).toTensor();

    output = output.squeeze(0);
    prediction.assign(output.data_ptr<float>(), output.data_ptr<float>() + output.numel());

    // Print predictions
    std::cout << "Predictions: [";
    for (size_t i = 0; i < prediction.size(); ++i) {
      std::cout << prediction[i];
      if (i + 1 < prediction.size()) std::cout << ", ";
    }
    std::cout << "]\n";
  /*
    // Write to a CSV file for plotting
    std::string csv_file = "cc_predictions.csv";

    std::ofstream file(csv_file, std::ios::out);
    if (file.is_open()) {
        file << "cc_x,cc_y\n";
        file.close();
    } else {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }

    float cc_x = prediction[0];
    float cc_y = prediction[1];

    // Append prediction to CSV
    std::ofstream file_append(csv_file, std::ios::app);
    if (file_append.is_open()) {
        file_append << cc_x << "," << cc_y << "\n";
        file_append.close();
    } else {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }
  */
  }

  return 0;
}

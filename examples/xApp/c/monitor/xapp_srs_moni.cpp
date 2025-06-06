#include <iostream>
#include <chrono>
#include <thread>

#include "../../../../src/xApp/e42_xapp_api.h"

int main(int argc, char *argv[])
{

  fr_args_t args = init_fr_args(argc, argv);

  // init the xApp
  init_xapp_api(&args);
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(1000ms); // wait after the xApp is initialized

  e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();

  assert(nodes.len > 0);

  std::cout << "Connected E2 nodes = " << +(nodes.len) << std::endl;

  for (int i = 0; i < nodes.len; i++) {
      e2_node_connected_xapp_t* n = &nodes.n[i];
      for (size_t j = 0; j < n->len_rf; j++) {
          std::cout << "Registered node: " << i << " ran func id = " << n->rf[j].id << std::endl;
      }
  }
  std::cout << "Hello world!" << std::endl;

  while(try_stop_xapp_api() == false)
     std::this_thread::sleep_for(1000ms);

  free_e2_node_arr_xapp(&nodes);
  std::cout << "Test C++ xApp run Successfully" << std::endl;
  return 0;
}

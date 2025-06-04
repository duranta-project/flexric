#include <iostream>
#include <chrono>
#include <thread>

#include "../../../../src/xApp/e42_xapp_api.h"

int main(int argc, char *argv[])
{
  std::cout << "hello world\n";

    fr_args_t args = init_fr_args(argc, argv);

    // init the xApp
    init_xapp_api(&args);
        using namespace std::chrono_literals;
    std::this_thread::sleep_for(2000ms); // wait after the xApp is initialized
  return 0;
}

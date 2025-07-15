find_package(Torch REQUIRED)


add_library(ml_cc
           src/proc_srs_channel.c
           src/cc_inference.cpp)

target_include_directories(ml_cc PUBLIC inc)
target_link_libraries(ml_cc PUBLIC "${TORCH_LIBRARIES}" m)

add_subdirectory(localization_plots)

# shared library loader
if(NOT DEFINED DFT_LIB_PATH)
  message(FATAL_ERROR "The path to dfts shared lib must be specified")
endif()
add_library(shlib_loader OBJECT src/load_module_shlib.c)

target_include_directories(shlib_loader PUBLIC inc)

target_compile_definitions(shlib_loader PRIVATE DFT_LIB_PATH="${DFT_LIB_PATH}")

# freq2time library
add_library(freq2time STATIC src/freq2time.c src/dfts_load.c)

target_include_directories(freq2time PUBLIC inc)

target_link_libraries(freq2time PRIVATE shlib_loader)



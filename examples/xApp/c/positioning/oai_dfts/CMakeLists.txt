# shared library loader
add_library(shlib_loader OBJECT src/load_module_shlib.c)

target_include_directories(shlib_loader PUBLIC inc)

# freq2time library
add_library(freq2time STATIC src/freq2time.c src/dfts_load.c)

target_include_directories(freq2time PUBLIC inc)

target_link_libraries(freq2time PRIVATE shlib_loader)



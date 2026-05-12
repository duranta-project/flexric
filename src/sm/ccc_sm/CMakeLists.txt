cmake_minimum_required(VERSION 3.15)
project(ccc_sm)

# Enable C++ support for ccc_sm.cc
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)

# Minimal source files for your use case
set(SM_ENCODING_CCC_SRC
    ../sm_proc_data.c 
    ../../util/byte_array.c 
    ../../util/time_now_us.c
    ../../util/alg_ds/alg/defer.c 
    ../../util/alg_ds/alg/eq_float.c 
    ../../util/alg_ds/ds/seq_container/seq_arr.c 
    ../../util/alg_ds/ds/seq_container/seq_ring.c 
    ../../util/alg_ds/ds/assoc_container/assoc_rb_tree.c
    ../../util/alg_ds/ds/assoc_container/bimap.c
    ie/ccc_data_ie.c
    dec/ccc_dec_plain.c
    enc/ccc_enc_plain.c
    ccc_sm_ric.c 
    ccc_sm_agent.c 
    ccc_sm.cc  
)

if(SM_ENCODING_CCC STREQUAL "PLAIN")
  # For now, just use basic source files
  set(SM_ENCODING_CCC_SRC_PLAIN ${SM_ENCODING_CCC_SRC})

  # Shared
  add_library(ccc_sm SHARED ${SM_ENCODING_CCC_SRC_PLAIN} )
  target_compile_options(ccc_sm PRIVATE -fPIC -fvisibility=hidden)

  # Static
  add_library(ccc_sm_static STATIC ${SM_ENCODING_CCC_SRC_PLAIN})

elseif(SM_ENCODING_CCC STREQUAL "ASN")
  message(FATAL_ERROR "CCC SM ASN not implemented")

elseif(SM_ENCODING_CCC STREQUAL "FLATBUFFERS" )
  message(FATAL_ERROR "CCC SM FB not implemented")
else()
  message(FATAL_ERROR "Unknown SM encoding type")
endif()

target_compile_definitions(ccc_sm PUBLIC ${SM_ENCODING_CCC} ${E2AP_VERSION} ${KPM_VERSION} )
target_compile_definitions(ccc_sm_static PUBLIC ${SM_ENCODING_CCC} ${E2AP_VERSION} ${KPM_VERSION} )

# SPDX-License-Identifier: LicenseRef-CSSL-1.0

cmake_minimum_required(VERSION 3.15) # setting this is required
project(dapp_sm)            # this sets the project name

set(SM_ENCODING_DAPP_SRC
  ../sm_proc_data.c 
  dapp_sm_agent.c 
  dapp_sm_ric.c 
  ../../util/byte_array.c 
  ../../util/alg_ds/alg/defer.c 
  ../../util/alg_ds/alg/eq_float.c 
  ../../util/alg_ds/ds/seq_container/seq_arr.c 
  ../../util/alg_ds/ds/seq_container/seq_ring.c 
  ../../util/alg_ds/ds/assoc_container/assoc_rb_tree.c
  ../../util/alg_ds/ds/assoc_container/bimap.c
  ie/dapp_data_ie.c 
  )

if(SM_ENCODING_DAPP STREQUAL "ASN")
  # The E2SM-DAPP ASN.1 codec lives in libe3; flexric only adapts its IR structs to it.
  find_package(libe3 0.3.0 CONFIG QUIET COMPONENTS e2sm_dapp)
  if(NOT libe3_FOUND)
    message(FATAL_ERROR "E2SM-DAPP needs libe3 >= 0.3.0 built with LIBE3_ENABLE_E2SM_DAPP; install it or set CMAKE_PREFIX_PATH")
  endif()
  # Consumers of dapp_sm_static (tests in other directories) resolve the link dependency too.
  set_target_properties(libe3::e2sm_dapp PROPERTIES IMPORTED_GLOBAL TRUE)
  # libe3_e2sm_dapp is a static C++ archive: the C++ runtime has to come with it.
  if(APPLE)
    set(DAPP_CXX_RUNTIME c++)
  else()
    set(DAPP_CXX_RUNTIME stdc++)
  endif()

  add_subdirectory(ie/ir)
  add_subdirectory(e3)

  set(SM_ENCODING_DAPP_SRC_ASN
    ${SM_ENCODING_DAPP_SRC}
    enc/dapp_enc_asn.c 
    dec/dapp_dec_asn.c 
    # This dependency sucks!
    $<TARGET_OBJECTS:e2ap_ran_func_obj>
    $<TARGET_OBJECTS:e2sm_dapp_ir_obj>
    $<TARGET_OBJECTS:e2_conv_obj>
    $<TARGET_OBJECTS:3gpp_derived_ie_obj>
    $<TARGET_OBJECTS:sm_common_ie_obj>
    $<TARGET_OBJECTS:e3sm_obj>
    )

  # Shared 
  add_library(dapp_sm SHARED ${SM_ENCODING_DAPP_SRC_ASN})

  target_include_directories(dapp_sm PRIVATE 
                                "${CMAKE_CURRENT_SOURCE_DIR}/ie"
                                "${CMAKE_CURRENT_SOURCE_DIR}/ie/ir"
                                "${CMAKE_CURRENT_SOURCE_DIR}/../../util"
                                "${CMAKE_CURRENT_SOURCE_DIR}/e3"
                              )
  target_compile_options(dapp_sm PRIVATE "-DASN_DISABLE_OER_SUPPORT" "-DASN_DISABLE_JER_SUPPORT" )
  target_compile_options(dapp_sm PRIVATE -fPIC -Wno-missing-field-initializers -Wno-unused-parameter -fvisibility=hidden)

  target_link_libraries(dapp_sm PRIVATE 
                          -lm
                          e2sm_dapp_e3_spectrum_asn1_obj
                          libe3::e2sm_dapp
                          ${DAPP_CXX_RUNTIME}
                        )
  # Keep libe3's C/C++ API and its asn1c runtime out of libdapp_sm.so's exports.
  if(APPLE)
    target_link_options(dapp_sm PRIVATE "LINKER:-load_hidden,$<TARGET_FILE:libe3::e2sm_dapp>")
  else()
    target_link_options(dapp_sm PRIVATE "LINKER:--exclude-libs,ALL")
  endif()

  # Static
  add_library(dapp_sm_static STATIC ${SM_ENCODING_DAPP_SRC_ASN})
  target_include_directories(dapp_sm_static PRIVATE 
                                "${CMAKE_CURRENT_SOURCE_DIR}/ie"
                                "${CMAKE_CURRENT_SOURCE_DIR}/ie/ir"
                                "${CMAKE_CURRENT_SOURCE_DIR}/../../util"
                                "${CMAKE_CURRENT_SOURCE_DIR}/e3"
                              )
  target_compile_options(dapp_sm_static PRIVATE "-DASN_DISABLE_JER_SUPPORT" "-DASN_DISABLE_OER_SUPPORT")
  target_compile_options(dapp_sm_static PRIVATE -Wno-missing-field-initializers -Wno-unused-parameter -fvisibility=hidden )
  target_link_libraries(dapp_sm_static PRIVATE -lm libe3::e2sm_dapp ${DAPP_CXX_RUNTIME})  
else()
  message(FATAL_ERROR "E2SM-DAPP only supports the ASN encoding")
endif()

target_compile_definitions(dapp_sm PUBLIC ${SM_ENCODING_DAPP} ${E2AP_VERSION} ${KPM_VERSION}  )
target_compile_definitions(dapp_sm_static PUBLIC ${SM_ENCODING_DAPP} ${E2AP_VERSION} ${KPM_VERSION}   )

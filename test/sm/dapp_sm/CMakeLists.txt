# SPDX-License-Identifier: LicenseRef-CSSL-1.0

set(SM_ENCODING_DAPP "ASN" CACHE STRING "The DAPP SM encoding to use")
set_property(CACHE SM_ENCODING_DAPP PROPERTY STRINGS "ASN")
message(STATUS "Selected DAPP SM_ENCODING: ${SM_ENCODING_DAPP}")

if(SM_ENCODING_DAPP STREQUAL "ASN")
  add_executable(test_dapp_sm
                      main.c
                      ../../rnd/fill_rnd_data_dapp.c
                      ../../../src/util/time_now_us.c
                      ../../../src/util/alg_ds/alg/defer.c
                      )
endif()

target_compile_definitions(test_dapp_sm PUBLIC ${SM_ENCODING_DAPP})
target_link_libraries(test_dapp_sm PUBLIC dapp_sm_static e2sm_dapp_e3_spectrum_asn1_obj dapp_sm_static -pthread -lm)


enable_testing()
add_test(Unit_test_DAPP test_dapp_sm)

# SPDX-License-Identifier: LicenseRef-CSSL-1.0

project(test_enc_dec_dapp_sm)

add_executable(test_enc_dec_dapp_sm
  ../../../rnd/fill_rnd_data_dapp.c
  ../../../../src/util/time_now_us.c
  ../../../../src/util/alg_ds/alg/defer.c
  main.c)

target_link_libraries(test_enc_dec_dapp_sm PUBLIC
  dapp_sm_static
  e2sm_dapp_e3_spectrum_asn1_obj
  dapp_sm_static
  -lm
  )

target_compile_options(test_enc_dec_dapp_sm PRIVATE -Wno-missing-field-initializers -Wno-unused-parameter)

target_compile_options(test_enc_dec_dapp_sm PRIVATE "-DASN_DISABLE_OER_SUPPORT" "-DASN_DISABLE_JER_SUPPORT")

#####
### CTest
####
enable_testing()
add_test(Unit_test_enc_dec_DAPP test_enc_dec_dapp_sm)

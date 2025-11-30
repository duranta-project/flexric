# SPDX-License-Identifier: LicenseRef-CSSL-1.0

if(SM_ENCODING_CCC STREQUAL "JSON")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})

  add_executable(test_ccc_sm
                 main.c
                 ../../../src/util/time_now_us.c
                 )

  target_link_libraries(test_ccc_sm PUBLIC ccc_sm_static -pthread)
  target_compile_definitions(test_ccc_sm PUBLIC ${SM_ENCODING_CCC}
                                                ${E2AP_VERSION}
                                                ${KPM_VERSION})

  enable_testing()
  add_test(Unit_test_CCC_SM test_ccc_sm)

else()
  message(STATUS "CCC SM unit test skipped (SM_ENCODING_CCC=${SM_ENCODING_CCC})")
endif()

# SPDX-License-Identifier: LicenseRef-CSSL-1.0

#####################################
# Unit test E2 SETUP RESPONSE handling
#####################################

add_executable(test_setup_response
  test_setup_response.c
  )

target_link_libraries(test_setup_response
  PUBLIC
  e2_agent
  -pthread
  -lsctp
  -ldl)

# e2_agent sets these PRIVATE, and e2_agent_t's layout depends on them.
target_compile_definitions(test_setup_response PRIVATE ${E2AP_ENCODING} ${E2AP_VERSION} ${KPM_VERSION})

enable_testing()
add_test(Unit_test_setup_response test_setup_response)

#####################################
# Unit test: every way the SCTP association can end ends the E2 session
#####################################

add_executable(test_association_loss
  test_association_loss.c
  )

target_link_libraries(test_association_loss
  PUBLIC
  e2_agent
  -pthread
  -lsctp
  -ldl)

target_compile_definitions(test_association_loss PRIVATE ${E2AP_ENCODING} ${E2AP_VERSION} ${KPM_VERSION})

add_test(Unit_test_association_loss test_association_loss)

#####################################
# Unit test: E2 RESET REQUEST and RESET RESPONSE handling
#####################################

add_executable(test_reset
  test_reset.c
  )

target_link_libraries(test_reset
  PUBLIC
  e2_agent
  -pthread
  -lsctp
  -ldl)

target_compile_definitions(test_reset PRIVATE ${E2AP_ENCODING} ${E2AP_VERSION} ${KPM_VERSION})

add_test(Unit_test_reset test_reset)

#####################################
# Unit test: an E2 SETUP FAILURE with criticality ignore on the TransactionID
#####################################

# Built from the generated E2AP ASN.1 types, which exist for v2 and v3 only.
if(E2AP_VERSION STREQUAL "E2AP_V2" OR E2AP_VERSION STREQUAL "E2AP_V3")
  if(E2AP_VERSION STREQUAL "E2AP_V2")
    set(TEST_E2AP_ASN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/e2ap/v2_03/ie/asn/")
  else()
    set(TEST_E2AP_ASN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../src/lib/e2ap/v3_01/ie/asn/")
  endif()

  add_executable(test_setup_failure
    test_setup_failure.c
    )

  target_link_libraries(test_setup_failure
    PUBLIC
    e2_agent
    -pthread
    -lsctp
    -ldl)

  target_compile_definitions(test_setup_failure PRIVATE ${E2AP_ENCODING} ${E2AP_VERSION} ${KPM_VERSION})
  target_include_directories(test_setup_failure PRIVATE ${TEST_E2AP_ASN_DIR})

  add_test(Unit_test_setup_failure test_setup_failure)
endif()

#####################################
# Unit test: the E2 agent's SCTP timers, from fr_args_t or the config file to the socket
#####################################

add_executable(test_sctp_timers
  test_sctp_timers.c
  )

target_link_libraries(test_sctp_timers
  PUBLIC
  e2_agent
  -pthread
  -lsctp
  -ldl)

target_compile_definitions(test_sctp_timers PRIVATE ${E2AP_ENCODING} ${E2AP_VERSION} ${KPM_VERSION})

add_test(Unit_test_sctp_timers test_sctp_timers)

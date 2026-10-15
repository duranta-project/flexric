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
# Unit test: a SCTP shutdown ends the E2 session
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

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

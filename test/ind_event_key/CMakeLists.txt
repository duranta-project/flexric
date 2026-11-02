# SPDX-License-Identifier: LicenseRef-CSSL-1.0

#####################################
# Unit test the subscription registry of the E2 agent
#####################################

add_executable(test_ind_event_key
  test_ind_event_key.c
  )

target_link_libraries(test_ind_event_key
  PUBLIC
  e2_agent
  -pthread
  -lsctp
  -ldl)

# e2_agent sets these PRIVATE, and e2_agent_t's layout depends on them.
target_compile_definitions(test_ind_event_key PRIVATE ${E2AP_ENCODING} ${E2AP_VERSION} ${KPM_VERSION})

enable_testing()
add_test(Unit_test_ind_event_key test_ind_event_key)

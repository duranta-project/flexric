# SPDX-License-Identifier: LicenseRef-CSSL-1.0

#####################################
# Unit test the routing of aperiodic indications
#####################################

add_executable(test_aperiodic_routing
  test_aperiodic_routing.c
  )

target_link_libraries(test_aperiodic_routing
  PUBLIC
  e2_agent
  -pthread
  -lsctp
  -ldl)

# e2_agent sets these PRIVATE, and e2_agent_t's layout depends on them.
target_compile_definitions(test_aperiodic_routing PRIVATE ${E2AP_ENCODING} ${E2AP_VERSION} ${KPM_VERSION})

enable_testing()
add_test(Unit_test_aperiodic_routing test_aperiodic_routing)

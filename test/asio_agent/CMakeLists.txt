# SPDX-License-Identifier: LicenseRef-CSSL-1.0

#####################################
# Unit test the epoll loop of the E2 agent
#####################################

add_executable(test_asio_eintr
  test_asio_eintr.c
  )

target_link_libraries(test_asio_eintr
  PUBLIC
  e2_agent
  -pthread
  -lsctp
  -ldl)

enable_testing()
add_test(Unit_test_asio_eintr test_asio_eintr)

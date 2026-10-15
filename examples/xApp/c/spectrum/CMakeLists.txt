# SPDX-License-Identifier: LicenseRef-CSSL-1.0

add_executable(xapp_spectrum
  xapp_spectrum.c
  ../../../../src/util/alg_ds/alg/defer.c
)

target_link_libraries(xapp_spectrum PUBLIC
  e42_xapp
  -pthread
  -lsctp
  -ldl
)
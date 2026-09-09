# SPDX-License-Identifier: LicenseRef-CSSL-1.0

add_executable(xapp_rc_handover
  xapp_rc_handover.c
  ../../../../src/util/alg_ds/alg/defer.c
  )

target_link_libraries(xapp_rc_handover
                      PUBLIC
                      e42_xapp
                      asn1_f1ap
                      -pthread
                      -lsctp
                      -ldl
                      )

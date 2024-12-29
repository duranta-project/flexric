# SPDX-License-Identifier: LicenseRef-CSSL-1.0

add_executable(xapp_mac_ctrl
  mac_ctrl.c
  ../../../../src/util/alg_ds/alg/defer.c
  )

 target_link_libraries(xapp_mac_ctrl
 						PUBLIC
 						e42_xapp
 						-pthread
 						-lsctp
 						-ldl
 						)
 						
add_executable(xapp_rc_handover_ctrl
    Handover.c
    ../../../../src/util/alg_ds/alg/defer.c
)

# Link necessary libraries
target_link_libraries(xapp_rc_handover_ctrl
    PUBLIC
    e42_xapp
    pthread
    sctp
    dl
)

add_executable(xapp_energy_saving_with_CU
    energy_saving_with_CU.c
    ../../../../src/util/alg_ds/alg/defer.c
)

# Link necessary libraries
target_link_libraries(xapp_energy_saving_with_CU
    PUBLIC
    e42_xapp
    pthread
    sctp
    dl
)
add_executable(xapp_energy_saving_with_LB
    energy_saving_with_LB.c
    ../../../../src/util/alg_ds/alg/defer.c
)

# Link necessary libraries
target_link_libraries(xapp_energy_saving_with_LB
    PUBLIC
    e42_xapp
    pthread
    sctp
    dl
)
						




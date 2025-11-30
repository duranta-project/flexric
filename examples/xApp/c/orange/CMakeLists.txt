# SPDX-License-Identifier: LicenseRef-CSSL-1.0

include_directories(src/xApp)

add_executable(xapp_es_with_cell_util
    xapp_es_with_cell_util.c
    ../../../../src/util/alg_ds/alg/defer.c
)

# Link necessary libraries
target_link_libraries(xapp_es_with_cell_util
    PUBLIC
    e42_xapp
    pthread
    sctp
    dl
)

add_executable(xapp_rf_reconfiguration
    rf_reconfiguration_xapp.c
    ../../../../src/util/alg_ds/alg/defer.c
)

# Link necessary libraries
target_link_libraries(xapp_rf_reconfiguration
    PUBLIC
    e42_xapp
    pthread
    sctp
    dl
)

add_executable(xapp_test_ccc 
            test_ccc.c
                )

 target_link_libraries(xapp_test_ccc 
 						PUBLIC
 						e42_xapp
                        ccc_sm_static  
 						-pthread
 						-lsctp
 						-ldl
 						)
 						
add_executable(xapp_kpm_rc_Enhanced 
  xapp_kpm_rc_Enhanced.c
  ../../../../src/util/alg_ds/alg/defer.c
  ../../../../src/util/alg_ds/alg/murmur_hash_32.c
  ../../../../src/util/alg_ds/ds/assoc_container/assoc_ht_open_address.c
  )
 
target_link_libraries(xapp_kpm_rc_Enhanced
                      PUBLIC
                      e42_xapp
                      -pthread
                      -lsctp
                      -ldl
                      )
target_compile_definitions(xapp_kpm_rc_Enhanced PRIVATE KPM_MEAS_LIST="${KPM_MEAS_LIST_ENHANCED}")

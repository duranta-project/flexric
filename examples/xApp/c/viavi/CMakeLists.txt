add_executable(xapp_viavi_kpm
                xapp_viavi_kpm.c
                ../../../../src/util/alg_ds/alg/defer.c
              )

target_link_libraries(xapp_viavi_kpm
                      PUBLIC
                      e42_xapp
                      -pthread
                      -lsctp
                      -ldl
	             )

add_executable(xapp_viavi_rc
                xapp_viavi_rc.c
                ../../../../src/util/alg_ds/alg/defer.c
              )

target_link_libraries(xapp_viavi_rc
                      PUBLIC
                      e42_xapp
                      -pthread
                      -lsctp
                      -ldl
                      )

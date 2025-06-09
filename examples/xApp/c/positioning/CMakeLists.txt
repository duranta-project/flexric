find_package(Torch REQUIRED)

add_executable(xapp_srs_ind
		xapp_srs_indication.c
                ../../../../src/util/alg_ds/alg/defer.c
                srs_fapi/debug.c
                srs_fapi/nfapi.c
                srs_fapi/srs_fapi_p7.c
              )

 target_link_libraries(xapp_srs_ind
                      PUBLIC
                      e42_xapp
                      -pthread
                      -lsctp
                      -ldl
                      )

add_executable(xapp_srs_pos_cpp
                xapp_srs_pos.cpp
                srs_fapi/debug.c
                srs_fapi/nfapi.c
                srs_fapi/srs_fapi_p7.c
              )

 target_link_libraries(xapp_srs_pos_cpp
                      PRIVATE
                        "${TORCH_LIBRARIES}"
                      PUBLIC
                        e42_xapp
                        -pthread
                        -lsctp
                        -ldl
                     )

find_package(Torch REQUIRED)

add_subdirectory(ai_ml)
add_subdirectory(oai_dfts)

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

add_executable(xapp_srs_channel
                xapp_srs_channel.cpp
                srs_fapi/debug.c
                srs_fapi/nfapi.c
                srs_fapi/srs_fapi_p7.c
                ai_ml/src/proc_srs_channel.c
              )


 target_include_directories(xapp_srs_channel PRIVATE ai_ml/inc)
 target_link_libraries(xapp_srs_channel
                      PRIVATE
                        "${TORCH_LIBRARIES}"
                         ml_cc
                         gui_if
                         freq2time
                      PUBLIC
                        e42_xapp
                        -pthread
                        -lsctp
                        -ldl
                        m
                     )

add_executable(xapp_srs_pos_cpp
                xapp_srs_pos.cpp
                srs_fapi/debug.c
                srs_fapi/nfapi.c
                srs_fapi/srs_fapi_p7.c
              )
 target_include_directories(xapp_srs_pos_cpp PRIVATE ai_ml/inc)
 target_link_libraries(xapp_srs_pos_cpp
                      PRIVATE
                        "${TORCH_LIBRARIES}"
                         ml_cc
                      PUBLIC
                        e42_xapp
                        -pthread
                        -lsctp
                        -ldl
                     )

find_package(Torch REQUIRED)

add_subdirectory(ai_ml)
add_subdirectory(srs_fapi)

###################################################################
# OAI DFTs
###################################################################
include(FetchContent)

Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(oai-dfts
    GIT_REPOSITORY https://gitlab.eurecom.fr/schmidtr/oai_dfts
    GIT_TAG dfts_flexric
    GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(oai-dfts)

###################################################################
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

add_executable(xapp_cc_testbed
                xapp_cc_testbed.cpp
                srs_fapi/debug.c
                srs_fapi/nfapi.c
                srs_fapi/srs_fapi_p7.c
                ai_ml/src/proc_srs_channel.c
                "${oai-dfts_SOURCE_DIR}/src/dfts_load.c"
              )


 target_include_directories(xapp_cc_testbed PRIVATE ai_ml/inc "${oai-dfts_SOURCE_DIR}/inc")
 target_link_libraries(xapp_cc_testbed
                      PRIVATE
                        "${TORCH_LIBRARIES}"
                         ml_cc
                         gui_if
                      PUBLIC
                        e42_xapp
                        -pthread
                        -lsctp
                        -ldl
                        m
                     )

add_dependencies(xapp_cc_testbed dfts)

add_executable(xapp_srs_emulator
                xapp_srs_emulator.cpp
                srs_fapi/debug.c
                srs_fapi/nfapi.c
                srs_fapi/srs_fapi_p7.c
                ai_ml/src/proc_srs_channel.c
              )


 target_include_directories(xapp_srs_emulator PRIVATE ai_ml/inc)
 target_link_libraries(xapp_srs_emulator
                      PRIVATE
                        "${TORCH_LIBRARIES}"
                         ml_cc
                         gui_if
                      PUBLIC
                        e42_xapp
                        -pthread
                        -lsctp
                        -ldl
                        m
                     )

add_executable(xapp_srs_torch_minimal
                xapp_srs_torch_minimal.cpp
                srs_fapi/debug.c
                srs_fapi/nfapi.c
                srs_fapi/srs_fapi_p7.c
                ai_ml/src/proc_srs_channel.c
              )


 target_include_directories(xapp_srs_torch_minimal PRIVATE ai_ml/inc)
 target_link_libraries(xapp_srs_torch_minimal
                      PRIVATE
                        "${TORCH_LIBRARIES}"
                         ml_cc
                      PUBLIC
                        e42_xapp
                        -pthread
                        -lsctp
                        -ldl
                        m
                     )

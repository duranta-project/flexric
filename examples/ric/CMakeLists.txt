###########
# nearRT-RIC
###########

add_subdirectory(o1)

add_executable(nearRT-RIC near_ric.c)
target_link_libraries(nearRT-RIC
                     PUBLIC
                      telnet_o1
                      near_ric
                      ${FlatCC}
                      -pthread
                      -lsctp
                      -ldl)
target_include_directories(nearRT-RIC PRIVATE ${CMAKE_SOURCE_DIR}/o1)


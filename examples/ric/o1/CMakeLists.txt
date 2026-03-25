add_library(telnet_o1 OBJECT telnet_o1.c)

target_include_directories(telnet_o1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
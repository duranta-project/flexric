set(SM_ENCODING_SRS "PLAIN" CACHE STRING "The SRS SM encoding to use")
set_property(CACHE SM_ENCODING_SRS PROPERTY STRINGS "PLAIN" "ASN" "FLATBUFFERS")
message(STATUS "Selected SRS SM_ENCODING: ${SM_ENCODING_SRS}")

if(SM_ENCODING_SRS  STREQUAL "PLAIN")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR} )
  add_executable(test_srs_sm
                      main.c 
                      ../../rnd/fill_rnd_data_srs.c
                      ../../../src/util/time_now_us.c
                      )              
elseif(SM_ENCODING_SRS STREQUAL "ASN")
  message(FATAL_ERROR "Not implemented ASN encoding")
elseif(SM_ENCODING_SRS STREQUAL "FLATBUFFERS")
  message(FATAL_ERROR "Not implemented FLATBUFFERS encoding")
endif()

target_compile_definitions(test_srs_sm PUBLIC ${SM_ENCODING_SRS})
target_link_libraries(test_srs_sm PUBLIC srs_sm_static srs_fapi -pthread)

enable_testing()
add_test(Unit_test_SRS test_srs_sm)

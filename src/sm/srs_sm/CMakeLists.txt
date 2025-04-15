cmake_minimum_required(VERSION 3.15) # setting this is required
project(srs_sm)            # this sets the project name

set(SM_ENCODING_SRS_SRC
                      ../sm_proc_data.c 
                      srs_sm_ric.c 
                      srs_sm_agent.c 
                     ../../util/byte_array.c 
                     ../../util/alg_ds/alg/defer.c 
                     ../../util/alg_ds/alg/eq_float.c 
                     ../../util/alg_ds/ds/seq_container/seq_arr.c 
                     ../../util/alg_ds/ds/seq_container/seq_ring.c 
                     ../../util/alg_ds/ds/assoc_container/assoc_rb_tree.c
                     ../../util/alg_ds/ds/assoc_container/bimap.c
                     # This dependency sucks!
                     $<TARGET_OBJECTS:e2ap_ran_func_obj>
                     ie/srs_data_ie.c
                      )

if(SM_ENCODING_SRS STREQUAL "PLAIN")
  set(SM_ENCODING_SRS_SRC_ASN 
    ${SM_ENCODING_SRS_SRC}
    enc/srs_enc_plain.c 
    dec/srs_dec_plain.c 
    )

  # Shared
  add_library(srs_sm SHARED ${SM_ENCODING_SRS_SRC_ASN} )
  target_compile_options(srs_sm PRIVATE -fPIC -fvisibility=hidden)

  # Static
  add_library(srs_sm_static STATIC ${SM_ENCODING_SRS_SRC_ASN})

else()
  message(FATAL_ERROR "Unknown SM encoding type")
endif()

# ToDo: Dependency from KPM_VERSION needs to be removed!
target_compile_definitions(srs_sm PUBLIC ${SM_ENCODING_SRS} ${E2AP_VERSION}  ${KPM_VERSION} )
target_compile_definitions(srs_sm_static PUBLIC ${SM_ENCODING_SRS} ${E2AP_VERSION}  ${KPM_VERSION} )



add_library(srs_fapi
        debug.c
        nfapi.c
        srs_fapi_p7.c
        nr_fapi_srs_indication_test.c
)
target_include_directories(srs_fapi PRIVATE /usr/include/hdf5/serial)

target_link_libraries(srs_fapi PRIVATE hdf5_serial)

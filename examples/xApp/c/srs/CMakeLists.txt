macro(add_boolean_option name val helpstr adddef)
  if(DEFINED ${name})
    set(value ${${name}})
  else(DEFINED ${name})
    set(value ${val})
  endif()
  set(${name} ${value} CACHE STRING "${helpstr}")
  set_property(CACHE ${name} PROPERTY TYPE BOOL)
  if (${value} AND ${adddef})
    add_definitions("-D${name}")
  endif (${value} AND ${adddef})
endmacro(add_boolean_option)


add_boolean_option(OAI_AERIAL OFF "Activate OAI's AERIAL driver" OFF)
add_subdirectory(localization_plots)

add_executable(xapp_srs_ind
		xapp_srs_indication.c
                ../../../../src/util/alg_ds/alg/defer.c
                srs_fapi/debug.c
                srs_fapi/nfapi.c
                srs_fapi/srs_fapi_p7.c
              )

if (OAI_AERIAL)
target_compile_definitions(xapp_srs_ind PRIVATE ENABLE_AERIAL)
endif()
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
              )


if (OAI_AERIAL)
target_compile_definitions(xapp_srs_channel PRIVATE ENABLE_AERIAL)
endif()
 target_link_libraries(xapp_srs_channel
                      PRIVATE
                         gui_if
                      PUBLIC
                        e42_xapp
                        -pthread
                        -lsctp
                        -ldl
                        m
                     )


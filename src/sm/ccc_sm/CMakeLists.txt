# SPDX-License-Identifier: LicenseRef-CSSL-1.0
cmake_minimum_required(VERSION 3.15)
project(ccc_sm)

# Enable C++ support for ccc_sm.cc
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── nlohmann/json ───────────
# CCC JSON encode/decode depends on e2sm_ccc.hpp (quicktype-generated C++),
# which uses nlohmann::json. Other JSON libraries are not interchangeable here.
#
# Prefer the system package:
#   Ubuntu/Debian: sudo apt-get install nlohmann-json3-dev
#   Fedora/RHEL:   sudo dnf install json-devel
# Fallback: FetchContent downloads nlohmann/json v3.11.3 if the package is absent.
find_package(nlohmann_json 3.2.0 QUIET)
if(nlohmann_json_FOUND)
  message(STATUS "CCC SM: found nlohmann_json (system) >= 3.2.0")
else()
  message(STATUS
    "CCC SM: nlohmann_json not found via find_package – "
    "falling back to FetchContent (install nlohmann-json3-dev to use the system package)")
  include(FetchContent)
  FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
  )
  FetchContent_MakeAvailable(nlohmann_json)
endif()

# Explicit check: CCC requires this specific CMake target (nlohmann), not another JSON lib.
if(NOT TARGET nlohmann_json::nlohmann_json)
  message(FATAL_ERROR
    "CCC SM requires nlohmann/json (CMake target nlohmann_json::nlohmann_json).\n"
    "  Other JSON libraries (e.g. json-c, cJSON) are not supported for CCC.\n"
    "  Install the development package and re-run cmake, for example:\n"
    "    Ubuntu/Debian: sudo apt-get install nlohmann-json3-dev\n"
    "    Fedora/RHEL:   sudo dnf install json-devel\n"
    "  Or allow network access so CMake FetchContent can download nlohmann/json.")
endif()

# Minimal source files for CCC SM
set(SM_ENCODING_CCC_SRC
    ../sm_proc_data.c 
    ../../util/byte_array.c 
    ../../util/time_now_us.c
    ../../util/alg_ds/alg/defer.c 
    ../../util/alg_ds/alg/eq_float.c 
    ../../util/alg_ds/ds/seq_container/seq_arr.c 
    ../../util/alg_ds/ds/seq_container/seq_ring.c 
    ../../util/alg_ds/ds/assoc_container/assoc_rb_tree.c
    ../../util/alg_ds/ds/assoc_container/bimap.c
    ie/ccc_data_ie.c
    dec/ccc_dec_json.c
    enc/ccc_enc_json.c
    ccc_sm_ric.c 
    ccc_sm_agent.c 
    ccc_sm.cc  
)

if(SM_ENCODING_CCC STREQUAL "JSON")
  set(SM_ENCODING_CCC_SRC_JSON ${SM_ENCODING_CCC_SRC})

  # Shared
  add_library(ccc_sm SHARED ${SM_ENCODING_CCC_SRC_JSON} )
  target_compile_options(ccc_sm PRIVATE -fPIC -fvisibility=hidden)
  target_link_libraries(ccc_sm PRIVATE stdc++ nlohmann_json::nlohmann_json)

  # Static
  add_library(ccc_sm_static STATIC ${SM_ENCODING_CCC_SRC_JSON})
  target_link_libraries(ccc_sm_static PRIVATE stdc++ nlohmann_json::nlohmann_json)

elseif(SM_ENCODING_CCC STREQUAL "PLAIN")
  message(FATAL_ERROR "CCC SM PLAIN encoding is no longer supported; use JSON")

elseif(SM_ENCODING_CCC STREQUAL "ASN")
  message(FATAL_ERROR "CCC SM ASN not implemented")

elseif(SM_ENCODING_CCC STREQUAL "FLATBUFFERS" )
  message(FATAL_ERROR "CCC SM FB not implemented")

else()
  message(FATAL_ERROR "Unknown SM encoding type: ${SM_ENCODING_CCC}")
endif()

target_compile_definitions(ccc_sm PUBLIC ${SM_ENCODING_CCC} ${E2AP_VERSION} ${KPM_VERSION} )
target_compile_definitions(ccc_sm_static PUBLIC ${SM_ENCODING_CCC} ${E2AP_VERSION} ${KPM_VERSION} )

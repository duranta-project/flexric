# SPDX-License-Identifier: LicenseRef-CSSL-1.0

add_subdirectory(service_models/spectrum_sm/asn/defs)

set(DAPP_E3_SRC
  # Generic E3
  dapp_dec_e3.c
  dapp_enc_e3.c

  # Spectrum SM
  service_models/spectrum_sm/asn/decoder.c
  service_models/spectrum_sm/asn/encoder.c
  service_models/spectrum_sm/ir/spectrum_report.c
  service_models/spectrum_sm/ir/spectrum_control.c

)

add_library(e3sm_obj OBJECT ${DAPP_E3_SRC})

target_include_directories(e3sm_obj PUBLIC
  # Spectrum SM
  ${CMAKE_CURRENT_SOURCE_DIR}/service_models/spectrum_sm/asn
  ${CMAKE_CURRENT_SOURCE_DIR}/service_models/spectrum_sm/asn/defs/compiled_output
  ${CMAKE_CURRENT_SOURCE_DIR}/service_models/spectrum_sm/ir
)

target_compile_options(e3sm_obj PRIVATE -Wno-missing-field-initializers -Wno-unused-parameter -fPIC -fvisibility=hidden)

set(DAPP_E3_IR_SRC
  service_models/spectrum_sm/ir/spectrum_report.c
  service_models/spectrum_sm/ir/spectrum_control.c
)

add_library(e3sm_ir_obj OBJECT ${DAPP_E3_IR_SRC})

target_include_directories(e3sm_ir_obj PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/service_models/spectrum_sm/ir
)

target_compile_options(e3sm_ir_obj PRIVATE -Wno-missing-field-initializers -Wno-unused-parameter -fPIC -fvisibility=hidden)

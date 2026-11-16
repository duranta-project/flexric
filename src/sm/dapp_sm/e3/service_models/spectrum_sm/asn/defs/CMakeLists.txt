# SPDX-License-Identifier: LicenseRef-CSSL-1.0

# Generated from e3sm_spectrum.asn with
#
#   asn1c -pdu=all -no-gen-BER -no-gen-UPER -no-gen-OER -no-gen-JER \
#         -fno-include-deps -fcompound-names -findirect-choice -no-gen-example \
#         -D compiled_output e3sm_spectrum.asn
#
# The output is committed, as it is for every other service model here.
#
# e3sm_spectrum.asn is a shared schema, kept byte-identical with the gNB's and
# the dApps' copies, so it declares types this side never sends (the sensing
# indication, the dApp control). asn1c generates all of them; only the report
# and xApp-control chain is compiled in here. The ASN.1 runtime those types
# need comes from e2sm_dapp_asn1_obj, which both link into dapp_sm -- this
# archive deliberately carries no second copy of it.

set(ASN_DAPP_XAPP_SRC
  compiled_output/Spectrum-PRBBlacklistReport.c
  compiled_output/Spectrum-DAppReportPayload.c
  compiled_output/Spectrum-DAppReportData.c
  compiled_output/Spectrum-PRBBlockedControl.c
  compiled_output/Spectrum-ConfigControl.c
  compiled_output/Spectrum-XAppControlPayload.c
  compiled_output/Spectrum-XAppControlData.c
)

add_library(e2sm_dapp_e3_spectrum_asn1_obj STATIC ${ASN_DAPP_XAPP_SRC})

target_include_directories(e2sm_dapp_e3_spectrum_asn1_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/compiled_output)

target_compile_options(e2sm_dapp_e3_spectrum_asn1_obj PUBLIC "-DASN_DISABLE_OER_SUPPORT")
target_compile_options(e2sm_dapp_e3_spectrum_asn1_obj PUBLIC "-DASN_DISABLE_JER_SUPPORT")
target_compile_options(e2sm_dapp_e3_spectrum_asn1_obj PRIVATE -Wno-missing-field-initializers -Wno-unused-parameter -fPIC -fvisibility=hidden)

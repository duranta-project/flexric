add_subdirectory(RRC_MESSAGES)

add_executable(xapp_kpm_moni 
		xapp_kpm_moni.c
                ../../../../src/util/alg_ds/alg/defer.c
              )

 target_link_libraries(xapp_kpm_moni
                      PUBLIC
                      e42_xapp
                      -pthread
                      -lsctp
                      -ldl
                      )

add_executable(xapp_gtp_mac_rlc_pdcp_moni 
		xapp_gtp_mac_rlc_pdcp_moni.c
                ../../../../src/util/alg_ds/alg/defer.c
              )

 target_link_libraries(xapp_gtp_mac_rlc_pdcp_moni
                      PUBLIC
                      e42_xapp
                      -pthread
                      -lsctp
                      -ldl
                      )

add_executable(xapp_srs_moni
		xapp_srs_moni.c
                ../../../../src/util/alg_ds/alg/defer.c
                srs_fapi/debug.c
                srs_fapi/nfapi.c
                srs_fapi/srs_fapi_p7.c
              )

 target_link_libraries(xapp_srs_moni
                      PUBLIC
                      e42_xapp
                      -pthread
                      -lsctp
                      -ldl
                      )

add_executable(xapp_srs_moni_cpp xapp_srs_moni.cpp)
target_link_libraries(xapp_srs_moni_cpp PUBLIC e42_xapp pthread sctp dl)

set(UE_ID_COMMON_E2SM_SRCS
  ../../../../src/sm/rc_sm/ie/asn/AMFPointer.c
  ../../../../src/sm/rc_sm/ie/asn/AMFRegionID.c
  ../../../../src/sm/rc_sm/ie/asn/AMFSetID.c
  ../../../../src/sm/rc_sm/ie/asn/AMF-UE-NGAP-ID.c
  ../../../../src/sm/rc_sm/ie/asn/ENB-ID.c
  ../../../../src/sm/rc_sm/ie/asn/ENB-UE-X2AP-ID-Extension.c
  ../../../../src/sm/rc_sm/ie/asn/ENB-UE-X2AP-ID.c
  ../../../../src/sm/rc_sm/ie/asn/GlobalENB-ID.c
  ../../../../src/sm/rc_sm/ie/asn/GlobalGNB-ID.c
  ../../../../src/sm/rc_sm/ie/asn/GlobalNgENB-ID.c
  ../../../../src/sm/rc_sm/ie/asn/GlobalNGRANNodeID.c
  ../../../../src/sm/rc_sm/ie/asn/GNB-CU-CP-UE-E1AP-ID.c
  ../../../../src/sm/rc_sm/ie/asn/GNB-CU-UE-F1AP-ID.c
  ../../../../src/sm/rc_sm/ie/asn/GNB-ID.c
  ../../../../src/sm/rc_sm/ie/asn/GUAMI.c
  ../../../../src/sm/rc_sm/ie/asn/GUMMEI.c
  ../../../../src/sm/rc_sm/ie/asn/MME-Code.c
  ../../../../src/sm/rc_sm/ie/asn/MME-Group-ID.c
  ../../../../src/sm/rc_sm/ie/asn/MME-UE-S1AP-ID.c
  ../../../../src/sm/rc_sm/ie/asn/NGENB-CU-UE-W1AP-ID.c
  ../../../../src/sm/rc_sm/ie/asn/NgENB-ID.c
  ../../../../src/sm/rc_sm/ie/asn/NG-RANnodeUEXnAPID.c
  ../../../../src/sm/rc_sm/ie/asn/PLMNIdentity.c
  ../../../../src/sm/rc_sm/ie/asn/RANUEID.c
  ../../../../src/sm/rc_sm/ie/asn/UEID-ENB.c
  ../../../../src/sm/rc_sm/ie/asn/UEID-EN-GNB.c
  ../../../../src/sm/rc_sm/ie/asn/UEID-GNB-CU-CP-E1AP-ID-Item.c
  ../../../../src/sm/rc_sm/ie/asn/UEID-GNB-CU-CP-E1AP-ID-List.c
  ../../../../src/sm/rc_sm/ie/asn/UEID-GNB-CU-CP-F1AP-ID-Item.c
  ../../../../src/sm/rc_sm/ie/asn/UEID-GNB-CU-F1AP-ID-List.c
  ../../../../src/sm/rc_sm/ie/asn/UEID-GNB-CU-UP.c
  ../../../../src/sm/rc_sm/ie/asn/UEID-GNB-DU.c
  ../../../../src/sm/rc_sm/ie/asn/UEID-GNB.c
  ../../../../src/sm/rc_sm/ie/asn/UEID.c
  ../../../../src/sm/rc_sm/ie/asn/UEID-NG-ENB-DU.c
  ../../../../src/sm/rc_sm/ie/asn/UEID-NG-ENB.c
)

add_executable(xapp_rc_moni 
               xapp_rc_moni.c
               ${UE_ID_COMMON_E2SM_SRCS}
               ../../../../src/util/alg_ds/alg/defer.c
              )
                  
target_link_libraries(xapp_rc_moni
                     PUBLIC
                     asn1_nr_rrc
                     3gpp_derived_ie_dec_asn_obj_rc
                     e42_xapp
                     -pthread
                     -lsctp
                     -ldl
                     )

// From nfapi_nr_interface_scf.h
#ifndef SRS_FAPI_DATA_H
#define SRS_FAPI_DATA_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../common.h"
#define NFAPI_HEADER_LENGTH 8

typedef struct {
  uint16_t phy_id;
  uint16_t message_id;
  uint32_t message_length;
  uint16_t spare;
} nfapi_nr_p4_p5_message_header_t;

typedef struct {
  uint16_t phy_id;
  uint16_t message_id;
  uint32_t message_length;
  uint16_t m_segment_sequence; /* This consists of 3 fields - namely, M, Segement & Sequence number*/
  uint32_t checksum;
  uint32_t transmit_timestamp;
} nfapi_nr_p7_message_header_t;

typedef struct {
  uint8_t num_msg;
  uint8_t opaque_handle;
  uint16_t message_id;
  uint32_t message_length;
} fapi_message_header_t;

typedef struct {
  uint16_t tag;                         // 0: Report is carried directly in the value field; 3: The offset from the end of the control portion of the message to the beginning of the report. Other values are reserved.
  uint32_t length;                      // Length of the actual report in bytes, without the padding bytes.
  uint32_t value[4*16384];                // tag=0: Only the most significant bytes of the size indicated by ‘length’ field are valid. Remaining bytes are zero padded to the nearest 32-bit bit boundary; Tag=2 Offset from the end of the control portion of the message to the payload is in the value field. Occupies 32-bits.
} nfapi_srs_report_tlv_t;

typedef struct {
  uint32_t handle;                      // The handle passed to the PHY in the the UL_TTI.request SRS PDU.
  uint16_t rnti;                        // The RNTI passed to the PHY in the UL_TTI.request SRS PDU. Value: 1 -> 65535.
  uint16_t timing_advance_offset;       // Timing advance TA measured for the UE in multiples of 16 * 64 * Tc / (2^u) [TS 38.213, Section 4.2]. Value: 0 -> 63. 0xffff will be set if this field is invalid.
  int16_t timing_advance_offset_nsec;   // Timing advance measured for the UE between the reference uplink time and the observed arrival time for the UE. Value: -16800 … +16800 nanoseconds. 0xffff should be set if this field is invalid.
  uint8_t srs_usage;                    // 0 – beamManagement; 1 – codebook; 2 – nonCodebook; 3 – antennaSwitching; 4 – 255: reserved; Note: This field matches the SRS usage field of the SRS PDU to which this report is linked.
  uint8_t report_type;                  // The type of report included in or pointed to by Report TLV depends on the SRS usage: Beam management (1: Beamforming report); Codebook (1: Normalized Channel I/Q Matrix); nonCodebook (1: Normalized Channel I/Q Matrix); antennaSwitch (1: Channel SVD Representation); all (0: null report)
  nfapi_srs_report_tlv_t report_tlv;
} nfapi_nr_srs_indication_pdu_t;

typedef struct {
  uint8_t normalized_iq_representation; // 0: 16-bit normalized complex number (iqSize = 2); 1: 32-bit normalized complex number (iqSize = 4)
  uint16_t num_gnb_antenna_elements;    // Ng: Number of gNB antenna elements. Value: 0511
  uint16_t num_ue_srs_ports;            // Nu: Number of sampled UE SRS ports. Value: 07
  uint16_t prg_size;                    // Size in RBs of a precoding resource block group (PRG) – to which the same digital beamforming gets applied. Value: 1->272
  uint16_t num_prgs;                    // Number of PRGs Np to be reported for this SRS PDU. Value: 0-> 272
  uint8_t channel_matrix[4096*2*8*4];    // Array of (numPRGs*Nu*Ng) entries of the type denoted by iqRepresentation H{PRG pI} [ueAntenna uI, gNB antenna gI] = array[uI*Ng*Np + gI*Np + pI]; uI: 0…Nu-1 (UE antenna index); gI: 0…Ng-1 (gNB antenna index); pI: 0…Np-1 (PRG index)
} nfapi_nr_srs_normalized_channel_iq_matrix_t;

typedef struct {
  nfapi_nr_p7_message_header_t header;
  uint16_t sfn;                         // SFN. Value: 0 -> 1023
  uint16_t slot;                        // Slot. Value: 0 -> 159
  uint16_t control_length;              // Size of control portion of SRS indication. 0 if reports are included inline; >0 if reports are concatenated to the end of the message.
  uint8_t number_of_pdus;               // Number of PDUs included in this message. Value: 0 -> 255
  nfapi_nr_srs_indication_pdu_t *pdu_list;
} nfapi_nr_srs_indication_t;

#endif

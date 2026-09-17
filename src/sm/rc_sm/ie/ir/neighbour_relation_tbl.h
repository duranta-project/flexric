/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef NEIGHBOUR_RELATION_TABLE_E2SM_RC_H
#define NEIGHBOUR_RELATION_TABLE_E2SM_RC_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "../../../../lib/3gpp/ie/nr_cgi.h"
#include "../../../../lib/3gpp/ie/eutra_cgi.h"

// CHOICE RAT type used by both Serving Cell PCI (9.3.39) and Serving Cell ARFCN (9.3.40)
typedef enum{
  NR_SERVING_CELL_E2SM_RC,
  EUTRA_SERVING_CELL_E2SM_RC,

  END_SERVING_CELL_E2SM_RC,
} serving_cell_rat_e;

// 9.3.39 Serving Cell PCI
typedef struct{
  // CHOICE RAT type
  // Mandatory
  serving_cell_rat_e type;

  union{
    uint16_t nr_pci;    // NR-PCI     [0..1007]
    uint16_t eutra_pci; // E-UTRA-PCI [0..503]
  };
} serving_cell_pci_t;

// 9.3.40 Serving Cell ARFCN
typedef struct{
  // CHOICE RAT type
  // Mandatory
  serving_cell_rat_e type;

  union{
    uint32_t nr_arfcn;    // NR-ARFCN
    uint32_t eutra_arfcn; // E-UTRA-ARFCN
  };
} serving_cell_arfcn_t;

// 9.3.44 NR Frequency Info -> NR Frequency Band Item
// (NR Frequency Info IE in TS 38.473 9.3.1.17)
typedef struct{
  // Frequency Band Indicator Nr
  // Mandatory
  uint16_t freq_band_indicator_nr;

  // Supported SUL Band List
  // Mandatory (list can be empty)
  uint16_t* sul_band_list;
  size_t sz_sul_band_list;

} nr_freq_band_item_t;

// 9.3.44 NR Frequency Info
typedef struct{
  // NR-ARFCN
  // Mandatory
  uint32_t nr_arfcn;

  // Frequency Band List
  // Mandatory [1..maxnoofNrCellBands]
  nr_freq_band_item_t* freq_band_list;
  size_t sz_freq_band_list;

  // Frequency Shift 7.5kHz
  // Optional
  bool* freq_shift_7p5khz;

} nr_freq_info_t;

// Xn X2 Established
// Used by both NR and E-UTRA neighbour cell items
typedef enum{
  TRUE_XN_X2_ESTABLISHED_E2SM_RC,
  FALSE_XN_X2_ESTABLISHED_E2SM_RC,

  END_XN_X2_ESTABLISHED_E2SM_RC,
} xn_x2_established_e;

// HO Validated
// Used by both NR and E-UTRA neighbour cell items
typedef enum{
  TRUE_HO_VALIDATED_E2SM_RC,
  FALSE_HO_VALIDATED_E2SM_RC,

  END_HO_VALIDATED_E2SM_RC,
} ho_validated_e;

// NR Mode Info
typedef enum{
  FDD_NR_MODE_INFO_E2SM_RC,
  TDD_NR_MODE_INFO_E2SM_RC,

  END_NR_MODE_INFO_E2SM_RC,
} nr_mode_info_e;

// Neighbour Cell Item - CHOICE RAN Type - NR
typedef struct{
  // NR CGI
  // Mandatory
  // 9.3.41
  nr_cgi_t nr_cgi;

  // NR PCI
  // Mandatory
  // 9.3.42
  // [0..1007]
  uint16_t nr_pci;

  // 5GS TAC
  // Mandatory
  // 9.3.43
  uint32_t five_gs_tac;

  // NR Mode Info
  // Mandatory
  nr_mode_info_e nr_mode_info;

  // NR Frequency Info
  // Mandatory
  // 9.3.44
  nr_freq_info_t nr_freq_info;

  // Xn X2 Established
  // Mandatory
  xn_x2_established_e xn_x2_established;

  // HO Validated
  // Mandatory
  ho_validated_e ho_validated;

  // Version
  // Mandatory
  // [1..65535]
  uint16_t version;

} neighbour_cell_choice_nr_t;

// Neighbour Cell Item - CHOICE RAN Type - E-UTRA
typedef struct{
  // E-UTRA CGI
  // Mandatory
  // 9.3.45
  eutra_cgi_t eutra_cgi;

  // E-UTRA PCI
  // Mandatory
  // 9.3.46
  // [0..503]
  uint16_t eutra_pci;

  // E-UTRA ARFCN
  // Mandatory
  // 9.3.47
  uint32_t eutra_arfcn;

  // E-UTRA TAC
  // Mandatory
  // 9.3.48
  uint16_t eutra_tac;

  // Xn X2 Established
  // Mandatory
  xn_x2_established_e xn_x2_established;

  // HO Validated
  // Mandatory
  ho_validated_e ho_validated;

  // Version
  // Mandatory
  // [1..65535]
  uint16_t version;

} neighbour_cell_choice_eutra_t;

// CHOICE RAN Type
typedef enum{
  NR_NEIGHBOUR_CELL_E2SM_RC,
  EUTRA_NEIGHBOUR_CELL_E2SM_RC,

  END_NEIGHBOUR_CELL_E2SM_RC,
} neighbour_cell_rat_e;

// Neighbour Cell List Item
typedef struct{
  // CHOICE RAN Type
  // Mandatory
  neighbour_cell_rat_e type;

  union{
    neighbour_cell_choice_nr_t choice_nr;
    neighbour_cell_choice_eutra_t choice_eutra;
  };

} neighbour_cell_item_t;

// 9.3.38 Neighbour Relation Information
typedef struct{

  // Serving Cell PCI
  // Mandatory
  // 9.3.39
  serving_cell_pci_t serving_cell_pci;

  // Serving Cell ARFCN
  // Mandatory
  // 9.3.40
  serving_cell_arfcn_t serving_cell_arfcn;

  // Neighbour Cell List
  // Mandatory [1..maxnoofNeighbourCell]
  neighbour_cell_item_t* neighbour_cell_list;
  size_t sz_neighbour_cell_list;

} neighbour_rela_tbl_t;

void free_neighbour_rela_tbl(neighbour_rela_tbl_t* src);

bool eq_neighbour_rela_tbl(neighbour_rela_tbl_t const* m0, neighbour_rela_tbl_t const* m1);

neighbour_rela_tbl_t cp_neighbour_rela_tbl(neighbour_rela_tbl_t const* src);

#endif

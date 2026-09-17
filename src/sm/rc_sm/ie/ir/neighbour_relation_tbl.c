/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "neighbour_relation_tbl.h"

#include <assert.h>
#include <string.h>

//////////////////////
// NR Frequency Info
//////////////////////

static
void free_nr_freq_band_item(nr_freq_band_item_t* src)
{
  assert(src != NULL);

  if(src->sz_sul_band_list > 0){
    assert(src->sul_band_list != NULL);
    free(src->sul_band_list);
  }
}

static
bool eq_nr_freq_band_item(nr_freq_band_item_t const* m0, nr_freq_band_item_t const* m1)
{
  if(m0 == m1)
    return true;

  if(m0 == NULL || m1 == NULL)
    return false;

  if(m0->freq_band_indicator_nr != m1->freq_band_indicator_nr)
    return false;

  if(m0->sz_sul_band_list != m1->sz_sul_band_list)
    return false;

  for(size_t i = 0; i < m0->sz_sul_band_list; ++i){
    if(m0->sul_band_list[i] != m1->sul_band_list[i])
      return false;
  }

  return true;
}

static
nr_freq_band_item_t cp_nr_freq_band_item(nr_freq_band_item_t const* src)
{
  assert(src != NULL);
  nr_freq_band_item_t dst = {0};

  dst.freq_band_indicator_nr = src->freq_band_indicator_nr;

  dst.sz_sul_band_list = src->sz_sul_band_list;
  if(src->sz_sul_band_list > 0){
    dst.sul_band_list = calloc(dst.sz_sul_band_list, sizeof(uint16_t));
    assert(dst.sul_band_list != NULL && "Memory exhausted");
    memcpy(dst.sul_band_list, src->sul_band_list, sizeof(uint16_t) * dst.sz_sul_band_list);
  }

  return dst;
}

static
void free_nr_freq_info(nr_freq_info_t* src)
{
  assert(src != NULL);

  assert(src->sz_freq_band_list > 0 && src->sz_freq_band_list < 65536);
  for(size_t i = 0; i < src->sz_freq_band_list; ++i){
    free_nr_freq_band_item(&src->freq_band_list[i]);
  }
  assert(src->freq_band_list != NULL);
  free(src->freq_band_list);

  if(src->freq_shift_7p5khz != NULL)
    free(src->freq_shift_7p5khz);
}

static
bool eq_nr_freq_info(nr_freq_info_t const* m0, nr_freq_info_t const* m1)
{
  if(m0 == m1)
    return true;

  if(m0 == NULL || m1 == NULL)
    return false;

  if(m0->nr_arfcn != m1->nr_arfcn)
    return false;

  if(m0->sz_freq_band_list != m1->sz_freq_band_list)
    return false;

  for(size_t i = 0; i < m0->sz_freq_band_list; ++i){
    if(eq_nr_freq_band_item(&m0->freq_band_list[i], &m1->freq_band_list[i]) == false)
      return false;
  }

  if(m0->freq_shift_7p5khz != m1->freq_shift_7p5khz){
    if(m0->freq_shift_7p5khz == NULL || m1->freq_shift_7p5khz == NULL)
      return false;

    if(*m0->freq_shift_7p5khz != *m1->freq_shift_7p5khz)
      return false;
  }

  return true;
}

static
nr_freq_info_t cp_nr_freq_info(nr_freq_info_t const* src)
{
  assert(src != NULL);
  nr_freq_info_t dst = {0};

  dst.nr_arfcn = src->nr_arfcn;

  assert(src->sz_freq_band_list > 0 && src->sz_freq_band_list < 65536);
  dst.sz_freq_band_list = src->sz_freq_band_list;
  dst.freq_band_list = calloc(dst.sz_freq_band_list, sizeof(nr_freq_band_item_t));
  assert(dst.freq_band_list != NULL && "Memory exhausted");

  for(size_t i = 0; i < src->sz_freq_band_list; ++i){
    dst.freq_band_list[i] = cp_nr_freq_band_item(&src->freq_band_list[i]);
  }

  if(src->freq_shift_7p5khz != NULL){
    dst.freq_shift_7p5khz = malloc(sizeof(bool));
    assert(dst.freq_shift_7p5khz != NULL && "Memory exhausted");
    *dst.freq_shift_7p5khz = *src->freq_shift_7p5khz;
  }

  return dst;
}

//////////////////////
// Neighbour Cell Item
//////////////////////

static
void free_neighbour_cell_choice_nr(neighbour_cell_choice_nr_t* src)
{
  assert(src != NULL);

  free_nr_cgi(&src->nr_cgi);

  free_nr_freq_info(&src->nr_freq_info);
}

static
bool eq_neighbour_cell_choice_nr(neighbour_cell_choice_nr_t const* m0, neighbour_cell_choice_nr_t const* m1)
{
  if(eq_nr_cgi(&m0->nr_cgi, &m1->nr_cgi) == false)
    return false;

  if(m0->nr_pci != m1->nr_pci)
    return false;

  if(m0->five_gs_tac != m1->five_gs_tac)
    return false;

  if(m0->nr_mode_info != m1->nr_mode_info)
    return false;

  if(eq_nr_freq_info(&m0->nr_freq_info, &m1->nr_freq_info) == false)
    return false;

  if(m0->xn_x2_established != m1->xn_x2_established)
    return false;

  if(m0->ho_validated != m1->ho_validated)
    return false;

  if(m0->version != m1->version)
    return false;

  return true;
}

static
neighbour_cell_choice_nr_t cp_neighbour_cell_choice_nr(neighbour_cell_choice_nr_t const* src)
{
  assert(src != NULL);
  neighbour_cell_choice_nr_t dst = {0};

  dst.nr_cgi = cp_nr_cgi(&src->nr_cgi);
  dst.nr_pci = src->nr_pci;
  dst.five_gs_tac = src->five_gs_tac;
  dst.nr_mode_info = src->nr_mode_info;
  dst.nr_freq_info = cp_nr_freq_info(&src->nr_freq_info);
  dst.xn_x2_established = src->xn_x2_established;
  dst.ho_validated = src->ho_validated;
  dst.version = src->version;

  return dst;
}

static
void free_neighbour_cell_choice_eutra(neighbour_cell_choice_eutra_t* src)
{
  assert(src != NULL);

  free_eutra_cgi(&src->eutra_cgi);
}

static
bool eq_neighbour_cell_choice_eutra(neighbour_cell_choice_eutra_t const* m0, neighbour_cell_choice_eutra_t const* m1)
{
  if(eq_eutra_cgi(&m0->eutra_cgi, &m1->eutra_cgi) == false)
    return false;

  if(m0->eutra_pci != m1->eutra_pci)
    return false;

  if(m0->eutra_arfcn != m1->eutra_arfcn)
    return false;

  if(m0->eutra_tac != m1->eutra_tac)
    return false;

  if(m0->xn_x2_established != m1->xn_x2_established)
    return false;

  if(m0->ho_validated != m1->ho_validated)
    return false;

  if(m0->version != m1->version)
    return false;

  return true;
}

static
neighbour_cell_choice_eutra_t cp_neighbour_cell_choice_eutra(neighbour_cell_choice_eutra_t const* src)
{
  assert(src != NULL);
  neighbour_cell_choice_eutra_t dst = {0};

  dst.eutra_cgi = cp_eutra_cgi(&src->eutra_cgi);
  dst.eutra_pci = src->eutra_pci;
  dst.eutra_arfcn = src->eutra_arfcn;
  dst.eutra_tac = src->eutra_tac;
  dst.xn_x2_established = src->xn_x2_established;
  dst.ho_validated = src->ho_validated;
  dst.version = src->version;

  return dst;
}

static
void free_neighbour_cell_item(neighbour_cell_item_t* src)
{
  assert(src != NULL);

  if(src->type == NR_NEIGHBOUR_CELL_E2SM_RC){
    free_neighbour_cell_choice_nr(&src->choice_nr);
  } else if(src->type == EUTRA_NEIGHBOUR_CELL_E2SM_RC){
    free_neighbour_cell_choice_eutra(&src->choice_eutra);
  } else {
    assert(0 != 0 && "Unknown type");
  }
}

static
bool eq_neighbour_cell_item(neighbour_cell_item_t const* m0, neighbour_cell_item_t const* m1)
{
  if(m0->type != m1->type)
    return false;

  if(m0->type == NR_NEIGHBOUR_CELL_E2SM_RC){
    if(eq_neighbour_cell_choice_nr(&m0->choice_nr, &m1->choice_nr) == false)
      return false;
  } else if(m0->type == EUTRA_NEIGHBOUR_CELL_E2SM_RC){
    if(eq_neighbour_cell_choice_eutra(&m0->choice_eutra, &m1->choice_eutra) == false)
      return false;
  } else {
    assert(0 != 0 && "Unknown type");
  }

  return true;
}

static
neighbour_cell_item_t cp_neighbour_cell_item(neighbour_cell_item_t const* src)
{
  assert(src != NULL);
  neighbour_cell_item_t dst = {0};

  dst.type = src->type;

  if(src->type == NR_NEIGHBOUR_CELL_E2SM_RC){
    dst.choice_nr = cp_neighbour_cell_choice_nr(&src->choice_nr);
  } else if(src->type == EUTRA_NEIGHBOUR_CELL_E2SM_RC){
    dst.choice_eutra = cp_neighbour_cell_choice_eutra(&src->choice_eutra);
  } else {
    assert(0 != 0 && "Unknown type");
  }

  return dst;
}

//////////////////////
// Neighbour Relation Table
//////////////////////

void free_neighbour_rela_tbl(neighbour_rela_tbl_t* src)
{
  assert(src != NULL);

  // Serving Cell PCI / ARFCN are plain scalar CHOICEs, nothing to free

  assert(src->sz_neighbour_cell_list > 0 && src->sz_neighbour_cell_list < 65536);
  for(size_t i = 0; i < src->sz_neighbour_cell_list; ++i){
    free_neighbour_cell_item(&src->neighbour_cell_list[i]);
  }
  assert(src->neighbour_cell_list != NULL);
  free(src->neighbour_cell_list);
}

bool eq_neighbour_rela_tbl(neighbour_rela_tbl_t const* m0, neighbour_rela_tbl_t const* m1)
{
  if(m0 == m1)
    return true;

  if(m0 == NULL || m1 == NULL)
    return false;

  if(m0->serving_cell_pci.type != m1->serving_cell_pci.type)
    return false;

  if(m0->serving_cell_pci.type == NR_SERVING_CELL_E2SM_RC){
    if(m0->serving_cell_pci.nr_pci != m1->serving_cell_pci.nr_pci)
      return false;
  } else {
    if(m0->serving_cell_pci.eutra_pci != m1->serving_cell_pci.eutra_pci)
      return false;
  }

  if(m0->serving_cell_arfcn.type != m1->serving_cell_arfcn.type)
    return false;

  if(m0->serving_cell_arfcn.type == NR_SERVING_CELL_E2SM_RC){
    if(m0->serving_cell_arfcn.nr_arfcn != m1->serving_cell_arfcn.nr_arfcn)
      return false;
  } else {
    if(m0->serving_cell_arfcn.eutra_arfcn != m1->serving_cell_arfcn.eutra_arfcn)
      return false;
  }

  if(m0->sz_neighbour_cell_list != m1->sz_neighbour_cell_list)
    return false;

  for(size_t i = 0; i < m0->sz_neighbour_cell_list; ++i){
    if(eq_neighbour_cell_item(&m0->neighbour_cell_list[i], &m1->neighbour_cell_list[i]) == false)
      return false;
  }

  return true;
}

neighbour_rela_tbl_t cp_neighbour_rela_tbl(neighbour_rela_tbl_t const* src)
{
  assert(src != NULL);
  neighbour_rela_tbl_t dst = {0};

  dst.serving_cell_pci = src->serving_cell_pci;
  dst.serving_cell_arfcn = src->serving_cell_arfcn;

  assert(src->sz_neighbour_cell_list > 0 && src->sz_neighbour_cell_list < 65536);
  dst.sz_neighbour_cell_list = src->sz_neighbour_cell_list;
  dst.neighbour_cell_list = calloc(dst.sz_neighbour_cell_list, sizeof(neighbour_cell_item_t));
  assert(dst.neighbour_cell_list != NULL && "Memory exhausted");

  for(size_t i = 0; i < src->sz_neighbour_cell_list; ++i){
    dst.neighbour_cell_list[i] = cp_neighbour_cell_item(&src->neighbour_cell_list[i]);
  }

  return dst;
}

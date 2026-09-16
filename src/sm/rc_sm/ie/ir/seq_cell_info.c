/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "seq_cell_info.h"

#include <assert.h>

void free_seq_cell_info( seq_cell_info_t* src)
{
  assert(src != NULL);

  // Cell Global ID
  // Mandatory
  // 9.3.36
  // 6.2.2.5
  free_cell_global_id(&src->cell_global_id);

  // Cell Context Information
  // Optional
  // OCTET STRING
  assert(src->cell_ctx_info== NULL && "Not implemented");

  // Cell Deleted
  // Optional
  // BOOLEAN
  assert(src->cell_del == NULL && "Not implemented");

  // Neighbour Relation Table
  // Optional
  // 9.3.38
  if(src->neighbour_rela_tbl != NULL){
    free_neighbour_rela_tbl(src->neighbour_rela_tbl);
    free(src->neighbour_rela_tbl);
  }

}

bool eq_seq_cell_info( seq_cell_info_t const* m0,  seq_cell_info_t const* m1)
{
  if(m0 == m1)
    return true;

  if(m0 == NULL || m1 == NULL)
    return false;

  // Cell Global ID
  // Mandatory
  // 9.3.36
  // 6.2.2.5
  if(eq_cell_global_id(&m0->cell_global_id, &m1->cell_global_id) == false)
    return false;

  // Cell Context Information
  // Optional
  // OCTET STRING
  assert(m0->cell_ctx_info== NULL && "Not implemented");
  assert(m1->cell_ctx_info== NULL && "Not implemented");

  // Cell Deleted
  // Optional
  // BOOLEAN
  assert(m0->cell_del == NULL && "Not implemented");
  assert(m1->cell_del == NULL && "Not implemented");

  // Neighbour Relation Table
  // Optional
  // 9.3.38
  if(m0->neighbour_rela_tbl != m1->neighbour_rela_tbl){
    if(m0->neighbour_rela_tbl == NULL || m1->neighbour_rela_tbl == NULL)
      return false;

    if(eq_neighbour_rela_tbl(m0->neighbour_rela_tbl, m1->neighbour_rela_tbl) == false)
      return false;
  }

  return true;
}


seq_cell_info_t cp_seq_cell_info(seq_cell_info_t const* src)
{
  assert(src != NULL);
  seq_cell_info_t dst = {0};

  // Cell Global ID
  // Mandatory
  // 9.3.36
  // 6.2.2.5
  dst.cell_global_id = cp_cell_global_id(&src->cell_global_id);

  // Cell Context Information
  // Optional
  // OCTET STRING
  assert(src->cell_ctx_info == NULL && "Not implemented");

  // Cell Deleted
  // Optional
  // BOOLEAN
  assert(src->cell_del == NULL && "Not implemented");

  // Neighbour Relation Table
  // Optional
  // 9.3.38
  if(src->neighbour_rela_tbl != NULL){
    dst.neighbour_rela_tbl = malloc(sizeof(neighbour_rela_tbl_t));
    assert(dst.neighbour_rela_tbl != NULL && "Memory exhausted");
    *dst.neighbour_rela_tbl = cp_neighbour_rela_tbl(src->neighbour_rela_tbl);
  }

  return dst;
}


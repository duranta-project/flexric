/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#ifndef __TOOLS_DEFS__H__
#define __TOOLS_DEFS__H__

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif


#define FOREACH_DFTSZ(SZ_DEF) \
  SZ_DEF(12)                  \
  SZ_DEF(24)                  \
  SZ_DEF(36)                  \
  SZ_DEF(48)                  \
  SZ_DEF(60)                  \
  SZ_DEF(64)                  \
  SZ_DEF(72)                  \
  SZ_DEF(96)                  \
  SZ_DEF(108)                 \
  SZ_DEF(120)                 \
  SZ_DEF(128)                 \
  SZ_DEF(144)                 \
  SZ_DEF(180)                 \
  SZ_DEF(192)                 \
  SZ_DEF(216)                 \
  SZ_DEF(240)                 \
  SZ_DEF(256)                 \
  SZ_DEF(288)                 \
  SZ_DEF(300)                 \
  SZ_DEF(324)                 \
  SZ_DEF(360)                 \
  SZ_DEF(384)                 \
  SZ_DEF(432)                 \
  SZ_DEF(480)                 \
  SZ_DEF(512)                 \
  SZ_DEF(540)                 \
  SZ_DEF(576)                 \
  SZ_DEF(600)                 \
  SZ_DEF(648)                 \
  SZ_DEF(720)                 \
  SZ_DEF(768)                 \
  SZ_DEF(864)                 \
  SZ_DEF(900)                 \
  SZ_DEF(960)                 \
  SZ_DEF(972)                 \
  SZ_DEF(1024)                \
  SZ_DEF(1080)                \
  SZ_DEF(1152)                \
  SZ_DEF(1200)                \
  SZ_DEF(1296)                \
  SZ_DEF(1440)                \
  SZ_DEF(1500)                \
  SZ_DEF(1536)                \
  SZ_DEF(1620)                \
  SZ_DEF(1728)                \
  SZ_DEF(1800)                \
  SZ_DEF(1920)                \
  SZ_DEF(1944)                \
  SZ_DEF(2048)                \
  SZ_DEF(2160)                \
  SZ_DEF(2304)                \
  SZ_DEF(2400)                \
  SZ_DEF(2592)                \
  SZ_DEF(2700)                \
  SZ_DEF(2880)                \
  SZ_DEF(2916)                \
  SZ_DEF(3000)                \
  SZ_DEF(3072)                \
  SZ_DEF(3240)                \
  SZ_DEF(4096)                \
  SZ_DEF(6144)                \
  SZ_DEF(8192)                \
  SZ_DEF(12288)               \
  SZ_DEF(18432)               \
  SZ_DEF(24576)               \
  SZ_DEF(36864)               \
  SZ_DEF(49152)               \
  SZ_DEF(98304)

#define FOREACH_IDFTSZ(SZ_DEF) \
  SZ_DEF(64)                   \
  SZ_DEF(128)                  \
  SZ_DEF(256)                  \
  SZ_DEF(512)                  \
  SZ_DEF(768)                  \
  SZ_DEF(1024)                 \
  SZ_DEF(1536)                 \
  SZ_DEF(2048)                 \
  SZ_DEF(3072)                 \
  SZ_DEF(4096)                 \
  SZ_DEF(6144)                 \
  SZ_DEF(8192)                 \
  SZ_DEF(12288)                \
  SZ_DEF(16384)                \
  SZ_DEF(18432)                \
  SZ_DEF(24576)                \
  SZ_DEF(32768)                \
  SZ_DEF(36864)                \
  SZ_DEF(49152)                \
  SZ_DEF(65536)                \
  SZ_DEF(98304)

typedef  void(*dftfunc_t)(uint8_t sizeidx,int16_t *sigF,int16_t *sig,unsigned char scale_flag);
typedef void (*idftfunc_t)(uint8_t sizeidx, int16_t *sigF, int16_t *sig, unsigned char scale_flag);
extern dftfunc_t dft;
extern idftfunc_t idft;
int load_dftslib(void);

#define SZ_ENUM(Sz) DFT_##Sz,
typedef enum dft_size_idx {
  FOREACH_DFTSZ(SZ_ENUM)
  DFT_SIZE_IDXTABLESIZE
}  dft_size_idx_t;

/*******************************************************************
*
* NAME :         get_dft
*
* PARAMETERS :   size of ofdm symbol
*
* RETURN :       function for discrete fourier transform
*
* DESCRIPTION :  get dft function depending of ofdm size
*
*********************************************************************/
#define FIND_ENUM(Sz) \
  case Sz:            \
    return DFT_##Sz;  \
    break;
static inline dft_size_idx_t get_dft(int size)
{
  switch (size) {
    FOREACH_DFTSZ(FIND_ENUM)
    default:
      printf("function get_dft : unsupported DFT size %d\n", size);
      break;
  }
  return DFT_SIZE_IDXTABLESIZE;
}

#define SZ_iENUM(Sz) IDFT_##Sz,
typedef enum idft_size_idx {
  FOREACH_IDFTSZ(SZ_iENUM)
  IDFT_SIZE_IDXTABLESIZE
}  idft_size_idx_t;
/*******************************************************************
*
* NAME :         get_idft
*
* PARAMETERS :   size of ofdm symbol
*
* RETURN :       index pointing to the dft func in the dft library
*
* DESCRIPTION :  get idft function depending of ofdm size
*
*********************************************************************/
#define FIND_iENUM(iSz) \
  case iSz:             \
    return IDFT_##iSz;  \
    break;

static inline idft_size_idx_t get_idft(int size)
{
  switch (size) {
    FOREACH_IDFTSZ(FIND_iENUM)
    default:
      assert(false);
      break;
  }
  return IDFT_SIZE_IDXTABLESIZE; // never reached and will trigger assertion in idft function
}


#ifdef __cplusplus
}
#endif

#endif //__PHY_TOOLS_DEFS__H__

/*
 * Copyright 2017 Cisco Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _NFAPI_H_
#define _NFAPI_H_

#if defined(__cplusplus)
extern "C" {
#endif

// todo : move to public_inc so can be used by vendor extensions

#define MAX_BAD_TAG 3
int nfapitooai_level(int nfapilevel);
uint8_t push8(uint8_t in, uint8_t **out, uint8_t *end);
uint8_t pushs8(int8_t in, uint8_t **out, uint8_t *end);
uint8_t push16(uint16_t in, uint8_t **out, uint8_t *end);
uint8_t pushs16(int16_t in, uint8_t **out, uint8_t *end);
uint8_t push32(uint32_t in, uint8_t **out, uint8_t *end);
uint8_t pushs32(int32_t in, uint8_t **out, uint8_t *end);

uint8_t pull8(uint8_t **in, uint8_t *out, uint8_t *end);
uint8_t pulls8(uint8_t **in, int8_t *out, uint8_t *end);
uint8_t pull16(uint8_t **in, uint16_t *out, uint8_t *end);
uint8_t pulls16(uint8_t **in, int16_t *out, uint8_t *end);

  /*! \brief Pull an arbitrary amount of bytes ( 0 to 4 ) into an uint32_t
 *  \param length the amount of bytes to pull
 *  \param in the buffer from where to pull the bytes
 *  \param out a pointer to the destination uint32_t
 *  \param end a pointer to the end of the buffer in
 *
 *  This function is used to pull `length` amount of bytes from `in` into `out`.
 *  Used where pulling the full 32 bits would cause the `in` pointer to go over `end`.
 *  It's currently used to pull the last bytes of a payload where the buffer does not contain padding at the end.
 */
uint8_t pullx32(uint8_t length, uint8_t **in, uint32_t *out, uint8_t *end);
uint8_t pull32(uint8_t **in, uint32_t *out, uint8_t *end);
uint8_t pulls32(uint8_t **in, int32_t *out, uint8_t *end);

#if defined(__cplusplus)
}
#endif

#endif

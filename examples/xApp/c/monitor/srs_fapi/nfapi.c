/*
 * Copyright (c) 2001-2016, Cisco Systems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * Neither the name of the Cisco Systems, Inc. nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sched.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <execinfo.h>

#include "nfapi.h"
#include "debug.h"

// What to do when an error happens (e.g., a push or pull fails)
static inline void on_error()
{
  // show the call stack
  int fd = STDERR_FILENO;
  static const char msg[] = "---stack trace---\n";
  __attribute__((unused)) int r = write(fd, msg, sizeof(msg) - 1);
  void *buffer[100];
  int nptrs = backtrace(buffer, sizeof(buffer) / sizeof(buffer[0]));
  backtrace_symbols_fd(buffer, nptrs, fd);

  // abort();
}

// Fundamental routines

uint8_t push8(uint8_t in, uint8_t **out, uint8_t *end)
{
  uint8_t *pOut = *out;

  if ((end - pOut) >= 1) {
    pOut[0] = in;
    (*out) += 1;
    return 1;
  } else {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s no space in buffer\n", __FUNCTION__);
    on_error();
    return 0;
  }
}

uint8_t pushs8(int8_t in, uint8_t **out, uint8_t *end)
{
  uint8_t *pOut = *out;

  if ((end - pOut) >= 1) {
    pOut[0] = in;
    (*out) += 1;
    return 1;
  } else {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s no space in buffer\n", __FUNCTION__);
    on_error();
    return 0;
  }
}

uint8_t push16(uint16_t in, uint8_t **out, uint8_t *end)
{
  uint8_t *pOut = *out;

  if ((end - pOut) >= 2) {
#ifdef FAPI_BYTE_ORDERING_BIG_ENDIAN
    pOut[1] = (in & 0xFF00) >> 8;
    pOut[0] = (in & 0xFF);
#else
    pOut[0] = (in & 0xFF00) >> 8;
    pOut[1] = (in & 0xFF);
#endif
    (*out) += 2;
    return 2;
  } else {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s no space in buffer\n", __FUNCTION__);
    on_error();
    return 0;
  }
}

uint8_t pushs16(int16_t in, uint8_t **out, uint8_t *end)
{
  uint8_t *pOut = *out;

  if ((end - pOut) >= 2) {
#ifdef FAPI_BYTE_ORDERING_BIG_ENDIAN
    pOut[1] = (in & 0xFF00) >> 8;
    pOut[0] = (in & 0xFF);
#else
    pOut[0] = (in & 0xFF00) >> 8;
    pOut[1] = (in & 0xFF);
#endif
    (*out) += 2;
    return 2;
  } else {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s no space in buffer\n", __FUNCTION__);
    on_error();
    return 0;
  }
}

uint8_t push32(uint32_t in, uint8_t **out, uint8_t *end)
{
  uint8_t *pOut = *out;

  if ((end - pOut) >= 4) {
#ifdef FAPI_BYTE_ORDERING_BIG_ENDIAN
    pOut[3] = (in & 0xFF000000) >> 24;
    pOut[2] = (in & 0xFF0000) >> 16;
    pOut[1] = (in & 0xFF00) >> 8;
    pOut[0] = (in & 0xFF);
#else
    pOut[0] = (in & 0xFF000000) >> 24;
    pOut[1] = (in & 0xFF0000) >> 16;
    pOut[2] = (in & 0xFF00) >> 8;
    pOut[3] = (in & 0xFF);
#endif
    (*out) += 4;
    return 4;
  } else {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s no space in buffer\n", __FUNCTION__);
    on_error();
    return 0;
  }
}

uint8_t pushs32(int32_t in, uint8_t **out, uint8_t *end)
{
  uint8_t *pOut = *out;

  if ((end - pOut) >= 4) {
#ifdef FAPI_BYTE_ORDERING_BIG_ENDIAN
    pOut[3] = (in & 0xFF000000) >> 24;
    pOut[2] = (in & 0xFF0000) >> 16;
    pOut[1] = (in & 0xFF00) >> 8;
    pOut[0] = (in & 0xFF);
#else
    pOut[0] = (in & 0xFF000000) >> 24;
    pOut[1] = (in & 0xFF0000) >> 16;
    pOut[2] = (in & 0xFF00) >> 8;
    pOut[3] = (in & 0xFF);
#endif
    (*out) += 4;
    return 4;
  } else {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s no space in buffer\n", __FUNCTION__);
    on_error();
    return 0;
  }
}

uint8_t pull8(uint8_t **in, uint8_t *out, uint8_t *end)
{
  uint8_t *pIn = *in;

  if ((end - pIn) >= 1) {
    *out = *pIn;
    (*in) += 1;
    return 1;
  } else {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s no space in buffer\n", __FUNCTION__);
    on_error();
    return 0;
  }
}

uint8_t pulls8(uint8_t **in, int8_t *out, uint8_t *end)
{
  uint8_t *pIn = *in;

  if ((end - pIn) >= 1) {
    *out = *pIn;
    (*in) += 1;
    return 1;
  } else {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s no space in buffer\n", __FUNCTION__);
    on_error();
    return 0;
  }
}

uint8_t pull16(uint8_t **in, uint16_t *out, uint8_t *end)
{
  uint8_t *pIn = *in;

  if ((end - pIn) >= 2) {
#ifdef FAPI_BYTE_ORDERING_BIG_ENDIAN
    *out = ((pIn[1]) << 8) | pIn[0];
#else
    *out = ((pIn[0]) << 8) | pIn[1];
#endif
    (*in) += 2;
    return 2;
  } else {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s no space in buffer\n", __FUNCTION__);
    on_error();
    return 0;
  }
}

uint8_t pulls16(uint8_t **in, int16_t *out, uint8_t *end)
{
  uint8_t *pIn = *in;

  if ((end - pIn) >= 2) {
#ifdef FAPI_BYTE_ORDERING_BIG_ENDIAN
    *out = ((pIn[1]) << 8) | pIn[0];
#else
    *out = ((pIn[0]) << 8) | pIn[1];
#endif
    (*in) += 2;
    return 2;
  } else {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s no space in buffer\n", __FUNCTION__);
    on_error();
    return 0;
  }
}

uint8_t pullx32(uint8_t length, uint8_t **in, uint32_t *out, uint8_t *end)
{
  uint8_t *pIn = *in;
  if (length > 4) {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s Can't pull more than 4 bytes (%d) into a uint32_t\n", __FUNCTION__, length);
    on_error();
    return 0;
  }
  if ((end - pIn) >= length) {
    for (int i = 0; i < length; i++) {
#ifdef FAPI_BYTE_ORDERING_BIG_ENDIAN
      *out |= ((uint32_t)pIn[i] << (8 * i));
#else
      *out = ((uint32_t)pIn[i] << (8 * (length - 1 - i)));
#endif
    }
    (*in) += length;
    return length;
  } else {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s no space in buffer\n", __FUNCTION__);
    on_error();
    return 0;
  }
}

uint8_t pull32(uint8_t **in, uint32_t *out, uint8_t *end)
{
  uint8_t *pIn = *in;

  if ((end - pIn) >= 4) {
#ifdef FAPI_BYTE_ORDERING_BIG_ENDIAN
    *out = ((uint32_t)pIn[3] << 24) | (pIn[2] << 16) | (pIn[1] << 8) | pIn[0];
#else
    *out = ((uint32_t)pIn[0] << 24) | (pIn[1] << 16) | (pIn[2] << 8) | pIn[3];
#endif
    (*in) += 4;
    return 4;
  } else {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s no space in buffer\n", __FUNCTION__);
    on_error();
    return 0;
  }
}

uint8_t pulls32(uint8_t **in, int32_t *out, uint8_t *end)
{
  uint8_t *pIn = *in;

  if ((end - pIn) >= 4) {
#ifdef FAPI_BYTE_ORDERING_BIG_ENDIAN
    *out = (pIn[3] << 24) | (pIn[2] << 16) | (pIn[1] << 8) | pIn[0];
#else
    *out = (pIn[0] << 24) | (pIn[1] << 16) | (pIn[2] << 8) | pIn[3];
#endif
    (*in) += 4;
    return 4;
  } else {
    NFAPI_TRACE(NFAPI_TRACE_ERROR, "%s no space in buffer\n", __FUNCTION__);
    on_error();
    return 0;
  }
}


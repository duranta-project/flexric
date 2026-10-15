/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef NTP_TIME_H
#define NTP_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// RFC 5905 timestamps count seconds since 1900-01-01T00:00:00Z, Unix time since 1970-01-01T00:00:00Z
#define NTP_UNIX_EPOCH_OFFSET_S 2208988800ULL

// Seconds since the Unix epoch -> 32-bit NTP seconds (era 0 wraps in 2036, as per RFC 5905)
static inline uint32_t unix_s_to_ntp32(uint64_t unix_s)
{
  return (uint32_t)(unix_s + NTP_UNIX_EPOCH_OFFSET_S);
}

// 32-bit NTP seconds -> seconds since the Unix epoch. A clear MSB can only be
// a date past the era 0 rollover (2036), the same disambiguation used by Wireshark
static inline uint64_t ntp32_to_unix_s(uint32_t ntp_s)
{
  uint64_t s = ntp_s;
  if (!(ntp_s & 0x80000000u))
    s += 1ULL << 32;
  return s - NTP_UNIX_EPOCH_OFFSET_S;
}

// Microseconds since the Unix epoch -> 64-bit NTP timestamp (32-bit seconds, 32-bit fraction)
static inline uint64_t unix_us_to_ntp64(uint64_t unix_us)
{
  const uint64_t s = unix_us / 1000000 + NTP_UNIX_EPOCH_OFFSET_S;
  const uint64_t frac = (((unix_us % 1000000) << 32) + 500000) / 1000000;
  return ((s & 0xFFFFFFFFULL) << 32) | frac;
}

// 64-bit NTP timestamp -> microseconds since the Unix epoch; exact for any whole microsecond
static inline uint64_t ntp64_to_unix_us(uint64_t ntp)
{
  const uint64_t frac_us = (((ntp & 0xFFFFFFFFULL) * 1000000) + (1ULL << 31)) >> 32;
  return ntp32_to_unix_s((uint32_t)(ntp >> 32)) * 1000000 + frac_us;
}

#ifdef __cplusplus
}
#endif

#endif

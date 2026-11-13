/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef NTP_TIME_H
#define NTP_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Seconds since the Unix epoch -> 32-bit NTP seconds (era 0 wraps in 2036, as per RFC 5905)
uint32_t unix_s_to_ntp32(uint64_t unix_s);

// 32-bit NTP seconds -> seconds since the Unix epoch. A clear MSB can only be
// a date past the era 0 rollover (2036), the same disambiguation used by Wireshark
uint64_t ntp32_to_unix_s(uint32_t ntp_s);

// Microseconds since the Unix epoch -> 64-bit NTP timestamp (32-bit seconds, 32-bit fraction)
uint64_t unix_us_to_ntp64(uint64_t unix_us);

// 64-bit NTP timestamp -> microseconds since the Unix epoch; exact for any whole microsecond
uint64_t ntp64_to_unix_us(uint64_t ntp);

#ifdef __cplusplus
}
#endif

#endif

#pragma once

#ifndef TRUDP_OPTIONS_H
#define TRUDP_OPTIONS_H

#include <stdbool.h>
#include <stdint.h>

#include "trudp_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Global options and controls

/**
 * Enable extra debug logs in sendto function.
 *
 * @param enable - boolean, if true - enables dozens of debugging messages
 * when sending packets.
 */
TRUDP_API void trudpSetOption_DBG_sendto(bool enable);

/**
 * Enable extra debug logs while sending keepalive pings
 *
 * @param enable - boolean
 */
TRUDP_API void trudpSetOption_DBG_echoKeepalivePing(bool enable);

/**
 * Enable dumping incoming packet headers
 *
 * @param enable - boolean, should we dump headers of received packets
 */
TRUDP_API void trudpSetOption_DBG_dumpDataPacketHeaders(bool enable);

/**
 * Set core trudp timeout before first keepalive ping packet
 * by default 10 seconds
 *
 * @param delay_ms - milliseconds, must be non-negative, zero value sets to
 * default
 */
TRUDP_API void trudpSetOption_CORE_keepaliveFirstPingDelayMs(int64_t delay_ms);

/**
 * Set core trudp timeout before subsequent keepalive ping packet, in case we
 * didn't get reply to first one
 * by default 1 second
 *
 * @param delay_ms - milliseconds, must be non-negative, zero value sets to
 * default
 */
TRUDP_API void trudpSetOption_CORE_keepaliveNextPingDelayMs(int64_t delay);

/**
 * Set core trudp disconnect timeout since last received packet
 * by default 14.39 seconds
 *
 * @param timeout_ms - milliseconds, must be non-negative, zero value sets to
 * default
 */
TRUDP_API void trudpSetOption_CORE_disconnectTimeoutDelayMs(int64_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* TRUDP_OPTIONS_H */

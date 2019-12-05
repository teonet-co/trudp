#pragma once

#ifndef TRUDP_OPTIONS_H
#define TRUDP_OPTIONS_H

#include <stdbool.h>

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

#ifdef __cplusplus
}
#endif

#endif /* TRUDP_OPTIONS_H */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "teobase/logging.h"

#include "trudp_options.h"
#include "trudp_const.h"

extern bool trudpOpt_DBG_sendto;
bool trudpOpt_DBG_sendto = false;

TRUDP_API void trudpSetOption_DBG_sendto(bool enable) {
    trudpOpt_DBG_sendto = enable;
}

extern bool trudpOpt_DBG_echoKeepalivePing;
bool trudpOpt_DBG_echoKeepalivePing = false;

TRUDP_API void trudpSetOption_DBG_echoKeepalivePing(bool enable) {
    trudpOpt_DBG_echoKeepalivePing = enable;
    LTRACK_I("Trudp", "Set keepalive ping echo to %s",
             trudpOpt_DBG_echoKeepalivePing ? "true" : "false");
}

extern bool trudpOpt_DBG_dumpDataPacketHeaders;
bool trudpOpt_DBG_dumpDataPacketHeaders = false;

TRUDP_API void trudpSetOption_DBG_dumpDataPacketHeaders(bool enable) {
    trudpOpt_DBG_dumpDataPacketHeaders = enable;
}

// Send trudp ping every 10 sec
static const int64_t keepaliveFirstPingDefault_us = 10 * 1000000;

extern int64_t trudpOpt_CORE_keepaliveFirstPingDelay_us;
int64_t trudpOpt_CORE_keepaliveFirstPingDelay_us = keepaliveFirstPingDefault_us;

TRUDP_API void trudpSetOption_CORE_keepaliveFirstPingDelayMs(int64_t delay) {
    if (delay < 0) {
        LTRACK_E("Trudp", "Delay argument must be non-negative");
        abort();
    }

    trudpOpt_CORE_keepaliveFirstPingDelay_us =
        (delay == 0) ? keepaliveFirstPingDefault_us : delay * 1000;

    LTRACK_I("Trudp", "Changed keepalive first ping delay to %fsec",
             trudpOpt_CORE_keepaliveFirstPingDelay_us / 1000000.0f);
}

// Retry trudp ping every sec in case we didn't get reply for first
static const int64_t keepaliveNextPingDefault_us = 1000000;

extern int64_t trudpOpt_CORE_keepaliveNextPingDelay_us;
int64_t trudpOpt_CORE_keepaliveNextPingDelay_us = keepaliveNextPingDefault_us;

TRUDP_API void trudpSetOption_CORE_keepaliveNextPingDelayMs(int64_t delay) {
    if (delay < 0) {
        LTRACK_E("Trudp", "Delay argument must be non-negative");
        abort();
    }

    trudpOpt_CORE_keepaliveNextPingDelay_us =
        (delay == 0) ? keepaliveNextPingDefault_us : delay * 1000;

    LTRACK_I("Trudp", "Changed keepalive next ping delay to %fsec",
             trudpOpt_CORE_keepaliveNextPingDelay_us / 1000000.0f);
}

// Disconnect after timeout since last receved packet (14.39 sec)
static const int64_t disconnectTimeoutDefault_us = MAX_TRIPTIME_MIDDLE * 5;

extern int64_t trudpOpt_CORE_disconnectTimeoutDelay_us;
int64_t trudpOpt_CORE_disconnectTimeoutDelay_us = disconnectTimeoutDefault_us;

TRUDP_API void
trudpSetOption_CORE_disconnectTimeoutDelayMs(int64_t timeout_ms) {
    if (timeout_ms < 0) {
        LTRACK_E("Trudp", "Disconnect timeout argument must be non-negative");
        abort();
    }

    trudpOpt_CORE_disconnectTimeoutDelay_us =
        (timeout_ms == 0) ? disconnectTimeoutDefault_us : timeout_ms * 1000;

    LTRACK_I("Trudp", "Changed disconnect timeout to %fsec",
             trudpOpt_CORE_disconnectTimeoutDelay_us / 1000000.0f);
}

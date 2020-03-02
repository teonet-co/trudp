#include "trudp_options.h"

#include <stdbool.h>
#include <stdint.h>

extern bool trudpOpt_DBG_sendto;
bool trudpOpt_DBG_sendto = false;

void trudpSetOption_DBG_sendto(bool enable) {
    trudpOpt_DBG_sendto = enable;
}

extern bool trudpOpt_DBG_dumpDataPacketHeaders;
bool trudpOpt_DBG_dumpDataPacketHeaders = false;

void trudpSetOption_DBG_dumpDataPacketHeaders(bool enable) {
    trudpOpt_DBG_dumpDataPacketHeaders = enable;
}

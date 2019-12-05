#include "trudp_options.h"

#include <stdbool.h>
#include <stdint.h>

extern bool trudpOpt_DBG_sendto;
bool trudpOpt_DBG_sendto = false;

TRUDP_API void trudpSetOption_DBG_sendto(bool enable) {
    trudpOpt_DBG_sendto = enable;
}

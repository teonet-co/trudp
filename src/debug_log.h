#ifndef TRUDP_DEBUG_LOG_H
#define TRUDP_DEBUG_LOG_H

#if defined(__ANDROID__)
#include <android/log.h>
#elif defined(__linux__)
#include <stdio.h>
#elif defined(_WIN32)
// Set minimum supported version to Windows 7.
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define _WIN32_WINNT 0x0601

#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif

#define NTDDI_VERSION 0x06010000

#include <SDKDDKVer.h>

// Exclude rarely-used stuff from Windows headers.
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#else
#error Unsupported target OS.
#endif

static inline void debug_log_message(const char* message) {
#if defined(__ANDROID__)
    static const char* Tag = "TeonetClient";

    __android_log_print(ANDROID_LOG_ERROR, Tag, "%s", message);
#elif defined(__linux__)
    printf(message);
#else
    OutputDebugStringA(message);
#endif
}

#endif // TRUDP_DEBUG_LOG_H

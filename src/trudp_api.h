/*
 * The MIT License
 *
 * Copyright 2018 Kirill Scherba <kirill@scherba.ru>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * \file   trudp_api.h
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on March 16, 2018, 4:07 PM
 */

#ifndef TRUDP_API_H
#define TRUDP_API_H

#include "teobase/platform.h"

// Default behavior is static library.
// No defines are needed to build or use static library.
// Define TRUDP_DYNAMIC and TRUDP_EXPORTS to build dynamic library.
// Define TRUDP_DYNAMIC to import dynamic library.
// Use TRUDP_API macro on all public API functions.
// Use TRUDP_INTERNAL macro on functions used only in this library.
#if defined(TRUDP_DYNAMIC)
#if defined(TEONET_OS_WINDOWS)
#if defined(TRUDP_EXPORTS)
#define TRUDP_API __declspec(dllexport)
#else
#define TRUDP_API __declspec(dllimport)
#endif
#define TRUDP_INTERNAL
#else
#define TRUDP_API __attribute__((visibility("default")))
#define TRUDP_INTERNAL __attribute__((visibility("hidden")))
#endif
#else
#define TRUDP_API
#define TRUDP_INTERNAL
#endif

#endif /* TRUDP_API_H */

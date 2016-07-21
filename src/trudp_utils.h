/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   trudp_utils.h
 * Author: kirill
 *
 * Created on July 21, 2016, 12:33 AM
 */

#ifndef TRUDP_UTILS_H
#define TRUDP_UTILS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

char *trudpMakeKey(char *addr, int port, int channel, size_t *key_length);

#ifdef __cplusplus
}
#endif

#endif /* TRUDP_UTILS_H */

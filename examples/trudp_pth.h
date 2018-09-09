/**
 * The MIT License
 *
 * Copyright 2016-2018 Kirill Scherba <kirill@scherba.ru>.
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
 * \file   trudp_pth.h
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on August 2, 2018, 12:37 PM
 */

#ifndef TRUDP_PTH_H
#define TRUDP_PTH_H


#ifdef __cplusplus
extern "C" {
#endif
  
// Application options structure
typedef struct trudp_options {

    // Integer options
    int debug; // = 0;
    int show_statistic; // = 0;
    int show_statistic_page;
    int show_send_queue; // = 0;
    int show_snake; // = 0;
    int listen; // = 0;
    int numeric; // = 0;
    int dont_send_data; // 0
    int buf_size; // = 4096;
    int send_delay; // = 1000 ms -> Test application send delay

    // String options
    char *local_address; // = NULL;
    char *local_port; // = NULL;
    char *remote_address; // = NULL;
    char *remote_port; // = NULL;

    // Calculated options
    int remote_port_i;
    int local_port_i;
} trudp_options;
  
    
typedef struct trudp_data {
    
    trudp_options *o; // Options
    int fd; ///< UDP socket descriptor
    void *rq; ///< Pointer to readQueue
    void *td; ///< Pointer to trudpData
    int running; ///< Processing thread running flag
    pthread_t tid; ///< Processing thread id
    pthread_mutex_t mutex; ///< Processing thread mutex
    //pthread_mutex_t cv_mutex; ///< Condition variables mutex
    //pthread_cond_t cv_threshold; ///< Condition variable
    
} trudp_data_t;    

/**
 * Initialize TR-UDP 
 *
 * @param port Port (optional)
 *
 * @return Pointer to trudp_data_t
 */
trudp_data_t *trudp_init(trudp_options *o);

/**
 * Destroy TR-UDP
 *
 * @param tru Pointer to trudp_data_t
 */
void trudp_destroy(trudp_data_t *tru);

void *trudp_connect(trudp_data_t *tru, const char *address, int port);

void trudp_disconnect(trudp_data_t *tru, void *td);

void *trudp_recv(trudp_data_t *tru, void **td, size_t *msg_length);

void trudp_free_recv_data(trudp_data_t *tru, void *data);

int trudp_send(trudp_data_t *tru, void *td, void *msg, size_t msg_length);


#ifdef __cplusplus
}
#endif

#endif /* TRUDP_PTH_H */


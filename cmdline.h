/*
 * cmdline.h
 *
 *  Created on: Jul 20, 2017
 *      Author: E9990291
 */

#ifndef CMDLINE_H_
#define CMDLINE_H_

typedef struct cmd_line_desc* cmd_line_desc_ptr_t;

typedef void (*rx_callback_t)(char);
typedef void (*tx_done_callback_t)();
typedef void (*timer_10ms_callback_t)();
typedef int (*uart_tx_nb_t)(const char*, int len);
typedef int (*uart_tx_tc_t)(const char*, int len, int timeout_ms);
typedef int (*uart_rx_tc_t)(char*, int len, int timeout_ms);

int init_cmd_line(const char* prompt,
                  rx_callback_t rx_cb,
                  tx_done_callback_t tx_done_cb,
                  timer_10ms_callback_t tim_cb,
                  uart_rx_tc_t rx_fn,
                  uart_tx_tc_t tx_tc_fn,
                  uart_tx_nb_t yx_nb_fn);



#endif /* CMDLINE_H_ */

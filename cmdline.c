/*
 * cmdline.c
 *
 *  Created on: Jul 20, 2017
 *      Author: E9990291
 */

#include "cmdline.h"

struct cmd_line_desc {
  rx_callback_t rx_callback;
  tx_done_callback_t tx_done_callback;
  uart_rx_tc_t uart_rx_tc;
  uart_tx_nb_t uart_tx_nb;
  uart_tx_tc_t uart_tx_tc;
  timer_10ms_callback_t tim_callback;
  const char* prompt_text;
};


/*
 * cmdline.h
 *
 *  Created on: Jul 20, 2017
 *      Author: E9990291
 */

#ifndef CMDLINE_H_
#define CMDLINE_H_

#include "stdio.h"
#include "cmd_desc.h"

#define CMD_LINE_LIST_ITEM(item) item,
#define CMD_LINE_SUBLIST_MARK              (const cmd_desc_t*)(1)
#define CMD_LINE_INCLUDE_SUBLIST(sublist)  CMD_LINE_LIST_ITEM(CMD_LINE_SUBLIST_MARK)CMD_LINE_LIST_ITEM((cmd_desc_t*)(sublist))
#define CMD_LINE_MAX_CMD_LEN               (32)
#define CMD_LINE_MAX_INDENT                (5)

#define CMD_LINE_SUCCESS                    (0)
#define CMD_LINE_ERR_GENERAL                (1)
#define CMD_LINE_ERR_PARSE                  (2)
#define CMD_LINE_ERR_CMD_NOT_FOUND          (3)
#define CMD_LINE_NO_CMD                     (4)
#define CMD_LINE_ERR_OUT_OF_MEM             (5)

#define CMD_LINE_PRINTF_ERR_PARSE           (-1)
#define CMD_LINE_PRINTF_ERR_LENGTH          (-2)

typedef struct cmd_line_desc* cmd_line_desc_ptr_t;

typedef void (*rx_callback_t)(char);
typedef void (*tx_done_callback_t)();
typedef void (*timer_10ms_callback_t)();
typedef int (*uart_tx_nb_t)(const char*, int len);
typedef int (*uart_tx_tc_t)(const char*, int len, int timeout_ms);
typedef int (*uart_rx_tc_t)(cmd_line_desc_ptr_t cmd_line_desc, char*, int len, int timeout_ms);

typedef int (*cmd_do_fcn_t)(int argc, char** argv, cmd_line_desc_ptr_t cmd_line_desc);

typedef struct cmd_desc_st {
  const char* name;
  const char* one_line;
  const char* description;
  cmd_do_fcn_t cmd_fcn;
} cmd_desc_t;

typedef struct cmd_line_init_st {
  const char* prompt;
  uart_rx_tc_t rx_fn;
  uart_tx_tc_t tx_tc_fn;
  uart_tx_nb_t tx_nb_fn;
  char* tx_buf;
  int tx_buf_len;
  char* rx_buf;
  int rx_buf_len;
  char* line_buf;
  uint32_t line_buf_size;
  uint32_t max_args_cnt;
  uint32_t io_timeout_ms;
  char** args_starts;
  char echo_enabled;
  const cmd_desc_t** cmd_root_lis;
} cmd_line_init_t;

cmd_line_desc_ptr_t cmd_line_init(const cmd_line_init_t* init);

int cmd_line_process(cmd_line_desc_ptr_t cmd_line_desc);

const char* cmd_line_get_prompt(cmd_line_desc_ptr_t cmd_line_desc);

int cmd_line_printf(cmd_line_desc_ptr_t cmd_line_desc, const char *format, ...);

int cmd_line_printf_tk(cmd_line_desc_ptr_t cmd_line_desc, const char *format, ...);

char cmd_line_getchar(cmd_line_desc_ptr_t cmd_line_desc);

char cmd_line_getchar_tk(cmd_line_desc_ptr_t cmd_line_desc);

const cmd_desc_t** cmd_line_get_root_list(cmd_line_desc_ptr_t cmd_line_desc);

void cmd_line_uart_rx_cb(cmd_line_desc_ptr_t cmd_line_desc, char c);

const cmd_desc_t* cmd_line_find_command_by_name(const cmd_desc_t** cmd_line_root_list, const char* cmd_name, uint8_t indent_level);

#endif /* CMDLINE_H_ */

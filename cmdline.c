/*
 * Command line processor internals
 *
 *  Created on: Jul 20, 2017
 *  Author: Michal Horn
 */

#include "cmdline.h"
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

/**
 * Definition of command line instance descriptor.
 *
 * Since the descriptor itself is defined as ADT - using incomplete data type in
 * the header file, all following data is hidden from the command line user.
 */
struct cmd_line_desc {
  uart_rx_tc_t uart_rx_tc;
  uart_tx_nb_t uart_tx_nb;
  uart_tx_tc_t uart_tx_tc;
  int io_timeout_ms;
  const char* prompt_text;
  char* tx_buf;
  int tx_buf_len;
  int tx_err_cnt;
  char* rx_buf;
  /* FIXME: make it separate ring buffer type. */
  int rx_buf_head;
  int rx_buf_tail;
  int rx_buf_max_len;
  int rx_err_cnt;
  char* line_buf;
  int line_buf_len;
  int line_buf_current;
  char** arg_starts;
  int max_args;
  char echo_enabled;
  const cmd_desc_t** cmd_line_root_list;
};


/**
 * Command lines pool
 */
struct cmd_lines_pool_st {
  struct cmd_line_desc cmd_lines_pool[CMD_LINES_MAX_CNT];
  uint8_t current_cmd_line;
};

/**
 * Memory pool for command lines allocation. To not use malloc and heap.
 */
static struct cmd_lines_pool_st cmd_lines_pool;

/* Internal functions. */
int rb_push(cmd_line_desc_ptr_t cmd_line_desc, char c) {
  int next = cmd_line_desc->rx_buf_head + 1;
  if (next >= cmd_line_desc->rx_buf_max_len) {
      next = 0;
  }

  if (next == cmd_line_desc->rx_buf_tail) {
      return -1;
  }

  cmd_line_desc->rx_buf[cmd_line_desc->rx_buf_head] = c;
  cmd_line_desc->rx_buf_head = next;

  return 0;
}

int rb_pop(cmd_line_desc_ptr_t cmd_line_desc, char* c) {
  if (cmd_line_desc->rx_buf_head == cmd_line_desc->rx_buf_tail) {
      return -1;
  }

  int next = cmd_line_desc->rx_buf_tail + 1;
  if(next >= cmd_line_desc->rx_buf_max_len)
      next = 0;

  *c = cmd_line_desc->rx_buf[cmd_line_desc->rx_buf_tail];
  cmd_line_desc->rx_buf_tail = next;

  return 0;
}

int cmd_line_printf_parse(cmd_line_desc_ptr_t cmd_line_desc, const char *format, ...) {
  va_list args;
  va_start (args, format);
  int ret_val = vsnprintf(cmd_line_desc->tx_buf, cmd_line_desc->tx_buf_len, format, args);
  if (ret_val < 0) {
      return CMD_LINE_PRINTF_ERR_PARSE;
  }
  else if (ret_val > cmd_line_desc->tx_buf_len) {
      return CMD_LINE_PRINTF_ERR_LENGTH;
  }
  va_end (args);
  return strlen(cmd_line_desc->tx_buf);
}

/* Public functions */
const cmd_desc_t* cmd_line_find_command_by_name(const cmd_desc_t** cmd_line_root_list, const char* cmd_name, uint8_t indent_level) {
  if (cmd_name == NULL || cmd_line_root_list == NULL) {
      return NULL;
  }

  if (indent_level >= CMD_LINE_MAX_INDENT) {
      return NULL;
  }

  int i;
  const cmd_desc_t* cmd_ptr = NULL;

  for (i = 0; 1; i++) {
      if (cmd_line_root_list[i] == CMD_LINE_SUBLIST_MARK) {
          cmd_ptr = cmd_line_find_command_by_name(cmd_line_root_list[i+1], cmd_name, indent_level+1);
          if (cmd_ptr != NULL) {
              break;
          }
      }
      else if (cmd_line_root_list[i] == NULL) {
          break;
      }
      else {
          if (strcmp(cmd_line_root_list[i]->name, cmd_name) == 0) {
              cmd_ptr = cmd_line_root_list[i];
              break;
          }
      }
  }

  return cmd_ptr;
}

cmd_line_desc_ptr_t cmd_line_init(const cmd_line_init_t* init) {
  if (init == NULL) {
      return NULL;
  }

  if (cmd_lines_pool.current_cmd_line >= CMD_LINES_MAX_CNT) {
      return NULL;
  }

  cmd_line_desc_ptr_t cmd_line_desc = &cmd_lines_pool.cmd_lines_pool[cmd_lines_pool.current_cmd_line++];
  cmd_line_desc->arg_starts = init->args_starts;
  cmd_line_desc->cmd_line_root_list = init->cmd_root_lis;
  cmd_line_desc->echo_enabled = init->echo_enabled;
  cmd_line_desc->io_timeout_ms = init->io_timeout_ms;
  cmd_line_desc->line_buf = init->line_buf;
  cmd_line_desc->line_buf_current = 0;
  cmd_line_desc->line_buf_len = init->line_buf_size;
  cmd_line_desc->max_args = init->max_args_cnt;
  cmd_line_desc->prompt_text = init->prompt;
  cmd_line_desc->rx_buf = init->rx_buf;
  cmd_line_desc->rx_buf_head = 0;
  cmd_line_desc->rx_buf_max_len = init->rx_buf_len;
  cmd_line_desc->rx_buf_tail = 0;
  cmd_line_desc->rx_err_cnt = 0;
  cmd_line_desc->tx_buf = init->tx_buf;
  cmd_line_desc->tx_buf_len = init->tx_buf_len;
  cmd_line_desc->tx_err_cnt = 0;
  cmd_line_desc->uart_rx_tc = init->rx_fn;
  cmd_line_desc->uart_tx_nb = init->tx_nb_fn;
  cmd_line_desc->uart_tx_tc = init->tx_tc_fn;

  return cmd_line_desc;
}

int cmd_line_process(cmd_line_desc_ptr_t cmd_line_desc) {
  int retVal = CMD_LINE_NO_CMD;
  int8_t c = cmd_line_getchar(cmd_line_desc);
  if (c == EOF) {
      return retVal;
  }

  if (isprint(c)) {
    if (cmd_line_desc->line_buf_current < cmd_line_desc->line_buf_len) {
        if (cmd_line_desc->echo_enabled) {
            cmd_line_printf_tk(cmd_line_desc, "%c", c, 1000);
        }
        cmd_line_desc->line_buf[cmd_line_desc->line_buf_current++] = c;
    }
  }
  else if (c == '\r') {
      if (cmd_line_desc->echo_enabled) {
          cmd_line_printf_tk(cmd_line_desc, "%c", c, 1000);
      }
  }
  else if (c == '\n') {
      if (cmd_line_desc->echo_enabled) {
          cmd_line_printf_tk(cmd_line_desc, "%c", c, 1000);
      }
      cmd_line_desc->line_buf[cmd_line_desc->line_buf_current] = '\0';
      int i;
      int argc = 0;
      cmd_line_desc->arg_starts[argc] = &cmd_line_desc->line_buf[0];
      for (i = 0; i < cmd_line_desc->line_buf_current; i++) {
          if (cmd_line_desc->line_buf[i] == ' ') {
              cmd_line_desc->line_buf[i] = '\0';
              argc++;
              cmd_line_desc->arg_starts[argc] = &cmd_line_desc->line_buf[i+1];
          }
      }
      cmd_line_desc->line_buf_current = 0;
      const cmd_desc_t* cmd_ptr = cmd_line_find_command_by_name(cmd_line_desc->cmd_line_root_list, cmd_line_desc->arg_starts[0], 0);
      if (cmd_ptr != NULL) {
          retVal = cmd_ptr->cmd_fcn(argc, cmd_line_desc->arg_starts, cmd_line_desc);
      }
      else {
          retVal =  CMD_LINE_ERR_CMD_NOT_FOUND;
      }
  }
  return retVal;
}

const char* cmd_line_get_prompt(cmd_line_desc_ptr_t cmd_line_desc) {
  return cmd_line_desc->prompt_text;
}

int cmd_line_printf(cmd_line_desc_ptr_t cmd_line_desc, const char *format, ...) {
  /* FIXME: use ring buffer to add characters, otherwise they may be overwritten by next call. */
  int str_len = cmd_line_printf_parse(cmd_line_desc, format);
  if (str_len > 0) {
      if (cmd_line_desc->uart_tx_nb(cmd_line_desc->tx_buf, str_len) != CMD_LINE_SUCCESS) {
          cmd_line_desc->tx_err_cnt++;
          return -CMD_LINE_ERR_GENERAL;
      }
  }
  return str_len;
}

int cmd_line_printf_tk(cmd_line_desc_ptr_t cmd_line_desc, const char *format, ...) {
  int str_len = cmd_line_printf_parse(cmd_line_desc, format);
  if (str_len > 0) {
      if (cmd_line_desc->uart_tx_tc(cmd_line_desc->tx_buf, str_len, 1000) != CMD_LINE_SUCCESS) {
          cmd_line_desc->tx_err_cnt++;
          return -CMD_LINE_ERR_GENERAL;
      }
  }
  return str_len;
}

char cmd_line_getchar(cmd_line_desc_ptr_t cmd_line_desc) {
  char c;
  if (rb_pop(cmd_line_desc, &c) < 0) {
      return EOF;
  }
  return c;
}

char cmd_line_getchar_tk(cmd_line_desc_ptr_t cmd_line_desc) {
  char c;
  while (rb_pop(cmd_line_desc, &c) < 0) ;
  return c;
}

const cmd_desc_t** cmd_line_get_root_list(cmd_line_desc_ptr_t cmd_line_desc) {
  return cmd_line_desc->cmd_line_root_list;
}

void cmd_line_uart_rx_cb(cmd_line_desc_ptr_t cmd_line_desc, char c) {
  if (rb_push(cmd_line_desc, c) != 0) {
      cmd_line_desc->rx_err_cnt++;
  }
}

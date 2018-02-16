/*
 * =Command line processor module header file
 *
 * Include this file to your code where:
 * * you define commands,
 * * you want to use IO functions
 * * you want to call the command line main loop function @ref cmd_line_process.
 *
 *  Created on: Jul 20, 2017
 *  Author: Michal Horn
 */

/** @defgroup Cmd_Line_Lib_Cmd_Def Commands
 *  <h1> Module for defining lists of commands. </h1>
 *  <h2>Command</h2>
 *  <p>Command is defined by the @ref cmd_desc_t data type. Every command is identified by its unique name (@ref cmd_desc_st.name).</p>
 *  <p>Command has a brief one line description (@ref cmd_desc_st.one_line) and a detailed description (@ref cmd_desc_st.description)</p>
 *  <p>The routine done by the command is defined as a function with the @ref cmd_do_fcn_t notation.</p>
 *  <h2>List of commands</h2>
 *  <p>Commands are grouped together into lists. The List is an array of commands, terminated by @ref CMD_LINE_END_OF_LIST.</p>
 *  <p>Lists can be nested by @ref CMD_LINE_INCLUDE_SUBLIST macro. The maximum depth is defined by @ref CMD_LINE_MAX_INDENT.</p>
 *  <h2>Basic commands</h2>
 *  <p>The Command line module is shipped with a basic set of commands defined in @ref basic_cmd_list. Feel free to use this list as the root list for @ref cmd_line_init or to include it as a sublist into your own commands list.</p>
 */

#ifndef CMDLINE_H_
#define CMDLINE_H_

#include "stdio.h"

#define CMD_LINE_LIST_ITEM(item) item,
#define CMD_LINE_SUBLIST_MARK              (const cmd_desc_t*)(1) /**< Mark of sublist inclusion. */

/*! \addtogroup Cmd_Line_Lib_Cmd_Def
*  @{
*/
#define CMD_LINE_INCLUDE_SUBLIST(sublist)  CMD_LINE_LIST_ITEM(CMD_LINE_SUBLIST_MARK)CMD_LINE_LIST_ITEM((cmd_desc_t*)(sublist)) /**< Includes commands sublist. */
#define CMD_LINE_END_OF_LIST               NULL /**< Marks the end of list/sublist. */
/**@}*/ // Cmd_Line_Lib_Cmd_Def

/*! \addtogroup Cmd_Line_Lib_Config Configuration
*  @{
*/
#define CMD_LINE_MAX_CMD_LEN               (32) /**< Maximal command line length. */
#define CMD_LINE_MAX_INDENT                (5)  /**< Maximal sublists indentation. */
#define CMD_LINES_MAX_CNT                  (2)  /**< Maximal number of command line instances. */
/**@}*/ // Cmd_Line_Lib_Config

/*! \addtogroup Cmd_Line_Lib_Interface
*  @{
*/
/*! \addtogroup Cmd_Line_Lib_Return_Values Return values
*  @{
*/
/* FIXME: Make enumeration. */
#define CMD_LINE_SUCCESS                    (0) /**< Command line function exited with success. */
#define CMD_LINE_ERR_GENERAL                (1) /**< Unexpected error. */
#define CMD_LINE_ERR_PARSE                  (2) /**< Command parsing error. */
#define CMD_LINE_ERR_CMD_NOT_FOUND          (3) /**< Command not found. */
#define CMD_LINE_NO_CMD                     (4) /**< No command to execute. */
#define CMD_LINE_ERR_OUT_OF_MEM             (5) /**< Out of memory error. */

#define CMD_LINE_PRINTF_ERR_PARSE           (-1)  /**< IO function error - parsing parameters failed. */
#define CMD_LINE_PRINTF_ERR_LENGTH          (-2)  /**< IO function error - buffer size exceeded. */
/**@}*/ // Cmd_Line_Lib_Return_Values

typedef struct cmd_line_desc* cmd_line_desc_ptr_t;  /**< Command line instance descriptor. */
/**@}*/ // Cmd_Line_Lib_Interface

/*! \addtogroup Cmd_Line_Lib_Interface Interface
*  @{
*/
/*! \addtogroup Cmd_Line_Lib_Interface_Wrappers Wrappers
*  @{
*/
typedef int (*uart_tx_nb_t)(const char*, int len);
typedef int (*uart_tx_tc_t)(const char*, int len, int timeout_ms);
typedef int (*uart_rx_tc_t)(cmd_line_desc_ptr_t cmd_line_desc, char*, int len, int timeout_ms);
/**@}*/ // Cmd_Line_Lib_Interface_Wrappers
/**@}*/ // Cmd_Line_Lib_Interface

/*! \addtogroup Cmd_Line_Lib_Cmd_Def
*  @{
*/
typedef int (*cmd_do_fcn_t)(int argc, char** argv, cmd_line_desc_ptr_t cmd_line_desc);

/**
 * Command descriptor
 */
typedef struct cmd_desc_st {
  const char* name;         /**< Command name. Alphanumeric value without white spaces. */
  const char* one_line;     /**< Brief command description. Fits into one line. */
  const char* description;  /**< Detailed command description with syntax description and example. */
  cmd_do_fcn_t cmd_fcn;     /**< Command executive function. */
} cmd_desc_t;
/**@}*/ // Cmd_Line_Lib_Cmd_Def

/*! \addtogroup Cmd_Line_Lib_Interface
*  @{
*/
/**
 * Command line initialization data structure.
 *
 * Declare and fill this structure in your program to pass it into @ref cmd_line_init function.
 * @ingroup Cmd_Line_Lib_Interface
 */
typedef struct cmd_line_init_st {
  const char* prompt;     /**< String to be used as prompt, signaling that command line processor is ready for another command. */
  uart_rx_tc_t rx_fn;     /**< Pointer to blocking char receive function. Define the wrapper function in your program. */
  uart_tx_tc_t tx_tc_fn;  /**< Pointer to blocking transmit function. Define the wrapper function in your program. */
  uart_tx_nb_t tx_nb_fn;  /**< Pointer to nonblocking transmit function. Define the wrapper function in your program. */
  char* tx_buf;           /**< Transmit buffer. Define in your program. */
  int tx_buf_len;         /**< Transmit buffer size. */
  char* rx_buf;           /**< Receive buffer.  Define in your program. */
  int rx_buf_len;         /**< Receive buffer size. */
  char* line_buf;         /**< Buffer for complete received line to be processed.  Define in your program. */
  uint32_t line_buf_size; /**< Line buffer size. */
  uint32_t max_args_cnt;  /**< Maximum number of arguments.  */
  uint32_t io_timeout_ms; /**< Transmission timeout in milliseconds. */
  char** args_starts;     /**< Buffer of pointers to beginnings of each argument.  Define in your program. */
  char echo_enabled;      /**< 1 - Echo received characters, 0 - no echo. */
  const cmd_desc_t** cmd_root_lis;  /**< Pointer to root command list. */
} cmd_line_init_t;

/**
 * Initialize the command line processor.
 *
 * Call this function at the start of your program, before any other command line functions.
 *
 * @param [in] init Command line instance initialization data.
 * @return Command line instance descriptor.
 */
cmd_line_desc_ptr_t cmd_line_init(const cmd_line_init_t* init);

/**
 * Command line processor main function.
 *
 * Call this function periodically, e.g. in your main application loop, to process received commands.
 *
 * @param [in] cmd_line_desc Command line instance descriptor.
 * @return  @ref CMD_LINE_SUCCESS if a command was successfully executed,
 *          @ref CMD_LINE_NO_CMD when the command line processor exited successfully without executing any command,
 *          or error code in case of failure.
 */
int cmd_line_process(cmd_line_desc_ptr_t cmd_line_desc);

/**
 * Get the command line prompt string.
 *
 * @param [in] cmd_line_desc Command line instance descriptor.
 * @return Pointer to the command line instance prompt string.
 */
const char* cmd_line_get_prompt(cmd_line_desc_ptr_t cmd_line_desc);

/*! \addtogroup Cmd_Line_Lib_Public_IO_Funtions Input Output
*  @{
*/
/**
 * Print formatted string - nonblocking.
 *
 * C stdio printf like function for printing formatted text to the command line output.
 * Returns immediately. All characters that will not fit into transmit buffer (@ref cmd_line_init_t.tx_buf) are discarded.
 *
 * @param [in] cmd_line_desc Command line instance descriptor.
 * @param [in] format printf like formatted string.
 *
 * @return Number of printed characters, or negative error code value.
 */
int cmd_line_printf(cmd_line_desc_ptr_t cmd_line_desc, const char *format, ...);

/**
 * Print formatted string - blocking.
 *
 * C stdio printf like function for printing formatted text to the command line output.
 * Blocks until all characters are printed.
 *
 * @param [in] cmd_line_desc Command line instance descriptor.
 * @param [in] format printf like formatted string.
 *
 * @return Number of printed characters, or negative error code value.
 */
int cmd_line_printf_tk(cmd_line_desc_ptr_t cmd_line_desc, const char *format, ...);

/**
 * Get char - nonblocking.
 *
 * Returns immediately.
 *
 * @param [in] cmd_line_desc Command line instance descriptor.
 * @return Received character if any, EOF if the receive buffer is empty.
 */
char cmd_line_getchar(cmd_line_desc_ptr_t cmd_line_desc);

/**
 * Get char - blocking.
 *
 * Waits for character reception if the receive buffer is empty.
 *
 * @param [in] cmd_line_desc Command line instance descriptor.
 * @return Received character.
 */
char cmd_line_getchar_tk(cmd_line_desc_ptr_t cmd_line_desc);
/**@}*/ // Cmd_Line_Lib_Public_IO_Funtions

/**
 * Get the root commands list.
 *
 * @param [in] cmd_line_desc Command line instance descriptor.
 * @return The pointer to the root commands list.
 */
const cmd_desc_t** cmd_line_get_root_list(cmd_line_desc_ptr_t cmd_line_desc);

/**
 * Character received callback.
 *
 * Call this when a character is received, e.g. in your UART RX ISR.
 *
 * @param [in] cmd_line_desc Command line instance descriptor.
 * @param [in] c Received character.
 */
void cmd_line_uart_rx_cb(cmd_line_desc_ptr_t cmd_line_desc, char c);

/**
 * Find command by its name.
 *
 * @param [in] cmd_line_root_list The root command list to search for command.
 * @param [in] cmd_name The name of the command to be looked for.
 * @param [in] indent_level Must be set to 0.
 *
 * @return Pointer to the command descriptor, or NULL if no such command is defined.
 */
const cmd_desc_t* cmd_line_find_command_by_name(const cmd_desc_t** cmd_line_root_list, const char* cmd_name, uint8_t indent_level);
/**@}*/ // Cmd_Line_Lib_Interface

#endif /* CMDLINE_H_ */

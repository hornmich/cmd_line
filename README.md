# Embedded command line

[Introduction](#introduction)

[Features](#features)

[Footprint](#footprint)

[Quick start](#quick-start)

[Default commands](#default-commands)

[IO Functions](#io-functions)


## Introduction

This project aims to provide easy to use, platform independent and versatile module for implementing interactive command line for bare metal (or RTOS driven) embedded systems.

See the [complete Doxygen documentation](https://hornmich.github.io/cmd_line/index.html "Complete Doxygen documentation").

See the [example for Nucleo-F103RB board](https://github.com/hornmich/cmd_line_nucleo_ex "Nucleo-F103RB example") with FreeRTOS and STM32 CubeMX generated HAL

## Features

* Multiple instances
* Embedded Help for the commands
* Commands with parameters
* Easy user commands definition
* Supports code reuse by nesting lists of commands
* Hardware abstraction - easy porting
* Provides IO tools for your commands

## Footprint

TBD
* Stack size: XXXXX
* Flash memory: XXXXX
* RAM: XXXXX

## Quick start

To start using the Command line module just:

* clone the repository (preferably as a submodule) into your project,
* add
``` C
	#include "cmdline.h"
```
in your source code where you plan to use the module, for example the `main.c`. 
* Define your commands
* Fill in the initialization parameters for the instance configuration
* Create your command line instance 
* Call periodically the main command line routine from RTOS task or in the main application cycle.

### Configuration

The Command line modules configuration is separated into global configuration, that applies for all instances and to instance configuration, affecting just one command line instance.

#### Global configuration

The global configuration is implemented as a set of C preprocessor macros, defined in `cmdline.h` file, in Doxygen group `Cmd_Line_Lib_Config`.

There are three macros defined. You will most of the time be fine with the default values. However if you are looking for memory footprint optimization, this is a good place to start.

* `CMD_LINE_MAX_CMD_LEN` - Specifies the maximal number of characters for one line input, including command name and all parameters. This option affects the RW memory section usage.
* `CMD_LINE_MAX_INDENT` - Specifies the maximal indentation of the nested command lists. This option extensively affects the usage of the stack.
* `CMD_LINES_MAX_CNT` - Specifies the maximal number of Command line instances. E.g. if you want to have two separated command lines with different set of commands for UART1 and UART2 interfaces. This option affects heavily the use of RW memory section.

#### Instance configuration

The instance specific configuration is encapsulated by a `cmd_line_init_t` structure. In order to configure you instance, you need to define and fill in an instance of this structure and pass it to a `cmd_line_init`.

The structer consists of the following parameters:
* `prompt` - Pointer to a string that will be used as a prompt. E.G "-->". Can be NULL.
* `rx_fn` - Pointer to a wrapper function for blocking character reception. This function has to be define by you and pointed to by this parameter.
* `tx_tc_fn` - Pointer to a wrapper function for blocking data transmit function. This function has to be define by you and pointed to by this parameter.
* `tx_nb_fn` - Pointer to a wrapper function for not blocking data transmit function. This function has to be define by you and pointed to by this parameter.
* `tx_buf` - Pointer to a buffer for storing data to be transmitted. This buffer has to be define by you and pointed to by this parameter, e.g. `cmd_line_tx_buf[CMD_LINE_TX_BUF_LEN];`
* `tx_buf_len` - The size of the `tx_buf` buffer. 
* `rx_buf` - Pointer to a buffer for storing received data. This buffer has to be define by you and pointed to by this parameter.
* `rx_buf_len` - The size of the `rx_buf` buffer, e.g. `cmd_line_rx_buf[CMD_LINE_RX_BUF_LEN];` 
* `line_buf` - Buffer used internally to store one complete line for further processing. This buffer has to be define by you and pointed to by this parameter, e.g. `cmd_line_line_buf[CMD_LINE_LINE_BUF_LEN];`
* `line_buf_size` - The size of the `line_buf` buffer. 
* `max_args_cnt` - Specifies maximal number of parameters for any command.
* `io_timeout_ms` - Specifies the timeout durationn for IO operations in miliseconds.
* `args_starts` - Buffer used internally to point to arguments starts. This buffer has to be define by you and pointed to by this parameter, e.g. `char* cmd_line_args_starts[CMD_LINE_MAX_ARGS_CNT];`
* `echo_enabled` - Enables or disables echo.
* `cmd_root_lis` - Pointer to root list of commands.

After filling in all parameters, pass the structure to the init function, like in the following example:
``` C
  cmd_line_desc_ptr_t command_line;

  cmd_line_init_t cmd_line_init_data = {
      .prompt = CMD_LINE_PROMPT,
      .tx_tc_fn = uart_tx_tc,
      .tx_nb_fn = uart_tx_nb,
      .rx_fn = uart_rx_tc,
      .tx_buf = cmd_line_tx_buf,
      .tx_buf_len = CMD_LINE_TX_BUF_LEN,
      .rx_buf = cmd_line_rx_buf,
      .rx_buf_len = CMD_LINE_RX_BUF_LEN,
      .line_buf = cmd_line_line_buf,
      .line_buf_size = CMD_LINE_LINE_BUF_LEN,
      .max_args_cnt = CMD_LINE_MAX_ARGS_CNT,
      .io_timeout_ms = CMD_LINE_IO_TIMEOUT_MS,
      .args_starts = cmd_line_args_starts,
      .echo_enabled = CMD_LINE_ECHO_ENABLED,
      .cmd_root_lis = nucleo_cmd_list,
  };
  
  command_line = cmd_line_init(&cmd_line_init_data);
  if (command_line == NULL) {
      Error_Handler();
  }
```

### Define wrappers

To make the module as portable as possible, there is an abstraction layer for the IO functions, called wrapper functions.

This layer consists of 3 function pointers in the initialization structure with the following signature:

 * `int (*uart_tx_nb_t)(const char*, int len)`
 * `int (*uart_tx_tc_t)(const char*, int len, int timeout_ms)` 
 * `int (*uart_rx_tc_t)(cmd_line_desc_ptr_t cmd_line_desc, char*, int len, int timeout_ms)`
 
 In your code you therefore need to define the functions with that signature and in the body put your hardware specific function calls. See the example below of one concrete implementation:
 
 ``` C
void uart_rx_cb(char c) {

}

int uart_tx_nb(const char* str, int len) {
  if (HAL_UART_Transmit_DMA(&huart2, (uint8_t*)str, len) != HAL_OK) {
      return CMD_LINE_ERR_GENERAL;
  }
  return CMD_LINE_SUCCESS;
}

int uart_tx_tc(const char* str, int len, int timeout_ms) {
  if (HAL_UART_Transmit(&huart2, (uint8_t*)str, len, timeout_ms) != HAL_OK) {
      return CMD_LINE_ERR_GENERAL;
  }
  return CMD_LINE_SUCCESS;
}

int uart_rx_tc(cmd_line_desc_ptr_t cmd_line_desc, char* str, int len, int timeout_ms) {
  if (HAL_UART_Receive(&huart2, (uint8_t*)str, len, timeout_ms) != HAL_OK) {
      return CMD_LINE_ERR_GENERAL;
  }
  return CMD_LINE_SUCCESS;
}
 ```

### Define commands

Commands are grouped into lists. Depending on your global configuration macro `CMD_LINE_MAX_INDENT`, the list can contain other sublists or direct commands.

#### Root list

Each command line instance is linked with a root list of commands. Default root list is defined in `cmd.c` and `cmd.h` files as `basic_cmd_list`. Feel free to use and modify this root list as needed, for example by inserting you own set of sommands as sublists.

#### Sublist

You can use any command list as a sublist by using `CMD_LINE_INCLUDE_SUBLIST(sublist )` macro.

E.g. inclusion of your own list of commands into the default root commands would look like this:

``` C
const cmd_desc_t* basic_cmd_list[] =
  {
      &cmd_desc_help,
      &cmd_desc_version,
      CMD_LINE_INCLUDE_SUBLIST(mylist),
      CMD_LINE_END_OF_LIST
  };
```

Using of sublist feature can help you in better code organization and code reuse between multiple command line instances.

#### Command

Before you will define your own lists of commands, it is important to understand forst how to define a command.

Every command is defined by a function and by metadata. The command function has the following notation:

``` C
    int (*cmd_do_fcn_t)(int argc, char** argv, cmd_line_desc_ptr_t cmd_line_desc)
```

The body of the function is the place where to put the code that will be executed when the command is invoked.

The metadata are a structure with the fields:
* `name` - The name of the command without any white characters. The name is used to invoke the command.
* `one_line` - A brief description of the command.
* `description` - A detailed description of the command, including parameters and expected output.
* `cmd_fcn` - Pointer to the command function, described above.

The command list is actually an array of pointers to descriptors. Therefore creating a list is done simply by defining the array of `cmd_desc_t` pointers.

#### Example
See the example of LED control commands list.

The header file with external declaration of the array of descriptors:

``` C
#ifndef LED_CMD_H_
#define LED_CMD_H_

#include "cmdline.h"

extern const cmd_desc_t* led_cmd_list[];

#endif /* LED_CMD_H_ */
```

The source file with the command function bodies, the metadata filled and assigned to the array declared in the header file.

``` C
#include "led_cmd.h"
#include "stm32f1xx_hal.h"

int cmd_do_ledon(int argc, char** argv, cmd_line_desc_ptr_t cmd_line_desc) {
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  cmd_line_printf_tk(cmd_line_desc, "LED ON\r\n");
  return CMD_LINE_SUCCESS;
}

int cmd_do_ledoff(int argc, char** argv, cmd_line_desc_ptr_t cmd_line_desc) {
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
  cmd_line_printf_tk(cmd_line_desc, "LED OFF\r\n");
  return CMD_LINE_SUCCESS;
}

int cmd_do_led(int argc, char** argv, cmd_line_desc_ptr_t cmd_line_desc) {
  if (argc == 0) {
      GPIO_PinState led_state = HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin);
      if (led_state == GPIO_PIN_SET) {
          cmd_line_printf_tk(cmd_line_desc, "LED is ON\r\n");
      }
      else {
          cmd_line_printf_tk(cmd_line_desc, "LED is OFF\r\n");
      }
  }

  else {
      switch (argv[1][0]) {
        case '0':
          HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
          cmd_line_printf_tk(cmd_line_desc, "LED OFF\r\n");
          break;
        case '1':
          HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
          cmd_line_printf_tk(cmd_line_desc, "LED ON\r\n");
          break;
        default:
          cmd_line_printf_tk(cmd_line_desc, "Invalid parameter.\r\n");
          break;
      }
  }

  return CMD_LINE_SUCCESS;
}


cmd_desc_t cmd_desc_led_on =
    {
        .name = "ledon",
        .one_line = "Set LD2 on",
        .description = "ledon\r\n"
            "Set LD2 on Nucleo board on.\r\n"
            "\r\n"
            "Example:\r\n"
            "\t--> ledon\r\n"
            "\tLED ON\r\n",
        .cmd_fcn = &cmd_do_ledon
    };

cmd_desc_t cmd_desc_led_off =
    {
        .name = "ledoff",
        .one_line = "Set LD2 off",
        .description = "ledoff\r\n"
            "Set LD2 on Nucleo board off.\r\n"
            "\r\n"
            "Example:\r\n"
            "\t--> ledoff\r\n"
            "\tLED OFF\r\n",
        .cmd_fcn = &cmd_do_ledoff
    };

cmd_desc_t cmd_desc_led =
    {
        .name = "led",
        .one_line = "Set or get LED state",
        .description = "led {state}\r\n"
            "If state present, sets the LD2 state to its value.\r\n"
            "If state is not present, reads the current LD2 state."
            "\r\n"
            "Example:\r\n"
            "\t--> led 1\r\n"
            "\tLED ON\r\n",
            "\t--> led\r\n"
            "\tLED is ON\r\n",
        .cmd_fcn = &cmd_do_led
    };

const cmd_desc_t* led_cmd_list[] =
  {
      &cmd_desc_led_on,
      &cmd_desc_led_off,
      &cmd_desc_led,
      NULL
  };
```

To use this custom list of commands as a sublist you just need to put the include directive to the cmd.c file and modify the default list:

``` C
#include "led_cmd.h"
#include "cmd.h"

/* Defauld commands definiton cutted of...
	.....
 */
const cmd_desc_t* basic_cmd_list[] =
  {
      &cmd_desc_help,
      &cmd_desc_version,
      CMD_LINE_INCLUDE_SUBLIST(led_cmd_list),
      CMD_LINE_END_OF_LIST
  };
```

Or if this is the only set of commands, you may reference to it as the root list in the initialization structure.

## Default commands

The module is shipped with a very basic set of commands that alows you to start righ now and evaluate the function of your configuration (wrapers, buffers, UART periphery, RTOS, etc.)

This basic set contais the following commands:
* `help` - If used standalone, prints the list of all commands with their brief description. If used with another commands names as parameters, prints detailes description for each one of them.
* `version` - Prints out the firmware version.

## IO Functions

The module defines a set of IO functions that may be usefull in your commands to print texts to the terminal or to get user input:

* `cmd_line_printf` - Not blocking C printf like function.
* `cmd_line_printf_tk` - Blocking C printf like function.
* `cmd_line_getchar` - Not blocking C like getchar function.
* `cmd_line_getchar_tk` - Blocking C like getchar function.

Feel free to use these utility functions for you help.

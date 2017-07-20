/*
 * cmd.c
 *
 *  Created on: 20. 7. 2017
 *      Author: michal
 */
#include "stdlib.h"
#include "cmd.h"
#include "string.h"

/* Redefine this variable in your code to get proper firmware version string. */
__weak const char* cmd_line_fw_version = "unknown";

void cmd_print_comands_brief(cmd_line_desc_ptr_t cmd_line_desc, const cmd_desc_t** cmd_list, uint8_t indent_level) {
  if (cmd_line_desc == NULL) {
      return;
  }
  if (indent_level >= CMD_LINE_MAX_INDENT) {
      return;
  }

  int i;
  for (i = 0; 1; i++) {
      if (cmd_list[i] == CMD_LINE_SUBLIST_MARK) {
          cmd_print_comands_brief(cmd_line_desc, cmd_list[i+1], indent_level+1);
          i++;
      }
      else if (cmd_list[i] == NULL) {
          break;
      }
      else {
          cmd_line_printf_tk(cmd_line_desc, "%s - %s\r\n", cmd_list[i]->name, cmd_list[i]->one_line);
      }
  }
}

void cmd_print_command_description(cmd_line_desc_ptr_t cmd_line_desc, const cmd_desc_t** cmd_list, const char* cmd_name) {
  const cmd_desc_t* cmd_ptr = cmd_line_find_command_by_name(cmd_list, cmd_name, 0);
  if (cmd_ptr != NULL) {
      cmd_line_printf_tk(cmd_line_desc, "%s\r\n", cmd_ptr->description);
  }
}

int cmd_do_help(int argc, char** argv, cmd_line_desc_ptr_t cmd_line_desc) {

  const cmd_desc_t** root_list = cmd_line_get_root_list(cmd_line_desc);
  if (argc > 0) {
      int argi;
      for (argi = 1; argi < argc+1; argi++) {
          cmd_print_command_description(cmd_line_desc, root_list, argv[argi]);
      }
  }
  else {
      cmd_print_comands_brief(cmd_line_desc, root_list, 0);
  }

  return CMD_LINE_SUCCESS;
}

int cmd_do_version(int argc, char** argv, cmd_line_desc_ptr_t cmd_line_desc) {
  cmd_line_printf_tk(cmd_line_desc, "Firmware version: %s\r\n", cmd_line_fw_version);
  return CMD_LINE_SUCCESS;
}

cmd_desc_t cmd_desc_help =
    {
        .name = "help",
        .one_line = "Prints help for built in commands.",
        .description = "help [command]\r\n"
            "Prints a complete list of commands with short description or a\r\n"
            "complete description for one command, if the name is given as parameter.\r\n"
            "\r\n"
            "Example:\r\n"
            "\t--> help help\r\n"
            "\thelp [command]\r\n"
            "\t...\r\n",
        .cmd_fcn = &cmd_do_help
    };

cmd_desc_t cmd_desc_version =
    {
        .name = "version",
        .one_line = "Prints The version of the firmware.",
        .description = "version\r\n"
            "Prints The version of the firmware.\r\n"
            "\r\n"
            "Example:\r\n"
            "\t--> version\r\n"
            "\tFirmware version: 1.0a\r\n",
        .cmd_fcn = &cmd_do_version
    };

const cmd_desc_t* basic_cmd_list[] =
  {
      &cmd_desc_help,
      &cmd_desc_version,
      NULL
  };

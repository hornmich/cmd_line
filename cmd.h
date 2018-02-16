/** @defgroup Cmd_Line_Lib_Basic_Cmds Basic Commands
 * Command line basic set of commands.
 * Feel free to include in your ow list of command or to use as root list.
 */

/*! \addtogroup Cmd_Line_Lib_Cmd_Def
*  @{
*/
/*! \addtogroup Cmd_Line_Lib_Basic_Cmds
*  @{
*/
/*
 * Command line basic set of commands header.
 *
 * Include this file to any module, where you want to use @ref CMD_LINE_INCLUDE_SUBLIST
 * to include the basic set of commands.
 *
 * The basic commands are:
 * * help - to print the list of all commands
 * * version - to print the firmware version string. @see cmd_line_fw_version
 *
 *  Created on: 20. 7. 2017
 *  Author: Michal Horn
 */

#ifndef CMD_H_
#define CMD_H_

#include "cmd_desc.h"
#include "cmdline.h"

/**
 * List of basic commands.
 *
 * The basic commands are:
 * * help - to print list of all commands and/or their detailed description.
 * * version - to print he firmware version. @see cmd_line_fw_version.
 *
 * Use this with @ref CMD_LINE_INCLUDE_SUBLIST to include as a sublist.
 */
extern const cmd_desc_t* basic_cmd_list[];

/**
 * Firmware version string.
 *
 * Is weakly defined. Redefine in your code to get proper string.
 */
extern const char* cmd_line_fw_version;
/**@}*/ // Cmd_Line_Lib_Basic_Cmds
/**@}*/ // Cmd_Line_Lib_Cmd_Def

#endif /* CMD_H_ */

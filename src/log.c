//*****************************************************************************
//
// log.c - Interface for logging system information
//
// MIT License
//
// Copyright (c) 2019 Keisuke Tomizawa
//
// 	Permission is hereby granted, free of charge, to any person obtaining a copy
// 	of this software and associated documentation files (the "Software"), to deal
// 	in the Software without restriction, including without limitation the rights
// 	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// 	copies of the Software, and to permit persons to whom the Software is
// 	furnished to do so, subject to the following conditions:
//
// 	The above copyright notice and this permission notice shall be included in all
// 	copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "utils/uartstdio.h"
#include "log.h"

//*****************************************************************************
//
// Configuration Defines
//
//*****************************************************************************
#define LOG_OUTPUT_BUFFER_SIZE 64  // Maximum amount of characters that the 
                                   // log can output via UART
#define LOG_NUM_SUB_SYSTEMS    5   // Number of subsystems


//*****************************************************************************
//
// Internal variables
//
//*****************************************************************************
static char buffer[LOG_OUTPUT_BUFFER_SIZE] = "";       // Ouput Buffer
static uint32_t sub_system_levels[LOG_NUM_SUB_SYSTEMS];// Maintains logging
                                                       // level for each
                                                       // subsystem

//*****************************************************************************
//
//! Initializes the loggging module
//
//*****************************************************************************
void log_init()
{
	// Set each logging level to 0 (log all messages)
	for (uint32_t i = 0; i < LOG_NUM_SUB_SYSTEMS; i++)
		sub_system_levels[i] = 0;
}

//*****************************************************************************
//
//! Convert logging level of type enum e_log_level to a human readable string
//! 
//! \param level is the logging level to convert to a string
//! 
//! This function is used to convert a logging level represented as a 
//! enum e_log_level type to a string. When adding additional logging levels,
//! add them to the switch cases. 
//!
//! \return pointer to first character of string
//! 
//
//*****************************************************************************
char* log_level_to_string(enum e_log_level level)
{
	switch(level)
	{
		case LOG_LEVEL_NONE:
			return "NONE";
		case LOG_LEVEL_INFO_ONLY:
			return "INFO_ONLY";
		case LOG_LEVEL_DEBUG:
			return "DEBUG";
		case LOG_LEVEL_WARNING:
			return "WARNING";
		case LOG_LEVEL_ERROR:
			return "ERROR";
		case LOG_LEVEL_CRITICAL:
			return "CRITICAL";
		default:
			return "UNDEFINED";
	}
}

//*****************************************************************************
//
//! Convert subsystem of type enum e_log_sub_system to a human readable
//! string
//! 
//! \param sys is the subsystem to convert
//! 
//! This function is used to convert a subsystem represented as a enum 
//! e_log_level type to a string. When adding additional subsystems,
//! add them to the switch case.
//!
//! \return pointer to first character of string
//! 
//
//*****************************************************************************
char *log_sub_system_to_string(enum e_log_sub_system sys)
{
	switch(sys)
	{
		case LOG_SUB_SYSTEM_LED:
			return "LED";
		case LOG_SUB_SYSTEM_BUTTON:
			return "BUTTON";
		case LOG_SUB_SYSTEM_CMD:
			return "CMD";
		case LOG_SUB_SYSTEM_SENSOR_LUX:
			return "SENSOR_LUX";
		case LOG_SUB_SYSTEM_I2C0:
			return "I2C0";
		default:
			return "UNDEFINED";
	}
}

//*****************************************************************************
//
//! Log a message
//! 
//! \param sys is the subsystem of the message.
//! \param level is the logging level.
//! \param msg is the message to log.
//! 
//! This function is used to log a message on the output stream with the
//! following format "*LOG* SubSys:<sys> Lvl:<lvl> Msg:<msg>".
//! Currently the function outputs to UART0.
//!
//! \return None.
//! 
//
//*****************************************************************************
void log_msg(enum e_log_sub_system sys, enum e_log_level level, char *msg)
{
	#ifndef LOG_GLOBAL_OFF
	if (sys >= LOG_NUM_SUB_SYSTEMS)
	{
		UARTprintf("\n*LOG* INVALID SUB SYSTEM");
		return;
	}
	
	if (sub_system_levels[sys] <= level)
	{
		snprintf(buffer, LOG_OUTPUT_BUFFER_SIZE, 
		  "\n*LOG* SubSys:%s Lvl:%s Msg:\"%s\"\n>", 
			log_sub_system_to_string(sys), 
			log_level_to_string(level),
			msg 
		);
		
		UARTprintf(buffer);
	}
	#endif
}

//*****************************************************************************
//
//! Log a message with a uint32 value
//! 
//! \param sys is the subsystem of the message.
//! \param level is the logging level.
//! \param msg is the message to log.
//! \param value is the value to log
//! 
//! This function is used to log a message on the output stream with the
//! following format "*LOG* SubSys:<sys> Lvl:<lvl> Msg:<msg> Val:<value>".
//! Currently the function outputs to UART0.
//!
//! \return None.
//! 
//
//*****************************************************************************
void log_msg_value(enum e_log_sub_system sys, enum e_log_level level, char *msg, uint32_t value)
{
	#ifndef LOG_GLOBAL_OFF
	if (sys >= LOG_NUM_SUB_SYSTEMS)
	{
		UARTprintf("\n*LOG* INVALID SUB SYSTEM");
		return;
	}
	
	if (sub_system_levels[sys] <= level )
	{
		char str_value[32];
		snprintf(str_value, LOG_OUTPUT_BUFFER_SIZE, 
			"%d", value);
		
		char buffer[LOG_OUTPUT_BUFFER_SIZE] = "";
		snprintf(buffer, LOG_OUTPUT_BUFFER_SIZE, 
		"\n*LOG* SubSys:%s Lvl:%s Msg:\"%s\" Val:%d\n>", 
			log_sub_system_to_string(sys), 
			log_level_to_string(level),
			msg,
			value
		);
		
		UARTprintf(buffer);
	}
	#endif
}

//*****************************************************************************
//
//! Sets the output level for a specific subsystem.
//! 
//! \param sys is the subsystem to set the level of
//! \param level is level to set to
//! 
//! This function is used to set the logging level of a specific subsystem. 
//! The logging module only logs messages if the subsystem logging level 
//! is set to a lower value than the logging level set by this method. 
//!
//! The avaiable logging levels are (from low to high value)
//!	LOG_LEVEL_INFO_ONLY, 
//!	LOG_LEVEL_DEBUG, 
//!	LOG_LEVEL_WARNING, 
//!	LOG_LEVEL_ERROR, 
//!	LOG_LEVEL_CRITICAL,
//!	LOG_LEVEL_NONE
//!
//! For example, setting the logging level to LOG_LEVEL_WARNING will only log
//! messages labeled as LOG_LEVEL_INFO_ONLY, LOG_LEVEL_DEBUG and 
//! LOG_LEVEL_WARNING. 
//! 
//! \return None.
//! 
//
//*****************************************************************************
void log_output_level_set(enum e_log_sub_system sys, enum e_log_level level)
{
	sub_system_levels[sys] = level;
}

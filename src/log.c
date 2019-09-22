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

#define LOG_OUTPUT_BUFFER_SIZE 64
#define LOG_NUM_SUB_SYSTEMS 5

static char buffer[LOG_OUTPUT_BUFFER_SIZE] = "";

static uint32_t sub_system_levels[LOG_NUM_SUB_SYSTEMS];

void log_init()
{
	for (uint32_t i = 0; i < LOG_NUM_SUB_SYSTEMS; i++)
		sub_system_levels[i] = 0;
}

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

void log_output_level_set(enum e_log_sub_system sys, enum e_log_level level)
{
	sub_system_levels[sys] = level;
}

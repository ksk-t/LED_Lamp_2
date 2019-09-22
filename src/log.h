//*****************************************************************************
//
// log.h - Headers for the loging module
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

#ifndef LOG_H
#define LOG_H

//*****************************************************************************
//
//! Enum containing the projects various sub-systems.
//
//*****************************************************************************
enum e_log_sub_system {
	LOG_SUB_SYSTEM_LED,
	LOG_SUB_SYSTEM_BUTTON,
	LOG_SUB_SYSTEM_SENSOR_LUX,
	LOG_SUB_SYSTEM_CMD,
	LOG_SUB_SYSTEM_I2C0
};

//*****************************************************************************
//
//! Enum containing the different logging levels.
//
//*****************************************************************************
enum e_log_level {
	LOG_LEVEL_INFO_ONLY, 
	LOG_LEVEL_DEBUG, 
	LOG_LEVEL_WARNING, 
	LOG_LEVEL_ERROR, 
	LOG_LEVEL_CRITICAL,
	LOG_LEVEL_NONE
};

//*****************************************************************************
//
// Public function prototypes.
//
//*****************************************************************************
void log_init(void);
void log_msg(enum e_log_sub_system sys, enum e_log_level level, char *msg);
void log_output_level_set(enum e_log_sub_system sys, enum e_log_level level);
void log_msg_value(enum e_log_sub_system sys, enum e_log_level level, char *msg, uint32_t value);

#endif 

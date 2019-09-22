//*****************************************************************************
//
// cmd.c - Inteperates commands recieved from an IO
//
// This module follows the command pattern architecture. 
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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "led.h"
#include "log.h"
#include "tsl2591.h"
#include "console.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "utils/uartstdio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"

//*****************************************************************************
//
// Function Prototypes
//
//*****************************************************************************
void cmd_ver(void);
void cmd_help(void);
void cmd_set_brightness(void);
void cmd_led_off(void);
void cmd_led_on(void);
void cmd_load_profile(void);
void cmd_set_fade_time_interval(void);
void cmd_set_lux_sensitivity(void);
void cmd_lux_read(void);
void cmd_led_update_hw(void);

//*****************************************************************************
//
// Commands are stored in an array containing a cmdStruct type for each
// command. The array is terminated by a blank entry. The cmdStruct contains
// the command name, function to call if the command is recieved, and a 
// brief description of the command to print when the help command is called
//
//*****************************************************************************
typedef void(*functionPointertype)(void);
struct cmdStruct
{
	char const *name;
	functionPointertype execute;
	char const *help;
};

static const struct cmdStruct cmdList[] =
{
	{"ver", &cmd_ver, "Display firmware version" },
	{"setb", &cmd_set_brightness, "Set the LED brightness to a specific level"},
	{"ledoff", &cmd_led_off, "Turn off LEDs"},
	{"ledon", &cmd_led_on, "Turn on LEDs"},
	{"profile", &cmd_load_profile, "Load profile by index"},
	{"fadetimeint", &cmd_set_fade_time_interval, "Set fade time interval"},
	{"sens", &cmd_set_lux_sensitivity, "Set lux sensitivity"},
	{"lux", &cmd_lux_read, "Read lux sensor"},
	{"uphw", &cmd_led_update_hw, "Update LED brightness"},
	{"help", &cmd_help, ""},
	{"", 0, ""}
};

// TODO: Replace with function call to LED.c
// OBSOLETE
struct ledTypeNames 
{
	const char* name;
	uint32_t type;
};

static const struct ledTypeNames ledList[] =
{
	{"g", LED_ONBOARD_GREEN},
	{"b", LED_ONBOARD_BLUE},
	{"r", LED_ONBOARD_RED},
};

void cmd_init()
{
	// Initalized dependencies 
	console_init();
}

//*****************************************************************************
//
//! Executes a command with the given name.
//! 
//! \param cmd points to the name of the command.
//! 
//! This function is used to execute a command based on its name. The available
//! commands are the commands defined in the cmdList array
//! 
//! \param Returns true if command executed successfully and false otherwise
//!
// 
//*****************************************************************************
bool cmd_exectute(const char *cmd)
{
	uint32_t i = 0;
	struct cmdStruct currentCmd = cmdList[i];
	while (strcmp(currentCmd.name, "") != 0)
	{
		if (strcmp(cmd, currentCmd.name) == 0)
		{
			currentCmd.execute();
			return true;
		}
		currentCmd = cmdList[++i];
	}
	
	return false;
}

//*****************************************************************************
//
//! Command to print firmware version
//! 
//! \param None.
//!
// 
//*****************************************************************************
void cmd_ver(void)
{
	// TODO: Need to find a way to not hardcode the firmware version
	UARTprintf("Firmware Version: 0.0.1\n");
}


//*****************************************************************************
//
//! Command to print available commands and their descriptions
//! 
//! \param None.
//!
// 
//*****************************************************************************
void cmd_help(void)
{
	uint32_t i = 0;
	struct cmdStruct currentCmd = cmdList[i];
	UARTprintf("\nAvailable Commands\n------------------\n");
	while (strcmp(currentCmd.name, "") != 0)
	{
		UARTprintf(currentCmd.name);
		UARTprintf("\t\t");
		UARTprintf(currentCmd.help);
		UARTprintf("\n");
		currentCmd = cmdList[++i];
	}
	UARTprintf("\n");
}


//*****************************************************************************
//
//! Command to disable all LED outputs
//! 
//! \param None.
//!
// 
//*****************************************************************************
void cmd_led_off(void)
{
	led_sw_enable_set(false);
}

//*****************************************************************************
//
//! Command to enable all LED outputs
//! 
//! \param None.
//!
// 
//*****************************************************************************
void cmd_led_on(void)
{
	led_sw_enable_set(true);
}

//*****************************************************************************
//
//! Command to set the brightness of a specified LED. 
//! 
//! \param None.
//!
// 
//*****************************************************************************
void cmd_set_brightness(void)
{
	static const size_t numLeds = sizeof(ledList) / sizeof(ledList[1]); // TODO: Obsolete
	
	char buffer[UART_RX_BUFFER_SIZE];
  uint32_t led_type;
	uint8_t led_brightness;
	
	// Get led type
	UARTprintf("Enter LED type: ");
	UARTgets(buffer, UART_RX_BUFFER_SIZE);
	bool foundMatch = false;
	for (size_t i = 0; i < numLeds; i++)
	{
		if (strcmp(ledList[i].name,buffer) == 0)
		{
			foundMatch = true;
			led_type = ledList[i].type;
		}			
	}
	
	if (!foundMatch)
	{
		UARTprintf("Invalid LED type\n");
		return;
	}
	
	// Get led brightness
	UARTprintf("Enter LED brightness: ");
	UARTgets(buffer, UART_RX_BUFFER_SIZE);
	led_brightness = atoi(buffer);
	
	led_sw_brightness_set(led_type, led_brightness);
	led_update_hw_start();
}

//*****************************************************************************
//
//! Command to load specific LED profile index
//! 
//! \param None.
//!
// 
//*****************************************************************************
void cmd_load_profile(void)
{
	char buffer[UART_RX_BUFFER_SIZE];
	uint8_t profile_index;
	
	// Get led type
	UARTprintf("Enter profile index: ");
	UARTgets(buffer, UART_RX_BUFFER_SIZE);
	profile_index = atoi(buffer);
	
	led_profile_load(profile_index);
}

//*****************************************************************************
//
//! Command to set fade interval time (time between each brightness step)
//! 
//! \param None.
//!
// 
//*****************************************************************************
void cmd_set_fade_time_interval(void)
{
	char buffer[UART_RX_BUFFER_SIZE];
	uint32_t time_interval;
	
	// Get led type
	UARTprintf("Enter fade time interval: ");
	UARTgets(buffer, UART_RX_BUFFER_SIZE);
	time_interval = atoi(buffer);
	
	led_time_interval_set(time_interval);
}

//*****************************************************************************
//
//! Command to set lux sensitivity
//! 
//! \param None.
//!
// 
//*****************************************************************************
void cmd_set_lux_sensitivity(void)
{
	char buffer[UART_RX_BUFFER_SIZE];
	uint32_t sensitivity;
	
	// Get sensivity value
	UARTFlushRx();
	UARTprintf("Enter sensitivity: ");
	UARTgets(buffer, UART_RX_BUFFER_SIZE);
	
	sensitivity = strtol(buffer, NULL, 10);
	led_lux_sensitivity_set(sensitivity);
	led_update_hw_start();
}

//*****************************************************************************
//
//! Command to read lux value
//! 
//! \param None.
//!
// 
//*****************************************************************************
void cmd_lux_read(void)
{
	char buffer[UART_RX_BUFFER_SIZE];
	uint32_t lux = 0;
	
	if (tsl2591_lux_get(&lux) == 0)
	{
		sprintf(buffer, "Lux: %d\n", lux);	
	} else
	{
		UARTprintf("Enable to read lux sensor.\n");
	}
}

//*****************************************************************************
//
//! Command to update led to match software configuration
//! 
//! \param None.
//!
// 
//*****************************************************************************
void cmd_led_update_hw(void)
{
	led_update_hw_start();
}

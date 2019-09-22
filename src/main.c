//*****************************************************************************
//
// main.c - Program entry point
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
#include <stdlib.h>

#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"

#include "led.h"
#include "delay.h"
#include "cmd.h"
#include "button.h"
#include "common_aux.h"
#include "log.h"
#include "tsl2591.h"
#include "console.h"
#include "timer_ext.h"

int main(void)
{
	// Set System clock to 16Mhz. Must be called first
	SysCtlClockSet(SYSCTL_USE_OSC | SYSCTL_OSC_INT | SYSCTL_SYSDIV_1);
	
	// Initialize various sub systems
	console_init();
	log_init();
	led_init();
	button_init();
	
	// Set logging level
	log_output_level_set(LOG_SUB_SYSTEM_BUTTON, LOG_LEVEL_NONE);
	log_output_level_set(LOG_SUB_SYSTEM_SENSOR_LUX, LOG_LEVEL_NONE);
	log_output_level_set(LOG_SUB_SYSTEM_LED, LOG_LEVEL_NONE);
	
	// Load LED profile
	led_profile_load(0);
	
	// Continuously poll serial buffer for command. Could move this to timer interrupt instead
	UARTprintf("BOOT\n");
	UARTprintf(">");
	char buffer[UART_RX_BUFFER_SIZE];
	while (1)
	{
		if (UARTPeek('\r') != -1)
		{
			UARTgets(buffer, UART_RX_BUFFER_SIZE);
			if (!cmd_exectute(buffer))
					UARTprintf("Invalid command.\n");
			
			UARTprintf(">");
		}
	}
}


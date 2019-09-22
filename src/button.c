//*****************************************************************************
//
// button.c - Interface to use GPIO ports as button inputs
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
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTx	HERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"

#include "button.h"
#include "common_aux.h"
#include "log.h"
#include "led.h"
#include "utils/uartstdio.h"

//*****************************************************************************
//
// Module Configuration Defines
//
//*****************************************************************************
#define BUTTON_DEBOUNCE_POLL_RATE 10  // Frequency at which to poll the buttons
                                      //  in ms
#define BUTTON_TIMER_PRESCALE     255 // Button timer prescale value

//*****************************************************************************
//
// Interrupt Handlers
//
//*****************************************************************************

//*****************************************************************************
//
//! TIMER0A is used continuously poll the button state every 
//! BUTTON_DEBOUNCE_POLL_RATE milliseconds. If a valid button press is detected
//! by the debounce function, the timer will determine which button was press
//! and execute the appropriate function.
//
//*****************************************************************************
void TIMER0A_Handler(void)
{
	
	// Clear interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	
	// toggle indicates which pin has changed debounce state. 
	// A bit value of 1 means the pin has changed state.
	// state indicates the debounced state of the pin. A bit 
	// value of 1 means the pin is high, 0 if low.
	unsigned char toggle, state;
	state = debounce(GPIOPinRead(BUTTON_GPIO_BASE, BUTTON_POWER | BUTTON_CHANGE_PROFILE) & 0xFF,
		&toggle);
	
	if ((toggle & BUTTON_POWER)) 
	{
		if ((state & BUTTON_POWER))
		{
			log_msg(LOG_SUB_SYSTEM_BUTTON, LOG_LEVEL_DEBUG, "BUTTON_POWER Release");
			
		}else
		{
			// Toggle LED Enable
			led_sw_enable_toggle();
			
			log_msg(LOG_SUB_SYSTEM_BUTTON, LOG_LEVEL_DEBUG, "BUTTON_POWER Press");
		}
	}
	
	if ((toggle & BUTTON_CHANGE_PROFILE)) 
	{
		if ((state & BUTTON_CHANGE_PROFILE))
		{
			log_msg(LOG_SUB_SYSTEM_BUTTON, LOG_LEVEL_DEBUG, "BUTTON_CHANGE_PROFILE Release");
		}else
		{
			// Load next LED profile
			led_profile_load_next();

			log_msg(LOG_SUB_SYSTEM_BUTTON, LOG_LEVEL_DEBUG, "BUTTON_CHANGE_PROFILE Press");	
		}
	}
}

//*****************************************************************************
//
//! Determines if there is a valid button press. 
//!  
//! This function is used to software debounce the buttons. It is based on a
//! debounce algorithm developed by CompuPhase. 
//! Link to site: https://www.compuphase.com/electronics/debouncing.htm
//! 
//! LICENSING INFORMATOIN
//! "Debouncing switches with vertical counters" by Compuphase is licensed
//! under CC BY 3.0 https://creativecommons.org/licenses/by-sa/3.0/
//! 
//! \param sample is the current raw GPIO state
//! \param toggle indicates if an pin has changed debounce state a bit value of 
//! 1 means the pin has changed state.
//!
//! In essence the algorithm utilizes a vertical counter to monitor the change
//! state of a GPIO port. While somewhat difficult to understand, the 
//! algorithm has the advantage of being completely loopless, allowing it to
//! be very efficient.
//!
//! A timer should be used to continously call the function at a fixed
//! frequency.
//! 
//! \return the debounce state of the GPIO port where a bit field value of 1 
//! means the GPIO pin's debounce state is active and 0 means inactive
//
//*****************************************************************************
unsigned char debounce(unsigned char sample, unsigned char *toggle)
{
    static unsigned char state = 0xFF, cnt0, cnt1;
    unsigned char delta;

    delta = sample ^ state;
    cnt1 = (cnt1 ^ cnt0) & delta;
    cnt0 = ~cnt0 & delta;

    *toggle = delta & ~(cnt0 | cnt1);
    state ^= *toggle;

    return state;
}

//*****************************************************************************
//
//! Initializes the button module
//!  
//! This function initialized the modules and peripherals that this module
//! depends on. It must be called before any of other function called.
//! 
//! \return None.
// 
//*****************************************************************************
void button_init(void)
{
	//***************************************************************************
	//
	// Initialize GPIO used for buttons
	//
	//***************************************************************************
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){ }
	
  __disable_irq();
  //
  // Unlock PF0 so we can change it to a GPIO input
  // Once we have enabled (unlocked) the commit register then re-lock it
  // to prevent further changes.  PF0 is muxed with NMI thus a special case.
  //
  HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
  HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
  HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	__enable_irq();
	
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
	
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, 0, GPIO_PIN_TYPE_STD_WPU);
	
	GPIOIntTypeSet(GPIO_PORTF_BASE,  GPIO_PIN_0 | GPIO_PIN_4, GPIO_FALLING_EDGE);
	
	//***************************************************************************
	//
	// Initialize Timers used for the debounce function
	//
	//***************************************************************************
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)){};
			
	// Configure timers as periodic
	TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC );
			
			
	// Set Prescale
	TimerPrescaleSet(TIMER0_BASE, TIMER_A, BUTTON_TIMER_PRESCALE);
		
	// Set load values
	TimerLoadSet(TIMER0_BASE, TIMER_A, ms_to_clockticks(BUTTON_TIMER_PRESCALE , BUTTON_DEBOUNCE_POLL_RATE, UINT16_MAX)); 
	
	// Enable interrupt on timeout (value goes to 0)
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
			
	// Enable system level interrupt
	IntEnable(INT_TIMER0A);
			
	// Enable timer 
	TimerEnable(TIMER0_BASE, TIMER_A);
}


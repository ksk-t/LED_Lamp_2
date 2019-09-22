//*****************************************************************************
//
// led.c - Interface to control LEDs
//
// This module is used to control the brightness of LEDs using PWM. To change
// the brightness of and LED, first the software brightness must be changed. 
// Then the Timer1A is enabled to incrementally brighten or dim the LEDs, 
// resulting in a smooth fade effect. 
// 
// Adding additional LEDs require two steps:
// 1. Adding the led info to the led_list
// 2. Providing the necessary functions in init(void) for setting up the
//    hardware
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

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_pwm.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "log.h"
#include "tsl2591.h"
#include "common_aux.h"
#include "timer_ext.h"
#include "led.h"

//*****************************************************************************
//
// Defines used to configure the LED controller
//
//*****************************************************************************
#define LED_BRIGHTNESS_EXP_POINT     100        // The brightness change is linear until this
                                                // 	point, compensating for the fact that changes
																								// 	in LED brightness are less noticable at
																								// 	higher brightness levels

#define LED_MAX_BRIGHTNESS_LEVEL     255        // Maximum brightness level
#define LED_TIMER_PRESCALE           255        // Prescale value for the timers
#define LED_TIMER_MAX_LOAD_VALUE     UINT16_MAX // Maximum load value for the timers
#define LED_LUX_CHANGE_HYSTERESIS    30         // Maximum change in lux required to trigger
                                                // 	an LED brightness change
#define LED_LUX_UPDATE_RATE          1000       // Frequency to read new lux sensor value, in ms
                                                // 	Max value 1000 (for 16 bit timer)
#define LED_STEP_TIME_INTERVAL       10         // Time between a single LED brightness step
                                                // 	A higher value will yield slower fade effect
                                                // 	Max value 1000 (for 16 bit timer)
																								
//*****************************************************************************
//
// The led_list array provides a list containing the various LEDs. The array 
// is an array of led_info which provdies the led name, type, pwm base 
// register, pwm out value, pwm pin bit, and pwn gen number. The last four of 
// these values are defines defined in the "driverlib/pwm.h" file. 
//
//*****************************************************************************
struct led_info
{
	const char *name;
	const uint32_t pwm_base_register;
	const uint32_t pwm_out;
	const uint32_t pwm_out_bit;
	const uint32_t pwm_gen;
	uint32_t current_brightness;
	uint32_t previous_brightness;
	uint32_t desired_brightness;
};

static struct led_info LED_LIST[] = 
{
	{ "r", PWM1_BASE, PWM_OUT_5, PWM_OUT_5_BIT , PWM_GEN_2, 0, 0, 0},
	{ "b", PWM1_BASE, PWM_OUT_6, PWM_OUT_6_BIT , PWM_GEN_3, 0, 0, 0},
	{ "g", PWM1_BASE, PWM_OUT_7, PWM_OUT_7_BIT , PWM_GEN_3, 0, 0, 0},
	{ "", 0,0,0,0,0,0, 0} // Terminal entry
};

//*****************************************************************************
//
// led_profile_list is an array of the led_profile struct that defines various 
// led brightness settings. led_profile contains the names and brightnesses of 
// two leds. 
//
// Note: There should be no more than 254 led profiles. Anymore will cause an
//       overflow error
//
//*****************************************************************************
struct led_profile
{
	uint8_t led1_type;
	uint8_t led1_brightness;
	uint8_t led2_type;
	uint8_t led2_brightness;
};

static const struct led_profile led_profile_list[] = 
{
	{LED_ONBOARD_BLUE, 100 , LED_ONBOARD_GREEN, 255 },
	{LED_ONBOARD_BLUE, 0, LED_ONBOARD_GREEN, 255 },
	{LED_ONBOARD_BLUE, 0, LED_ONBOARD_GREEN, 100 },
};
static uint8_t num_profiles = 3;

//*****************************************************************************
//
// Defines the index of the current led profile
//
//*****************************************************************************
static uint8_t _current_profile_index;
static uint8_t _brightness_interval;
static uint8_t _time_internval;
static uint32_t _num_leds;
static uint32_t _lux_sensor_sensitivity;
static uint32_t _max_lux;
static float _brightness_scale;
static bool lux_sensor_found;

//*****************************************************************************
//
// Software enable for the LEDs. 
//
//*****************************************************************************
static bool _sw_enable;


//*****************************************************************************
//
// Prototypes for private functions
//
//*****************************************************************************
void led_hw_brightness_set(uint32_t led_type, uint32_t brightness);

//*****************************************************************************
//
// Timer interrupt handlers
// 
//*****************************************************************************

//*****************************************************************************
//
// TIMER1A is used to increment/decrement the LED brightness in steps with a 
// variable time in between each step. This yield a smooth fade in/out effect
// when updating the LEDs' brightness levels. This handler will disable the 
// timer once all the LED's have reached the goal brightness.
//
// The time between each brightness step is defined by LED_STEP_TIME_INTERVAL.
// 
//*****************************************************************************
void TIMER1A_Handler(void)
{
	// Clear interrupt
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	
	bool no_change = true;
	uint32_t current_brightness, desired_brightness_w_scale;
	
	// Loop through each of the LEDs and increase brightness by a single step if necessary
	for (uint32_t i = 0; i < _num_leds; i++)
	{
		current_brightness = LED_LIST[i].current_brightness;
		desired_brightness_w_scale = LED_LIST[i].desired_brightness * _brightness_scale;
		
		if (current_brightness > desired_brightness_w_scale)
		{
			led_hw_brightness_set(i, current_brightness - _brightness_interval);
			no_change = false;
		}else if (current_brightness < desired_brightness_w_scale)
		{
			led_hw_brightness_set(i, current_brightness + _brightness_interval);
			no_change = false;
		}
	}

	// Disable timer if no LED was changed, indicating completion
	if (no_change)
	{
		TimerDisable(TIMER1_BASE, TIMER_A);
	}
}

//*****************************************************************************
//
// TIMER1AB is used to continuously read new lux values. The handler will
// automatically enable TIMER1A to begin updating the LED brightness if a large
// enough change in lux is detected (set by the define 
// LED_LUX_CHANGE_HYSTERESIS). 
//
// The polling rate is defined by LED_LUX_UPDATE_RATE.
// 
//*****************************************************************************
void TIMER1B_Handler(void)	
{
	static uint32_t current_lux = UINT32_MAX;
	
	// Clear interrupt
	TimerIntClear(TIMER1_BASE, TIMER_TIMB_TIMEOUT);
	
	if (!timer_status_enable(TIMER1_BASE, TIMER_A) && led_sw_enable_get())
	{

		uint32_t new_lux;

		// Verify valid transaction with lux sensor
		if (tsl2591_lux_get(&new_lux) != 0)
		{
			log_msg(LOG_SUB_SYSTEM_LED, LOG_LEVEL_CRITICAL, "Lost connection with lux sensor");
			_brightness_scale = 1.0F;
			lux_sensor_found = false;
			TimerDisable(TIMER1_BASE, TIMER_B);
			return;
		}
		
		// Limit maxiumum lux value. Neccessary for proper calculation of brightness scale
		if (new_lux > _max_lux)
			new_lux = _max_lux;
	
		// Do not update lux if new lux does not exceed hysteresis
		if (current_lux > new_lux && current_lux - new_lux < LED_LUX_CHANGE_HYSTERESIS)
			return;
		if (current_lux < new_lux && new_lux - current_lux < LED_LUX_CHANGE_HYSTERESIS)
			return;

		// Calculate new brightness scale
		_brightness_scale = 1 - (((float)(_lux_sensor_sensitivity) / LED_MAX_LUX_SENSITIVITY) * ((float)(_max_lux - new_lux) / _max_lux)); 
	
		current_lux = new_lux;
		
		// Start timer that updates LED brightness
		led_update_hw_start();	
	}
}


//*****************************************************************************
//
// Function prototypes for private functions
//
//*****************************************************************************
void led_hw_brightness_set(uint32_t led_type, uint32_t brightness);
void led_update_hw_start(void);
uint32_t led_num_leds_get(void);


//*****************************************************************************
//
//! Initializes the PWM modules related to each LED
//!  
//! This function sets up the PWM modules for each LED. It must be called 
//! before any of the LED control functions can be called. 
//! 
//! \return None.
// 
//*****************************************************************************
void led_init(void)
{
	uint32_t lux_sensor_id;
	
	//***************************************************************************
	//
	// Initialize PWM used to control LED brightness
	//
	//***************************************************************************
	
  // Configure PWM Clock to match system
  SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

  // Enable the peripherals used by this program.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){}
  SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);  

  //Configure PF1,PF2,PF3 Pins as PWM
  GPIOPinConfigure(GPIO_PF1_M1PWM5);
  GPIOPinConfigure(GPIO_PF2_M1PWM6);
  GPIOPinConfigure(GPIO_PF3_M1PWM7);
  GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

  //Configure PWM Options
  //PWM_GEN_2 Covers M1PWM4 and M1PWM5
  //PWM_GEN_3 Covers M1PWM6 and M1PWM7 See page 207 4/11/13 DriverLib doc
   while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){}
   PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN 
		| PWM_GEN_MODE_NO_SYNC); 
   PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN 
		| PWM_GEN_MODE_NO_SYNC); 
	
	// Calculate period. See led_hw_brightness_set() for more info on equation 
  // used
	uint32_t period = LED_MAX_BRIGHTNESS_LEVEL * LED_MAX_BRIGHTNESS_LEVEL 
		 / LED_BRIGHTNESS_EXP_POINT;
		 
  //Set the Period (expressed in clock ticks)
  PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, 650);
  PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, 650);
		
  // Enable PWM generator block
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);
		
	//***************************************************************************
	//
	// Initialize Timer used to change LED brightness in steps
	//
	//***************************************************************************
	
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)){};
	
	TimerConfigure(TIMER1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC | 
		TIMER_CFG_B_PERIODIC);
	TimerPrescaleSet(TIMER1_BASE, TIMER_BOTH, LED_TIMER_PRESCALE);
		
	TimerLoadSet(TIMER1_BASE, TIMER_A, ms_to_clockticks(LED_TIMER_PRESCALE , 
		LED_STEP_TIME_INTERVAL, LED_TIMER_MAX_LOAD_VALUE)); 
	TimerLoadSet(TIMER1_BASE, TIMER_B, ms_to_clockticks(LED_TIMER_PRESCALE , 
		LED_LUX_UPDATE_RATE, LED_TIMER_MAX_LOAD_VALUE));
		
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT | TIMER_TIMB_TIMEOUT);
	IntEnable(INT_TIMER1A);
	IntEnable(INT_TIMER1B);

	//***************************************************************************
	//
	// Initialize dependencies
	//
	//***************************************************************************
	tsl2591_init();
	
	// Detect presense of lux sensor
	if (tsl2591_id_get(&lux_sensor_id) == 0 && lux_sensor_id == TSL2591_DEVICE_ID)
	{
		lux_sensor_found = true;
	}
	else
	{
		lux_sensor_found = false;
		log_msg(LOG_SUB_SYSTEM_LED, LOG_LEVEL_CRITICAL, "Unable to connect to lux");
		_brightness_scale = 1.0F;
	}
	
	//***************************************************************************
	//
	// Initialize module variables
	//
	//***************************************************************************
	led_time_interval_set(5);
	led_brightness_step_set(1);
	_num_leds = led_num_leds_get();
	_current_profile_index = 0;
	_lux_sensor_sensitivity = 0;
	_max_lux = 200;
	_sw_enable = true;
	
	// Synchronize sw and hw brightness
	for (uint32_t i = 0; i < _num_leds; i++)
	{
		led_sw_brightness_set(i, 0);
		led_hw_brightness_set(i, 0);
	}
	
	if (lux_sensor_found)
	{
		// Enable timer for reading the lux sensor 
		TimerEnable(TIMER1_BASE, TIMER_B);
	}
}

//*****************************************************************************
//
//! Gets the number of LEDs defined in LED_LIST
//! 
//! \return Number of LEDs in LED_LIST
// 
//*****************************************************************************
uint32_t led_num_leds_get(void)
{
	uint32_t num_leds = 0;
	
	// Determine number of LEDs in LED_LIST. Only run once
	while (LED_LIST[num_leds].pwm_base_register != 0)
		num_leds++;
	
	return num_leds;
}

//*****************************************************************************
//
//! Reads the current output state of the LED's PWM.
//! 
//! \return Returns true if enabled, false if disabled
// 
//*****************************************************************************
bool led_output_state_get(uint32_t led_type)
{
	// Direct register access is required as TI did not provide a function
	// in its library to read the output state of the PWM
	if (HWREG(PWM1_BASE + PWM_O_ENABLE) & LED_LIST[led_type].pwm_out_bit)
		return true;
	else
		return false;
}

//*****************************************************************************
//
//! Sets the current output state of the LED's PWM.
//! 
//! \return  Returns true if enabled, false if disabled
// 
//*****************************************************************************
void led_output_state_set(uint32_t led_type, bool enable)
{
	PWMOutputState(LED_LIST[led_type].pwm_base_register, LED_LIST[led_type].pwm_out_bit, 
		enable);
}

//*****************************************************************************
//
//! Sets the brightness of the selected led
//! 
//! \param led_type specifies the LED to set the brightness of
//! \param brightness is the brightness to set the LED to
//!
//!
//! \return None.
// 
//*****************************************************************************
void led_hw_brightness_set(uint32_t led_type, uint32_t brightness)
{	
	uint32_t new_pulsewidth = 0;
	
	// Disable output if settings brightness to 0
	if (brightness == 0)
	{
		led_output_state_set(led_type, false);
	}
	// Enable led output if disabled
	else if (!led_output_state_get(led_type))
	{
		led_output_state_set(led_type, true);
	}
	 
	// Exponentially increase PWM pulsewidth after LED_BRIGHTNESS_EXP_POINT is 
	// reached. This compensates for the fact that the LED brightness is less
	// sensetive to higher PWM duty cycles
	if (brightness >= LED_BRIGHTNESS_EXP_POINT)
	{
		new_pulsewidth = brightness * brightness / LED_BRIGHTNESS_EXP_POINT;
	}else
	{
		new_pulsewidth = brightness;
	}
	
	PWMPulseWidthSet(LED_LIST[led_type].pwm_base_register, 
		LED_LIST[led_type].pwm_out, new_pulsewidth);
	
	LED_LIST[led_type].current_brightness = brightness;
}

//*****************************************************************************
//
//! Sets the software brightness of the selected led
//! 
//! \param led_type specifies the LED to set the brightness of
//! \param brightness is the brightness to set the LED to
//!
//!
//! \return None.
// 
//*****************************************************************************
void led_sw_brightness_set(uint32_t led_type, uint32_t brightness)
{	
	LED_LIST[led_type].desired_brightness = brightness;
}

//*****************************************************************************
//
//! Sets the time interval in ms used for the fade effect. The time interval is
//! the time between each brightness step in the fade effect
//!
//! \param interval is the new time interval value
//!
//! \return None.
// 
//*****************************************************************************
void led_time_interval_set(uint32_t ms)
{
	TimerLoadSet(TIMER1_BASE, TIMER_A, ms_to_clockticks(LED_TIMER_PRESCALE , 
		ms, LED_TIMER_MAX_LOAD_VALUE)); 
	
	_time_internval = ms;
}

//*****************************************************************************
//
//! Gets the time interval in ms used for the fade effect. The time interval is
//! the time between each brightness step in the fade effect
//!
//! \return Returns the time interval
// 
//*****************************************************************************
uint8_t led_time_interval_get(void)
{
	return _time_internval;
}

//*****************************************************************************
//
//! Sets the brighntess interval used for the fade effect. The brightness 
//! interval is the amount of brightness steps to adjust the brightness
//! between each time interval
//! 
//! \param interval is the new brightness interval
//!
//! \return  None.
// 
//*****************************************************************************
void led_brightness_step_set(uint8_t interval)
{
	_brightness_interval = interval;
}

//*****************************************************************************
//
//! Sets the brighntess interval used for the fade effect. The brightness 
//! interval is the amount of brightness steps to adjust the brightness
//! between each time interval
//! 
//! \param interval is the new brightness interval
//!
//! \return  None.
// 
//*****************************************************************************
uint8_t led_brightness_step_get(void)
{
	return _brightness_interval;
}

//*****************************************************************************
//
//! Loads profile from led_profile_list using given index
//! 
//! \param interval is the new brightness interval
//!
//! This function loads the led profile defined in led_profile_list array. The 
//! passed index referes to the index of the array element. The function does
//! nothing if the index is outside the array bounds
//!
//! \return  None.
// 
//*****************************************************************************
void led_profile_load(uint8_t index)
{
	if (_sw_enable)
	{
		// Verify index is in range
		if (index >= num_profiles) 
			return;
		
		// Set SW brightness
		led_sw_brightness_set(led_profile_list[index].led1_type, 
			led_profile_list[index].led1_brightness);
		led_sw_brightness_set(led_profile_list[index].led2_type, 
			led_profile_list[index].led2_brightness);
		
		// Update HW
		led_update_hw_start();
		_current_profile_index = index;
	}

}

//*****************************************************************************
//
//! Sets the brighntess interval used for the fade effect. The brightness 
//! interval is the amount of brightness steps to adjust the brightness
//! between each time interval
//! 
//! \param interval is the new brightness interval
//!
//! \return  None.
// 
//*****************************************************************************
void led_profile_load_next(void)
{
	if (_sw_enable)
	{
		_current_profile_index = (_current_profile_index + 1) % num_profiles;
		led_profile_load(_current_profile_index);
	}
}


//*****************************************************************************
//
//! Sets the software enable of the LED controller
//! 
//! \param enable selects whether to enable/disable the LED controller. true 
//! enabled the controller, false disables the controller
//!
//! This function turns on and off all the LEDs managed by the controller with
//! a fade effect. 
//!
//! \return None.
// 
//*****************************************************************************
void led_sw_enable_set(bool enable)
{
	if (enable == _sw_enable)
		return;
	
	if (enable)
	{
		// Load previously saved brightness
		for (uint32_t i = 0; i < _num_leds; i++)
		{
			led_sw_brightness_set(i, LED_LIST[i].previous_brightness);
		}
	}
	else
	{
		// Save current brightness and set new brightness to 0
		for (uint32_t i = 0; i < _num_leds; i++)
		{
			LED_LIST[i].previous_brightness = LED_LIST[i].desired_brightness;
			led_sw_brightness_set(i, 0);
		}
	}
	
	// Update HW
	led_update_hw_start();
	_sw_enable = enable;
}

//*****************************************************************************
//
//! Gets the software enable state
//! 
//! \param None.
//!
//! \return true if software enable is active, false otherwise
// 
//*****************************************************************************
bool led_sw_enable_get(void)
{
	return _sw_enable;
}


//*****************************************************************************
//
//! Toggles the LED enable
//! 
//! \return  None.
// 
//*****************************************************************************
void led_sw_enable_toggle(void)
{
	if (_sw_enable)
		led_sw_enable_set(false);
	else
		led_sw_enable_set(true);
}

//*****************************************************************************
//
//! Sets the sensitivity to the lux sensor
//! 
//! \param sensitivity is the sensitivy value to set to. The higher the value,
//! the more senstive the LEDs are to the lux sensor
//!
//! The maximum sensitivy value is determined by the define 
//! LED_MAX_LUX_SENSITIVITY
//!
//! \return None. 
// 
//*****************************************************************************
void led_lux_sensitivity_set(uint32_t sensitivity)
{
	if (sensitivity > LED_MAX_LUX_SENSITIVITY)
		sensitivity = LED_MAX_LUX_SENSITIVITY;
	else
		_lux_sensor_sensitivity = sensitivity;
	
	log_msg_value(LOG_SUB_SYSTEM_LED, LOG_LEVEL_DEBUG, "Setting sensitivity",sensitivity);
}

//*****************************************************************************
//
//! Enables the timer responsible for fading the LEDs in/out.
//! 
//! \param None.
//!
//! This function should be called after changing the software brightness value
//! using the led_sw_brightness_set() function. 
//!
//! \return None. 
// 
//*****************************************************************************
void led_update_hw_start(void)
{
	TimerEnable(TIMER1_BASE, TIMER_A);
}

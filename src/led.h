//*****************************************************************************
//
// led.h - Headers for using the led controller functions
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

#ifndef LED_H
#define LED_H

	
#include <stdint.h>
#include <stdbool.h>

//
// The value of the LED macros that define
// an LED are associated with the its index
// in the LED_LIST variable
//

#define LED_ONBOARD_RED   0x00
#define LED_ONBOARD_BLUE  0x01
#define LED_ONBOARD_GREEN 0x02

#define LED_MAX_LUX_SENSITIVITY 255

//*****************************************************************************
//
// Public function prototypes.
//
//*****************************************************************************
void led_init(void);
void led_sw_brightness_set(uint32_t led_type, uint32_t brightness);
void led_sw_enable_set(bool enable);
void led_sw_enable_toggle(void);
bool led_sw_enable_get(void);
void led_update_hw_start(void);
void led_time_interval_set(uint32_t interval);
uint8_t led_time_interval_get(void);
void led_brightness_step_set(uint8_t interval);
void led_profile_load(uint8_t index);
void led_profile_load_next(void);
void led_lux_sensitivity_set(uint32_t sensitivity);
void led_max_lux_set(uint32_t max);

#endif

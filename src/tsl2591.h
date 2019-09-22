//*****************************************************************************
//
// tsl2591.h - Headers for the tsl2591 driver module
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

#ifndef TSL2591_H
#define TSL2591_H

#include <stdint.h>
#include <stdbool.h>

//
// I2C 7 Bit Address (0x28 also works)
//
#define TSL2591_ADDRESS 0x29

//*****************************************************************************
//
// The following are defines for the sensor's registers
//
//*****************************************************************************
#define TSL2591_COMMAND_MASK                  0x80
#define TSL2591_COMMAND_NORMAL_OPERATION_MASK 0xA0
#define TSL2591_REG_ENABLE                    0x00
#define TSL2591_REG_CONTROL                   0x01
#define TSL2591_REG_AILTL                     0x04
#define TSL2591_REG_AILTH                     0x05
#define TSL2591_REG_AIHTL                     0x06
#define TSL2591_REG_AIHTH                     0x07
#define TSL2591_REG_NPAILTL                   0x08
#define TSL2591_REG_NPAITLH                   0x09
#define TSL2591_REG_NPAIHTL                   0x0A
#define TSL2591_REG_NPAIHTH                   0x0B
#define TSL2591_REG_PRESIST                   0x0C
#define TSL2591_REG_PID                       0x11
#define TSL2591_REG_ID                        0x12
#define TSL2591_REG_STATUS                    0x13
#define TSL2591_REG_C0DATAL                   0x14
#define TSL2591_REG_C0DATAH                   0x15
#define TSL2591_REG_C1DATAL                   0x16
#define TSL2591_REG_C1DATAH                   0x17

//*****************************************************************************
//
// The following are defines for the bit fields for the ENABLE register
//
//*****************************************************************************
#define TSL2591_ENABLE_NPIEN 0x80 // No Persist Interrupt Enable
#define TSL2591_ENABLE_SAI   0x40 // Sleep after interrupt
#define TSL2591_ENABLE_AIEN  0x10 // ALS Interrupt Enable
#define TSL2591_ENABLE_AEN   0x02 // ALS Enable
#define TSL2591_ENABLE_PON   0x01 // Power ON

//*****************************************************************************
//
// The following are defines for the bit masks and field values for the 
// CONTROL register
//
//*****************************************************************************
#define TSL2591_CONTROL_SRESET      0x80 // System reset
#define TSL2591_CONTROL_GAIN_LOW    0x00 // Low gain mode
#define TSL2591_CONTROL_GAIN_MEDIUM 0x10 // Medium gain mode
#define TSL2591_CONTROL_GAIN_HIGH   0x20 // High gain mode
#define TSL2591_CONTROL_GAIN_MAX    0x30 // Maximum gain mode
#define TSL2591_CONTROL_ATIME_100   0x00 // 100ms integration time
#define TSL2591_CONTROL_ATIME_200   0x01 // 200ms integration time
#define TSL2591_CONTROL_ATIME_300   0x02 // 300ms integration time
#define TSL2591_CONTROL_ATIME_400   0x03 // 400ms integration time
#define TSL2591_CONTROL_ATIME_500   0x04 // 500ms integration time
#define TSL2591_CONTROL_ATIME_600   0x05 // 600ms integration time

//*****************************************************************************
//
// The following are defines for the bit masks and field values for the 
// STATUS register
//
//*****************************************************************************
#define TSL2591_STATUS_NPINTR       0x20 // Encountered no-presist interrupt
#define TSL2591_STATUS_AINT         0x10 // Encounted ALS interrupt
#define TSL2591_STATUS_AVALID       0x01 // Valid ALS

#define TSL2591_LUX_DF              408.0F // Lux cooefficient
#define TSL2591_DEVICE_ID           0x50   // Device ID

//*****************************************************************************
//
// Public function prototypes.
//
//*****************************************************************************
void tsl2591_init(void);
uint32_t tsl2591_id_get(uint32_t *id);
uint32_t tsl2591_lux_get(uint32_t *lux);
uint32_t tsl2591_gain_set(uint32_t gain);
uint32_t tsl2591_integratation_time_set(uint32_t time);
uint32_t tsl2591_als_valid(bool *completed_cycle);
	
#endif

//*****************************************************************************
//
// i2c_ext.h - Headers for using the i2c extenstion module function
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

#ifndef I2C_EXT_H
#define I2C_EXT_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_BUSY_POLL_ATTEMPTS      5000
#define I2C_MASTER_ERR_MAX_ATTEMPTS 0x00000001

//*****************************************************************************
//
// Public function prototypes.
//
//*****************************************************************************
void i2c_init(void);
uint32_t i2c_register_write(uint8_t addr, uint8_t reg, uint8_t data[], uint32_t num_bytes);
uint32_t i2c_register_read(uint8_t addr, uint8_t reg, uint8_t data[], uint32_t num_bytes);
uint32_t i2c_register_write_bit(uint8_t addr, uint8_t reg, uint8_t bit_mask, bool set_bit);

#endif

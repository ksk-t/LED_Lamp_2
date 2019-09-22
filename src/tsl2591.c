//*****************************************************************************
//
// tsl2591.c - Driver for the TSL2591 light sensor
//
// This module is used to provide an interface to interact with the TSL2591
// light sensor via I2C. 
//
// All functions, with the exception of the initialization function, will 
// return the status of the I2C transaction. A successful transcation will 
// return a 0. 
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
#include "tsl2591.h"
#include "log.h"
#include "delay.h"
#include "i2c_ext.h"

//*****************************************************************************
//
// The following are configuration defines for this module
//
//*****************************************************************************
#define TSL2591_BUFFER_SIZE    4 // Maximum number of bits that can be read
                                 //  or written with a single i2c transaction
		

//*****************************************************************************
//
// The RETURN_IF_ERROR is used to abort the function if the status indicates
// that an I2C error has occured. It causes the function return the error
// value. 
//
//*****************************************************************************
#define RETURN_IF_ERROR(x) do {if (x != 0) return x; }while(0);

//*****************************************************************************
//
// The following are internal variables
//
//*****************************************************************************
static uint8_t _bufferRX[TSL2591_BUFFER_SIZE]; // I2C receive buffer
static uint32_t _integration;                  // Current integration value 
static uint32_t _gain;                         // Current gain value

//*****************************************************************************
//
//! Initializes the TSL2591 module
//!  
//! This function initialized the modules that this module depends on.
//! It must be called before any of other function called
//! 
//! \return None.
// 
//*****************************************************************************
void tsl2591_init(void)
{
	// Initialize dependencies
	i2c_init();
}


//*****************************************************************************
//
//! Enables the internal oscillator and ALS function
//!  
//! This function is used to active the internal oscillator which permits the 
//! TSL2591's internal timers and ADC to operate. It also activates the ALS
//! function, beginning an acquisition. 
//! 
//! \return I2C transaction status, as one of \b I2C_MASTER_ERR_NONE, 
//! \b I2C_MASTER_ERR_ADDR_ACK, \b I2C_MASTER_ERR_DATA_ACK, 
//! \b I2C_MASTER_ERR_ARB_LOST, or \b I2C_MASTER_ERR_MAX_ATTEMPTS
// 
//*****************************************************************************
uint32_t tsl2591_enable(void)
{
	return i2c_register_write_bit(TSL2591_ADDRESS, TSL2591_COMMAND_NORMAL_OPERATION_MASK | TSL2591_REG_ENABLE,
		TSL2591_ENABLE_PON | TSL2591_ENABLE_AEN, 
		true);
}

//*****************************************************************************
//
//! Disables the internal oscillator and ALS function
//!  
//! This function is used to deactivate the internal oscillator which permits
//! the TSL2591's internal timers and ADC to operate. It also disables the ALS
//! function, terminating any acqusition. 
//! 
//! \return I2C transaction status, as one of \b I2C_MASTER_ERR_NONE, 
//! \b I2C_MASTER_ERR_ADDR_ACK, \b I2C_MASTER_ERR_DATA_ACK, 
//! \b I2C_MASTER_ERR_ARB_LOST, or \b I2C_MASTER_ERR_MAX_ATTEMPTS
// 
//*****************************************************************************
uint32_t tsl2591_disable(void)
{
	return i2c_register_write_bit(TSL2591_ADDRESS, TSL2591_COMMAND_NORMAL_OPERATION_MASK | TSL2591_REG_ENABLE,
		TSL2591_ENABLE_PON | TSL2591_ENABLE_AEN, 
		false);
}

//*****************************************************************************
//
//! Reads the current lux detected by the sensor
//!
//! \param lux is a pointer to the lux value read by the sensor. Unchanged if
//!        a I2C transaction error occurs.
//!  
//! This function is used to obtain a lux reading from the sensor. It is based
//! on the lux calculation function provided in Adafruit Industries' TSL2591
//! library written for the Arudino platform. 
//! Link to GITHUB page: https://github.com/adafruit/Adafruit_TSL2561
//!
//! LICENSE INFORMATION
//! Software License Agreement (BSD License)
//!
//! Copyright (c) 2014 Adafruit Industries
//! All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions are met:
//! 1. Redistributions of source code must retain the above copyright
//! notice, this list of conditions and the following disclaimer.
//! 2. Redistributions in binary form must reproduce the above copyright
//! notice, this list of conditions and the following disclaimer in the
//! documentation and/or other materials provided with the distribution.
//! 3. Neither the name of the copyright holders nor the
//! names of its contributors may be used to endorse or promote products
//! derived from this software without specific prior written permission.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
//! EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//! WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//! DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
//! DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//! (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//! LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//! ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//!(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//! SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//! 
//! \return I2C transaction status, as one of \b I2C_MASTER_ERR_NONE, 
//! \b I2C_MASTER_ERR_ADDR_ACK, \b I2C_MASTER_ERR_DATA_ACK, 
//! \b I2C_MASTER_ERR_ARB_LOST, or \b I2C_MASTER_ERR_MAX_ATTEMPTS
// 
//*****************************************************************************
uint32_t tsl2591_lux_get(uint32_t *lux)
{
	uint32_t status = 0;
	uint16_t ch0, ch1;
	float atime, again;
	float cpl;
	bool completed_int_cycle;
	
	ch0 = 0;
	ch1 = 0;
	
	switch(_integration)
	{
		case TSL2591_CONTROL_ATIME_100:
			atime = 100.0F;
			break;
		case TSL2591_CONTROL_ATIME_200:
			atime = 200.0F;
			break;
		case TSL2591_CONTROL_ATIME_300:
			atime = 300.0F;
			break;
		case TSL2591_CONTROL_ATIME_400:
			atime = 400.0F;
			break;
		case TSL2591_CONTROL_ATIME_500:
			atime = 500.0F;
			break;
		case TSL2591_CONTROL_ATIME_600:
			atime = 600.0F;
			break;
		default:
			atime = 100.0F;
		  break;
	}
	
	switch(_gain)
	{
		case TSL2591_CONTROL_GAIN_LOW:
			again = 1.0F;
			break;
		case TSL2591_CONTROL_GAIN_MEDIUM:
			again = 25.0F;
			break;
		case TSL2591_CONTROL_GAIN_HIGH:
			again = 428.0F;
			break;
		case TSL2591_CONTROL_GAIN_MAX:
			again = 9876.0F;
			break;
		default:
			again = 1.0F;
			break;
	}
	
	status = tsl2591_enable();
	RETURN_IF_ERROR(status);
	// Wait for complete integration cycle
	do 
	{
		status = tsl2591_als_valid(&completed_int_cycle);
		RETURN_IF_ERROR(status);
	}while (completed_int_cycle == false);
	
	status = tsl2591_disable();
	RETURN_IF_ERROR(status);
	
	// Read CH0 and CH1 upper and lower bytes
	status = i2c_register_read(TSL2591_ADDRESS, TSL2591_COMMAND_NORMAL_OPERATION_MASK | TSL2591_REG_C0DATAL,
		_bufferRX, 4);
	RETURN_IF_ERROR(status);
		
	ch0 = _bufferRX[0] | (_bufferRX[1] << 8);
	ch1 = _bufferRX[2] | (_bufferRX[3] << 8);
		
	// Check for overflow
	if ((ch0 == 0xFFFF || ch1 == 0xFFFF))
	{
		return UINT32_MAX;
	}
	
	// Calculate lux
	cpl = (atime * again) / TSL2591_LUX_DF;
	*lux = ( ((float)ch0 - (float)ch1 )) * (1.0F - ((float)ch1/(float)ch0) ) / cpl;
	
	log_msg_value(LOG_SUB_SYSTEM_SENSOR_LUX, LOG_LEVEL_DEBUG, "Lux Value", *lux);
	
	return status;
}

//*****************************************************************************
//
//! Gets the device ID
//!
//! \param id is a pointer to the device ID. Unchanged if a I2C transaction 
//!        error occurs.
//!  
//! This function is used to read the device ID of the lux sensor.
//! 
//! \return I2C transaction status, as one of \b I2C_MASTER_ERR_NONE, 
//! \b I2C_MASTER_ERR_ADDR_ACK, \b I2C_MASTER_ERR_DATA_ACK, 
//! \b I2C_MASTER_ERR_ARB_LOST, or \b I2C_MASTER_ERR_MAX_ATTEMPTS
// 
//*****************************************************************************
uint32_t tsl2591_id_get(uint32_t *id)
{
	uint32_t status;
	
	status = i2c_register_read(TSL2591_ADDRESS, TSL2591_COMMAND_NORMAL_OPERATION_MASK | TSL2591_REG_ID, _bufferRX, 1);
	
	RETURN_IF_ERROR(status);
	
	*id = _bufferRX[0];
	
	return status;
}

//*****************************************************************************
//
//! Determines if the device has completed a complete integration cycle 
//! and has valid data to read
//!
//! \param completed_cycle indicates if the device has completed a integration
//!        cycle. Is given value true if a integration cycle has been complete,
//!        false otherwise.
//! 
//! This function should be called before reading the ALS data register to
//! verify that the data there is valid.
//! 
//! \return I2C transaction status, as one of \b I2C_MASTER_ERR_NONE, 
//! \b I2C_MASTER_ERR_ADDR_ACK, \b I2C_MASTER_ERR_DATA_ACK, 
//! \b I2C_MASTER_ERR_ARB_LOST, or \b I2C_MASTER_ERR_MAX_ATTEMPTS
// 
//*****************************************************************************
uint32_t tsl2591_als_valid(bool *completed_cycle)
{
	uint32_t status;
	
	status = i2c_register_read(TSL2591_ADDRESS, TSL2591_COMMAND_NORMAL_OPERATION_MASK | TSL2591_REG_STATUS, _bufferRX, 1);
	
	RETURN_IF_ERROR(status);
	
	if (_bufferRX[0] & TSL2591_STATUS_AVALID)
	{
		*completed_cycle = true;
	}else
	{
		*completed_cycle = false;
	}
	
	return status;
}

//*****************************************************************************
//
//! Sets the gain of the integration amplifier for both photodiode channels
//!
//! \param gain is the gain value to set to, as one of 
//! \b TSL2591_CONTROL_GAIN_LOW, \b TSL2591_CONTROL_GAIN_MEDIUM,
//! \b TSL2591_CONTROL_GAIN_HIGH, \b TSL2591_CONTROL_GAIN_MAX
//! 
//! \return I2C transaction status, as one of \b I2C_MASTER_ERR_NONE, 
//! \b I2C_MASTER_ERR_ADDR_ACK, \b I2C_MASTER_ERR_DATA_ACK, 
//! \b I2C_MASTER_ERR_ARB_LOST, or \b I2C_MASTER_ERR_MAX_ATTEMPTS
// 
//*****************************************************************************
uint32_t tsl2591_gain_set(uint32_t gain)
{
	uint32_t status;
	
	// Read current state of CONTROL register
	status = i2c_register_read(TSL2591_ADDRESS, TSL2591_COMMAND_NORMAL_OPERATION_MASK | TSL2591_REG_CONTROL, _bufferRX, 1);
	
	RETURN_IF_ERROR(status);
	
	// Change CONTROL register value with new gain
	_gain = gain;
	_bufferRX[0] |= _gain;
	
	// Write new CONTROL register value
	status = i2c_register_write(TSL2591_ADDRESS, TSL2591_COMMAND_NORMAL_OPERATION_MASK | TSL2591_REG_CONTROL, _bufferRX, 1);
	
	return status;
}

//*****************************************************************************
//
//! Sets the internal ADC integration time for both photodiode channels
//!
//! \param gain is the gain value to set to, as one of 
//! \b TSL2591_CONTROL_ATIME_100, \b TSL2591_CONTROL_ATIME_200, 
//! \b TSL2591_CONTROL_ATIME_300, \b TSL2591_CONTROL_ATIME_400, 
//! \b TSL2591_CONTROL_ATIME_500, \b TSL2591_CONTROL_ATIME_600, 
//! 
//! \return I2C transaction status, as one of \b I2C_MASTER_ERR_NONE, 
//! \b I2C_MASTER_ERR_ADDR_ACK, \b I2C_MASTER_ERR_DATA_ACK, 
//! \b I2C_MASTER_ERR_ARB_LOST, or \b I2C_MASTER_ERR_MAX_ATTEMPTS
// 
//*****************************************************************************
uint32_t tsl2591_integratation_time_set(uint32_t integration)
{
	uint32_t status;
	
	// Read current state of CONTROL register
	status = i2c_register_read(TSL2591_ADDRESS, TSL2591_COMMAND_NORMAL_OPERATION_MASK | TSL2591_REG_CONTROL, _bufferRX, 1);
	
	RETURN_IF_ERROR(status);
	
	// Change CONTROL register value with new integration time
	_integration = integration;
	_bufferRX[0] |= integration;
	
	// Write new CONTROL register value
	status = i2c_register_write(TSL2591_ADDRESS, TSL2591_COMMAND_NORMAL_OPERATION_MASK | TSL2591_REG_CONTROL, _bufferRX, 1);
	
	return status;
}

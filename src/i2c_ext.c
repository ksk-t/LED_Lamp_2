//*****************************************************************************
//
// i2c_ext.c - Extenstion module for the I2C periphal.
//
// This module provides additional functions to the I2C driver library
// provided by TI. Included are commonly used I2C operations such as
// writing and reading to a register.
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

#include "i2c_ext.h"
#include <stdbool.h>
#include "log.h"
#include "delay.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/i2c.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

//*****************************************************************************
//
// Module Configuration Defines
//
//*****************************************************************************
#define I2C_MODULE_BASE_ADDRESS I2C0_BASE // Base address of the I2C peripheral

//*****************************************************************************
//
//! Initializes the I2C module
//!  
//! This function initialized the modules and peripherals that this module
//! depends on. It must be called before any of other function called.
//! 
//! \return None.
// 
//*****************************************************************************
void i2c_init(void)
{
	static bool initialized = false;
	
	// Only initalize once
	if (initialized)
		return;
	
	//***************************************************************************
	//
	// Initialize I2C0 Peripheral
	//
	//***************************************************************************
	
	//
	// Enable the I2C0 peripheral
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
	//
	// Wait for the I2C0 module to be ready.
	//
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C0))
	{ }
	
	//***************************************************************************
	//
	// Initialize GPIOB Peripheral for SDA and SCL
	//
	//***************************************************************************
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
	{ }
	
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);
  GPIOPinConfigure(GPIO_PB3_I2C0SDA);
	
	GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
	
	//
	// Initialize Master and Slave
	//
	I2CMasterInitExpClk(I2C_MODULE_BASE_ADDRESS, SysCtlClockGet(), false);
	
	initialized = true;
}

//*****************************************************************************
//
//! A blocking function that continues to block until the I2C master is no
//! longer busy.
//!  
//! \param base is the base address of the I2C peripheral
//!
//! This function continuously polls the I2C master to determine if it is 
//! free. If the function has to poll the master than MAX_BUSY_POLL_ATTEMPTS
//! times, it will return a I2C_MASTER_ERR_MAX_ATTEMPTS error. This prevents
//! an infinite loop if the master becomes stuck, for example, if the I2C rail
//! is missing a pullup.
//! It should be called after each byte tranfered. 
//! 
//! \return \b 0 if successful, \b I2C_MASTER_ERR_MAX_ATTEMPTS if failed
// 
//*****************************************************************************
uint32_t wait_for_free_master(uint32_t base)
{
	// Continuously poll master to check if busy
	for (uint32_t i = 0; i < MAX_BUSY_POLL_ATTEMPTS; i++)
	{
		if (!I2CMasterBusy(base))
		{
			return 0;
		}
	}
	
	// At this point max number of attempts reached, return error
	return I2C_MASTER_ERR_MAX_ATTEMPTS;
}

//*****************************************************************************
//
//! Reads a specified number of bytes from a given register value
//!  
//! \param addr is the seven bit address of the slave device, not including the 
//! r/w bit
//! \param reg is the regester to read from
//! \param data is the data that was read. it is up to the user to ensure 
//! that the array is big enough to fit all bytes
//! \param num_bytes is then number of bytes to read back
//! 
//! \return I2C transaction status, as one of \b I2C_MASTER_ERR_NONE, 
//! \b I2C_MASTER_ERR_ADDR_ACK, \b I2C_MASTER_ERR_DATA_ACK, 
//! \b I2C_MASTER_ERR_ARB_LOST, or \b I2C_MASTER_ERR_MAX_ATTEMPTS
// 
//*****************************************************************************
uint32_t i2c_register_read(uint8_t addr, uint8_t reg, uint8_t data[], uint32_t num_bytes)
{
	uint32_t status = 0;
	
	// Write register to read from
	I2CMasterSlaveAddrSet(I2C_MODULE_BASE_ADDRESS, addr, false);
	I2CMasterDataPut(I2C_MODULE_BASE_ADDRESS, reg);
	I2CMasterControl(I2C_MODULE_BASE_ADDRESS, I2C_MASTER_CMD_BURST_SEND_START);
	status = wait_for_free_master(I2C_MODULE_BASE_ADDRESS);
	
	// Error Check
	status |= I2CMasterErr(I2C_MODULE_BASE_ADDRESS);
	if (status != 0)
	{			
		I2CMasterControl(I2C_MODULE_BASE_ADDRESS, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
		log_msg_value(LOG_SUB_SYSTEM_I2C0, LOG_LEVEL_ERROR, "I2C Transfer error", status);
		return status;
	}
	
	// Read Register
	I2CMasterSlaveAddrSet(I2C_MODULE_BASE_ADDRESS, addr, true);
	for (uint32_t i = 0; i < num_bytes; i++)
	{
		if (num_bytes == 1)
		{
			// Only reading one byte in total
			I2CMasterControl(I2C_MODULE_BASE_ADDRESS, I2C_MASTER_CMD_SINGLE_RECEIVE);
		}else if (i == 0)
		{
			// Read first byte
			I2CMasterControl(I2C_MODULE_BASE_ADDRESS, I2C_MASTER_CMD_BURST_RECEIVE_START);
		}else if (i == num_bytes - 1)
		{
			// Read last byte
			I2CMasterControl(I2C_MODULE_BASE_ADDRESS, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
		}else
		{
			// Read intermediate byte
			I2CMasterControl(I2C_MODULE_BASE_ADDRESS, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
		}

		status = wait_for_free_master(I2C_MODULE_BASE_ADDRESS);
		
		// Error Check
		status |= I2CMasterErr(I2C_MODULE_BASE_ADDRESS);
		if (status != 0)
		{
			I2CMasterControl(I2C_MODULE_BASE_ADDRESS, I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);
			log_msg_value(LOG_SUB_SYSTEM_I2C0, LOG_LEVEL_ERROR, "I2C Transfer error", status);
			return status;			
		}else
		{
			data[i] = I2CMasterDataGet(I2C_MODULE_BASE_ADDRESS);
		}
	}
	
	return status;
}

//*****************************************************************************
//
//! Writes a specified number of bytes from a given register value
//!  
//! \param addr is the seven bit address of the slave device, not including the 
//! r/w bit
//! \param reg is the register to write to
//! \param data is the data to be written
//! \param num_bytes is then number of bytes to write
//! 
//! \return I2C transaction status, as one of \b I2C_MASTER_ERR_NONE, 
//! \b I2C_MASTER_ERR_ADDR_ACK, \b I2C_MASTER_ERR_DATA_ACK, 
//! \b I2C_MASTER_ERR_ARB_LOST, or \b I2C_MASTER_ERR_MAX_ATTEMPTS
// 
//*****************************************************************************
uint32_t i2c_register_write(uint8_t addr, uint8_t reg, uint8_t data[], uint32_t num_bytes)
{
	uint32_t status;
	
	I2CMasterSlaveAddrSet(I2C_MODULE_BASE_ADDRESS, addr, false);
	
	// Write register
	I2CMasterDataPut(I2C_MODULE_BASE_ADDRESS, reg);
	I2CMasterControl(I2C_MODULE_BASE_ADDRESS, I2C_MASTER_CMD_BURST_SEND_START);

	status = wait_for_free_master(I2C_MODULE_BASE_ADDRESS);
	
	// Error Check
	status |= I2CMasterErr(I2C_MODULE_BASE_ADDRESS);
	if (status != 0)
	{			
		I2CMasterControl(I2C_MODULE_BASE_ADDRESS, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
		log_msg_value(LOG_SUB_SYSTEM_I2C0, LOG_LEVEL_ERROR, "I2C Transfer error", status);
		return status;
	}
	
	// Write Data
	for (uint32_t i = 0; i < num_bytes; i++)
	{
		I2CMasterDataPut(I2C_MODULE_BASE_ADDRESS, data[i]);
		if (i == num_bytes - 1)
		{
			// Write last byte
			I2CMasterControl(I2C_MODULE_BASE_ADDRESS, I2C_MASTER_CMD_BURST_SEND_FINISH);
		}
		{
			// Write intermediate byte
			I2CMasterControl(I2C_MODULE_BASE_ADDRESS, I2C_MASTER_CMD_BURST_SEND_CONT);
		}

		status = wait_for_free_master(I2C_MODULE_BASE_ADDRESS);
		
		// Error Check
		status |= I2CMasterErr(I2C_MODULE_BASE_ADDRESS);
		if (status != 0)
		{			
			I2CMasterControl(I2C_MODULE_BASE_ADDRESS, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
			log_msg_value(LOG_SUB_SYSTEM_I2C0, LOG_LEVEL_ERROR, "I2C Transfer error", status);
			return status;
		}
	}
	
	return status;
}

//*****************************************************************************
//
//! Writes a specified bit to a single register
//!  
//! \param addr is the seven bit address of the slave device, not including the 
//! r/w bit
//! \param reg is the register to write to
//! \param bit_mask is the bit to set or clear
//! \param set_bit can be set true to set the bit or false to clear the bit
//! 
//! \return I2C transaction status, as one of \b I2C_MASTER_ERR_NONE, 
//! \b I2C_MASTER_ERR_ADDR_ACK, \b I2C_MASTER_ERR_DATA_ACK, 
//! \b I2C_MASTER_ERR_ARB_LOST, or \b I2C_MASTER_ERR_MAX_ATTEMPTS
// 
//*****************************************************************************
uint32_t i2c_register_write_bit(uint8_t addr, uint8_t reg, uint8_t bit_mask, bool set_bit)
{
	uint8_t data[1];
	uint8_t status = 0;
	
	// Read current state of register
	status = i2c_register_read(addr, reg, data, 1);
	
	// Modify appropriate bit
	if (set_bit)
	{
		// Set bit
		data[0] |= bit_mask;
	}else
	{
		// Clear bit
		data[0]  &= ~bit_mask;
	}
	
	// Update register
	status |= i2c_register_write(addr, reg, data, 1);
	
	return status;
}


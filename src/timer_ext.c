//*****************************************************************************
//
// timer_ext.c - Provides auxilary functions for the Timer peripheral
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

#include "inc/hw_types.h"
#include "inc/hw_timer.h"
#include "driverlib/timer.h"


//*****************************************************************************
//
//! Determines a specified timer is enabled or disabled
//!  
//! \param base is the base address of the timer peripheral
//! \param timer specifies if the timer timer a or b, as \b TIMER_A or 
//! \b TIMER_B
//! 
//! \return None.
// 
//*****************************************************************************
bool timer_status_enable(uint32_t base, uint32_t timer)
{
	uint32_t timer_en_bit_field; 
	
	// Determine bit field for enable bit
	if (timer == TIMER_A)
	{
		timer_en_bit_field = TIMER_CTL_TAEN;
	}else if (timer == TIMER_B)
	{
		timer_en_bit_field = TIMER_CTL_TBEN;
	}
	else
	{
		return false;
	}
	
	// Read HW register
	if (HWREG(base + TIMER_O_CTL) & timer_en_bit_field)
		return true;
	else
		return false;
}


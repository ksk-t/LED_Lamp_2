//*****************************************************************************
//
// common_aux.c - common auxilary functions
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
#include "common_aux.h"
#include "driverlib/sysctl.h"

//*****************************************************************************
//
//! Converts ms to equivalant number of clock ticks
//!
//! \param prescale is the timer prescale value
//! \param ms is the time in ms
//! \param max_val is the upper limit to the return (inclusive) 
//!
//! Important!: SysCtlClockSet() MUST be called before this function
//! 
//! \return  Returns clock ticks equivalant to ms
// 
//*****************************************************************************
uint32_t ms_to_clockticks(uint32_t prescale, uint32_t ms, uint32_t max_val)
{
    uint32_t clock_ticks = ms * (SysCtlClockGet() / (prescale + 1) / 1000);

    if (clock_ticks > max_val)
        return max_val;
    else
        return clock_ticks;
}

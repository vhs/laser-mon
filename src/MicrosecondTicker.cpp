/*
  MicrosecondTicker.h - esp8266 library that calls functions periodically

  Copyright (c) 2015 Logan Buchy. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <MicrosecondTicker.h>

#ifndef USE_US_TIMER
#error "Must define USE_US_TIMER"
#endif

extern "C"
{
  #include <ets_sys.h>
  #include "c_types.h"
  #include "eagle_soc.h"
  #include "ets_sys.h"
  #include "osapi.h"
}

const int ONCE   = 0;
const int REPEAT = 1;

MicrosecondTicker::MicrosecondTicker()
  : m_pTimer(new ETSTimer)
  , bInitialized(false)
{
  //os_timer_arm(m_pTimer, microseconds, ONCE);
}

MicrosecondTicker::~MicrosecondTicker()
{
  delete m_pTimer;
}

void MicrosecondTicker::attach_us(uint32_t microseconds, callback_t pCallback, void* arg, bool repeat)
{
  if (!bInitialized)
  {
    bInitialized = true;
    os_timer_setfn(m_pTimer, pCallback, arg);
  }
  os_timer_arm_us(m_pTimer, microseconds, (repeat)?REPEAT:ONCE);
}

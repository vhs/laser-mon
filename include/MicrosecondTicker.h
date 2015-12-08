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

#ifndef MICROSECOND_TICKER_H
#define MICROSECOND_TICKER_H

#include <stdint.h>
#include <stddef.h>

extern "C"
{
  typedef struct _ETSTIMER_ ETSTimer;
}

class MicrosecondTicker
{
public:
	MicrosecondTicker();
	~MicrosecondTicker();
	typedef void (*callback_t)(void*);

	void attach_us(uint32_t microseconds, callback_t pCallback, void* arg, bool repeat);

protected:
	ETSTimer* m_pTimer;
  bool bInitialized;
};


#endif //MICROSECOND_TICKER_H

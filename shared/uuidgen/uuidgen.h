/*
 Copyright 2004-2008 Matthew J. Battey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software distributed
	under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied. See the License for the
	specific language governing permissions and limitations under the License.




This software implements a platform independent UUID generator and iostream insertion
and extraction operators.
*/
#ifndef _UUIDGEN_H_
#define _UUIDGEN_H_

#include <memory.h>
#include <stdlib.h>

#if !defined(_WIN32) 

#	if defined(UUID_USE_STDINT)
#		include <stdint.h>
#	endif

#	ifdef UUID_USE_INTTYPES
#		include <inttypes.h>
#	endif

#	if !defined(UUID_UINT32)
#		define UUID_UINT32 uint32_t
#	endif

#	if !defined(UUID_INT32)
#		define UUID_INT32 int32_t
#	endif

#else // ifdefined _WIN32

#	if !defined(UUID_UINT32)
#		define UUID_UINT32 unsigned long
#	endif

#	if !defined(UUID_INT32)
#		define UUID_INT32 long
#	endif
#endif

#ifdef __cplusplus

#include <iostream>
#include <iomanip>

extern "C" {
#endif

struct uuid {
	union {
		unsigned char raw[16];
		struct {
			UUID_UINT32		d1;
			unsigned short	d2;
			unsigned short	d3;
			unsigned char	d4[8];
		} guid;
	};
};

void uuidgen(struct uuid* puuid);
void setuuidconf(const char* uuidconf);

#ifdef __cplusplus
};



inline std::ostream& operator<<(std::ostream& o, const uuid& u)
{
	std::ios::fmtflags oldflags = o.flags();
	char oldfill = o.fill('0');
	o << std::hex << std::setw(8) << u.guid.d1 << '-' << std::setw(4) << u.guid.d2 << '-' << std::setw(4) << u.guid.d3 << '-' 
	<< std::setw(4) << *(short*)u.guid.d4;
//	<< std::setw(2) << (short)u.guid.d4[0]
//	<< std::setw(2) << (short)u.guid.d4[1]
	o << '-';
	o << std::setw(2) << (short)u.guid.d4[2];
	o << std::setw(2) << (short)u.guid.d4[3];
	o << std::setw(2) << (short)u.guid.d4[4];
	o << std::setw(2) << (short)u.guid.d4[5];
	o << std::setw(2) << (short)u.guid.d4[6];
	o << std::setw(2) << (short)u.guid.d4[7];
	o.flags(oldflags);
	o.fill(oldfill);
	return o;
}

inline std::istream& operator>>(std::istream& i, uuid& u)
{
	char	buffer[37];
	int		start = i.gcount();
	
	i.read(buffer,36);

	if ((i.gcount()-start) == 36) {
		char	tmp[3];
		char*	p;

		buffer[8] = 0;
		buffer[13] = 0;
		buffer[18] = 0;
		buffer[23] = 0;
		u.guid.d1 = strtoul(buffer,&p,16);
		u.guid.d2 = (unsigned short)strtoul(buffer+9,&p,16);
		u.guid.d3 = (unsigned short)strtoul(buffer+14,&p,16);
		*(short*)u.guid.d4 = (unsigned short)strtoul(buffer+19,&p,16);

		for(int x=0;x<6;x++) {
			memcpy(tmp,buffer+24+(x*2),2);
			u.guid.d4[x+2] = (unsigned char)strtoul(tmp,&p,16);
		}
	}
	return i;
}

#endif

#endif

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




This software implements utility classes
*/

#include "AddressParser.h"
#include <stdlib.h>

#ifndef _WIN32
	#include <netdb.h>
#endif

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AddressParser::Address::Address(const std::string& addr, unsigned short port)
{
	this->port = port;
	this->name = addr;

	if (addr == "*") {
		this->address = INADDR_ANY;
	} else {
		if ( (this->address = ::inet_addr(addr.c_str())) == (INADDR_NONE)) {
			struct hostent*	hp = ::gethostbyname(addr.c_str());
			if (hp)
				this->address = *(in_addr_t*)hp->h_addr;
		}
	}
}


AddressParser::AddressParser(const std::string& src)
{
	string::size_type		b, e=string::npos;
	string	addstr;

	while ((e=src.find(";",b=e+1)) != string::npos) {
		addstr = src.substr(b,e-b);
		string::size_type p = addstr.find(":");
		if (p != string::npos) {

			if (p+1 < addstr.length())
				addresses.push_back(Address(addstr.substr(0,p), (unsigned short)(atoi(addstr.substr(p+1).c_str()) & 0x00FFFF)));
			else {
				addresses.push_back(Address(addstr.substr(0,p))); 
			}
		} else {
			addresses.push_back(Address(addstr));
		}
	}

	if (b < src.length()) {
		addstr = src.substr(b);
		string::size_type p = addstr.find(":");
		if (p != string::npos) {

			if (p+1 < addstr.length())
				addresses.push_back(Address(addstr.substr(0,p), (unsigned short)(atoi(addstr.substr(p+1).c_str()) & 0x00FFFF))); 
			else {
				addresses.push_back(Address(addstr.substr(0,p))); 
			}
		} else {
			addresses.push_back(Address(addstr));
		}
	}
}

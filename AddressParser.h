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

#if !defined(AFX_ADDRESSPARSER_H__EC82175F_B65D_468E_84EA_1DE71132D00A__INCLUDED_)
#define AFX_ADDRESSPARSER_H__EC82175F_B65D_468E_84EA_1DE71132D00A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>
#include "tcpsocket/tcpsocket.h"

class AddressParser  
{
public:
	//---------------------------------------------------------

	class Address {
	public:
		Address() {
			address = INADDR_NONE;
			port = 0xFFFF;
		}

		Address(const std::string& addr, unsigned short port=0xFFFF);

		Address(const Address& src) {
			operator=(src);
		}

		const Address& operator=(const Address& src) {
			name = src.name;
			address = src.address;
			port = src.port;
			return src;
		}

		std::string			name;
		in_addr_t			address;
		unsigned short		port;
	};

	//---------------------------------------------------------

	AddressParser(const std::string& src);


	typedef std::vector<Address> AddressList;
	AddressList	addresses;
};

#endif // !defined(AFX_ADDRESSPARSER_H__EC82175F_B65D_468E_84EA_1DE71132D00A__INCLUDED_)

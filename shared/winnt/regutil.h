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




This software implements a Win32 registry access class
*/
#ifndef _REGUTIL_H_
#define _REGUTIL_H_

#pragma warning(disable:4290) // Disable warning about exception specifications

#include <Winerror.h>
#include <string>
#include <exception>

class CRegKeyException : public std::exception {
private:
	std::string msg;
public:

	CRegKeyException() throw() 	{ }
    CRegKeyException(const std::exception& rhs) throw() { 
		msg = rhs.what();
	}
    std::exception& operator=(const std::exception& rhs) throw() {
		msg = rhs.what();
		return *this;
	}
    virtual ~CRegKeyException() throw()	{ }
    virtual const char *what() const throw() {
		return msg.c_str();
	}
};

class CRegKey
{
private:
	HKEY	regkey;

public:
	CRegKey(HKEY base, const std::string& path, REGSAM access=KEY_ALL_ACCESS) throw (CRegKeyException) {
		long err = ::RegOpenKeyEx(base,path.c_str(),0,access,&regkey);
		if (err != ERROR_SUCCESS) {
			char buffer[1024];
			::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,
							GetLastError(),MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),buffer,sizeof(buffer),NULL);
			throw CRegKeyException(buffer);
		}
	}

	virtual ~CRegKey() {
		::RegCloseKey(regkey);
	}

	unsigned long getInt(const std::string& name) throw (CRegKeyException) {
		DWORD			size = sizeof(unsigned long);
		unsigned long	value;
		DWORD			type = REG_DWORD;

		long			err = ::RegQueryValueEx(regkey,name.c_str(),0,&type,(unsigned char*)&value,&size);

		if (err != ERROR_SUCCESS) {
			char buffer[1024];
			::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,
							GetLastError(),MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),buffer,sizeof(buffer),NULL);
			throw CRegKeyException(buffer);
		}
		return value;
	}

	void setInt(const std::string& name, unsigned long value) throw (CRegKeyException) {
		DWORD	size = sizeof(unsigned long);
		long	err = ::RegSetValueEx(regkey,name.c_str(),0,REG_DWORD,(const unsigned char*)&value,size);

		if (err != ERROR_SUCCESS) {
			char buffer[1024];
			::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,
							GetLastError(),MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),buffer,sizeof(buffer),NULL);
			throw CRegKeyException(buffer);
		}
	}

	std::string getString(const std::string& name) throw (CRegKeyException) {
		char	value[1024];
		DWORD	size=sizeof(value);
		DWORD	type = REG_SZ;
		long	err = ::RegQueryValueEx(regkey,name.c_str(),0,&type,(unsigned char*)value,&size);

		if (err != ERROR_SUCCESS) {
			char buffer[1024];
			::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,
							GetLastError(),MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),buffer,sizeof(buffer),NULL);
			throw CRegKeyException(buffer);
		}
		return value;
	}


	void setString(const std::string& name, const std::string value) throw (CRegKeyException) {
		DWORD	size = value.length();
		long	err = ::RegSetValueEx(regkey,name.c_str(),0,REG_SZ,(const unsigned char*)value.c_str(),size);

		if (err != ERROR_SUCCESS) {
			char buffer[1024];
			::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,
							GetLastError(),MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),buffer,sizeof(buffer),NULL);
			throw CRegKeyException(buffer);
		}
	}
};

/*
HKEY_CLASSES_ROOT
HKEY_CURRENT_CONFIG
HKEY_CURRENT_USER
HKEY_LOCAL_MACHINE
HKEY_USERS
*/

#endif

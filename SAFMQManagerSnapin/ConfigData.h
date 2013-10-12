/*
 Copyright 2005 Matthew J. Battey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software distributed
	under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied. See the License for the
	specific language governing permissions and limitations under the License.




This software implements a platform independent Store and Forward Message Queue.
*/
// ConfigData.h: interface for the CConfigData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIGDATA_H__EC652238_D7E8_40BA_A1A0_631CB545BB03__INCLUDED_)
#define AFX_CONFIGDATA_H__EC652238_D7E8_40BA_A1A0_631CB545BB03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>
#include <set>

using namespace std;

class CConfigEntry
{
public:
	CConfigEntry() {
		line = -1;
	}

	// Used to find an item in the set.
	CConfigEntry(const string& name) {
		this->name = name;
		line = -1;
	}

	CConfigEntry(const CConfigEntry& entry) {
		*this = entry;
	}

	const CConfigEntry& operator=(const CConfigEntry& entry) {
		name = entry.name;
		value = entry.value;
		line = entry.line;
		return *this;
	}

	bool operator<(const CConfigEntry& rhs) const {
		return name < rhs.name;
	}

	string	name;
	string	value;
	int		line;
};


class CConfigData  
{
public:
	static const string	PORT;
	static const string ADDRESS;
	static const string QUEUE_DIR;
	static const string CONFIG_DIR;
	
	static const string ENABLE_FORWARDING;
	static const string ACCEPT_FORWARDS;
	static const string MAX_FILE_SIZE;
	static const string FILE_HANDLE_CACHE_SIZE;

	static const string STARTUP_LOG;
	static const string SHUTDOWN_LOG;
	static const string SIGNON_LOG;
	static const string USER_LOG;
	static const string GROUP_LOG;
	static const string QUEUE_LOG;
	static const string MESSAGE_LOG;
	static const string FORWARD_LOG;
	static const string INFO_LOG;

	static const string SSL_PORT;
	static const string SSL_ADDRESS;
	static const string SSL_CERT;
	static const string SSL_KEY;
	static const string SSL_CA;
	static const string SSL_CA_DIR;
	static const string SSL_REQUIRE_CERT;

	static const string SSL_CLIENT_CERT;
	static const string SSL_CLIENT_KEY;


protected:
	vector<string>		lines; // used to keep the format of the original file, comments, etc.
	set<CConfigEntry>	items;

public:
	CConfigData();
	virtual ~CConfigData();

	string getSetting(const string& name, const string& def = "");
	void applySetting(const string& name, const string& value);

	void storeSettings();

protected:
	void loadData();

};

#endif // !defined(AFX_CONFIGDATA_H__EC652238_D7E8_40BA_A1A0_631CB545BB03__INCLUDED_)

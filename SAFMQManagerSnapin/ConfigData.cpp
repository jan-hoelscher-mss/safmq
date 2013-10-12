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
// ConfigData.cpp: implementation of the CConfigData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ConfigData.h"
#include <fstream>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const string CConfigData::PORT = "port";
const string CConfigData::ADDRESS = "address";
const string CConfigData::QUEUE_DIR = "queue_dir";
const string CConfigData::CONFIG_DIR = "config_dir";
	
const string CConfigData::ENABLE_FORWARDING = "enable_forwarding";
const string CConfigData::ACCEPT_FORWARDS = "accept_forwards";
const string CConfigData::MAX_FILE_SIZE = "max_file_size";
const string CConfigData::FILE_HANDLE_CACHE_SIZE = "file_handle_cache_size";

const string CConfigData::STARTUP_LOG = "startup_log";
const string CConfigData::SHUTDOWN_LOG = "shutdown_log";
const string CConfigData::SIGNON_LOG = "signon_log";
const string CConfigData::USER_LOG = "user_log";
const string CConfigData::GROUP_LOG = "group_log";
const string CConfigData::QUEUE_LOG = "queue_log";
const string CConfigData::MESSAGE_LOG = "message_log";
const string CConfigData::FORWARD_LOG = "forward_log";
const string CConfigData::INFO_LOG = "info_log";

const string CConfigData::SSL_PORT = "ssl_port";
const string CConfigData::SSL_ADDRESS = "ssl_address";
const string CConfigData::SSL_CERT = "ssl_cert";
const string CConfigData::SSL_KEY = "ssl_key";
const string CConfigData::SSL_CA = "ssl_ca";
const string CConfigData::SSL_CA_DIR = "ssl_ca_dir";
const string CConfigData::SSL_REQUIRE_CERT = "ssl_require_cert";

const string CConfigData::SSL_CLIENT_CERT = "ssl_client_cert";
const string CConfigData::SSL_CLIENT_KEY = "ssl_client_key";

const char* WHITE_SPACE="\r\n\t ";

string& trim(string& str) {
	int	b = str.find_first_not_of(WHITE_SPACE);
	int e = str.find_last_not_of(WHITE_SPACE);

	if (b != string::npos)
		str = str.substr(b, e+1);
	return str;
}


CConfigData::CConfigData()
{
	loadData();
}

CConfigData::~CConfigData()
{

}


string CConfigData::getSetting(const string& name, const string& def)
{
	set<CConfigEntry>::iterator item = items.find(CConfigEntry(name));
	if (item != items.end())
		return item->value;
	return def;
}

void CConfigData::applySetting(const string& name, const string& value)
{
	set<CConfigEntry>::iterator item = items.find(CConfigEntry(name));
	if (item != items.end()) {
		item->value = value;
	} else {
		CConfigEntry	entry;

		entry.name = name;
		entry.value = value;
		items.insert(entry);
	}
}


void CConfigData::loadData()
{
	int		lineNo;
	string	line;
	fstream	f("c:\\windows\\safmq.conf", ios::in);

	if (f.good()) {
		lineNo = 0;
		getline(f, line);
		while (f.good()) {
			lines.push_back(line);

			trim(line);

			if (line.length()) {
				string::size_type	o;
				if (line[0] != '#' && (o=line.find(":")) != string::npos) {
					string key = line.substr(0,o);
					string value;
					if (line.length() >= o+1)
						value = line.substr(o+1);
					trim(key);
					trim(value);

					if (key.length()) {
						CConfigEntry entry;
						entry.name = key;
						entry.value = value;
						entry.line = lineNo;
						items.insert(entry);

						ATLTRACE("%s:%s:%ld\n", key.c_str(), value.c_str(), lineNo);
					}
				}
			}

			lineNo++;
			getline(f, line);
		}
		f.close();
	}
}

void CConfigData::storeSettings()
{
	set<CConfigEntry>::iterator item;

	for(item = items.begin(); item != items.end(); ++item) {
		if (item->line >= 0) {
			if (item->line > (int)lines.size())
				lines.reserve(item->line + 1);
			lines[item->line] = item->name + ": " + item->value;
		} else {
			item->line = lines.size();
			lines.push_back(item->name + ": " + item->value);
		}
	}

	fstream	f("c:\\windows\\safmq.conf", ios::out);
	if (f.good()) {
		for(vector<string>::iterator line = lines.begin(); line != lines.end(); ++line) {
			f << *line << endl;
		}
		f.close();
	}

}
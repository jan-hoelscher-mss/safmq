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




This software implements a platform independent Store and Forward Message Queue.
*/
#pragma warning(disable:4786)
#include <fstream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include "param_config.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#if !defined(CONFIG_ENV)
#define CONFIG_ENV CONFIG_ENVIRONMENT_VARIABLE
#endif

#define STRINGIZE(s) #s

using namespace std;

static std::string trim(const std::string& src)
{
	std::string::size_type b = src.find_first_not_of(" \t\r\n");
	std::string::size_type e = src.find_last_not_of(" \t\r\n");

	if (b <= e && b != std::string::npos)
		return src.substr(b, e-b+1);

	return "";
}

Config::Config(const std::string& cfg) throw (int)
{
	const char	*params = getenv(CONFIG_ENV);

	if (!params)
		params = cfg.c_str();

	fstream	*f = new fstream(params, ios::in);
	if (!f->good()) {  // couldn't open the param file, try and open one in the default location
		delete f;

#ifdef unix
		char tmp[1024] = CONFIG_DIR; // set in makefile.in

		if (tmp[strlen(tmp)-1] != '/')
			strcat(tmp, "/");

		strcat(tmp,params);
		f = new fstream(tmp, ios::in);
		if (!f->good()) {
			delete f;
			throw -1;
		}
#elif defined(_WIN32)
		char	tmp[1024];
		GetWindowsDirectory(tmp,sizeof(tmp));
		if (tmp[strlen(tmp)-1] != '\\') 
			strcat(tmp,"\\");
		strcat(tmp,params);
		f = new fstream(tmp, ios::in);
		if (!f->good()) {
			delete f;
			throw -1;
		}
#else
		throw -1;
#endif
	}

	std::string				line;
	std::string				key, value;
	std::string::size_type		o;
	getline(*f,line);
	while (f->good()) {
		if ( ((o=line.find(":")) != std::string::npos) && (o+1 < line.length())) {
			key = trim(line.substr(0,o));
			value = trim(line.substr(o+1));

			if (key != "" && key[0] != '#')
				m_params.insert(map<std::string,std::string>::value_type(key.c_str(),value.c_str()));
		}
		getline(*f,line);
	}
	f->close();
	delete f;
}

const std::string& Config::getParam(const std::string& param, const std::string& def) const
{
	map<std::string,std::string>::const_iterator	i = m_params.find(param);
	if (i != m_params.end()) {
		return i->second;
	}
	return def;
}

int Config::getIntParam(const std::string& param, int def) const
{
	map<std::string,std::string>::const_iterator	i = m_params.find(param);
	if (i != m_params.end())
		return atoi(i->second.c_str());
	return def;
}


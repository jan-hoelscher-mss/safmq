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
#include "DirectoryList.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>
#endif

using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


DirectoryList::DirectoryList(const std::string& directory, const std::string& filter)
{
#ifdef _WIN32
	// NOTE: WINDOWS Directory Listing done here.
	::WIN32_FIND_DATA	file;
	::SYSTEMTIME		systime;
	::HANDLE			hFind;
	struct tm			t;

	hFind = ::FindFirstFile((directory+DIRSEP+filter).c_str(), &file);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			::FileTimeToSystemTime(&file.ftCreationTime, &systime);
			t.tm_year	= systime.wYear - 1900;
			t.tm_mon	= systime.wMonth;
			t.tm_mday	= systime.wDay;
			t.tm_hour	= systime.wHour;
			t.tm_min	= systime.wMinute;
			t.tm_sec	= systime.wSecond;

			push_back(DirListItem(file.cFileName,mktime(&t),file.nFileSizeLow));
		} while (::FindNextFile(hFind, &file));
		::FindClose(hFind);
	}
#else
	int l;
	std::string	myfilter = filter;

	// Convert standard directory file wildcards to a regular expression.
	l=0;
	while ( (l=myfilter.find(".",l)) != std::string::npos) {
		myfilter.replace(l,1,"\\.");
		l+=2;
	}
	l=0;
	while ( (l=myfilter.find("*",l)) != std::string::npos) {
		myfilter.replace(l,1,".*");
		l+=2;
	}
	// Do the UNIX style directory list
	DIR*	dir = ::opendir(directory.c_str());
	if (dir) {
		struct dirent	*ent;
		struct stat		stats;
		regex_t			preg;
		::regcomp(&preg, myfilter.c_str(), REG_NOSUB);
		while ( (ent=readdir(dir)) != NULL) {
			if (::regexec(&preg, ent->d_name, 0, NULL, 0) == 0) {
				::stat((directory+DIRSEP+ent->d_name).c_str(), &stats);
				push_back(DirListItem(ent->d_name, stats.st_ctime, stats.st_size));
			}
		}
		::regfree(&preg);
		::closedir(dir);
	}

#endif
}

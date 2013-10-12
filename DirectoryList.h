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
#if !defined(_DIRECTORYLIST_H_)
#define _DIRECTORYLIST_H_

#pragma warning(disable: 4786)

#ifdef _WIN32
	#include <direct.h>
	#define DIRSEP "\\"
	#define STAT _stat
#else
	#include <sys/stat.h>
	#include <sys/types.h>
	#define DIRSEP "/"
	#define STAT stat
#endif
#include <string>
#include <vector>
#include <time.h>
#include "safmq_defs.h"

struct DirListItem {
	DirListItem() {}
	DirListItem(const std::string& fname, time_t c_time, size_t size) {
		this->fname		= fname;
		this->c_time	= c_time;
		this->size		= size;
	}
	DirListItem(const DirListItem& item) {
		operator=(item);
	}
	DirListItem& operator=(const DirListItem& item) {
		fname	= item.fname;
		c_time	= item.c_time;
		size	= item.size;
		return *this;
	}
	std::string	fname;
	time_t	c_time;
	size_t	size;
};


class DirectoryList : public std::vector<DirListItem>
{
public:
	DirectoryList(const std::string& directory, const std::string& filter);
};

#endif // !defined(_DIRECTORYLIST_H_)

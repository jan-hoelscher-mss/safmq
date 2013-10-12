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


#if !defined(_STREAM_CACHE_H)
#define _STREAM_CACHE_H

#include "Mutex.h"
//#include "Log.h"
#include <fstream>
#include <errno.h>
#include <string>

namespace safmq {

struct StreamCacheEntry {
	std::string			filename;
	std::fstream		*stream;
	bool				inuse;
	std::ios::openmode	openmode;

	StreamCacheEntry(): stream(NULL), inuse(false) {

	}
	StreamCacheEntry(const StreamCacheEntry& src) {
		*this = src;
	}
	virtual ~StreamCacheEntry() {}

	const StreamCacheEntry& operator=(const StreamCacheEntry& src) {
		filename	= src.filename;
		stream		= src.stream;
		inuse		= src.inuse;
		openmode	= src.openmode;
		return *this;
	}
};

/**
 * A class to cache open file streams
 */
class StreamCache {
protected:
	int 				cacheSize;
	int					entryCount;
	SAFMQ_UINT32		accessCount;
	SAFMQ_UINT32		hitCount;
	SAFMQ_UINT32		reopenCount;

	StreamCacheEntry	*entries;
	Mutex				mtx;

public:
	/**
	 * Constructs the stream cache setting the cache size.
	 */
	StreamCache(int cacheSize) {
		this->cacheSize = cacheSize;
		entryCount = 0;
		entries = new StreamCacheEntry[cacheSize];
		accessCount = hitCount = reopenCount = 0;
	}

	/**
	 * Destroys the cache and releases the contestns
	 */
	virtual ~StreamCache() {
		for(int x=0; x<entryCount; ++x) {
			entries[x].stream->flush();
			entries[x].stream->close();
			delete entries[x].stream;
		}
		delete [] entries;
	}

	/**
	 * The number of times the cache has been accessed
	 * @return The total number of times the cache has been accessed
	 */
	SAFMQ_UINT32 getAccessCount() {
		return accessCount;
	}

	/**
	 * The number of times the requested file was found in the cache
	 * @return The number of times the requested file was found
	 */
	SAFMQ_UINT32 getHitCount() {
		return hitCount;
	}

	/**
	 * The number of times the file had to be re-opened
	 * @return The number of partial hits (file had to be reopened for i/o access)
	 */
	SAFMQ_UINT32 getReopenCount() {
		return reopenCount;
	}


	// TODO: What to do with ios::trunc?
	/**
	 * Obtains a stream from the cache
	 * @param filename The name of the file
	 * @param mode The access mode for the file (i.e. ios::in, ios::out, etc)
	 */	
	std::fstream* get(const std::string& filename, std::ios::openmode mode) {
		MutexLock lock(&mtx); // unlocks upon exit from method
		++accessCount;

		//cout << "Request to open: " << filename << endl;
		//Log::getLog()->Info("Request to open %s", filename.c_str());

		int	x;
		StreamCacheEntry	*entry = NULL;

		for(x=0; x < entryCount; ++x) {
			if (filename == entries[x].filename) {
				entry = entries + x;
				break; // to preserve x's value
			} 
		}
		if (entry != NULL) {
			//Log::getLog()->Info("Found existing file handle: %s", entry->filename.c_str());

			++hitCount;
			// The entry was found
			if (x > 0) {
				StreamCacheEntry tmp = entries[x];
				entries[x] = entries[x-1];
				entries[x-1] = tmp;
				entry = entries + (x-1);
			}

			// validate openmode, if the current mode masked by mode does not equal the mode then reopen
			if ((entry->openmode & mode & (std::ios::in|std::ios::out)) != (mode & (std::ios::in|std::ios::out)) ) {

				++reopenCount;
				//Log::getLog()->Info("Reopening: %s", entry->filename.c_str());
				//std::cout << "Reopening:" << entry->filename << std::endl;
				// reopen the file
				entry->stream->flush();	
				entry->stream->close();
				delete entry->stream;
				entry->stream = new std::fstream();
				entry->openmode = std::ios::in | std::ios::out | (mode & ~(std::ios::app|std::ios::ate));

				entry->stream->open(entry->filename.c_str(), entry->openmode);
			}

			if (mode & (std::ios::app|std::ios::ate)) {
				//Log::getLog()->Info("Pre Seek: %d", (int)entry->stream->good());
				//errno = 0;
				if (entry->openmode & std::ios::out)
					entry->stream->seekp(0, std::ios::end);
				//Log::getLog()->Info("Post seekp: %d", (int)entry->stream->good());
				//errno = 0;
				//Log::getLog()->Info("tellg(): %d", entry->stream->tellg());
				if (entry->openmode & std::ios::in)
					entry->stream->seekg(0, std::ios::beg);
				int err = errno;
				//Log::getLog()->Info("Post seekg: good:%d, bad:%d, fail:%d, eof:%d %s", (int)entry->stream->good(), (int)entry->stream->bad(), (int)entry->stream->fail(), (int)entry->stream->eof(), strerror(err));
			} else {
				//Log::getLog()->Info("else Pre Seek: %d", (int)entry->stream->good());
				if (entry->openmode & std::ios::out)
					entry->stream->seekp(0, std::ios::beg);
				//Log::getLog()->Info("else Post seekp: %d", (int)entry->stream->good());
				if (entry->openmode & std::ios::in)
					entry->stream->seekg(0, std::ios::beg);
				//Log::getLog()->Info("else Post seekg: %d", (int)entry->stream->good());
			}
		} else {
			if (entryCount < cacheSize) {
				entry = entries + entryCount;
				++entryCount;
			} else {
				if (!entries[entryCount-1].inuse) {
					entries[entryCount-1].stream->close();
					delete entries[entryCount-1].stream;
				}
				entry = entries + (entryCount - 1);
			}
			//std::cout << "First Open:" << filename << std::endl;
			//Log::getLog()->Info("First Open: %s", filename.c_str());

			entry->filename = filename;
			entry->stream = new std::fstream();
			entry->openmode = mode;

			//Log::getLog()->Info("Info: mode&ios::in %d", mode & std::ios::in);
			//Log::getLog()->Info("Info: mode&ios::out %d", mode & std::ios::out);
			//Log::getLog()->Info("Info: mode&ios::app %d", mode & std::ios::app);
			//Log::getLog()->Info("Info: mode&ios::ate %d", mode & std::ios::ate);

			//errno = 0;
			entry->stream->open(filename.c_str(), entry->openmode);
			int err = errno; // save errno
			//std::cout << "errno: " << err << std::endl;
			//Log::getLog()->Info("errno: %ld", err);
			errno = err;
			if (!(entry->stream->good())) {
				//std::cout << "First Open Error: " << strerror(err) << std::endl;
				//Log::getLog()->Info("%s, %s", filename.c_str(), strerror(err));
			}
			if (entry->stream->fail()) {
				//std::cout << "First Open Error/Fail: " << strerror(err) << std::endl;
				//Log::getLog()->Info("%s, %s", filename.c_str(), strerror(err));
			}
			errno = err;
		}

		entry->inuse = true;
		//Log::getLog()->Info("Got Stream: %p", entry->stream);
		return entry->stream;
	}

	/**
	 * releases the stream back to the cache for reuse
	 * @param f the stream from the cache
	 */
	void release(std::fstream* f) {
		MutexLock lock(&mtx); // unlocks upon exit from method
		//Log::getLog()->Info("Release Stream: %p", f);

		for(int x=0; x<entryCount; ++x) {
			if (entries[x].stream == f) {
				//Log::getLog()->Info("Releaseing: %s stream good: %d", entries[x].filename.c_str(), (int)f->good());
				entries[x].stream->flush();
				//Log::getLog()->Info("After flush state: %d", (int)f->good());
				entries[x].inuse = false;
				return;
			}
		}

		f->close(); // close the stream it isn't in the cache any longer
		delete f;
	}

	/**
	 * delete the file and remove from the cache
	 * @param filename the file to be removed
	 */
	void remove(const std::string& filename) {
		MutexLock lock(&mtx); // unlocks upon exit from method

		for(int x=0; x<entryCount; ++x) {
			if (entries[x].filename == filename) {
				// remove the entry
				if (!entries[x].inuse) {
					//Log::getLog()->Info("NOTE: Closing and removing %s", filename.c_str());
					entries[x].stream->close();
					delete entries[x].stream;
				} else {
					//Log::getLog()->Info("NOTE: Unable to close %s, inuse", filename.c_str());
				}

				// shift the remaining entries up
				for(++x; x < entryCount; ++x) {
					entries[x-1] = entries[x];
				}
				--entryCount;
				break;
			}
		}
		//Log::getLog()->Info("NOTE: Removing file %s", filename.c_str());
		::remove(filename.c_str());
	}

	void listEntries() {
		for(int x=0;x<entryCount;++x)
			std::cout << entries[x].filename << " " << entries[x].inuse << std::endl;
	}
};

}
#endif

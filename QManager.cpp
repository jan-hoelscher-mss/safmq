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
#pragma warning(disable: 4786)
#pragma warning(disable:4996) // Disable warnings about deprecated functions
#include <iostream>
#include <fstream>
#include <errno.h>
#include "QManager.h"
#include "QStorage.h"
#include "DirectoryList.h"
#include "comdefs.h"
#include "main.h"
#include "Log.h"

#ifndef _WIN32
#include <unistd.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace std;

const char config_file[] = "queues.dat";

QManager::QManager() : Thread()
{
	queueDir = pcfg->getParam(QUEUE_DIR_PARAM, DEFAULT_QUEUE_DIR);
	if (queueDir.length() == 0)
		queueDir = "queues";

	maxFileSize = pcfg->getIntParam(MAX_FILE_SIZE, DEFAULT_MAX_FILE_SIZE);

	LoadQueueConfig();

	std::string param = pcfg->getParam(ENABLE_FOWARDING_PARAM,"no");
	if (stricmp(param.c_str(),"yes")==0 || param=="1") {
		ErrorCode ec = CreateQueue(FORWARD_QUEUE_NAME,"admin");
	}

	int streamCacheSize = pcfg->getIntParam(FILE_HANDLE_CACHE_SIZE, DEFAULT_FILE_HANDLE_CACHE_SIZE);
	if (streamCacheSize < 0) {
		streamCacheSize = DEFAULT_FILE_HANDLE_CACHE_SIZE;
		Log::getLog()->Info("File Handle Cache Size less than zero, defaulting to: %ld", streamCacheSize );
	} else if (streamCacheSize > MAX_FILE_HANDLE_CACHE_SIZE) {
		streamCacheSize = DEFAULT_FILE_HANDLE_CACHE_SIZE;
		Log::getLog()->Info("File Handle Cache Size greater than %ld, defaulting to: %ld", MAX_FILE_HANDLE_CACHE_SIZE, streamCacheSize );
	}

	Log::getLog()->Info("Maximum Queue storage file size set to: %ld", maxFileSize);
	Log::getLog()->Info("File Handle Cache size set to: %ld", streamCacheSize);

	streams = new StreamCache(streamCacheSize);
}

QManager::~QManager()
{
	for(QueueMap::iterator queue = queues.begin(); queue != queues.end(); queue++)
		delete queue->second.pqueue;
	queues.clear();
	delete streams;
}

StreamCache* QManager::GetStreamCache()
{
	return streams;
}

void QManager::LoadQueueConfig()
{
	QUEUE_CONFIG_ENTRY qent;

	fstream in((queueDir+DIRSEP+config_file).c_str(), ios::in);
	if (in.good()) {
		while (in.good()) {
			in.read((char*)&qent,sizeof(qent));
			if (in.good()) {
				CreateQueue(std::string(qent.queuename,SAFMQ_QNAME_LENGTH).c_str(),
								std::string(qent.owner,SAFMQ_USER_NAME_LENGTH).c_str(),false);
			}
		}
		in.close();
	}
}

void QManager::StoreQueueConfig()
{
	QUEUE_CONFIG_ENTRY	qent;
	fstream out((queueDir+DIRSEP+config_file).c_str(), ios::out);
	if (out.good()) {
		QueueMap::iterator q;
		for(q = queues.begin(); out.good() && q!=queues.end(); q++) {
			memset(&qent,0,sizeof(qent));
			std::char_traits<char>::copy(qent.queuename,q->first.c_str(), std::min(q->first.length(), (size_t)SAFMQ_QNAME_LENGTH));
			std::char_traits<char>::copy(qent.owner, q->second.owner.c_str(), std::min(q->second.owner.length(),(size_t)SAFMQ_USER_NAME_LENGTH));
			out.write((const char*)&qent, sizeof(qent));
		}
		out.close();
	}
}

QStorage* QManager::GetQueue(const std::string& queuename)
{
	MutexLock	lock(&mtx);
	QueueMap::iterator queue = queues.find(queuename);
	if (queue != queues.end()) {
		utilization[queue->second.pqueue]++;
		return queue->second.pqueue;
	}
	return NULL;
}

void QManager::ReleaseQueue(QStorage* pqueue)
{
	MutexLock			lock(&mtx);
	QueueMap::iterator	queue;

	for(queue = queues.begin(); queue != queues.end(); ) {
		// Delete the queue if no one is using it any more.
		QueueMap::iterator tmp = queue++;  // use a temp so that we don't loose the iterator if we delete

		if (tmp->second.pqueue == pqueue) {
			int util = (--utilization[tmp->second.pqueue]);

			if (util == 0 && tempQueues.find(tmp->first) != tempQueues.end()) {
				std::string qname = tmp->first;

				sockaddr_in	addr;
				memset(&addr,0,sizeof(addr));
				Log::getLog()->Queue(Log::deleted,"temp",&addr,qname.c_str());
				deleteQueue(qname);
			}
		}
	}
}

ErrorCode QManager::CreateQueue(const std::string& queuename, const std::string& user, bool store, bool temporary)
{
	MutexLock	lock(&mtx);
	QueueMap::iterator	queue = queues.find(queuename);
	if (queue == queues.end()) {
		std::string fname;

		// No control characters or slashes.
		const char*	invalid_chars = "\\/"
									"\001\002\003\004\005\006\007\010"
									"\011\012\013\014\015\016\017\020"
									"\021\022\023\024\025\026\027\030"
									"\031\032\033\034\035\036\037\040";

		if (queuename.find_first_of(invalid_chars) != std::string::npos) {
			return EC_INVALIDNAME;
		}

		#ifdef _WIN32
		mkdir((queueDir+DIRSEP+w2acopy(fname,queuename)).c_str());
		#else
		int err = mkdir((queueDir+DIRSEP+w2acopy(fname,queuename)).c_str(),0775);
		#endif

		queues.insert(QueueMap::value_type(queuename,QInfo(new QStorage(queuename, queueDir+DIRSEP+w2acopy(fname,queuename), maxFileSize ),user)));
		if (store)
			StoreQueueConfig();
		if (temporary)
			tempQueues.insert(queuename);
		return EC_NOERROR;
	}
	return EC_ALREADYDEFINED;
}

ErrorCode QManager::DeleteQueue(const std::string& queuename)
{
	MutexLock	lock(&mtx);
	return deleteQueue(queuename);
}

ErrorCode QManager::deleteQueue(const std::string& queuename)
{
	QueueMap::iterator	queue = queues.find(queuename);
	if (queue != queues.end()) {
		if (utilization[queue->second.pqueue] <= 0) {
			delete queue->second.pqueue; // this closes down the queue.

			std::string fname;
			std::string queuepath = queueDir+DIRSEP+w2acopy(fname,queuename);
			DirectoryList::size_type	x;
			
			// Remove all the files in the directory
			DirectoryList	dl(queuepath,"*.q");
			for(x=0;x<dl.size();x++) {
				//remove((queuepath+DIRSEP+dl[x].fname).c_str());
				streams->remove((queuepath+DIRSEP+dl[x].fname));
			}
			//streams->listEntries();

			if (rmdir((queueDir+DIRSEP+w2acopy(fname,queuename)).c_str()) != 0) {
				int err = errno;
				if (err == ENOTEMPTY) {
					// TODO: ERROR Do something in this case.
				
				}
				Log::getLog()->Info("ERROR: Unable to remove %s, %s", (queueDir+DIRSEP+queuename).c_str(), strerror(err));
			}
			queues.erase(queue);
			StoreQueueConfig();
			return EC_NOERROR;
		} else
			return EC_QUEUEOPEN;
	}
	return EC_DOESNOTEXIST;
}

void QManager::EnumQueues(std::vector<QManager::QData>& queuenames)
{
	MutexLock			lock(&mtx);
	QueueMap::iterator	i;

	queuenames.clear();

	for(i = queues.begin(); i != queues.end(); i++)
		queuenames.push_back(QData(i->first.c_str(),i->second.owner.c_str()));
}

void QManager::stop()
{
	Thread::stop();
	quitSignal.Set();
}

void* QManager::run()
{
	unsigned int count = 1;

	while (!m_bStop) {
		quitSignal.WaitFor(1000);
		if (!m_bStop) {
			MutexLock	lock(&mtx);
			for(QueueMap::iterator queue = queues.begin(); !m_bStop && queue != queues.end(); queue++)
				queue->second.pqueue->TTLPurge();
		}
		if ( ++count % 60 == 0 ) {
			// TODO: Remove this output
			double a = streams->getAccessCount();
			double h = streams->getHitCount();
			double r = streams->getReopenCount();
			if (a != 0.0)
				Log::getLog()->Info("Stream Hit Ratio: %0.2lf/%0.2lf (a:%0.0lf h:%0.0lf r:%0.0lf)",
					100*h/a, 100*(h-r)/a, a, h, r);
		}
	}
	return NULL;
}

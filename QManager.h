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
#ifndef _QMANAGER_H_
#define _QMANAGER_H_

#pragma warning(disable: 4786)

#include <map>
#include <string>
#include <set>
#include "Mutex.h"
#include "safmq.h"
#include "safmq-Signal.h"
#include "thdlib.h"
#include "StreamCache.h"

using namespace safmq;

class QManager : public Thread
{
protected:
	struct QInfo {
		QInfo() : pqueue(NULL) {}
		QInfo(const QInfo& src) {
			operator=(src);
		}
		QInfo(QStorage* pqueue, const std::string& owner) {
			this->pqueue = pqueue;
			this->owner = owner;
		}
		QInfo& operator=(const QInfo& src) {
			pqueue = src.pqueue;
			owner = src.owner;
			return *this;
		}
		QStorage*	pqueue;
		std::string	owner;
	};
public:
	struct QData {
		QData() {}
		QData(const QData& src) {
			operator=(src);
		}
		QData(const std::string& queuename, const std::string& owner) {
			this->queuename = queuename;
			this->owner = owner;
		}
		QData& operator=(const QData& src) {
			queuename = src.queuename;
			owner = src.owner;
			return *this;
		}
		std::string	queuename;
		std::string	owner;
	};
public:
	QManager();
	virtual ~QManager();

	virtual QStorage* GetQueue(const std::string& queuename);
	virtual void ReleaseQueue(QStorage* queue);
	virtual ErrorCode CreateQueue(const std::string& queuename, const std::string& user, bool store=true, bool temporary=false);
	virtual ErrorCode DeleteQueue(const std::string& queuename);

	virtual void EnumQueues(std::vector<QData>& queuenamess);

	virtual void stop();

	virtual StreamCache* GetStreamCache();
protected:
	virtual ErrorCode deleteQueue(const std::string& queuename);

	virtual void StoreQueueConfig();
	virtual void LoadQueueConfig();

	virtual void* run();


	typedef std::map<std::string, QInfo>		QueueMap;
	typedef std::map<QStorage*, SAFMQ_INT32>	UtilizationMap;
	typedef std::set<std::string>				TempQueues;

	UtilizationMap							utilization;
	TempQueues								tempQueues;
	QueueMap								queues;
	Mutex									mtx;
	std::string								queueDir;
	SAFMQ_INT32								maxFileSize;
	Signal									quitSignal;
	StreamCache								*streams;
};

#endif // !defined(_QMANAGER_H_)

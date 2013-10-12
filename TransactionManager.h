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

#if !defined(_TRANSACTIONMANAGER_H_)
#define _TRANSACTIONMANAGER_H_

#include "ServiceThread.h"

namespace safmq {

class TransactionManager  
{
public:
	enum transType { bad, enqueue, retrieve };

protected:

	struct TData {
		TData() {
			cursor = -1;
			type = bad,
			commitable = false;
		}
		
		TData(QStorage::CursorHandle cursor, transType type, bool commitable=false) {
			this->cursor = cursor;
			this->type = type;
			this->commitable = commitable;
		}
		
		TData(const TData& src) {
			copy(src);
		}
		
		const TData& operator=(const TData& src) {
			copy(src);
			return *this;
		}

		void copy(const TData& src) {
			this->cursor = src.cursor;
			this->type = src.type;
			this->commitable = src.commitable;
		}

		QStorage::CursorHandle	cursor;
		transType				type;
		bool					commitable;
	};

	#define TKey_qid first
	#define TKey_uuid second

	typedef std::pair<ServiceThread::QueueHandle,uuid> TKey;

	typedef std::map<TKey,TData>	TMap;


	bool				inTransaction;
	ServiceThread*		pParent;
	TMap				transactions;

public:
	TransactionManager(ServiceThread* pParent);
	virtual ~TransactionManager();

	bool BeginTransaction();
	void CommitTransaction();
	void RollbackTransaction();
	bool EndTransaction();


	void Enqueue(ServiceThread::QueueHandle qid, const uuid& msgid, const QStorage::CursorHandle& cursor);
	void BeginRetrieve(ServiceThread::QueueHandle qid, const uuid& msgid, const QStorage::CursorHandle& cursor);
	ErrorCode FinalizeRetrieve(ServiceThread::QueueHandle qid, const uuid& msgid);
	void ShutdownQueue(ServiceThread::QueueHandle qid);
};

}

#endif // !defined(_TRANSACTIONMANAGER_H_)


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

#include "TransactionManager.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace safmq;

TransactionManager::TransactionManager(ServiceThread* pParent)
{
	inTransaction = false;
	this->pParent = pParent;
}

TransactionManager::~TransactionManager()
{

}

/**
Begin holding transactions
*/
bool TransactionManager::BeginTransaction()
{
	//OutputDebugString("BeginTransaction");
	if (!inTransaction) {
		inTransaction = true;
		return true;
	}
	return false;
}

void TransactionManager::CommitTransaction()
{
	if (inTransaction) {
		TMap::iterator	trans, current;
		for(trans=transactions.begin();trans!=transactions.end();) {
			current = trans++;
			switch (current->second.type) {
				case retrieve:
					if (current->second.commitable) {
						pParent->commitRetrieve(current->first.TKey_qid, current->second.cursor);
						transactions.erase(current);
					}
					break;

				case enqueue:
					// Handle Enqueue Transactions
					pParent->commitEnqueue(current->first.TKey_qid, current->second.cursor);
					transactions.erase(current);
					break;

				default:
					transactions.erase(current);
					break;
			}
		}
	}
}

void TransactionManager::RollbackTransaction()
{
	if (inTransaction) {
		TMap::iterator	trans,current;
		for(trans=transactions.begin();trans!=transactions.end();) {
			current = trans++;
			switch (current->second.type) {
				case retrieve:
					// Handle retrieve Transactions
					if (current->second.commitable) {
						pParent->rollbackRetrieve(current->first.TKey_qid, current->second.cursor);
						transactions.erase(current);
					}
					break;

				case enqueue:
					// Handle Enqueue Transactions
					pParent->rollbackEnqueue(current->first.TKey_qid, current->second.cursor);
					transactions.erase(current);
					break;

				default:
					transactions.erase(current);
					break;
			}
		}	
	}
}

bool TransactionManager::EndTransaction()
{
	if (inTransaction) {
		CommitTransaction();
		inTransaction = false;
		return true;
	}
	return false;
}

void TransactionManager::BeginRetrieve(ServiceThread::QueueHandle qid, const uuid& msgid, const QStorage::CursorHandle& cursor)
{
	//OutputDebugString("Begin Retrieve\n");
	transactions[TKey(qid,msgid)] = TData(cursor,retrieve);
}

void TransactionManager::Enqueue(ServiceThread::QueueHandle qid, const uuid& msgid, const QStorage::CursorHandle& cursor)
{
	if (inTransaction) {
		transactions[TKey(qid,msgid)] = TData(cursor,enqueue);
	} else {
		pParent->commitEnqueue(qid,cursor);
	}
}



ErrorCode TransactionManager::FinalizeRetrieve(ServiceThread::QueueHandle qid, const uuid& msgid)
{
	if (inTransaction) {
		//OutputDebugString("Finalize Retrieve Transaction\n");
		// Mark the transaction as commitable
		TMap::iterator trans = transactions.find(TKey(qid,msgid));
		if (trans != transactions.end()) {
			trans->second.commitable = true;
			return EC_NOERROR;
		}
	} else {
		//OutputDebugString("Finalize Retrieve Single\n");
		// Not in a transaction, commit the retrieve now
		TMap::iterator trans = transactions.find(TKey(qid,msgid));
		if (trans != transactions.end()) {
			ErrorCode ret = pParent->commitRetrieve(qid,trans->second.cursor);
			if (ret == EC_NOERROR)
				transactions.erase(trans);
			return ret;
		}
	}
	return EC_DOESNOTEXIST;	
}

void TransactionManager::ShutdownQueue(ServiceThread::QueueHandle qid)
{
	TMap::iterator	trans = transactions.lower_bound(TKey(qid,UUID_NULL));

	while (trans != transactions.end() && trans->first.first == qid) {

		switch (trans->second.type) {
			case retrieve:
				// Handle retrieve Transactions
				if (trans->second.commitable) {
					pParent->rollbackRetrieve(trans->first.TKey_qid, trans->second.cursor);
				}
				break;

			case enqueue:
				// Handle Enqueue Transactions
				pParent->rollbackEnqueue(trans->first.TKey_qid, trans->second.cursor);
				break;
		}

		transactions.erase(trans);
		trans = transactions.lower_bound(TKey(qid,UUID_NULL));
	}
}

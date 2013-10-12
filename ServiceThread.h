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
#if !defined(_SERVICETHREAD_H_)
#define _SERVICETHREAD_H_

#pragma warning(disable: 4786)

#include "tcpsocket/socstream.h"
#include "safmq.h"
#include "QStorage.h"
#include "thdlib.h"


namespace safmq {
class TransactionManager;
class CommandFactory;




class ServiceThread : public Thread
{
public: // typedefs
	typedef SAFMQ_INT32 QueueHandle;

public: // implementation
	explicit ServiceThread(tcpsocket::socstream* client, bool trusted=false, const std::string& trustedUser="");
	virtual ~ServiceThread();

	bool isTrusted() const;
	const std::string& getUsername() const;
	void setUsername(const std::string& username);
	const sockaddr_in* getPeername() const;

	void getClientInfo(ClientInfo& ci);

	QStorage* getOpenQueue(QueueHandle qh);
	TransactionManager* getTransactionManager();
	bool loggedIn();
	void addCursor(QueueHandle queueH, QStorage::CursorHandle cursorH);
	void removeCursor(QueueHandle queueH, QStorage::CursorHandle cursorH);

	ServiceThread::QueueHandle registerQueue(QStorage* pqueue);

	ErrorCode shutdownQueue(ServiceThread::QueueHandle queueID);

protected: // implementation
	void* run();

	void UnknownMessage(char command);
	void Login();

protected: //implementation
	ErrorCode commitRetrieve(ServiceThread::QueueHandle qid, const QStorage::CursorHandle& cursor);
	ErrorCode rollbackRetrieve(ServiceThread::QueueHandle qid, const QStorage::CursorHandle& cursor);
	ErrorCode commitEnqueue(ServiceThread::QueueHandle qid, const QStorage::CursorHandle& cursor);
	ErrorCode rollbackEnqueue(ServiceThread::QueueHandle qid, const QStorage::CursorHandle& cursor);

protected: // data
	typedef std::map<QueueHandle, QStorage*>							QueueMap;
	typedef std::set<std::pair<QueueHandle, QStorage::CursorHandle> >	CursorSet;
	tcpsocket::socstream*	client;
	std::string				username;
	struct ::sockaddr_in	peername;

	bool					logged_in;
	bool					trusted;

	QueueMap				queues;
	CursorSet				cursors;
	TransactionManager*		transactionManager;
	CommandFactory*			commandFactory;
	Mutex					loginMtx;

	friend class TransactionManager;
};


class ClientRegistry 
{
public:
	static void RegisterClient(ServiceThread* pService);
	static void UnregisterClient(ServiceThread* pService);
	static void GetClientInfo(std::list<ClientInfo>& info);
private:
	static std::list<ServiceThread*>	clients;
	static Mutex						mtx;
};


} // end of samfq namespace

#endif // !defined(_SERVICETHREAD_H_)

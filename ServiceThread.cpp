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
#include <time.h>
#include <iostream>
#include "main.h"

#include "ServiceThread.h"
#include "TransactionManager.h"

#include "QStorage.h"
#include "comdefs.h"
#include "server_operators.h"
#include "QManager.h"
#include "QAccessControl.h" 
#include "Log.h"
#include "Command.h"

extern QManager*	theQueueManager;

using namespace std;

namespace safmq {
	const char* EC_Decode(ErrorCode ec);
}

std::list<ServiceThread*> ClientRegistry::clients;
Mutex ClientRegistry::mtx;

void ClientRegistry::RegisterClient(ServiceThread* pService)
{
	MutexLock lock(&mtx);
	clients.push_back(pService);
}

void ClientRegistry::UnregisterClient(ServiceThread* pService)
{
	MutexLock lock(&mtx);
	clients.remove(pService);
}

void ClientRegistry::GetClientInfo(std::list<ClientInfo>& info)
{
	MutexLock lock(&mtx);
	ClientInfo ci;

	info.clear();

	for(std::list<ServiceThread*>::iterator svc = clients.begin(); svc != clients.end(); svc++) {
		(*svc)->getClientInfo(ci);
		info.push_back(ci);
	}
}

ServiceThread::ServiceThread(tcpsocket::socstream* client, bool trusted, const std::string& trustedUser) : Thread(Thread::detached)
{
	this->client = client;
	this->trusted = trusted;
	logged_in = false;

	if (trusted) {
		username = trustedUser;
	}

	transactionManager = new TransactionManager(this);
	client->getpeername(&peername);

	ClientRegistry::RegisterClient(this);

	commandFactory = CommandFactory::getFactory(SAFMQ_PROTOCOL_MAJOR_VERSION);
}

ServiceThread::~ServiceThread()
{
	ClientRegistry::UnregisterClient(this);

	// Shutdown all the open queues and transactions.
	QueueMap::iterator	queue = queues.begin();
	while(queue != queues.end()) {
		shutdownQueue(queue->first);
		queue = queues.begin();
	}

	try {
		client->close();
	} catch (tcpsocket::SocketException) {
	} catch (std::exception) {
	}

	delete transactionManager;
	// Release the memory used by iostream connected to the client
	delete client;
}

bool ServiceThread::isTrusted() const
{
	return trusted;
}

const std::string& ServiceThread::getUsername() const
{
	return username;
}

void ServiceThread::setUsername(const std::string& username)
{
	this->username = username;
}

const sockaddr_in* ServiceThread::getPeername() const
{
	return &peername;
}


void ServiceThread::getClientInfo(ClientInfo& ci)
{
	MutexLock lock(&loginMtx);
	ci.username = username;
	ci.address = peername.sin_addr.s_addr;
	ci.address = peername.sin_port;
}

TransactionManager* ServiceThread::getTransactionManager()
{
	return transactionManager;
}

QStorage* ServiceThread::getOpenQueue(QueueHandle qh)
{
	QueueMap::iterator qmi = queues.find(qh);
	if (qmi != queues.end()) {
		return qmi->second;
	}
	return NULL;
}

void ServiceThread::addCursor(QueueHandle queueH, QStorage::CursorHandle cursorH)
{
	cursors.insert(CursorSet::value_type(queueH,cursorH));
}

void ServiceThread::removeCursor(QueueHandle queueH, QStorage::CursorHandle cursorH)
{
	CursorSet::iterator	cur = cursors.find(CursorSet::value_type(queueH,cursorH));
	if (cur != cursors.end())
		cursors.erase(cur);
}

ServiceThread::QueueHandle ServiceThread::registerQueue(QStorage* pqueue)
{
	QueueHandle newQueue = 4891;

	QueueMap::reverse_iterator	lastqueue = queues.rbegin();
	if (lastqueue != queues.rend())
		newQueue = lastqueue->first+1;
	queues[newQueue] = pqueue;
	return newQueue;
}

void* ServiceThread::run()
{
	char	cmdID;

	while (client->good()) {
		cmdID = -1;
		client->read(&cmdID,1);
		//Log::getLog()->Info("Command: %ld, 0x%p, client->good():%ld", (long)command, this, client->good());
		if (client->good()) {
			try {
				Command* cmd = commandFactory->build(cmdID, this);
				if (cmd) {
					cmd->readRequest(*client);
					if (cmd->perform() == 0)
						cmd->writeResponse(*client);
					delete cmd;
				} else {
					if (cmdID == LOGIN) {
						Login();
					} else
						UnknownMessage(cmdID);
				}
			} catch (InvalidBufferException& e) {
				// TODO: Log this error
				cerr << e.what() << endl;
				*client << RESPONSE_RESPONSE(EC_UNSUPPORTED_PROTOCOL) << flush;
				client->close();
			}
		}
	}
	//Log::getLog()->Info("Terminating connection with peer 0x%p", this);

	delete this;
	return NULL;
}

void ServiceThread::UnknownMessage(char command)
{
	SAFMQ_INT32 size;

	client->read((char*)&size, sizeof(size));
	if (client->good()) {
		char	c;

		size = ntohl(size);
		Log::getLog()->Info("Unknown message command: %ld length: %ld", (int)command, size);
		
		// flush the input buffer of the message
		for(;client->good() && size>0;size--)
			client->read(&c,sizeof(c));

	} else {
		Log::getLog()->Info("Unknown message command: %ld", (int)command);
	}
	*client << RESPONSE_RESPONSE(EC_NOTIMPLEMENTED) << flush;
}

void ServiceThread::Login()
{
	LOGIN_PARAMS params;
	*client >> params;

	if (client->good()) {
		std::string	tmpname = std::string(params.username,SAFMQ_USER_NAME_LENGTH).c_str();
		std::string	password = std::string(params.password, SAFMQ_PASSWORD_LENGTH).c_str();
		//Log::getLog()->Info("Login msg:%s cert:%s", tmpname.c_str(), username.c_str());

		if (trusted) {
			// for trusted certificate, if the name was sent and does not match the mapped name
			// or if the password was sent and the names match, apply loggin
			if (tmpname.empty() || (tmpname == username && password.empty()) ) {
				// loged in via digital certificate
				logged_in = true;
				Log::getLog()->SignOn(Log::none, username.c_str(), &peername);
				*client << RESPONSE_RESPONSE(EC_NOERROR) << LOGIN_RESPONSE() << flush;
				return;
			}
		}
		
		loginMtx.Lock();
		username = tmpname;
		loginMtx.Unlock();

		if (params.major_version > SAFMQ_PROTOCOL_MAJOR_VERSION) {
			*client << RESPONSE_RESPONSE(EC_UNSUPPORTED_PROTOCOL) << flush;
			Log::getLog()->SignOn(Log::error, username.c_str(), &peername, EC_Decode(EC_UNSUPPORTED_PROTOCOL));
		} else {

			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->verifyPassword(username, password);
			if (res == SecurityControl::GRANTED) {
				logged_in = true;
				*client << RESPONSE_RESPONSE(EC_NOERROR) << LOGIN_RESPONSE() << flush;
				Log::getLog()->SignOn(Log::none, username.c_str(), &peername);
				commandFactory = CommandFactory::getFactory(params.major_version);
			} else {
				*client << RESPONSE_RESPONSE(EC_NOTAUTHORIZED) << flush;
				Log::getLog()->SignOn(Log::error, username.c_str(), &peername, EC_Decode(EC_NOTAUTHORIZED));
			}
		}
	}
}

ErrorCode ServiceThread::commitRetrieve(ServiceThread::QueueHandle qid, const QStorage::CursorHandle& cursor)
{
	QueueMap::iterator queue = queues.find(qid);
	if (queue != queues.end()) {
		queue->second->FinalizeRetrieve(cursor);
		return EC_NOERROR;
	}
	return EC_NOTOPEN;
}

ErrorCode ServiceThread::rollbackRetrieve(ServiceThread::QueueHandle qid, const QStorage::CursorHandle& cursor)
{
	QueueMap::iterator queue = queues.find(qid);
	if (queue != queues.end()) {
		queue->second->CancelRetrieve(cursor);
		return EC_NOERROR;
	}
	return EC_NOTOPEN;
}

ErrorCode ServiceThread::commitEnqueue(ServiceThread::QueueHandle qid, const QStorage::CursorHandle& cursor)
{
	QueueMap::iterator queue = queues.find(qid);
	if (queue != queues.end()) {
		queue->second->FinalizeEnqueue(cursor);
		return EC_NOERROR;
	}
	return EC_NOTOPEN;
}

ErrorCode ServiceThread::rollbackEnqueue(ServiceThread::QueueHandle qid, const QStorage::CursorHandle& cursor)
{
	QueueMap::iterator queue = queues.find(qid);
	if (queue != queues.end()) {
		queue->second->CancelEnqueue(cursor);
		return EC_NOERROR;
	}
	return EC_NOTOPEN;
}

ErrorCode ServiceThread::shutdownQueue(ServiceThread::QueueHandle queueID)
{
// TODO: remove	PendingTransactionsMap::iterator	trans = transactions.lower_bound(PendingTransactionsMap::key_type(queueID,UUID_NULL));
	QueueMap::iterator					queue = queues.find(queueID);
	CursorSet::iterator					cur;

	if (queue != queues.end()) {
		// Cancel all pending transactions
		transactionManager->ShutdownQueue(queueID);
		/*
		while (trans != transactions.end() && trans->first.first == queueID) {
			if (queue != queues.end())
				queue->second->CancelRetrieve(trans->second.transCursor);
			transactions.erase(trans);
			trans = transactions.lower_bound(PendingTransactionsMap::key_type(queueID,UUID_NULL));
		}
		*/

		// Close al open cursors
		cur = cursors.lower_bound(CursorSet::value_type(queueID,0));
		while (cur != cursors.end() && cur->first == queueID) {
			if (queue != queues.end())
				queue->second->CloseCursor(cur->second);
			cursors.erase(cur);
			cur = cursors.lower_bound(CursorSet::value_type(queueID,0));
		}

		// Release the queue			
		theQueueManager->ReleaseQueue(queue->second);
		// Erase the queue from the list of open queues.
		queues.erase(queue);
		return EC_NOERROR;
	} else
		return EC_NOTOPEN;
}

bool ServiceThread::loggedIn()
{
	if (!logged_in)
		*client << RESPONSE_RESPONSE(EC_NOTLOGGEDIN) << flush;
	return logged_in;
}




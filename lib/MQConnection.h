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
#ifndef _MQCONNECTION_H_
#define _MQCONNECTION_H_

#ifdef _WIN32
#pragma warning(disable: 4786)
#endif

#include "tcpsocket/socstream.h"
#include "safmq.h"
#include <string>
#include <vector>

namespace safmq {

/**
Represents a security profile for a user or user group in relation to a particular queue.
*/
struct QueuePermissions {
protected:
	QueuePermissions(const std::string& entity="", bool isgroup=false, bool read=false, bool write=false, bool destroy=false, bool change_security=false) {
		this->entity = entity;
		this->isgroup = isgroup;
		this->read = read;
		this->write = write;
		this->destroy = destroy;
		this->change_security = change_security;
	}
public:
	QueuePermissions(const QueuePermissions& src) {
		operator=(src);
	}
	QueuePermissions& operator=(const QueuePermissions& src) {
		this->entity = src.entity;
		this->isgroup = src.isgroup;
		this->read = src.read;
		this->write = src.write;
		this->destroy = src.destroy;
		this->change_security = src.change_security;
		return *this;
	}

public:
	/// The name of the user or user group
	std::string entity;
	/// Indicates this profile is for a group if false it is for a user
	bool	isgroup;
	/// Indicates the entity may read from the queue
	bool	read;
	/// Indicates the entity may write to the queue
	bool	write;
	/// Indicates the entity may delete the queue
	bool	destroy;
	/// Indicates the entity may alter the security of the queue
	bool	change_security;

friend class safmq::MQConnection;
};

/**
A class for connecting to a SAFMQ message queue server.
*/
class MQConnection  
{
public:
	typedef SAFMQ_UINT32 QueueHandle;
	typedef SAFMQ_UINT32 CursorHandle;

	struct QData {
		QData() {}
		QData(const QData& ent) {
			operator=(ent);
		}
		QData(const std::string& queuename, const std::string& owner) {
			this->queuename = queuename;
			this->owner = owner;
		}
		QData& operator=(const QData& ent) {
			this->queuename = ent.queuename;
			this->owner = ent.owner;
			return *this;
		}
		std::string	queuename;
		std::string	owner;
	};

	struct NameDesc {
		NameDesc(const std::string& nm="", const std::string& desc="") : name(nm.c_str()), description(desc.c_str()) {}
		NameDesc(const NameDesc& nd) : name(nd.name.c_str()), description(nd.description.c_str()) {}

		std::string name;
		std::string description;
	};

	typedef std::vector<QData>			QDATA_VECTOR;
	typedef std::vector<std::string>	NAME_VECTOR;
	typedef std::vector<NameDesc>		USER_VECTOR;
public:
	MQConnection(tcpsocket::socstream* server, const std::string& username, const std::string& password) throw (ErrorCode);
	virtual ~MQConnection();
	
	virtual ErrorCode OpenQueue(const std::string& queuename, MQConnection::QueueHandle& qhandle);
	virtual ErrorCode CloseQueue(MQConnection::QueueHandle qhandle);
	virtual ErrorCode GetQueueStatistics(MQConnection::QueueHandle qhandle, bool includeStorageBytes, bool includeMessageBytes, QueueStatistics& stats);
	virtual ErrorCode GetServerStatistics(QueueStatistics& stats);
	virtual ErrorCode GetClientInfo(std::list<ClientInfo>& clients);
	
	virtual ErrorCode EnqueueWithRelay(const std::string& destination_url, QueueMessage& msg);
		
	virtual ErrorCode Enqueue(MQConnection::QueueHandle qhandle, QueueMessage& msg);

	virtual ErrorCode Retrieve(MQConnection::QueueHandle qhandle, bool retrievebody, SAFMQ_INT32 timeoutseconds, QueueMessage& msg);
	virtual ErrorCode RetrieveID(MQConnection::QueueHandle qhandle, bool retrievebody, uuid& id, SAFMQ_INT32 timeoutseconds, QueueMessage& msg);
	virtual ErrorCode RetrieveCursor(MQConnection::QueueHandle qhandle, bool retrievebody, MQConnection::CursorHandle cursorID, QueueMessage& msg);

	virtual ErrorCode PeekFront(MQConnection::QueueHandle qhandle, bool retrievebody, SAFMQ_INT32 timeoutseconds, QueueMessage& msg);
	virtual ErrorCode PeekID(MQConnection::QueueHandle qhandle, bool retrievebody, uuid& id, SAFMQ_INT32 timeoutseconds, QueueMessage& msg);
	virtual ErrorCode PeekCursor(MQConnection::QueueHandle qhandle, bool retrievebody, MQConnection::CursorHandle cursorID, QueueMessage& msg);

	virtual ErrorCode OpenCursor(MQConnection::QueueHandle qhandle, MQConnection::CursorHandle& cursorID);
	virtual ErrorCode CloseCursor(MQConnection::QueueHandle qhandle, MQConnection::CursorHandle cursorID);
	virtual ErrorCode AdvanceCursor(MQConnection::QueueHandle qhandle, MQConnection::CursorHandle cursorID);
	virtual ErrorCode SeekID(MQConnection::QueueHandle qhandle, uuid& id, SAFMQ_INT32 timeoutseconds, MQConnection::CursorHandle cursorID);
	virtual ErrorCode TestCursor(MQConnection::QueueHandle qhandle, MQConnection::CursorHandle cursorID);

	virtual ErrorCode BeginTransaction();
	virtual ErrorCode CommitTransaction();
	virtual ErrorCode RollbackTransaction();
	virtual ErrorCode EndTransaction();


	virtual ErrorCode EnumerateQueues(QDATA_VECTOR& qnames);
	virtual ErrorCode EnumerateUsers(USER_VECTOR& users);
	virtual ErrorCode EnumerateGroups(NAME_VECTOR& groups);
	
	virtual ErrorCode CreateUser(const std::string& username, const std::string& password, const std::string& description);
	virtual ErrorCode DeleteUser(const std::string& username);
	virtual ErrorCode UserSetPermissions(const std::string& username, bool modifyqueues, bool modifyusers, bool modifygroups);
	virtual ErrorCode UserGetPermissions(const std::string& username, bool& modifyqueues, bool& modifyusers, bool& modifygroups);
	virtual ErrorCode SetPassword(const std::string& username, const std::string& password);

	virtual ErrorCode CreateGroup(const std::string& groupname);
	virtual ErrorCode DeleteGroup(const std::string& groupname);
	virtual ErrorCode GroupSetPermissions(const std::string& groupname, bool modifyqueues, bool modifyusers, bool modifygroups);
	virtual ErrorCode GroupGetPermissions(const std::string& groupname, bool& modifyqueues, bool& modifyusers, bool& modifygroups);

	virtual ErrorCode GroupAddUser(const std::string& groupname, const std::string& username);
	virtual ErrorCode GroupDeleteUser(const std::string& groupname, const std::string& username);
	virtual ErrorCode GroupGetUsers(const std::string& groupname, NAME_VECTOR& users);
	virtual ErrorCode UserGetGroups(const std::string& username, NAME_VECTOR& groups);

	virtual ErrorCode CreateQueue(const std::string& queuename);
	virtual ErrorCode CreateTempQueue(std::string& tmpQueueName, MQConnection::QueueHandle& tmpQueue);
	virtual ErrorCode DeleteQueue(const std::string& queuename);
	virtual ErrorCode QueueSetUserPermission(const std::string& queuename, const std::string& username, bool rd, bool wr, bool destroy, bool changesecurity);
	virtual ErrorCode QueueDeleteUserPermission(const std::string& queuename, const std::string& username);
	virtual ErrorCode QueueSetGroupPermission(const std::string& queuename, const std::string& groupname, bool rd, bool wr, bool destroy, bool changesecurity);
	virtual ErrorCode QueueDeleteGroupPermission(const std::string& queuename, const std::string& groupname);
	virtual ErrorCode QueueEnumeratePermissions(const std::string& queuename, std::vector<QueuePermissions>& perms);

	virtual ErrorCode AddUserIdentity(const std::string& subjectDN, const std::string& issuerDN, const std::string& username);
	virtual ErrorCode RemoveUserIdentity(const std::string& subjectDN, const std::string& issuerDN);
	virtual ErrorCode EnumerateUserIdentities(const std::string& username, std::vector<safmq::X509Identity>& ids);


	virtual SAFMQ_INT32 GetServerMajorProtocolVersion();
	virtual SAFMQ_INT32 GetServerMinorProtocolVersion();

protected:
	ErrorCode getResponseCode();	
	ErrorCode ackRetrieve(MQConnection::QueueHandle qhandle, const QueueMessage& msg);
	ErrorCode ackRetrieveCursor(MQConnection::QueueHandle qhandle, const QueueMessage& msg);

protected:
	tcpsocket::socstream*	server;
	SAFMQ_INT32				server_major_protocol_version;
	SAFMQ_INT32				server_minor_protocol_version;
};

} // end of safmq namespace

#endif

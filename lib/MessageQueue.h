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
#ifndef _MESSAGEQUEUE_H_
#define _MESSAGEQUEUE_H_

#include "MQConnection.h"

namespace safmq {

class MQFactory;

/**
A class representing a message queue.  This class is intended to simplify
access to message queues by providing reading and writing functionality.
*/
class MessageQueue  
{
public:
	MessageQueue(MQConnection* con, const std::string& qname, bool free_con = false) throw (ErrorCode);
	virtual ~MessageQueue();

	virtual ErrorCode GetQueueStatistics(bool includeStorageBytes, bool includeMessageBytes, QueueStatistics& stats);

	virtual ErrorCode Enqueue(QueueMessage& msg);

	virtual ErrorCode Retrieve(bool retrievebody, SAFMQ_INT32 timeout, QueueMessage& msg);
	virtual ErrorCode RetrieveID(bool retrievebody, uuid& id, SAFMQ_INT32 timeout, QueueMessage& msg);
	virtual ErrorCode RetrieveCursor(bool retrievebody, MQConnection::CursorHandle cursorID, QueueMessage& msg);

	virtual ErrorCode PeekFront(bool retrievebody, SAFMQ_INT32 timeout, QueueMessage& msg);
	virtual ErrorCode PeekID(bool retrievebody, uuid& id, SAFMQ_INT32 timeout, QueueMessage& msg);
	virtual ErrorCode PeekCursor(bool retrievebody, MQConnection::CursorHandle cursorID, QueueMessage& msg);

	virtual ErrorCode OpenCursor(MQConnection::CursorHandle& cursorID);
	virtual ErrorCode CloseCursor(MQConnection::CursorHandle cursorID);
	virtual ErrorCode AdvanceCursor(MQConnection::CursorHandle cursorID);
	virtual ErrorCode SeekID(uuid& id, SAFMQ_INT32 timeout, MQConnection::CursorHandle cursorID);
	virtual ErrorCode TestCursor(MQConnection::CursorHandle cursorID);

	virtual MQConnection*	getConnection() { return con; }
protected: // behavior
	MQConnection*				con;
	MQConnection::QueueHandle	que;
	bool						free_con;

	friend class MQFactory;
};

} // endof safmq namespace

#endif

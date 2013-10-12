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
#include "MessageQueue.h"

using namespace safmq;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/**
Constructs the message queue from an already established connection.

@param con [in] A connection to a safmq message queue server
@param qname [in] The name of a message queue to be opened
@param free_con [in] Indicates the connection should be freed when the object is destroyed
@exception ErrorCode - Thrown incase of an error opening the queue
*/
MessageQueue::MessageQueue(MQConnection* con, const std::string& qname, bool free_con) throw (ErrorCode)
{
	this->free_con = free_con;
	this->con = con;
	ErrorCode err = con->OpenQueue(qname,que);
	if (err != EC_NOERROR) {
		if (free_con)
			delete con;
		throw err;
	}
}

/**
Closes the message queue and optionally frees the associated MQConnection.
*/
MessageQueue::~MessageQueue()
{
	con->CloseQueue(que);
	if (free_con)
		delete con;
}

/**
Retrieves performance statistics for the message queue.

@param includeStorageBytes [in] Requests storage bytes (on disk including purged messages) returned
@param includeMessageBytes [in] Requests message bytes, bytes in queue (excluding purged messages)
@param stats [out] Statistics about a queue
@return EC_NOERROR, EC_NOTOPEN, EC_NETWORKERROR
*/
ErrorCode MessageQueue::GetQueueStatistics(bool includeStorageBytes, bool includeMessageBytes, QueueStatistics& stats)
{
	return con->GetQueueStatistics(que, includeStorageBytes, includeMessageBytes, stats);
}

/**
Places the message <code>msg</code> on the queue specified by <code>qhandle</code>

@param msg [in/out] The message to be sent to the server, the message id and timestamp
					will be set by the server upon success
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_WRONGMESSAGETYPE
*/
ErrorCode MessageQueue::Enqueue(QueueMessage& msg)
{
	return con->Enqueue(que,msg);
}

/**
Retreives a message from the safmq server and removes it from the queue.

@param retreivebody [in] Flags the server to retreive the message body
@param timeoutseconds [in] The number of seconds to wait for a message to be published on the queue
							in the case the queue is empty.  -1 causes an indefinate wait.
@param msg [in] The message to be sent to the server, the message id and timestamp
					will be set by the server upon success
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_NOMOREMESSAGES, EC_TIMEDOUT
*/
ErrorCode MessageQueue::Retrieve(bool retrievebody, SAFMQ_INT32 timeout, QueueMessage& msg)
{
	return con->Retrieve(que,retrievebody,timeout,msg);
}

/**
Retreives a message from the safmq server with a recipt id equal to the passed id and removes it from the queue.

@param retreivebody [in] Flags the server to retreive the message body
@param id [in] The uuid recipt id of a message to search for in the message queue
@param timeoutseconds [in] The number of seconds to wait for a message to be published on the queue
							in the case the queue is empty.  -1 causes an indefinate wait.
@param msg [in] The message to be sent to the server, the message id and timestamp
					will be set by the server upon success
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_TIMEDOUT
*/
ErrorCode MessageQueue::RetrieveID(bool retrievebody, uuid& id, SAFMQ_INT32 timeout, QueueMessage& msg)
{
	return con->RetrieveID(que,retrievebody,id,timeout,msg);
}

/**
Retreives a message from the safmq server located by the passed cursor to the passed id and removes it from the queue.

@param qhandle [in] A handle to a previously opened queue
@param retreivebody [in] Flags the server to retreive the message body
@param cursorID [in] The id of a queue cursor point to the message to be retreived
@param msg [in] The message to be sent to the server, the message id and timestamp
					will be set by the server upon success
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTOPEN, EC_INVALIDCURSOR
*/
ErrorCode MessageQueue::RetrieveCursor(bool retrievebody, MQConnection::CursorHandle cursorID, QueueMessage& msg)
{
	return con->RetrieveCursor(que,retrievebody,cursorID,msg);
}

/**
Retreives a message from the safmq server and leaves it in the queue.

@param retreivebody [in] Flags the server to retreive the message body
@param timeoutseconds [in] The number of seconds to wait for a message to be published on the queue
							in the case the queue is empty.  -1 causes an indefinate wait.
@param msg [in] The message to be sent to the server, the message id and timestamp
					will be set by the server upon success
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_NOMOREMESSAGES, EC_TIMEDOUT
*/
ErrorCode MessageQueue::PeekFront(bool retrievebody, SAFMQ_INT32 timeout, QueueMessage& msg)
{
	return con->PeekFront(que,retrievebody, timeout, msg);
}

/**
Retreives a message from the safmq server with a recipt id equal to the passed id and leaves it in the queue.

@param retreivebody [in] Flags the server to retreive the message body
@param id [in] The uuid recipt id of a message to search for in the message queue
@param timeoutseconds [in] The number of seconds to wait for a message to be published on the queue
							in the case the queue is empty.  -1 causes an indefinate wait.
@param msg [in] The message to be sent to the server, the message id and timestamp
					will be set by the server upon success
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_TIMEDOUT
*/
ErrorCode MessageQueue::PeekID(bool retrievebody, uuid& id, SAFMQ_INT32 timeout, QueueMessage& msg)
{
	return con->PeekID(que,retrievebody,id,timeout,msg);
}

/**
Retreives a message from the safmq server located by the passed cursor to the passed id and leaves it in the queue.

@param retreivebody [in] Flags the server to retreive the message body
@param cursorID [in] The id of a queue cursor point to the message to be retreived
@param msg [in] The message to be sent to the server, the message id and timestamp
					will be set by the server upon success
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTOPEN, EC_INVALIDCURSOR
*/
ErrorCode MessageQueue::PeekCursor(bool retrievebody, MQConnection::CursorHandle cursorID, QueueMessage& msg)
{
	return con->PeekCursor(que,retrievebody,cursorID,msg);
}

/**
Opens a message cursor for iterating through a message queue.  The cursor is positioned
initially at the first message in the queue.

@param cursorID [out] Receives the id of the new cursor
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_NOTOPEN
*/
ErrorCode MessageQueue::OpenCursor(MQConnection::CursorHandle& cursorID)
{
	return con->OpenCursor(que,cursorID);
}

/**
Closes a previously opened message cursor.

@param cursorID [in] The id of the cursor to close
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTOPEN
*/
ErrorCode MessageQueue::CloseCursor(MQConnection::CursorHandle cursorID)
{
	return con->CloseCursor(que,cursorID);
}

/**
Advances the cursor to the next message in the queue.

@param cursorID [in] The id of the cursor to advance
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTOPEN, EC_NOMOREMESSAGES
*/
ErrorCode MessageQueue::AdvanceCursor(MQConnection::CursorHandle cursorID)
{
	return con->AdvanceCursor(que,cursorID);
}

/**
Locates a message from the safmq server with a recipt id equal to the passed id and points the cursor at it.

@param id [in] The uuid recipt id of a message to search for in the message queue
@param timeoutseconds [in] The number of seconds to wait for a message to be published on the queue
							in the case the queue is empty.  -1 causes an indefinate wait.
@param cursorID [in] The id of the cursor to advance
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_TIMEDOUT
*/
ErrorCode MessageQueue::SeekID(uuid& id, SAFMQ_INT32 timeout, MQConnection::CursorHandle cursorID)
{
	return con->SeekID(que,id,timeout,cursorID);
}

/**
Determines if a cursor is valid
@param cursorID [in] A previously opened cursor
@return EC_NETWORKERROR, EC_NOTOPEN, EC_INVALIDCURSOR
*/
ErrorCode MessageQueue::TestCursor(MQConnection::CursorHandle cursorID)
{
	return con->TestCursor(que,cursorID);
}

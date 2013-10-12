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
#include "MQConnection.h"
#include "comdefs.h"
#include "client_operators.h"
#include "url/urldecode.h"

using namespace std;
using namespace safmq;

#define FORWARD_QUEUE_NAME "_FORWARD_"
#define WNCPY(d,s,dl,sl) std::char_traits<char>::copy(d, s, std::min((size_t)dl,sl))

//////////////////////////////////////////////////////////////////////
// Conconst struction/Deconst struction
//////////////////////////////////////////////////////////////////////

/**
Constructs a construction object and establishes a connection to a safmq server.

@param server [in] A stream connected to the server
@param username The user logging into this server
@param password The password assoicated with the user logging into this server
*/
MQConnection::MQConnection(tcpsocket::socstream* server, const std::string& username, const std::string& password) throw (ErrorCode)
{
	this->server = server;

	LOGIN_PARAMS	params;

	memset(&params,0,sizeof(params));
	WNCPY(params.username, username.c_str(), SAFMQ_USER_NAME_LENGTH, username.length());
	WNCPY(params.password, password.c_str(), SAFMQ_PASSWORD_LENGTH, password.length());

	server->put((char)LOGIN) << params << flush;
	ErrorCode err = getResponseCode();
	if (err != EC_NOERROR && err != EC_NOTIMPLEMENTED) {
		delete server;
		throw err;
	} else if (err == EC_NOERROR) {
		LOGIN_RESPONSE resp;
		*server >> resp;
		if (server->good()) {
			server_major_protocol_version = resp.major_version;
			server_minor_protocol_version = resp.minor_version;
			if (server_major_protocol_version < SAFMQ_PROTOCOL_MAJOR_VERSION) {
				throw EC_UNSUPPORTED_PROTOCOL;
			}
		} else
			throw EC_SERVERUNAVAIL;
	}
}

/**
Closes the connection to the server and deallocates resourceses associated with
this MQConnection.  Destroying the connection also closes all associated queues
and cursors.<br>

<p>Additionally destroying the connection rollsback and ends any pending
transactions.  Thus if the following occurs:<br><br>
<pre>
MQConnection* con  = new MQConnection("safmq://myserver","my name","my password");
MessageQueue* que = new MessageQueue(con,"my queue");
QueueMessage	msg;

con->BeginTransaction();
que->Retrieve(true,-1,msg);
delete queue;
delete con;
</pre>
<br>

The net effect of the snippet is to retrieve the first message from the queue,
then roll back the transaction as if <code>con->RollbackTransaction()</code> was
called, then close the queue and connection to the server.
*/
MQConnection::~MQConnection()
{
	server->close();
	delete server;
}

SAFMQ_INT32 MQConnection::GetServerMajorProtocolVersion()
{
	return server_major_protocol_version;
}

SAFMQ_INT32 MQConnection::GetServerMinorProtocolVersion()
{
	return server_minor_protocol_version;
}


ErrorCode MQConnection::getResponseCode()
{
	SAFMQ_INT32 response_code;
	server->read((char*)&response_code,sizeof(response_code));
	if (server->good())
		return (ErrorCode)ntohl(response_code);
	return EC_NETWORKERROR;
}

ErrorCode MQConnection::ackRetrieve(MQConnection::QueueHandle qhandle, const QueueMessage& msg)
{
	RETRIEVE_ACK_PARAMS	params;
	params.queueID = qhandle;
	params.msgID = msg.getMessageID();

	server->put((char)RETRIEVE_ACK) << params << flush;
	if (server->good())
		return getResponseCode();
	return EC_NETWORKERROR;
}

ErrorCode MQConnection::ackRetrieveCursor(MQConnection::QueueHandle qhandle, const QueueMessage& msg)
{
	RETRIEVE_ACK_PARAMS	params;
	params.queueID = qhandle;
	params.msgID = msg.getMessageID();

	server->put((char)RETRIEVE_CURSOR_ACK) << params << flush;
	if (server->good())
		return getResponseCode();
	return EC_NETWORKERROR;
}


/**
Opens a message queue for reading and writing
@param queuename [in] The name of the queue to open
@param qhandle [out] Receives the handle to the newly opened queue
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_DOESNOTEXIST, EC_FORWARDNOTALLOWED
*/
ErrorCode MQConnection::OpenQueue(const std::string& queuename, MQConnection::QueueHandle& qhandle)
{
	ErrorCode ret;
	QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	params;

	memset(&params,0,sizeof(params));
	WNCPY(params.queuename, queuename.c_str(), SAFMQ_QNAME_LENGTH, queuename.length());
	server->put((char)QUEUE_OPEN) << params << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			server->read((char*)&qhandle, sizeof(qhandle));
			if (!server->good())
				ret = EC_NETWORKERROR;
		}
	} else
		ret = EC_NETWORKERROR;
	return ret;
}

/**
Closes an already opened queue handle.  Closing a queue also closes
all associated cursors.

@param qhandle [in] The queue handle to close
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTOPEN
*/
ErrorCode MQConnection::CloseQueue(MQConnection::QueueHandle qhandle)
{
	ErrorCode ret = EC_NOERROR;
	QUEUE_CLOSE_OPEN_CURSOR_PARAMS	param;
	param.queueID = qhandle;
	server->put((char)QUEUE_CLOSE) << param << flush;
	if (server->good()) {
		ret = getResponseCode();
	} else
		ret = EC_NETWORKERROR;

	return ret;
}

/**
Retrieves performance statistics for a specific queue.

@param qhandle [in] A handle to an open queue
@param includeStorageBytes [in] Requests storage bytes (on disk including purged messages) returned
@param includeMessageBytes [in] Requests message bytes, bytes in queue (excluding purged messages)
@param stats [out] Statistics about a queue
@return EC_NOERROR, EC_NOTOPEN, EC_NETWORKERROR
*/
ErrorCode MQConnection::GetQueueStatistics(MQConnection::QueueHandle qhandle, bool includeStorageBytes, bool includeMessageBytes, QueueStatistics& stats)
{
	QUEUE_STATS_RESPONSE resp;
	QUEUE_STATS_PARAMS	params;

	params.queueID = qhandle;
	params.includeStorageBytes = includeStorageBytes;
	params.includeMessageBytes = includeMessageBytes;

	server->put((char)QUEUE_STATS) << params << flush;
	if (server->good()) {
		(*server) >> resp;
		if (resp.errorcode == EC_NOERROR)
			stats = resp.stats;
	}
	return (ErrorCode)resp.errorcode;
}

/**
Retrieves performance statistics for an entire server.

@param stats [out] Statistics about a queue
@return EC_NOERROR, EC_NOTOPEN, EC_NETWORKERROR
*/
ErrorCode MQConnection::GetServerStatistics(QueueStatistics& stats)
{
	QUEUE_STATS_RESPONSE resp;

	server->put((char)SERVER_STATS) << flush;
	if (server->good()) {
		(*server) >> resp;
		if (resp.errorcode == EC_NOERROR)
			stats = resp.stats;
	}
	return (ErrorCode)resp.errorcode;
}

/**
Retreives information about connected clients.

@param clients[out] List of connected clients
@return EC_NOERROR, EC_NETWORKERROR
*/
ErrorCode MQConnection::GetClientInfo(std::list<ClientInfo>& clients)
{
	CLIENT_INFO_RESPONSE resp;
	server->put((char)CLIENT_INFO) << flush;
	resp.errorcode = EC_NETWORKERROR;
	if (server->good()) {
		(*server) >> resp;
		if (resp.errorcode == EC_NOERROR) {
			clients.clear();
			clients.assign(resp.clientInfo.begin(), resp.clientInfo.end());
		}
	}
	return (ErrorCode)resp.errorcode;
}

/**
Enqueues a message for message relay forwarding to another queue/queue server.  After
completion, the return code will be EC_NOERROR on success and the <code>msg</code>'s 
timestamp and message will have been set.  It is important to note if the resposne
queue name is not set, any errors from the final destination queue/queue server will
be lost.  It is suggested to use roundtripping unless error determination is not
required, otherwise errors generated by the final destination server such as:<br>
	<table border=0>
	<tr><td>EC_DOESNOTEXIST
	<tr><td>EC_NOTAUTHORIZED
	<tr><td>EC_WRONGMESSAGETYPE
	<tr><td>EC_NOTOPEN
	<tr><td>EC_FORWARDNOTALLOWED
	<tr><td>EC_DUPLICATEMSGID
	</table>
will not returned to the client.
 

@param url A safmq url in the format safmq://user:password@server:port/queuename -or- for ssl safmqs://user:password@server:port/queuename.<br>
			Note: the port specification is optional.
@param msg The message to be sent.

@return EC_NOERROR on success
	EC_INVALIDNAME incase of an invalid url <br>
	EC_NETWORKERROR<br>
	EC_NOTAUTHORIZED <br>
	EC_NOTOPEN <br>
	EC_WRONGMESSAGETYPE <br>
	EC_FORWARDNOTALLOWED
*/
ErrorCode MQConnection::EnqueueWithRelay(const std::string& destination_url, QueueMessage& msg)
{
	ErrorCode ret;
	try {
		URL::urldecode	url(destination_url);
		std::string	dest;

		QueueHandle	forward;
		ret = OpenQueue(FORWARD_QUEUE_NAME,forward);
		if (ret == EC_NOERROR) {
			if (url.getProtocol().length())
				dest = url.getProtocol() + ":";
			dest += "//";
			if (url.getUser().length()) {
				dest += url.getUser();
				if (url.getPassword().length())
					dest += ":" + url.getPassword();
				dest += "@";
			}
			dest += url.getDestination();
			if (url.getPort().length())
				dest += ":" + url.getPort();
			dest += url.getResource();

			msg.setLabel(dest+_WS("?label=")+msg.getLabel());
			ret = Enqueue(forward,msg);

			ErrorCode ec = CloseQueue(forward);
			if (ret == EC_NOERROR && ec != EC_NOERROR)
				ret = ec;
		} else if (ret == EC_DOESNOTEXIST) {
			ret = EC_FORWARDNOTALLOWED;
		}
	} catch (URL::urlexception e) {
		ret = EC_INVALIDNAME;
	}
	return ret;
}

/**
Places the message <code>msg</code> on the queue specified by <code>qhandle</code>

@param qhandle [in] A handle to a previously opened queue
@param msg [in/out] The message to be sent to the server, the message id and timestamp
					will be set by the server upon success
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_WRONGMESSAGETYPE
*/
ErrorCode MQConnection::Enqueue(MQConnection::QueueHandle qhandle, QueueMessage& msg)
{
	ErrorCode ret = EC_NETWORKERROR;
	ENQUEUE_PARAMS	params;

	msg.msg.BodySize = msg.getBufferSize();
	params.queueID = qhandle;
	params.msg = &msg;

	server->put((char)ENQUEUE) << params << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			ENQUEUE_RESPONSE_DATA	data;
			*server >> data;
			if (server->good()) {
				msg.msg.MessageID = data.msgID;
				msg.msg.TimeStamp = data.timestamp;
			} else {
				ret = EC_NETWORKERROR;
			}
		}
	}
	return ret;
}

/**
Retreives a message from the safmq server and removes it from the queue.

@param qhandle [in] A handle to a previously opened queue
@param retreivebody [in] Flags the server to retreive the message body
@param timeoutseconds [in] The number of seconds to wait for a message to be published on the queue
							in the case the queue is empty.  -1 causes an indefinate wait.
@param msg [in] The message to be sent to the server, the message id and timestamp
					will be set by the server upon success
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_NOMOREMESSAGES, EC_TIMEDOUT
*/
ErrorCode MQConnection::Retrieve(MQConnection::QueueHandle qhandle, bool retrievebody, SAFMQ_INT32 timeoutseconds, QueueMessage& msg)
{
	ErrorCode	ret = EC_NOERROR;

	RETRIEVE_PEEK_FRONT_PARAMS	param;
	param.queueID = qhandle;
	param.retrievebody = retrievebody;
	param.timeoutseconds = timeoutseconds;

	server->put((char)RETRIEVE) << param << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			*server >> msg.msg;
			if (server->good()) {
				if (retrievebody) {
					bufstream *buf = msg.getBufferStream();
					buf->reserve(msg.msg.BodySize);
					buf->seekg(0);
					server->read(buf->buffer(), msg.msg.BodySize);
					if (server->good()) {
						buf->seekp(msg.msg.BodySize);
					} else {
						buf->seekp(0);
						ret = EC_NETWORKERROR;
					}
				}
				if (ret == EC_NOERROR)
					ret = ackRetrieve(qhandle,msg);
			} else
				ret = EC_NETWORKERROR;
		}
	} else
		ret = EC_NETWORKERROR;
	return ret;
}

/**
Retreives a message from the safmq server with a recipt id equal to the passed id and removes it from the queue.

@param qhandle [in] A handle to a previously opened queue
@param retreivebody [in] Flags the server to retreive the message body
@param id [in] The uuid recipt id of a message to search for in the message queue
@param timeoutseconds [in] The number of seconds to wait for a message to be published on the queue
							in the case the queue is empty.  -1 causes an indefinate wait.
@param msg [in] The message to be sent to the server, the message id and timestamp
					will be set by the server upon success
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_TIMEDOUT
*/
ErrorCode MQConnection::RetrieveID(MQConnection::QueueHandle qhandle, bool retrievebody, uuid& id, SAFMQ_INT32 timeoutseconds, QueueMessage& msg)
{
	ErrorCode	ret = EC_NOERROR;

	RETRIEVE_ID_PEEK_ID_PARAMS	param;
	param.queueID = qhandle;
	param.retrievebody = retrievebody;
	param.reciptID = id;
	param.timeoutseconds = timeoutseconds;

	server->put((char)RETRIEVE_ID) << param << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			*server >> msg.msg;
			if (server->good()) {
				if (retrievebody) {
					bufstream *buf = msg.getBufferStream();
					buf->reserve(msg.msg.BodySize);
					buf->seekg(0);
					server->read(buf->buffer(), msg.msg.BodySize);
					if (server->good()) {
						buf->seekp(msg.msg.BodySize);
					} else {
						buf->seekp(0);
						ret = EC_NETWORKERROR;
					}
				}
				if (ret == EC_NOERROR)
					ret = ackRetrieve(qhandle,msg);
			} else
				ret = EC_NETWORKERROR;
		}
	} else
		ret = EC_NETWORKERROR;
	return ret;
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
ErrorCode MQConnection::RetrieveCursor(MQConnection::QueueHandle qhandle, bool retrievebody, MQConnection::CursorHandle cursorID, QueueMessage& msg)
{
	ErrorCode	ret = EC_NOERROR;

	PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS	param;
	param.queueID = qhandle;
	param.retrievebody = retrievebody;
	param.cursorID = cursorID;

	server->put((char)RETRIEVE_CURSOR) << param << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			*server >> msg.msg;
			if (server->good()) {
				if (retrievebody) {
					bufstream *buf = msg.getBufferStream();
					buf->reserve(msg.msg.BodySize);
					buf->seekg(0);
					server->read(buf->buffer(), msg.msg.BodySize);
					if (server->good()) {
						buf->seekp(msg.msg.BodySize);
					} else {
						buf->seekp(0);
						ret = EC_NETWORKERROR;
					}
				}
				if (ret == EC_NOERROR)
					ret = ackRetrieveCursor(qhandle,msg);
			} else
				ret = EC_NETWORKERROR;
		}
	} else
		ret = EC_NETWORKERROR;
	return ret;
}

/**
Retreives a message from the safmq server and leaves it in the queue.

@param qhandle [in] A handle to a previously opened queue
@param retreivebody [in] Flags the server to retreive the message body
@param timeoutseconds [in] The number of seconds to wait for a message to be published on the queue
							in the case the queue is empty.  -1 causes an indefinate wait.
@param msg [in] The message to be sent to the server, the message id and timestamp
					will be set by the server upon success
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_NOMOREMESSAGES, EC_TIMEDOUT
*/
ErrorCode MQConnection::PeekFront(MQConnection::QueueHandle qhandle, bool retrievebody, SAFMQ_INT32 timeoutseconds, QueueMessage& msg)
{
	ErrorCode	ret = EC_NOERROR;

	RETRIEVE_PEEK_FRONT_PARAMS	param;
	param.queueID = qhandle;
	param.retrievebody = retrievebody;
	param.timeoutseconds = timeoutseconds;

	server->put((char)PEEK_FRONT) << param << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			*server >> msg.msg;
			if (server->good()) {
				if (retrievebody) {
					bufstream *buf = msg.getBufferStream();
					buf->reserve(msg.msg.BodySize);
					buf->seekg(0);
					server->read(buf->buffer(), msg.msg.BodySize);
					if (server->good()) {
						buf->seekp(msg.msg.BodySize);
					} else {
						buf->seekp(0);
						ret = EC_NETWORKERROR;
					}
				}
			} else
				ret = EC_NETWORKERROR;
		}
	} else
		ret = EC_NETWORKERROR;
	return ret;
}

/**
Retreives a message from the safmq server with a recipt id equal to the passed id and leaves it in the queue.

@param qhandle [in] A handle to a previously opened queue
@param retreivebody [in] Flags the server to retreive the message body
@param id [in] The uuid recipt id of a message to search for in the message queue
@param timeoutseconds [in] The number of seconds to wait for a message to be published on the queue
							in the case the queue is empty.  -1 causes an indefinate wait.
@param msg [in] The message to be sent to the server, the message id and timestamp
					will be set by the server upon success
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_TIMEDOUT
*/
ErrorCode MQConnection::PeekID(MQConnection::QueueHandle qhandle, bool retrievebody, uuid& id, SAFMQ_INT32 timeoutseconds, QueueMessage& msg)
{
	ErrorCode	ret = EC_NOERROR;

	RETRIEVE_ID_PEEK_ID_PARAMS	param;
	param.queueID = qhandle;
	param.retrievebody = retrievebody;
	param.reciptID = id;
	param.timeoutseconds = timeoutseconds;

	server->put((char)PEEK_ID) << param << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			*server >> msg.msg;
			if (server->good()) {
				if (retrievebody) {
					bufstream *buf = msg.getBufferStream();
					buf->reserve(msg.msg.BodySize);
					buf->seekg(0);
					server->read(buf->buffer(), msg.msg.BodySize);
					if (server->good()) {
						buf->seekp(msg.msg.BodySize);
					} else {
						buf->seekp(0);
						ret = EC_NETWORKERROR;
					}
				}
			} else
				ret = EC_NETWORKERROR;
		}
	} else
		ret = EC_NETWORKERROR;
	return ret;
}

/**
Retreives a message from the safmq server located by the passed cursor to the passed id and leaves it in the queue.

@param qhandle [in] A handle to a previously opened queue
@param retreivebody [in] Flags the server to retreive the message body
@param cursorID [in] The id of a queue cursor point to the message to be retreived
@param msg [in] The message to be sent to the server, the message id and timestamp
					will be set by the server upon success
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTOPEN, EC_INVALIDCURSOR
*/
ErrorCode MQConnection::PeekCursor(MQConnection::QueueHandle qhandle, bool retrievebody, MQConnection::CursorHandle cursorID, QueueMessage& msg)
{
	ErrorCode	ret = EC_NOERROR;

	PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS	param;
	param.queueID = qhandle;
	param.retrievebody = retrievebody;
	param.cursorID = cursorID;

	server->put((char)PEEK_CURSOR) << param << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			*server >> msg.msg;
			if (server->good()) {
				if (retrievebody) {
					bufstream *buf = msg.getBufferStream();
					buf->reserve(msg.msg.BodySize);
					buf->seekg(0);
					server->read(buf->buffer(), msg.msg.BodySize);
					if (server->good()) {
						buf->seekp(msg.msg.BodySize);
					} else {
						buf->seekp(0);
						ret = EC_NETWORKERROR;
					}
				}
			} else
				ret = EC_NETWORKERROR;
		}
	} else
		ret = EC_NETWORKERROR;
	return ret;
}

/**
Opens a message cursor for iterating through a message queue.  The cursor is positioned
initially at the first message in the queue.

@param qhandle [in] A handle to a previously opened queue
@param cursorID [out] Receives the id of the new cursor
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_NOTOPEN
*/
ErrorCode MQConnection::OpenCursor(MQConnection::QueueHandle qhandle, MQConnection::CursorHandle& cursorID)
{
	ErrorCode ret = EC_NOERROR;
	QUEUE_CLOSE_OPEN_CURSOR_PARAMS	param;
	param.queueID = qhandle;
	server->put((char)OPEN_CURSOR) << param << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			server->read((char*)&cursorID,sizeof(cursorID));
			if (!server->good())
				ret = EC_NETWORKERROR;
		}
	} else
		ret = EC_NETWORKERROR;

	return ret;
}

/**
Closes a previously opened message cursor.

@param qhandle [in] A handle to a previously opened queue
@param cursorID [in] The id of the cursor to close
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTOPEN
*/
ErrorCode MQConnection::CloseCursor(MQConnection::QueueHandle qhandle, MQConnection::CursorHandle cursorID)
{
	ErrorCode ret = EC_NOERROR;
	CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS	param;
	param.queueID = qhandle;
	param.cursorID = cursorID;

	server->put((char)CLOSE_CURSOR) << param << flush;
	if (server->good()) {
		ret = getResponseCode();
	} else
		ret = EC_NETWORKERROR;

	return ret;
}

/**
Advances the cursor to the next message in the queue.

@param qhandle [in] A handle to a previously opened queue
@param cursorID [in] The id of the cursor to advance
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTOPEN, EC_NOMOREMESSAGES
*/
ErrorCode MQConnection::AdvanceCursor(MQConnection::QueueHandle qhandle, MQConnection::CursorHandle cursorID)
{
	ErrorCode ret = EC_NOERROR;
	CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS	param;
	param.queueID = qhandle;
	param.cursorID = cursorID;

	server->put((char)ADVANCE_CURSOR) << param << flush;
	if (server->good()) {
		ret = getResponseCode();
	} else
		ret = EC_NETWORKERROR;

	return ret;
}


/**
Locates a message from the safmq server with a recipt id equal to the passed id and points the cursor at it.

@param qhandle [in] A handle to a previously opened queue
@param id [in] The uuid recipt id of a message to search for in the message queue
@param timeoutseconds [in] The number of seconds to wait for a message to be published on the queue
							in the case the queue is empty.  -1 causes an indefinate wait.
@param cursorID [in] The id of the cursor to advance
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_TIMEDOUT
*/
ErrorCode MQConnection::SeekID(MQConnection::QueueHandle qhandle, uuid& id, SAFMQ_INT32 timeoutseconds, MQConnection::CursorHandle cursorID)
{
	ErrorCode ret = EC_NOERROR;
	SEEK_ID_PARAMS	param;
	param.queueID = qhandle;
	param.cursorID = cursorID;
	param.reciptID = id;
	param.timeoutseconds = timeoutseconds;

	server->put((char)SEEK_ID) << param << flush;
	if (server->good()) {
		ret = getResponseCode();
	} else
		ret = EC_NETWORKERROR;

	return ret;
}

/**
Determines if a cursor is valid
@param qhandle [in] A handle to a previously opened queue
@param cursorID [in] A previously opened cursor
@return EC_NETWORKERROR, EC_NOTOPEN, EC_INVALIDCURSOR
*/
ErrorCode MQConnection::TestCursor(MQConnection::QueueHandle qhandle, MQConnection::CursorHandle cursorID)
{
	ErrorCode ret = EC_NOERROR;
	CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS	param;
	param.queueID = qhandle;
	param.cursorID = cursorID;

	server->put((char)TEST_CURSOR) << param << flush;
	if (server->good()) {
		ret = getResponseCode();
	} else
		ret = EC_NETWORKERROR;

	return ret;
}

/**
Begins a transaction on the SAFMQ server.  If a call to <code>CommitTransaction()</code>
or <code>EndTransaction()</code> is not made before destrying the connection,
the transaction is rolled back to the state before calling <code>BeginTransaction()</code>.


@return EC_NETWORKERROR, EC_NOERROR if successful, EC_ALREADYOPEN if a transaction is already open
@see MQConnection#EndTransaction() -- To close the transaction
@see MQConnection#CommitTransaction() -- To commit the transaction without closing it.
*/
ErrorCode MQConnection::BeginTransaction()
{
	SAFMQ_INT32 zero = 0;
	server->put((char)BEGIN_TRANSACTION).write((char*)&zero,sizeof(zero)) << flush;
	return getResponseCode();
}

/**
Commits a transaction on the SAFMQ server, but leaves the transaction open.  No
reading or writing from the queues is completed until the transaction is committed.
This command causes any pending transactions to be performed.

@return EC_NETWORKERROR,EC_NOERROR if successful
*/
ErrorCode MQConnection::CommitTransaction()
{
	SAFMQ_INT32 zero = 0;
	server->put((char)COMMIT_TRANSACTION).write((char*)&zero,sizeof(zero)) << flush;
	return getResponseCode();
}

/**
Rolls back a transaction on the SAFMQ server.  This command undoes any
uncommited transactions (calls to <code>Enqueue(), Retrieve(), RetrieveID(), </code>
or <code>RetrieveCursor()</code>).

@return EC_NETWORKERROR, EC_NOERROR
*/
ErrorCode MQConnection::RollbackTransaction()
{
	SAFMQ_INT32 zero = 0;
	server->put((char)ROLLBACK_TRANSACTION).write((char*)&zero,sizeof(zero)) << flush;
	return getResponseCode();
}

/**
Closes and commits a transaction on the SAMFQ server.  No reading or writing
from the queues is completed until the transaction is committedCauses any pending
transactions to be performed.

@return EC_NETWORKERROR, EC_NOERROR, EC_NOTOPEN
*/
ErrorCode MQConnection::EndTransaction()
{
	SAFMQ_INT32 zero = 0;
	server->put((char)END_TRANSACTION).write((char*)&zero,sizeof(zero)) << flush;
	return getResponseCode();
}


/**
List all queues on the safmq server.

@param qnames [out] Receives the list of queue names
@return EC_NOERROR, EC_NETWORKERROR
*/
ErrorCode MQConnection::EnumerateQueues(MQConnection::QDATA_VECTOR& qnames)
{
	ErrorCode	ret = EC_NETWORKERROR;
	
	qnames.clear();

	server->put((char)ENUM_QUEUES).flush();
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			SAFMQ_INT32	nqueues;
			server->read((char*)&nqueues,sizeof(nqueues));
			if (server->good()) {
				QUEUE_CONFIG_ENTRY	qent;
				nqueues = ntohl(nqueues);
				for(SAFMQ_INT32 x=0;server->good() && x < nqueues; x++) {
					*server >> qent;
					if (server->good()) {
						qnames.push_back(QData(	std::string(qent.queuename, SAFMQ_QNAME_LENGTH).c_str(), 
												std::string(qent.owner,SAFMQ_USER_NAME_LENGTH).c_str()));
					}
				}
				if (!server->good())
					ret = EC_NETWORKERROR;
			} else
				ret = EC_NETWORKERROR;
		}
	}
	return ret;
}

/**
Lists all users on the safmq server.

@param users [out] The list of user ids and logins
@return EC_NOERROR, EC_NETWORKERROR
*/
ErrorCode MQConnection::EnumerateUsers(MQConnection::USER_VECTOR& users)
{
	ErrorCode	ret = EC_NETWORKERROR;
	
	users.clear();
	server->put((char)ENUM_USERS).flush();
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			SAFMQ_INT32	nUsers;
			server->read((char*)&nUsers,sizeof(nUsers));
			if (server->good()) {
				char username[SAFMQ_USER_NAME_LENGTH];
				char desc[SAFMQ_DESCRIPTION_LENGTH];

				nUsers = ntohl(nUsers);
				for(SAFMQ_INT32 x=0;server->good() && x < nUsers; x++) {
					server->read((char*)username,sizeof(username));
					if (server->good()) {
						server->read((char*)desc,sizeof(desc));
						if (server->good()) {
							users.push_back(NameDesc(std::string(username,SAFMQ_USER_NAME_LENGTH).c_str(), std::string(desc,SAFMQ_DESCRIPTION_LENGTH).c_str()));
						}
					}
				}
			} 
			if (!server->good())
				ret = EC_NETWORKERROR;
		}
		
	}
	return ret;
}

/**
Lists all groups on the safmq server.

@param groups [out] The list of group ids
@return EC_NOERROR, EC_NETWORKERROR
*/
ErrorCode MQConnection::EnumerateGroups(MQConnection::NAME_VECTOR& groups)
{
	ErrorCode	ret = EC_NETWORKERROR;
	
	groups.clear();
	server->put((char)ENUM_GROUPS).flush();
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			SAFMQ_INT32	nGroups;
			server->read((char*)&nGroups,sizeof(nGroups));
			if (server->good()) {
				char	groupname[SAFMQ_GROUP_NAME_LENGTH];

				nGroups = ntohl(nGroups);
				for(SAFMQ_INT32 x=0;server->good() && x < nGroups; x++) {
					server->read((char*)groupname,sizeof(groupname));
					if (server->good())
						groups.push_back(std::string(groupname,SAFMQ_GROUP_NAME_LENGTH).c_str());
				}
			} 
			if (!server->good())
				ret = EC_NETWORKERROR;
		}
		
	}
	return ret;
}

/**
Creates a user account on the safmq server.

@param username [in] The user id of the user account
@param password [in] The account's password
@param description [in] The description of the account
@return EC_NOERROR, EC_NETWORKERROR, EC_ALREADYDEFINED, EC_NOTAUTHORIZED
*/
ErrorCode MQConnection::CreateUser(const std::string& username, const std::string& password, const std::string& description)
{
	USER_CREATE_PARAMS	params;
	ErrorCode			ret = EC_NETWORKERROR;

	memset(&params,0,sizeof(params));

	WNCPY(params.username,username.c_str(),SAFMQ_USER_NAME_LENGTH,username.length());
	WNCPY(params.password,password.c_str(),SAFMQ_PASSWORD_LENGTH,password.length());
	WNCPY(params.description,description.c_str(),NUMELEM(params.description),description.length());

	server->put((char)USER_CREATE) << params << flush;
	if (server->good())
		ret = getResponseCode();
	return ret;
}
/**
Removes a user account.

@param username [in] The user id of the user account
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_DOESNOTEXIST
*/
ErrorCode MQConnection::DeleteUser(const std::string& username)
{
	USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS	params;
	ErrorCode							ret = EC_NETWORKERROR;

	memset(&params,0,sizeof(params));
	WNCPY(params.username,username.c_str(),SAFMQ_USER_NAME_LENGTH,username.length());
	server->put((char)USER_DELETE) << params << flush;
	if (server->good())
		ret = getResponseCode();
	return ret;
}

/**
Sets user access permissions for an account

@param username [in] The user id to modify permissions
@param modifyqueues [in] Allows the user to add and remove queues
@param modifyusers [in] Allows the user to add and remove users
@param modifygroups [in] Allows the suer to add and remove groups

@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_DOESNOTEXIST
*/
ErrorCode MQConnection::UserSetPermissions(const std::string& username, bool modifyqueues, bool modifyusers, bool modifygroups)
{
	USER_SET_PERMS_PARAMS				params;
	ErrorCode							ret = EC_NETWORKERROR;
	
	memset(&params,0,sizeof(params));
	WNCPY(params.username,username.c_str(),SAFMQ_USER_NAME_LENGTH,username.length());
	params.modify_queues = modifyqueues;
	params.modify_users = modifyusers;
	params.modify_groups = modifygroups;
	server->put((char)USER_SET_PERMS) << params << flush;
	if (server->good())
		ret = getResponseCode();
	return ret;
}

/**
Retreives a users user access permissions.
@param username [in] The user id of the acount
@param modifyqueues [out] Flags whether the user may add and remove queues
@param modifyusers [out] Flags whether the user may add and remove users
@param modifygroups [out] Flags whether the user may add and remove groups

@return EC_NOERROR, EC_NETWORKERROR, EC_DOESNOTEXIST
*/
ErrorCode MQConnection::UserGetPermissions(const std::string& username, bool& modifyqueues, bool& modifyusers, bool& modifygroups)
{
	USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS	params;
	ErrorCode							ret = EC_NETWORKERROR;

	memset(&params,0,sizeof(params));
	WNCPY(params.username,username.c_str(),SAFMQ_USER_NAME_LENGTH,username.length());
	server->put((char)USER_GET_PERMS) << params << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data	perms;
			*server >> perms;
			if (server->good()) {
				modifyqueues = perms.modify_queues != 0;
				modifyusers = perms.modify_users != 0;
				modifygroups = perms.modify_groups != 0;
			} else
				ret = EC_NETWORKERROR;
		}
	}
	return ret;
}

/**
Sets/Changes the password for a user account
@param username [in] The user id of the acount
@param password [in] The new password for the account

@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_DOESNOTEXIST
*/
ErrorCode MQConnection::SetPassword(const std::string& username, const std::string& password)
{
	ErrorCode	ret = EC_NETWORKERROR;
	USER_SET_PASSWORD_PARAMS	params;

	memset(&params,0,sizeof(params));
	WNCPY(params.username, username.c_str(), SAFMQ_USER_NAME_LENGTH, username.length());
	WNCPY(params.password, password.c_str(), SAFMQ_PASSWORD_LENGTH, password.length());

	server->put((char)USER_SET_PASSWORD) << params << flush;
	if (server->good())
		ret = getResponseCode();
	return ret;
}

/**
Creates a user group.  User groups can have rights like users,
allowing multiple users to have the same security profile.

@param groupname [in] The name of the group to create

@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_ALREADYDEFINED
*/
ErrorCode MQConnection::CreateGroup(const std::string& groupname)
{
	GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	params;
	ErrorCode															ret = EC_NETWORKERROR;

	memset(&params,0,sizeof(params));
	WNCPY(params.groupname,groupname.c_str(),SAFMQ_GROUP_NAME_LENGTH,groupname.length());
	server->put((char)GROUP_CREATE) << params << flush;
	if (server->good())
		ret = getResponseCode();
	return ret;
}

/**
Removes a user group.

@param groupname [in] The name of the group to remove
@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_DOESNOTEXIST
*/
ErrorCode MQConnection::DeleteGroup(const std::string& groupname)
{
	GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	params;
	ErrorCode															ret = EC_NETWORKERROR;

	memset(&params,0,sizeof(params));
	WNCPY(params.groupname,groupname.c_str(),SAFMQ_GROUP_NAME_LENGTH,groupname.length());
	server->put((char)GROUP_DELETE) << params << flush;
	if (server->good())
		ret = getResponseCode();
	return ret;
}

/**
Sets user group access permissions for a user group

@param groupname [in] The name of the user group
@param modifyqueues [in] Allows the group to add and remove queues
@param modifyusers [in] Allows the group to add and remove users
@param modifygroups [in] Allows the group to add and remove groups

@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED, EC_DOESNOTEXIST
*/
ErrorCode MQConnection::GroupSetPermissions(const std::string& groupname, bool modifyqueues, bool modifyusers, bool modifygroups)
{
	GROUP_SET_PERMS_PARAMS				params;
	ErrorCode							ret = EC_NETWORKERROR;

	memset(&params,0,sizeof(params));
	WNCPY(params.groupname,groupname.c_str(),SAFMQ_GROUP_NAME_LENGTH,groupname.length());
	params.modify_queues = modifyqueues;
	params.modify_users = modifyusers;
	params.modify_groups = modifygroups;
	server->put((char)GROUP_SET_PERMS) << params << flush;
	if (server->good())
		ret = getResponseCode();
	return ret;
}

/**
Retreives a user group's user access permissions.

@param groupname [in] The name of the user group
@param modifyqueues [out] Flags whether the user may add and remove queues
@param modifyusers [out] Flags whether the user may add and remove users
@param modifygroups [out] Flags whether the user may add and remove groups

@return EC_NOERROR, EC_NETWORKERROR, EC_DOESNOTEXIST
*/
ErrorCode MQConnection::GroupGetPermissions(const std::string& groupname, bool& modifyqueues, bool& modifyusers, bool& modifygroups)
{
	GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	params;
	ErrorCode															ret = EC_NETWORKERROR;

	memset(&params,0,sizeof(params));
	WNCPY(params.groupname,groupname.c_str(),SAFMQ_GROUP_NAME_LENGTH,groupname.length());
	server->put((char)GROUP_GET_PERMS) << params << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data	perms;
			*server >> perms;
			if (server->good()) {
				modifyqueues = perms.modify_queues != 0;
				modifyusers = perms.modify_users != 0;
				modifygroups = perms.modify_groups != 0;
			} else
				ret = EC_NETWORKERROR;
		}
	}
	return ret;
}

/**
Adds a user to a user group.

@param groupname [in] The name of the user group
@param username [in] The name of the user account
@return EC_NOERROR, EC_NETWORKERROR, EC_DOESNOTEXIST, EC_NOTAUTHORIZED
*/
ErrorCode MQConnection::GroupAddUser(const std::string& groupname, const std::string& username)
{
	GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS	params;
	ErrorCode								ret = EC_NETWORKERROR;

	memset(&params,0,sizeof(params));
	WNCPY(params.groupname,groupname.c_str(),SAFMQ_GROUP_NAME_LENGTH,groupname.length());
	WNCPY(params.username,username.c_str(),SAFMQ_USER_NAME_LENGTH,username.length());

	server->put((char)GROUP_ADD_USER) << params << flush;
	if (server->good())
		ret = getResponseCode();
	return ret;
}

/**
Removes a user from a user group.

@param groupname [in] The name of the user group
@param username [in] The name of the user account
@return EC_NOERROR, EC_NETWORKERROR, EC_DOESNOTEXIST, EC_NOTAUTHORIZED
*/
ErrorCode MQConnection::GroupDeleteUser(const std::string& groupname, const std::string& username)
{
	GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS	params;
	ErrorCode								ret = EC_NETWORKERROR;

	memset(&params,0,sizeof(params));
	WNCPY(params.groupname,groupname.c_str(),SAFMQ_GROUP_NAME_LENGTH,groupname.length());
	WNCPY(params.username,username.c_str(),SAFMQ_USER_NAME_LENGTH,username.length());

	server->put((char)GROUP_DELETE_USER) << params << flush;
	if (server->good())
		ret = getResponseCode();
	return ret;
}

/**
Retrieves a list of users in a user group

@param groupname [in] The name of the user group
@param users [out] The list of users id's in the group
@return EC_NOERROR, EC_NETWORKERROR, EC_DOESNOTEXIST
*/
ErrorCode MQConnection::GroupGetUsers(const std::string& groupname, MQConnection::NAME_VECTOR& users)
{
	GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	params;
	ErrorCode															ret = EC_NETWORKERROR;

	memset(&params,0,sizeof(params));
	WNCPY(params.groupname,groupname.c_str(),SAFMQ_GROUP_NAME_LENGTH,groupname.length());

	users.clear();

	server->put((char)GROUP_GET_USERS) << params << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			SAFMQ_INT32	nUsers;
			SAFMQ_INT32	x;
			char	username[SAFMQ_USER_NAME_LENGTH];

			server->read((char*)&nUsers, sizeof(nUsers));
			if (server->good()) {
				nUsers = ntohl(nUsers);
				for(x=0;server->good() && x<nUsers;x++) {
					server->read((char*)username, sizeof(username));
					if (server->good()) {
						ntohwstr(username,SAFMQ_USER_NAME_LENGTH);
						users.push_back(std::string(username,SAFMQ_USER_NAME_LENGTH).c_str());
					}
				}
			}
			if (!server->good())
				ret = EC_NETWORKERROR;
		}
	}
	return ret;
}

/**
Retrieves a list of user groups the user belongs to.

@param username [in] The user id
@param groups [in] The list of groups the user belongs to

@return EC_NOERROR, EC_NETWORKERROR, EC_DOESNOTEXIST
*/
ErrorCode MQConnection::UserGetGroups(const std::string& username, MQConnection::NAME_VECTOR& groups)
{
	USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS	params;
	ErrorCode											ret = EC_NETWORKERROR;
	memset(&params,0,sizeof(params));
	WNCPY(params.username, username.c_str(), SAFMQ_USER_NAME_LENGTH, username.length());
	groups.clear();

	server->put((char)USER_GET_GROUPS) << params << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			SAFMQ_INT32	nGroups;
			SAFMQ_INT32	x;
			char	groupname[SAFMQ_GROUP_NAME_LENGTH];

			server->read((char*)&nGroups,sizeof(nGroups));
			if (server->good()) {
				nGroups = ntohl(nGroups);
				for(x=0;server->good() && x<nGroups; x++) {
					server->read((char*)groupname,sizeof(groupname));
					if (server->good()) {
						ntohwstr(groupname, SAFMQ_GROUP_NAME_LENGTH);
						groups.push_back(std::string(groupname).c_str());
					}
				}
			}
			if (!server->good())
				ret = EC_NETWORKERROR;
		}
	}
	return ret;
}

/**
Creates a new message queue.

@param queuename [in] The name of the queue to create.

@return EC_NOERROR, EC_NETWORKERROR, EC_ALREADYDEFINED, EC_NOTAUTHORIZED
*/
ErrorCode MQConnection::CreateQueue(const std::string& queuename)
{
	ErrorCode ret = EC_NETWORKERROR;
	QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	params;

	memset(&params,0,sizeof(params));
	WNCPY(params.queuename,queuename.c_str(),SAFMQ_QNAME_LENGTH,queuename.length());

	server->put((char)QUEUE_CREATE) << params << flush;
	if (server->good()) {
		ret = getResponseCode();
	}
	return ret;
}


/**
Creates a new temporary message queue.  The queue is removed from the server
once all clients close their handles to the queue, explicitly or implicity
by disconnecting from the server.<br>

After calling <code>CreateTempQueue</code> the SAFMQ server has created a queue,
flagged as temporary, returns the server generated name of the queue, and a handle
to that queue.  Closing the queue removes it from the system, there is no need to 
explicitly delete it.  Using the Closing technique, it is possible that the queue
may remain open, because other clients have access to the queue.

@param tmpQueueName [out] Receives the name of the temporary queue, relative to the server,
							when supplying this queue name to other clients, the server URL
							must be prepended to the name.  i.e. If the url
							safmq://localhost was used to open the connection use
							<code>"safmq://localhost/" + tmpQueue</code> when supplying
							the queue name to other clients.

@param tmpQueue [out] Receives a handle to the open queue

@return EC_NOERROR, EC_NETWORKERROR, EC_ALREADYDEFINED, EC_NOTAUTHORIZED
*/
ErrorCode MQConnection::CreateTempQueue(std::string& tmpQueueName, MQConnection::QueueHandle& tmpQueue)
{
	QUEUE_CREATE_TEMP_RESPONSE resp;
	
	resp.errorcode = EC_NETWORKERROR; // flag as a network error because the first failure will be writing to the server
	
	server->put((char)QUEUE_CREATE_TEMP) << flush;

	if (server->good()) {
		*server >> resp;
		if (resp.errorcode == EC_NOERROR) {
			tmpQueueName = resp.queueName;
			tmpQueue = resp.queueID;
		}
	}
	return (ErrorCode)resp.errorcode;
}

/**
Removes a message queue.  The command will fail in the case the queue
is already opened by another user.

@param queuename [in] The name of the message queue
@return EC_NOERROR, EC_NETWORKERROR, EC_QUEUEOPEN, EC_NOTAUTHORIZED
*/
ErrorCode MQConnection::DeleteQueue(const std::string& queuename)
{
	ErrorCode ret = EC_NETWORKERROR;  // flag as a network error because the first failure will be writing to the server
	QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	params;

	memset(&params,0,sizeof(params));
	WNCPY(params.queuename,queuename.c_str(),SAFMQ_QNAME_LENGTH,queuename.length());

	server->put((char)QUEUE_DELETE) << params << flush;
	if (server->good()) {
		ret = getResponseCode();
	}

	return ret;
}

/**
Sets a user account's permissions for accessing a queue. With out permissions, the user is assumed
to have no access rights to the queue.

@param queuename [in] The name of the queue
@param username [in] The name of a user account
@param rd [in] Allows the user to read from the queue
@param wr [in] Allows the user to write tot he queue
@param destroy [in] Allows the user to delete the queue
@param changesecurity [in] Allows the user to reconfigure the queue's security

@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED
*/
ErrorCode MQConnection::QueueSetUserPermission(const std::string& queuename, const std::string& username, bool rd, bool wr, bool destroy, bool changesecurity)
{
	ErrorCode					ret = EC_NETWORKERROR;
	QUEUE_SET_USER_PERMS_PARAMS	params;

	memset(&params,0,sizeof(params));
	WNCPY(params.queuename, queuename.c_str(), SAFMQ_QNAME_LENGTH, queuename.length());
	WNCPY(params.username, username.c_str(), SAFMQ_USER_NAME_LENGTH, username.length());
	params.read = rd;
	params.write = wr;
	params.destroy = destroy;
	params.change_security = changesecurity;
	server->put((char)QUEUE_SET_USER_PERMS) << params << flush;
	if (server->good()) 
		ret = getResponseCode();
	return ret;
}

/**
Removes user account's permissions for accessing a queue.  With out permissions, the user is assumed
to have no access rights to the queue.

@param queuename [in] The name of the queue
@param username [in] The name of a user account

@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED
*/
ErrorCode MQConnection::QueueDeleteUserPermission(const std::string& queuename, const std::string& username)
{
	ErrorCode					ret = EC_NETWORKERROR;
	QUEUE_DEL_USER_PERMS_PARAM	params;

	memset(&params,0,sizeof(params));
	WNCPY(params.queuename, queuename.c_str(), SAFMQ_QNAME_LENGTH, queuename.length());
	WNCPY(params.username, username.c_str(), SAFMQ_USER_NAME_LENGTH, username.length());
	server->put((char)QUEUE_DEL_USER_PERMS) << params << flush;
	if (server->good()) 
		ret = getResponseCode();
	return ret;
}

/**
Sets a user groups's permissions for accessing a queue. With out permissions, the group is assumed
to have no access rights to the queue.

@param queuename [in] The name of the queue
@param groupname [in] The name of a user group
@param rd [in] Allows the group to read from the queue
@param wr [in] Allows the group to write tot he queue
@param destroy [in] Allows the group to delete the queue
@param changesecurity [in] Allows the group to reconfigure the queue's security

@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED
*/
ErrorCode MQConnection::QueueSetGroupPermission(const std::string& queuename, const std::string& groupname, bool rd, bool wr, bool destroy, bool changesecurity)
{
	ErrorCode					ret = EC_NETWORKERROR;
	QUEUE_SET_GROUP_PERMS_PARAMS	params;

	memset(&params,0,sizeof(params));
	WNCPY(params.queuename, queuename.c_str(), SAFMQ_QNAME_LENGTH, queuename.length());
	WNCPY(params.groupname, groupname.c_str(), SAFMQ_GROUP_NAME_LENGTH, groupname.length());
	params.read = rd;
	params.write = wr;
	params.destroy = destroy;
	params.change_security = changesecurity;
	server->put((char)QUEUE_SET_GROUP_PERMS) << params << flush;
	if (server->good()) 
		ret = getResponseCode();
	return ret;
}

/**
Removes user account's permissions for accessing a queue.  With out permissions, the group is assumed
to have no access rights to the queue.

@param queuename [in] The name of the queue
@param groupname [in] The name of a user group

@return EC_NOERROR, EC_NETWORKERROR, EC_NOTAUTHORIZED
*/
ErrorCode MQConnection::QueueDeleteGroupPermission(const std::string& queuename, const std::string& groupname)
{
	ErrorCode					ret = EC_NETWORKERROR;
	QUEUE_DEL_GROUP_PERMS_PARAM	params;

	memset(&params,0,sizeof(params));
	WNCPY(params.queuename, queuename.c_str(), SAFMQ_QNAME_LENGTH, queuename.length());
	WNCPY(params.groupname, groupname.c_str(), SAFMQ_GROUP_NAME_LENGTH, groupname.length());
	server->put((char)QUEUE_DEL_GROUP_PERMS) << params << flush;
	if (server->good()) 
		ret = getResponseCode();
	return ret;
}

/**
Retreives a list of permissions for a given queue.

@param queuename [in] The name of the queue
@param perms [out] Receives the list of permissions
@return EC_NOERROR, EC_NETWORKERROR
@see QueuePermissions
*/
ErrorCode MQConnection::QueueEnumeratePermissions(const std::string& queuename, std::vector<QueuePermissions>& perms)
{
	ErrorCode					ret = EC_NETWORKERROR;
	QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	params;

	memset(&params,0,sizeof(params));
	WNCPY(params.queuename, queuename.c_str(), SAFMQ_QNAME_LENGTH, queuename.length());

	perms.clear();

	server->put((char)QUEUE_ENUM_PERMS) << params << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			SAFMQ_INT32 nPerms;
			SAFMQ_INT32 x;

			server->read((char*)&nPerms, sizeof(nPerms));
			if (server->good()) {
				nPerms = ntohl(nPerms);

				QUEUE_PERM_DATA	data;
				for(x=0;server->good() && x<nPerms; x++) {
					*server >> data;
					if (server->good())
						perms.push_back(QueuePermissions(std::string(data.entityname, MAX_ENTITY_LENGTH).c_str(),data.isgroup!=0,data.read!=0,data.write!=0,data.destroy!=0,data.change_security!=0));
				}
				if (!server->good())
					ret = EC_NETWORKERROR;
			}
		}
	}
	return ret;
}

/**
Adds an X509 digitital certificate identity mapping.  This method should be used
to provide for a passwordless authentication using SSL and X509 digital certificates.

<p><b>Example:</b> (the sample X509 certificate shipped with SAFMQ source)</p>
<pre>
Subject: C=US, ST=Nebraska, L=Omaha, O=SAFMQ, CN=Matthew Battey/emailAddress=mattbattey@users.sourceforge.net
Issuer:  C=US, ST=Nebraska, L=Omaha, O=SAFMQ, CN=Matthew Battey/emailAddress=mattbattey@users.sourceforge.net
</pre>

Thawte's Issuer DN:
<pre>
Issuer: C=ZA, ST=Western Cape, L=Cape Town, O=Thawte Consulting cc, OU=Certification Services Division, CN=Thawte Premium Server CA/emailAddress=premium-server@thawte.com
</pre>

<p> In this case the issuer and subject are the same value, because the certificate
shipped with SAFMQ is a "self-signed" certificate.  Typically the Subject DN will be set
by you and the Issuer DN will be determined by the organization certifying the authenticity
of the certificate's origin.</p>

<p><b>Note</b>: The Java implentation produces distinguished name strings in reverse
order in comparison to the default output from tools like OpenSSL.  SAFMQ uses OpenSSL
so these must be in the order that OpenSSL would report the value pair order.</p>

@param subjectDN [in] the Subject Distinguished Name of the certificate
@param issuerDN [in] the Issuer Distinguished Name of the certificate
@param username [in] the target SAFMQ login id (username)
@return EC_NOERROR upon success, EC_DOESNOTEXIST if the user ID does not exist, EC_NOTAUTHORIZED, EC_NETWORKERROR
*/
ErrorCode MQConnection::AddUserIdentity(const std::string& subjectDN, const std::string& issuerDN, const std::string& username)
{
	ErrorCode	ret = EC_NETWORKERROR;
	ADD_USER_IDENTITY_PARAMS	params;
	
	params.subjectDN = subjectDN;
	params.issuerDN = issuerDN;
	params.username = username;

	server->put((char)USER_ADD_IDENT) << params << flush;
	if (server->good()) {
		ret = getResponseCode();
	} 

	return ret;
}

/**
Removes an X509 digitital certificate identity mapping.  This method should be used
to provide for a passwordless authentication using SSL and X509 digital certificates.

@param subjectDN [in] the Subject Distinguished Name of the certificate
@param issuerDN [in] the Issuer Distinguished Name of the certificate
@return EC_NOERROR upon success, EC_DOESNOTEXIST if the user ID does not exist, EC_NOTAUTHORIZED, EC_NETWORKERROR
*/
ErrorCode MQConnection::RemoveUserIdentity(const std::string& subjectDN, const std::string& issuerDN)
{
	ErrorCode	ret = EC_NETWORKERROR;
	REMOVE_USER_IDENTITY	params;
	
	params.subjectDN = subjectDN;
	params.issuerDN = issuerDN;

	server->put((char)USER_DEL_IDENT) << params << flush;
	if (server->good()) {
		ret = getResponseCode();
	} 

	return ret;
}

/**
Lists a set of X509 subject name and issuer names associated with the passed SAFMQ user login id.

@param username [in] The user name used to search for X509 identities.
@param ids [out] This vector of safmq::X509Identity objects is populated upon successful return
@return EC_NOERROR on success, EC_NETWORKERROR on network issue
*/
ErrorCode MQConnection::EnumerateUserIdentities(const std::string& username, std::vector<safmq::X509Identity>& ids)
{
	ErrorCode	ret = EC_NETWORKERROR;
	ENUM_USER_IDENTITY	params;

	params.username = username;

	server->put((char)USER_ENUM_IDENT) << params << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			*server >> ids;
			if (!server->good())
				ret = EC_NETWORKERROR;
		}
	} 

	return ret;
}

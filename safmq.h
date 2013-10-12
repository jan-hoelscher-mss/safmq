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
#ifndef _SAFMQ_H_
#define _SAFMQ_H_

#include <queue>
#include <string>
#include <memory.h>
#include <wchar.h>
#include "bufstream.h"
#include "safmq_defs.h"
#include "uuidgen/uuidgen.h"

/**
Namespace containing class to implement the SAFMQ message queue server and client library.
*/
namespace safmq {

class QStorage;
class ServiceThread;
class SystemConnection;
class EnqueueCmd;
class RetrieveCmd;
class RetrieveIDCmd;
class PeekIDCmd;
class PeekFrontCmd;
class PeekCursorCmd;
class RetrieveCursorCmd;

class MQConnection;

/// An enumeration of message classes
enum MessageClassEnum {
	/// System error message class
	MC_SYSTEMERRMSG = -1, 
	/// User message class
	MC_USERMSG = 0
};

/// An enumeration of message body types
enum BodyTypeEnum {
	/// Body Type containing a 4 byte integer
	BT_LONG,
	/// Body Type containing a 2 byte integer
	BT_SHORT,
	/// Body type containing a 1 byte integer
	BT_CHAR,
	/// Body type containging single byte text data
	BT_TEXT,
	/// Body type containg multi byte text data
	BT_WTEXT,
	/// Body type containging binary data
	BT_BINARY,
	/// Body type containg no data
	BT_NONE
};

/// An enumeration of message priorities
enum MessagePriorityEnum {
	/// Standard message priority
	MP_STANDARD = 0,
	/// Low message prority
	MP_LOW = 0,
	/// Medium-Low message prority
	MP_MEDIUMLOW=1,
	/// Medium message prority
	MP_MEDIUM=2,
	/// Medium-High message prority
	MP_MEDIUMHIGH=3,
	/// High message prority
	MP_HIGH=4,
	/// Highest message prority
	MP_HIGHEST=5
};

/// An enumeration of error codes
enum ErrorCode {
	/// No error has occured
	EC_NOERROR = 0,
	/// The operation timed out
	EC_TIMEDOUT,
	/// The user is not authorized to take the action
	EC_NOTAUTHORIZED,
	/// The resource is already closed
	EC_ALREADYCLOSED,
	/// The resource does not exist
	EC_DOESNOTEXIST,
	/// The resource has not been opened
	EC_NOTOPEN,
	/// A network error occured
	EC_NETWORKERROR,
	/// The Time to Live for a message has expired
	EC_TTLEXPIRED,
	/// The cursor has been invalidated
	EC_CURSORINVALIDATED,
	/// The message contains a duplicate message ID
	EC_DUPLICATEMSGID,
	/// The server is unavailable
	EC_SERVERUNAVAIL,
	/// No more messages remain
	EC_NOMOREMESSAGES,
	/// A file system error has occured
	EC_FILESYSTEMERROR,
	/// User cannot change the group
	EC_CANNOTCHANGEGROUP,
	/// User cannot change the user
	EC_CANNOTCHANGEUSER,
	/// The resource is already defined
	EC_ALREADYDEFINED,
	/// The operation is not implemented
	EC_NOTIMPLEMENTED,
	/// The queue is currently open by another connection
	EC_QUEUEOPEN,
	/// The user is not logged in
	EC_NOTLOGGEDIN,
	/// Invalid name for a resource
	EC_INVALIDNAME,
	/// System does not allow message forwarding (relaying)
	EC_FORWARDNOTALLOWED,
	/// Wronge type for message
	EC_WRONGMESSAGETYPE,
	/// Wrong Protocol Version
	EC_UNSUPPORTED_PROTOCOL,
	/// Transaction already open
	EC_ALREADYOPEN,

	/// Undefined error
	EC_ERROR = 10000,
};

static const uuid UUID_NULL = { 0 };

template<class _TYPE>
class CTYPE : public std::ctype<_TYPE> {
public:
	CTYPE() {}
	virtual ~CTYPE() {}
};

/**
Copies a single byte string to a wide string
@param dest [out] Destination string
@param src [in] Source String
@return the destination string
*/
inline std::string& a2wcopy(std::string& dest, const std::string& src) {
	dest.erase();
	CTYPE<std::string::traits_type::char_type>	t;
	for(std::string::const_iterator i = src.begin(); i!= src.end(); i++) 
		dest.append(1, t.widen(*i));
	return dest;
}

/**
Copies a multi byte string to a single byte string
@param dest [out] The destination single byte string
@param src [in] The source multi byte string
@return the destination string
*/
inline std::string& w2acopy(std::string& dest, const std::string& src) {
	dest.erase();
	CTYPE<std::string::traits_type::char_type>	t;
	for(std::string::const_iterator i = src.begin(); i!= src.end(); i++) 
		dest.append(1, t.narrow(*i,' '));
	return dest;
}


/*
Basic definition for data storage into 

@field MessageID			Unique Identifier for this message
@field MessageClass			Indicates System Message or App Message
@field MessagePriority		Priority of the message
@field Label				Label for this Message
@field TimeStamp			Creation Timestamp for this message
@field TimeToLiveSeconds	Number of seconds since "TimeStamp" before this message is auto-removed from the queue
@field TTLErrorWanted		Flag indicating the TTL timeout error is wanted by client
@field ResponseQueueName	Queue responders should respond into
@field ReciptID				ID This message is responding to
@field BodyType				Type of the body
@field BodySize				Length of the body (in bytes)
*/
#pragma pack(1)
struct QMessage {
	QMessage() : MessageID(UUID_NULL), MessageClass(MC_USERMSG), MessagePriority(MP_STANDARD), 
				TimeStamp(0), TimeToLiveSeconds(-1), TTLErrorWanted(false), 
				ReciptID(UUID_NULL), BodyType(BT_NONE), BodySize(-1)
	{
		memset(Label,0,sizeof(Label));
		memset(ResponseQueueName,0,sizeof(ResponseQueueName));
	}
	uuid		MessageID;
	char		MessageClass;
	char		MessagePriority;
	char		Label[SAFMQ_MSGLBL_LENGTH];
	SAFMQ_INT32	TimeStamp;
	SAFMQ_INT32	TimeToLiveSeconds;
	char		TTLErrorWanted;
	char		ResponseQueueName[SAFMQ_QNAME_LENGTH];
	uuid		ReciptID;
	char		BodyType;
	SAFMQ_INT32	BodySize;	
};
#pragma pack()

/**
A class representing a message in a message queue.  This class is used to
transport data to and from the message queue server by interacting with a
<code>safmq::MessageQueue</code> or <code>safmq::MQConnection</code>.<br>

<p>An example of how to use a queue message:</p>
<pre>

safmq::MessageQueue	*que = safmq::MQFactory::BuildQueueConnection("safmq://myserver/myqueue","myname","mypassword");
safmq::QueueMessage	msg;

msg.setLabel("My Label");
msg.setBodyType(safmq::BT_TEXT);

std::ostream*	o = msg.getBufferStream();
*o << "Hello World";

que->Enqueue(msg);

delete que;

</pre>
@see safmq::MessageQueue
@see safmq::MQConnection
*/
class QueueMessage
{
public:
	/**
	Constructs the queue message
	*/
	QueueMessage() { init(); }

	/**
	Provides the message's id, set after being enqueued on a message queue server
	*/
	uuid getMessageID() const								{ return msg.MessageID; }
	/**
	Provides the message's message class.
	*/
	MessageClassEnum getMessageClass() const				{ return (MessageClassEnum) msg.MessageClass; }
	/**
	Sets the message's priority
	@param priority [in] The priority of the message
	*/
	void setMessagePriority(MessagePriorityEnum priority)	{ msg.MessagePriority = priority; }
	/**
	Provies the message's priority
	@return The priority of the message
	*/
	MessagePriorityEnum getMessagePriority() const			{ return (MessagePriorityEnum)msg.MessagePriority; }
	/**
	Sets the label of the message
	@param label [in] The new label for the message
	*/
	void setLabel(const std::string& label)					{ strncpy(msg.Label, label.c_str(), SAFMQ_MSGLBL_LENGTH); msg.Label[SAFMQ_MSGLBL_LENGTH-1] = '\0'; }
	/**
	Gets the label of the message
	@returns the label of the message
	*/
	std::string getLabel() const							{ return msg.Label; }
	/**
	Provides the timestamp of the message
	@return The message's timestamp
	*/
	time_t getTimeStamp()									{ return msg.TimeStamp; }
	/**
	Sets the time to live for the message once it has been enqueued.
	@param sec [in] The number of seconds the message is to remain in a message queue
	*/
	void setTimeToLiveSeconds(SAFMQ_INT32 sec)						{ msg.TimeToLiveSeconds = sec; }
	/**
	Provides the time to live for the message once it has been enqueued.
	@param The number of seconds the message is to remain in a message queue
	*/
	SAFMQ_INT32 getTimeToLiveSeconds()	const						{ return msg.TimeToLiveSeconds; }
	/**
	Flags a message should be auto responded to incase of a TTL error
	@param ttlwanted [in] Indicates the message should be auto responded to in case of a TTL error
	*/
	void setTTLErrorWanted(bool ttlwanted)					{ msg.TTLErrorWanted = ttlwanted; }
	/**
	Provides the flag indicating a message should be auto responded to incase of a TTL error
	@return the flag indicating a message should be auto responded to incase of a TTL error
	*/
	bool getTTLErrorWanted()								{ return msg.TTLErrorWanted==1; }
	/**
	Sets the name of the queue which responses should be sent to.
	@param url [in] The URL of the response queue.
	*/
	void setResponseQueueName(const std::string& url)		{ strncpy(msg.ResponseQueueName, url.c_str(), SAFMQ_QNAME_LENGTH); msg.ResponseQueueName[SAFMQ_QNAME_LENGTH-1] = '\0'; }
	/**
	Provides the name of the queue which responses should be sent to.
	@return The URL of the response queue.
	*/
	void getResponseQueueName(std::string& rq) const		{ rq = msg.ResponseQueueName; }
	/**
	Sets the message's recipt id.  Messages may be searched for by their recipt id.
	@param id [in] The recipt id to be assigned to this message.
	*/
	void setReciptID(const uuid& id)						{ msg.ReciptID = id; }
	/**
	Provides the message's recipt id.  Messages may be searched for by their recipt id.
	@return recipt id assigned to this message.
	*/
	uuid getReciptID() const								{ return msg.ReciptID; }
	/**
	Sets a hint twords the type of data in this message's body.
	@param bt [in] A hint at the type of data in this message
	*/
	void setBodyType(BodyTypeEnum bt)						{ msg.BodyType = bt; }
	/**
	Provides the body type hint.
	@return the body type hind
	*/
	BodyTypeEnum getBodyType() const						{ return (BodyTypeEnum)msg.BodyType; }

	/**
	Provides stream access to the buffer for this message
	@return The message's buffer stream
	*/
	bufstream* getBufferStream()							{ return &msgbody; }
	/**
	Provides the message's buffer in constant form
	@return A const reference to the buffer.
	*/
	const bufstream* getBufferStream() const				{ return &msgbody; }
	/**
	Provides the nubmer of bytes in the buffer
	@return The size of the buffer
	*/
	size_t getBufferSize() const 							{ return msgbody.length(); }
	/**
	Provides the size of the message body
	@return The nubmer of bytes in the body
	*/
	size_t getBodySize() const								{ if (msg.BodySize == -1) return msgbody.length(); return msg.BodySize; }
	/**
	Resets the receive buffer so the message can be re-enqueued
	*/
	void resetReceivedBuffer() {
		if (msg.BodySize != -1) {
			msgbody.clear();
			msgbody.seekg(0);
			msgbody.seekp(msg.BodySize);
		}
	}

protected:
	void init() {
		isread = false;
	}

protected: // data
	QMessage	msg;
protected: // data
	bufstream	msgbody;

	bool		isread;
	SAFMQ_INT32	value;

	friend class QStorage;
	friend class ServiceThread;
	friend class MQConnection;
	friend class SystemConnection;
	friend class EnqueueCmd;
	friend class RetrieveCmd;
	friend class RetrieveIDCmd;
	friend class PeekIDCmd;
	friend class PeekFrontCmd;
	friend class PeekCursorCmd;
	friend class RetrieveCursorCmd;

	friend std::ostream& operator<<(std::ostream& o, const struct ENQUEUE_PARAMS& params);
	friend std::istream& operator>>(std::istream& i, struct ENQUEUE_PARAMS& params);
};

/**
Set of queue performance statistics.  When queried, all fields except
<code>storageBytes</code> and <code>messageBytes</code> are returned.
The byte count fields (<code>storageBytes</code> and <code>messageBytes</code>)
require sequential access to the queue and/or file system and thus
cause a performance penalty to acquire.
*/
struct QueueStatistics {
	/// Number of messages in a queue
	SAFMQ_UINT32 messageCount;
	/// Number of bytes on disk, includes purged messages still on disk
	SAFMQ_UINT32 storageBytes;
	/// Number of bytes in the queue, does not included purged messages
	SAFMQ_UINT32 messageBytes;

	/// Number of messages enqueued in the last 10 seconds
	SAFMQ_INT32 enqueued10second;
	/// Number of messages enqueued in the last 60 seconds
	SAFMQ_INT32 enqueued60second;
	/// Number of messages enqueued in the last 300 seconds
	SAFMQ_INT32 enqueued300second;

	/// Number of messages retrieved in the last 10 seconds
	SAFMQ_INT32 retrieved10second;
	/// Number of messages retrieved in the last 60 seconds
	SAFMQ_INT32 retrieved60second;
	/// Number of messages retrieved in the last 300 seconds
	SAFMQ_INT32 retrieved300second;

	/// Number of messages "peeked" in the last 10 seconds
	SAFMQ_INT32 peeked10second;
	/// Number of messages "peeked" in the last 60 seconds
	SAFMQ_INT32 peeked60second;
	/// Number of messages "peeked" in the last 300 seconds
	SAFMQ_INT32 peeked300second;
};

/**
Information about a connected client.

*/
struct ClientInfo {
	ClientInfo() {}
	ClientInfo(const ClientInfo& src) {
		*this = src;
	}
	ClientInfo(const std::string& username, SAFMQ_INT32 address, short port) {
		this->username = username;
		this->address = address;
		this->port = port;
	}

	const ClientInfo& operator=(const ClientInfo& src) {
		username = src.username;
		address = src.address;
		port = src.port;
		return *this;
	}

	/// User name of the connected client
	std::string				username;

	SAFMQ_INT32				address;
	short					port;
};

} // end of safmq namespace

#endif

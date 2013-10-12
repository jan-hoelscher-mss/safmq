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
#ifndef _SERVER_OPERATORS_H_
#define _SERVER_OPERATORS_H_

#include "comdefs.h"
#include "Log.h"

namespace safmq {
class InvalidBufferException : std::exception {
public:
	InvalidBufferException(SAFMQ_INT32 expected, SAFMQ_INT32 received) throw() {
		std::stringstream	s;
		s << "Expected (" << expected << ") bytes, received (" << received << ")" << std::ends;
		m_what = s.str();
	}
    InvalidBufferException(const exception& rhs) throw() {
		*this = rhs;
	}
    InvalidBufferException& operator=(const exception& rhs) throw() {
		m_what = rhs.what();		
		return *this;
	}
    virtual ~InvalidBufferException() throw() { }
    virtual const char *what() const throw() {
		return m_what.c_str();
	}
protected:
	std::string m_what;
};


inline void consume(std::istream& i, SAFMQ_INT32 size)
{
	char	c;
	for(SAFMQ_INT32 x = size; x > 0; x--)
		i.read(&c,sizeof(c));
}

inline void evalsize(std::istream& i, SAFMQ_INT32 param_size)
{
	SAFMQ_INT32 size;

	// read the size and compare it to what was expected
	i.read((char*)&size,sizeof(size));
	size = ntohl(size);
	if (size != param_size) {
		// clear out the misunderstood message.
		consume(i, size);
		throw InvalidBufferException(param_size,size);
	}
}

inline std::istream& readstring(std::istream& i, std::string& str, SAFMQ_INT32 len)
{
	str.reserve(len);
	for( ; i.good() && len > 0; --len) {
		str.append(1, (char)i.get());
	}
	return i;
}

inline SAFMQ_INT32 readlong(std::istream& i)
{
	SAFMQ_INT32	l = 0;
	i.read((char*)&l, sizeof(l));
	return ntohl(l);
}

inline short readshort(std::istream& i)
{
	short s = 0;
	i.read((char*)&s, sizeof(s));
	return ntohs(s);
}

inline std::ostream& writelong(std::ostream& o, SAFMQ_INT32 l)
{
	l = htonl(l);
	o.write((char*)&l, sizeof(l));
	return o;
}

inline std::ostream& writeshort(std::ostream& o, short s)
{
	s = htons(s);
	o.write((char*)&s, sizeof(s));
	return o;
}

// /////////////////////////////////////////////////////////////////////////////
// Query Input Operators
// /////////////////////////////////////////////////////////////////////////////
inline std::istream& operator>>(std::istream& i, struct LOGIN_PARAMS& params) throw (InvalidBufferException) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		ntohwstr(params.username,NUMELEM(params.username));
		ntohwstr(params.password,NUMELEM(params.password));
		params.major_version = ntohl(params.major_version);
		params.minor_version = ntohl(params.minor_version);
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct USER_SET_PASSWORD_PARAMS& params) throw (InvalidBufferException) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		ntohwstr(params.username,NUMELEM(params.username));
		ntohwstr(params.password,NUMELEM(params.password));
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct USER_CREATE_PARAMS& params) throw (InvalidBufferException) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		ntohwstr(params.username,NUMELEM(params.username));
		ntohwstr(params.password,NUMELEM(params.password));
		ntohwstr(params.description,NUMELEM(params.description));
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct USER_SET_PERMS_PARAMS& params) throw (InvalidBufferException) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		ntohwstr(params.username,NUMELEM(params.username));
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct GROUP_SET_PERMS_PARAMS& params) throw (InvalidBufferException) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		ntohwstr(params.groupname,NUMELEM(params.groupname));
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS& params) throw (InvalidBufferException) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		ntohwstr(params.username,NUMELEM(params.username));
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS& params) throw (InvalidBufferException) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		ntohwstr(params.groupname,NUMELEM(params.groupname));
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS& params) throw (InvalidBufferException) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		ntohwstr(params.groupname,NUMELEM(params.groupname));
		ntohwstr(params.username,NUMELEM(params.username));
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS& params) throw (InvalidBufferException) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		ntohwstr(params.queuename,NUMELEM(params.queuename));
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct QUEUE_SET_USER_PERMS_PARAMS& params) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		ntohwstr(params.queuename,NUMELEM(params.queuename));
		ntohwstr(params.username,NUMELEM(params.username));
	}
	return i;
}


inline std::istream& operator>>(std::istream& i, struct QUEUE_DEL_USER_PERMS_PARAM& params) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		ntohwstr(params.queuename,NUMELEM(params.queuename));
		ntohwstr(params.username,NUMELEM(params.username));
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct QUEUE_SET_GROUP_PERMS_PARAMS& params) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		ntohwstr(params.queuename,NUMELEM(params.queuename));
		ntohwstr(params.groupname,NUMELEM(params.groupname));
	}
	return i;
}


inline std::istream& operator>>(std::istream& i, struct QUEUE_DEL_GROUP_PERMS_PARAM& params) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		ntohwstr(params.queuename,NUMELEM(params.queuename));
		ntohwstr(params.groupname,NUMELEM(params.groupname));
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct QUEUE_CLOSE_OPEN_CURSOR_PARAMS& params) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	return i;
}

inline std::istream& operator>>(std::istream& i, struct ENQUEUE_PARAMS& params) {
	// NOTE: Don't call eavlsize for this message data, it's variable length...
	SAFMQ_INT32	size;
	i.read((char*)&size,sizeof(size));
	size = ntohl(size);

	i.read((char*)&params.queueID, sizeof(params.queueID));
	if (i.good()) {
		i.read((char*)&params.msg->msg, sizeof(QMessage));
		if (i.good()) {
			ntohwstr(params.msg->msg.Label,				NUMELEM(params.msg->msg.Label));
			params.msg->msg.TimeStamp					= ntohl(params.msg->msg.TimeStamp);
			params.msg->msg.TimeToLiveSeconds			= ntohl(params.msg->msg.TimeToLiveSeconds);
			ntohwstr(params.msg->msg.ResponseQueueName,	NUMELEM(params.msg->msg.ResponseQueueName));
			params.msg->msg.BodySize					= ntohl(params.msg->msg.BodySize);
			
			// TODO: Eval size vs. sizeof(queueID) and sizeof(QMessage) and msg.BodySize;

			bufstream*	buf = params.msg->getBufferStream();
			buf->reserve(params.msg->msg.BodySize);
			buf->seekg(0);
			i.read(buf->buffer(),params.msg->msg.BodySize);
			if (i.good())
				buf->seekp(params.msg->msg.BodySize);
			else
				buf->seekp(0);
		}
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct RETRIEVE_PEEK_FRONT_PARAMS& params) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		params.timeoutseconds = ntohl(params.timeoutseconds);;
	}
	return i;
}
inline std::istream& operator>>(std::istream& i, struct RETRIEVE_ACK_PARAMS& params) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	return i;
}
inline std::istream& operator>>(std::istream& i, struct RETRIEVE_ID_PEEK_ID_PARAMS& params) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		params.timeoutseconds = ntohl(params.timeoutseconds);;
	}
	return i;
}
inline std::istream& operator>>(std::istream& i, struct PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS& params) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	return i;
}
inline std::istream& operator>>(std::istream& i, struct CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS& params) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	return i;
}
inline std::istream& operator>>(std::istream& i, struct SEEK_ID_PARAMS& params) {
	evalsize(i,sizeof(params));

	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		params.timeoutseconds = ntohl(params.timeoutseconds);
	}
	return i;
}

// NOTE: if wchar_t is ever supported, this operator must be changed to recognize wide character byte ordering
inline std::istream& operator>>(std::istream& i, struct ADD_USER_IDENTITY_PARAMS& params) {
	SAFMQ_INT32 size;

	// read the size and compare it to what was expected
	i.read((char*)&size,sizeof(size));
	size = ntohl(size);

	int headerSize = sizeof(SAFMQ_INT32) * 3;

	if (size >= headerSize) {
		SAFMQ_INT32 subjectLen = readlong(i);
		if (!i.good()) return i;
		
		SAFMQ_INT32 issuerLen = readlong(i);
		if (!i.good()) return i;

		SAFMQ_INT32 userLen = readlong(i);
		if (!i.good()) return i;

		int expectedSize = subjectLen + issuerLen + userLen + headerSize;

		if (size == expectedSize) {
			if (!readstring(i, params.subjectDN, subjectLen).good()) return i;
			if (!readstring(i, params.issuerDN, issuerLen).good()) return i;
			if (!readstring(i, params.username, userLen).good()) return i;
		} else {
			consume(i, size - (expectedSize - headerSize)); // header has already been read
			throw InvalidBufferException(expectedSize, size);
		}
	} else {
		consume(i, size); // invalid size
		throw InvalidBufferException(headerSize, size);
	}
	return i;
}

// NOTE: if wchar_t is ever supported, this operator must be changed to recognize wide character byte ordering
inline std::istream& operator>>(std::istream& i, struct REMOVE_USER_IDENTITY& params) {
	SAFMQ_INT32 size;

	// read the size and compare it to what was expected
	i.read((char*)&size,sizeof(size));
	size = ntohl(size);

	int headerSize = sizeof(SAFMQ_INT32) * 2;

	if (size >= headerSize) {
		SAFMQ_INT32 subjectLen = readlong(i);
		if (!i.good()) return i;
		
		SAFMQ_INT32 issuerLen = readlong(i);
		if (!i.good()) return i;

		int expectedSize = subjectLen + issuerLen + headerSize;

		if (size == expectedSize) {
			if (!readstring(i, params.subjectDN, subjectLen).good()) return i;
			if (!readstring(i, params.issuerDN, issuerLen).good()) return i;
		} else {
			consume(i, size - (expectedSize - headerSize)); // header has already been read
			throw InvalidBufferException(expectedSize, size);
		}
	} else {
		consume(i, size); // invalid size
		throw InvalidBufferException(headerSize, size);
	}
	return i;
}

// NOTE: if wchar_t is ever supported, this operator must be changed to recognize wide character byte ordering
inline std::istream& operator>>(std::istream& i, struct ENUM_USER_IDENTITY& params) {
	SAFMQ_INT32 size;

	// read the size and compare it to what was expected
	i.read((char*)&size,sizeof(size));
	size = ntohl(size);

	int headerSize = sizeof(SAFMQ_INT32);

	if (size >= headerSize) {
		SAFMQ_INT32 userLen = readlong(i);
		if (!i.good()) return i;

		int expectedSize = userLen + headerSize;
		if (size == expectedSize) {
			if (!readstring(i, params.username, userLen).good()) return i;
		} else {
			consume(i, size - (expectedSize - headerSize)); // header has already been read
			throw InvalidBufferException(expectedSize, size);
		}
	} else {
		consume(i, size); // invalid size
		throw InvalidBufferException(headerSize, size);
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct QUEUE_STATS_PARAMS& params) {
	SAFMQ_UINT32 size;
	
	i.read((char*)&size, sizeof(size));
	size = ntohl(size);

	i.read((char*)&params, sizeof(params));
	params.queueID = ntohl(params.queueID);
	return i;
}



// /////////////////////////////////////////////////////////////////////////////
// Response Operators
// /////////////////////////////////////////////////////////////////////////////
inline std::ostream& operator<<(std::ostream& o, const struct QUEUE_CONFIG_ENTRY& val /*data element of ENUM_QUEUES_RESPONSE */)
{
	QUEUE_CONFIG_ENTRY	ent = val;

	htonwstr(ent.queuename,SAFMQ_QNAME_LENGTH);
	htonwstr(ent.owner,SAFMQ_USER_NAME_LENGTH);
	o.write((char*)&ent, sizeof(ent));
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const struct LOGIN_RESPONSE& val)
{
	LOGIN_RESPONSE	params = val;
	
	params.major_version = htonl(params.major_version);
	params.minor_version = htonl(params.minor_version);
	o.write((const char*)&params, sizeof(params));
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const RESPONSE_RESPONSE& val)
{
	RESPONSE_RESPONSE	resp = val;
	resp.errorcode = htonl(val.errorcode);
	o.write((char*)&resp,sizeof(resp));
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const ENUM_QUEUES_RESPONSE& val)
{
	ENUM_QUEUES_RESPONSE	resp;

	resp.errorcode = htonl(val.errorcode);
	resp.nQueues = htonl(val.nQueues);

	o.write((char*)&resp,sizeof(resp));
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const ENUM_USERS_RESPONSE& val)
{
	ENUM_USERS_RESPONSE	resp;

	resp.errorcode = htonl(val.errorcode);
	resp.nUsers = htonl(val.nUsers);

	o.write((char*)&resp,sizeof(resp));
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const ENUM_GROUPS_RESPONSE& val)
{
	ENUM_GROUPS_RESPONSE	resp;

	resp.errorcode = htonl(val.errorcode);
	resp.nGroups = htonl(val.nGroups);

	o.write((char*)&resp,sizeof(resp));
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const GROUP_GET_USERS_RESPONSE& val)
{
	GROUP_GET_USERS_RESPONSE	resp = val;

	resp.errorcode = htonl(val.errorcode);
	resp.nUsers = htonl(val.nUsers);
	o.write((char*)&resp,sizeof(resp));
	return o;
}
inline std::ostream& operator<<(std::ostream& o, const USER_GET_GROUPS_RESPONSE & val)
{
	USER_GET_GROUPS_RESPONSE	resp = val;

	resp.errorcode = htonl(val.errorcode);
	resp.nGroups = htonl(val.nGroups);
	o.write((char*)&resp,sizeof(resp));
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const OPEN_QUEUE_RESPOSNE& val)
{
	OPEN_QUEUE_RESPOSNE resp = val;

	resp.errorcode	= htonl(val.errorcode);
	o.write((char*)&resp,sizeof(resp));
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const ENQUEUE_RESPONSE& val)
{
	ENQUEUE_RESPONSE	resp = val;

	resp.errorcode	= htonl(val.errorcode);
	resp.data.timestamp = htonl(val.data.timestamp);
	o.write((char*)&resp,sizeof(resp));
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const struct USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE& resp)
{
	o.write((char*)&resp,sizeof(resp));
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const struct QUEUE_PERM_DATA& val)
{
	QUEUE_PERM_DATA resp = val;
	htonwstr(resp.entityname, sizeof(resp.entityname));
	o.write((char*)&resp,sizeof(resp));
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const struct QUEUE_ENUM_PERMS_RESPOSNE& val)
{
	QUEUE_ENUM_PERMS_RESPOSNE	resp = val;
	resp.nPerms = htonl(resp.nPerms);
	o.write((char*)&resp,sizeof(resp));
	return o;
}

/**
Writes everything but the body.
*/
//inline std::ostream& operator<<(std::ostream& o, const RETRIEVE_RESPONSE& val)
//{
//	RETRIEVE_RESPONSE resp = val;

//	resp.errorcode					= htonl(val.errorcode);
//	resp.msg.msg.TimeStamp			= htonl(resp.msg.msg.TimeStamp);
//	resp.msg.msg.TimeToLiveSeconds	= htonl(resp.msg.msg.TimeToLiveSeconds);
//	resp.msg.msg.BodySize			= htonl(resp.msg.msg.BodySize);
	
//	o.write((char*)&resp.errorcode, sizeof(resp.errorcode));
//	o.write((char*)&resp.msg.msg, sizeof(resp.msg.msg));
//	return o;
//}


inline std::ostream& operator<<(std::ostream& o, const OPEN_CURSOR_RESPONSE& val)
{
	OPEN_CURSOR_RESPONSE	resp = val;
	resp.errorcode	= htonl(val.errorcode);
	o.write((char*)&resp,sizeof(resp));
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const std::vector<safmq::X509Identity> ids)
{
	std::vector<safmq::X509Identity>::const_iterator	i;

	// Step 1: Write the number of entries as a 4 byte integer
	// Step 2: For each identity in the vector, write the identity
	// Step 2.1: Write the Subject length
	// Step 2.2: Write the Subject string
	// Step 2.3: Write the Issuer length
	// Step 2.4: Write the Issuer string

	if (writelong(o, ids.size()).good()) {
		for(i = ids.begin(); i != ids.end() && o.good(); ++i) {
			if (writelong(o, i->subjectDN.length()).good())
				if (o.write(i->subjectDN.c_str(), i->subjectDN.length()).good())
					if (writelong(o, i->issuerDN.length()).good())
						o.write(i->issuerDN.c_str(), i->issuerDN.length());
		}
	}
	return o;	
}

inline std::ostream& operator<<(std::ostream& o, const QUEUE_CREATE_TEMP_RESPONSE& resp) {
	if (writelong(o, resp.errorcode)) {
		if (resp.errorcode == EC_NOERROR) {
			if (writelong(o, resp.queueID)) {
				if (writelong(o, resp.queueName.length())) {
					o.write(resp.queueName.c_str(), resp.queueName.length());
				}
			}
		}
	}
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const QUEUE_STATS_RESPONSE& resp) {
	if (writelong(o, resp.errorcode)) {
		if (resp.errorcode == EC_NOERROR) {
			QueueStatistics stats;

			stats.messageCount = htonl(resp.stats.messageCount);
			stats.storageBytes = htonl(resp.stats.storageBytes);
			stats.messageBytes = htonl(resp.stats.messageBytes);

			stats.enqueued10second = htonl(resp.stats.enqueued10second);
			stats.enqueued60second = htonl(resp.stats.enqueued60second);
			stats.enqueued300second = htonl(resp.stats.enqueued300second);

			stats.retrieved10second = htonl(resp.stats.retrieved10second);
			stats.retrieved60second = htonl(resp.stats.retrieved60second);
			stats.retrieved300second = htonl(resp.stats.retrieved300second);

			stats.peeked10second = htonl(resp.stats.peeked10second);
			stats.peeked60second = htonl(resp.stats.peeked60second);
			stats.peeked300second = htonl(resp.stats.peeked300second);

			o.write((const char*)&stats, sizeof(stats));
		}
	}

	return o;
}

inline std::ostream& operator<<(std::ostream& o, CLIENT_INFO_RESPONSE& resp) {
	if (writelong(o, resp.errorcode)) {
		if (o.good()) {
			writelong(o, resp.clientInfo.size());
			if (o.good()) {
				for(std::list<ClientInfo>::iterator cli = resp.clientInfo.begin(); o.good() && cli != resp.clientInfo.end(); ++cli) {
					writelong(o, cli->username.length());
					o.write(cli->username.c_str(), cli->username.length());
					writelong(o, cli->address);
					writeshort(o, cli->port);
				}
			}
		}
	}
	return o;
}


} // end namespace safmq

#endif

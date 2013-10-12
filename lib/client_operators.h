/*
 Copyright 2005 Matthew J. Battey

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
#ifndef _CLIENT_OPERATORS_H_
#define _CLIENT_OPERATORS_H_
#include "comdefs.h"

namespace safmq {

inline bool writeSize(std::ostream& o, SAFMQ_UINT32 size) {
	size = htonl(size);
	o.write((const char*)&size,sizeof(size));
	return o.good();
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


// ////////////////////////////////////////////////////////////////////////////
// Response From Server input operators
// ////////////////////////////////////////////////////////////////////////////
inline std::istream& operator>>(std::istream& i, struct LOGIN_RESPONSE& params) {
	i.read((char*)&params, sizeof(params));
	if (i.good()) {
		params.major_version = ntohl(params.major_version);
		params.minor_version = ntohl(params.minor_version);
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct QUEUE_CONFIG_ENTRY& ent) {
	i.read((char*)&ent, sizeof(ent));
	if (i.good()) {
		ntohwstr(ent.queuename, SAFMQ_QNAME_LENGTH);
		ntohwstr(ent.owner, SAFMQ_USER_NAME_LENGTH);
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, ENQUEUE_RESPONSE_DATA& data) {
	i.read((char*)&data,sizeof(data));
	data.timestamp = ntohl(data.timestamp);
	return i;
}

inline std::istream& operator>>(std::istream& i, struct USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data& ent) {
	i.read((char*)&ent,sizeof(ent));
	return i;
}

inline std::istream& operator>>(std::istream& i, struct QUEUE_PERM_DATA& ent) {
	i.read((char*)&ent,sizeof(ent));
	if (i.good()) {
		ntohwstr(ent.entityname, sizeof(ent.entityname));
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, struct QUEUE_ENUM_PERMS_RESPOSNE& ent) {
	i.read((char*)&ent,sizeof(ent));
	if (i.good()) {
		ent.nPerms = ntohl(ent.nPerms);
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, std::vector<safmq::X509Identity>& ids) {
	ids.clear();

	int	count = readlong(i);
	for( ; i.good() && count > 0; --count) {
		safmq::X509Identity	id;
		int					len;
		len = readlong(i);
		if (i.good()) {
			if (readstring(i, id.subjectDN, len).good()) {
				len = readlong(i);
				if (i.good())
					if (readstring(i, id.issuerDN, len).good())
						ids.push_back(id);
			}
		}
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, QUEUE_CREATE_TEMP_RESPONSE& resp) {
	resp.errorcode = readlong(i);
	if (i.good()) {
		if (resp.errorcode == EC_NOERROR) {
			resp.queueID = readlong(i);
			if (i.good()) {
				long length = readlong(i);
				if (i.good()) {
					readstring(i, resp.queueName, length);
				}
			}
		}
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, CLIENT_INFO_RESPONSE& resp) {
	resp.errorcode = readlong(i);
	if (i.good() && resp.errorcode == EC_NOERROR) {
		long ciCount = readlong(i);
		long nameLen;
		ClientInfo	ci;
		for(int x = 0; x < ciCount && i.good(); ++x) {
			nameLen = readlong(i);
			readstring(i, ci.username, nameLen);
			ci.address = readlong(i);
			ci.port = readshort(i);
			resp.clientInfo.push_back(ci);
		}
	}
	return i;
}

inline std::istream& operator>>(std::istream& i, QUEUE_STATS_RESPONSE& resp) {
	resp.errorcode = readlong(i);
	if (i.good()) {
		if (resp.errorcode == EC_NOERROR) {
			i.read((char*)&resp.stats, sizeof(resp.stats));
			if (i.good()) {
				resp.stats.messageCount = ntohl(resp.stats.messageCount);
				resp.stats.storageBytes = ntohl(resp.stats.storageBytes);
				resp.stats.messageBytes = ntohl(resp.stats.messageBytes);

				resp.stats.enqueued10second = ntohl(resp.stats.enqueued10second);
				resp.stats.enqueued60second = ntohl(resp.stats.enqueued60second);
				resp.stats.enqueued300second = ntohl(resp.stats.enqueued300second);

				resp.stats.retrieved10second = ntohl(resp.stats.retrieved10second);
				resp.stats.retrieved60second = ntohl(resp.stats.retrieved60second);
				resp.stats.retrieved300second = ntohl(resp.stats.retrieved300second);

				resp.stats.peeked10second = ntohl(resp.stats.peeked10second);
				resp.stats.peeked60second = ntohl(resp.stats.peeked60second);
				resp.stats.peeked300second = ntohl(resp.stats.peeked300second);
			} else {
				resp.errorcode = EC_NETWORKERROR;
			}
		} // errorcode set to error already
	} else {
		resp.errorcode = EC_NETWORKERROR;
	}

	return i;
}

// ////////////////////////////////////////////////////////////////////////////
// Client Query Output Operators
// ////////////////////////////////////////////////////////////////////////////

inline std::ostream& operator<<(std::ostream& o, const struct LOGIN_PARAMS& val) {
	LOGIN_PARAMS	params = val;
	
	if (writeSize(o,sizeof(params))) {	
		htonwstr(params.username,NUMELEM(params.username));
		htonwstr(params.password,NUMELEM(params.password));
		params.major_version = htonl(params.major_version);
		params.minor_version = htonl(params.minor_version);
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const struct USER_SET_PASSWORD_PARAMS& val) {
	USER_SET_PASSWORD_PARAMS	params = val;
	
	if (writeSize(o,sizeof(params))) {	
		htonwstr(params.username,NUMELEM(params.username));
		htonwstr(params.password,NUMELEM(params.password));
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const struct ENQUEUE_PARAMS& params) {
	SAFMQ_INT32			size;

	size = sizeof(params.queueID) + sizeof(QMessage) + params.msg->msg.BodySize;
	if (writeSize(o,size)) {
		o.write((const char*)&params.queueID,sizeof(params.queueID));
		if (o.good()) {
			o << params.msg->msg;
			if (o.good()) {
				o.write(params.msg->getBufferStream()->buffer(), params.msg->msg.BodySize);
			}
		}
	}
	return o;
}


inline std::ostream& operator<<(std::ostream& o, const struct USER_CREATE_PARAMS& val) {
	USER_CREATE_PARAMS params = val;
	
	if (writeSize(o,sizeof(params))) {	
		htonwstr(params.username,NUMELEM(params.username));
		htonwstr(params.password,NUMELEM(params.password));
		htonwstr(params.description,NUMELEM(params.description));
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}

inline std::ostream& operator<<(std::ostream& o, struct USER_SET_PERMS_PARAMS& data) {
	USER_SET_PERMS_PARAMS params = data;
	
	if (writeSize(o,sizeof(params))) {	
		htonwstr(params.username,NUMELEM(params.username));
		o.write((char*)&params, sizeof(params));
	}
	return o;
}

inline std::ostream& operator<<(std::ostream& o, struct GROUP_SET_PERMS_PARAMS& data) {
	GROUP_SET_PERMS_PARAMS params = data;
	
	if (writeSize(o,sizeof(params))) {	
		htonwstr(params.groupname,NUMELEM(params.groupname));
		o.write((char*)&params, sizeof(params));
	}
	return o;
}


inline std::ostream& operator<<(std::ostream& o, const struct USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS& val) {
	USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS params = val;
	
	if (writeSize(o,sizeof(params))) {	
		htonwstr(params.username,NUMELEM(params.username));
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const struct GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS& val) {
	GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS params = val;
	
	if (writeSize(o,sizeof(params))) {	
		htonwstr(params.groupname,NUMELEM(params.groupname));
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const struct GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS& val) {
	GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS params = val;
	
	if (writeSize(o,sizeof(params))) {	
		htonwstr(params.groupname,NUMELEM(params.groupname));
		htonwstr(params.username,NUMELEM(params.username));
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const struct QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS& val) {
	QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS params = val;
	
	if (writeSize(o,sizeof(params))) {	
		htonwstr(params.queuename,NUMELEM(params.queuename));
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}

inline std::ostream& operator<<(std::ostream& o, struct QUEUE_SET_USER_PERMS_PARAMS& data) {
	QUEUE_SET_USER_PERMS_PARAMS	params = data;
	
	if (writeSize(o,sizeof(params))) {	
		htonwstr(params.queuename,NUMELEM(params.queuename));
		htonwstr(params.username,NUMELEM(params.username));
		o.write((char*)&params, sizeof(params));
	}
	return o;
}


inline std::ostream& operator<<(std::ostream& o, struct QUEUE_DEL_USER_PERMS_PARAM& data) {
	QUEUE_DEL_USER_PERMS_PARAM	params = data;
	
	if (writeSize(o,sizeof(params))) {	
		htonwstr(params.queuename,NUMELEM(params.queuename));
		htonwstr(params.username,NUMELEM(params.username));
		o.write((char*)&params, sizeof(params));
	}
	return o;
}

inline std::ostream& operator<<(std::ostream& o, struct QUEUE_SET_GROUP_PERMS_PARAMS& data) {
	QUEUE_SET_GROUP_PERMS_PARAMS	params = data;
	
	if (writeSize(o,sizeof(params))) {	
		htonwstr(params.queuename,NUMELEM(params.queuename));
		htonwstr(params.groupname,NUMELEM(params.groupname));
		o.write((char*)&params, sizeof(params));
	}
	return o;
}


inline std::ostream& operator<<(std::ostream& o, struct QUEUE_DEL_GROUP_PERMS_PARAM& data) {
	QUEUE_DEL_GROUP_PERMS_PARAM	params = data;
	
	if (writeSize(o,sizeof(params))) {	
		htonwstr(params.queuename,NUMELEM(params.queuename));
		htonwstr(params.groupname,NUMELEM(params.groupname));
		o.write((char*)&params, sizeof(params));
	}
	return o;
}


inline std::ostream& operator<<(std::ostream& o, const struct QUEUE_CLOSE_OPEN_CURSOR_PARAMS& params) {
	
	if (writeSize(o,sizeof(params))) {	
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const struct RETRIEVE_PEEK_FRONT_PARAMS& val) {
	RETRIEVE_PEEK_FRONT_PARAMS	params = val;
	
	if (writeSize(o,sizeof(params))) {	
		params.timeoutseconds = htonl(params.timeoutseconds);;
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}
inline std::ostream& operator<<(std::ostream& o, const struct RETRIEVE_ACK_PARAMS& params) {
	
	if (writeSize(o,sizeof(params))) {	
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}
inline std::ostream& operator<<(std::ostream& o, const struct RETRIEVE_ID_PEEK_ID_PARAMS& val) {
	RETRIEVE_ID_PEEK_ID_PARAMS	params = val;
	
	if (writeSize(o,sizeof(params))) {	
		params.timeoutseconds = htonl(params.timeoutseconds);;
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}
inline std::ostream& operator<<(std::ostream& o, const struct PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS& params) {
	if (writeSize(o,sizeof(params))) {	
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}
inline std::ostream& operator<<(std::ostream& o, const struct CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS& params) {
	if (writeSize(o,sizeof(params))) {	
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}
inline std::ostream& operator<<(std::ostream& o, const struct SEEK_ID_PARAMS& val) {
	SEEK_ID_PARAMS	params = val;
	
	if (writeSize(o,sizeof(params))) {	
		params.timeoutseconds = htonl(params.timeoutseconds);;
		o.write((const char*)&params, sizeof(params));
	}
	return o;
}


// NOTE: if wchar_t is supported this operator must be modified to recognize this
inline std::ostream& operator<<(std::ostream& o, const struct ADD_USER_IDENTITY_PARAMS& val) {
	SAFMQ_INT32 slen = val.subjectDN.length();
	SAFMQ_INT32 ilen = val.issuerDN.length();
	SAFMQ_INT32 ulen = val.username.length();

	size_t length = sizeof(SAFMQ_INT32)*3 + slen + ilen + ulen;
	if (writeSize(o, length))
		if (writelong(o, slen).good()) 
			if (writelong(o, ilen).good()) 
				if (writelong(o, ulen).good()) 
					if (o.write(val.subjectDN.c_str(), slen).good())
						if (o.write(val.issuerDN.c_str(), ilen).good())
							o.write(val.username.c_str(), ulen);	
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const struct REMOVE_USER_IDENTITY& val) {
	SAFMQ_INT32 slen = val.subjectDN.length();
	SAFMQ_INT32 ilen = val.issuerDN.length();

	size_t length = sizeof(SAFMQ_INT32)*2 + slen + ilen ;
	if (writeSize(o, length))
		if (writelong(o, slen).good()) 
			if (writelong(o, ilen).good()) 
				if (o.write(val.subjectDN.c_str(), slen).good())
					o.write(val.issuerDN.c_str(), ilen);
	return o;
}

inline std::ostream& operator<<(std::ostream& o, const struct ENUM_USER_IDENTITY& val) {
	SAFMQ_INT32 ulen = val.username.length();

	size_t length = sizeof(SAFMQ_INT32) + ulen;
	if (writeSize(o, length))
		if (writelong(o, ulen).good()) 
			o.write(val.username.c_str(), ulen);	
	return o;
}


inline std::ostream& operator<<(std::ostream& o, const struct QUEUE_STATS_PARAMS& val) {
	size_t length = sizeof(QUEUE_STATS_PARAMS);
	if ( writeSize(o, length) ) {
		if ( writelong(o, val.queueID) ) {
			o.put(val.includeStorageBytes);
			o.put(val.includeMessageBytes);
		}
	}
	return o;
}

}

#endif


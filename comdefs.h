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
#ifndef _COMDEFS_H_
#define _COMDEFS_H_

// NOTE: Assumes safmq.h has been already included

#include <istream>
#include <ostream>
#include <sstream>

#ifdef _WIN32
	#include <winsock.h>
#else
	#include <netinet/in.h>
#endif

namespace safmq {

/**
The Current SAFMQ Protocol Major Version.  Note: Major Version will change when a 
previously established message buffer changes.
*/
#define SAFMQ_PROTOCOL_MAJOR_VERSION 1

/**
The Current SAFMQ Protocol Minor Version.  Note: Minor Version will change when
a new message buffer is added.
*/
#define SAFMQ_PROTOCOL_MINOR_VERSION 2


enum SAFMQ_COM_CMD {
	LOGIN = 0,
	ENUM_QUEUES		= 1,
	SERVER_STATS	= 2,
	CLIENT_INFO		= 3,

	USER_CREATE		= 10,
	USER_DELETE		= 11,
	ENUM_USERS		= 12,
	USER_SET_PERMS	= 13,
	USER_GET_PERMS	= 14,
	USER_GET_GROUPS	= 15,
	USER_SET_PASSWORD	= 16,
	USER_ADD_IDENT		= 17,
	USER_DEL_IDENT		= 18,
	USER_ENUM_IDENT		= 19,
	
	GROUP_CREATE	= 20,
	GROUP_DELETE	= 21,
	ENUM_GROUPS		= 22,

	GROUP_ADD_USER		= 30,
	GROUP_DELETE_USER	= 31,
	GROUP_GET_USERS		= 32,
	GROUP_SET_PERMS		= 33,
	GROUP_GET_PERMS		= 34,

	QUEUE_CREATE			= 40,
	QUEUE_DELETE			= 41,
	QUEUE_SET_USER_PERMS	= 42,
	QUEUE_DEL_USER_PERMS	= 43,
	QUEUE_SET_GROUP_PERMS	= 44,
	QUEUE_DEL_GROUP_PERMS	= 45,
	QUEUE_ENUM_PERMS		= 46,
	QUEUE_CREATE_TEMP		= 47,
	QUEUE_STATS				= 48,

	QUEUE_OPEN		= 50,
	QUEUE_CLOSE		= 51,

	ENQUEUE_FORWARD = 60,

	ENQUEUE			= 70,
	RETRIEVE		= 71,
	RETRIEVE_ACK	= 72,
	RETRIEVE_ID		= 73,
	PEEK_ID			= 74,
	PEEK_FRONT		= 75,
	RETRIEVE_CURSOR_ACK = 76,

	OPEN_CURSOR		= 80,
	CLOSE_CURSOR	= 81,
	ADVANCE_CURSOR	= 82,
	SEEK_ID			= 83,
	TEST_CURSOR		= 84,
	PEEK_CURSOR		= 85,
	RETRIEVE_CURSOR	= 86,

	BEGIN_TRANSACTION		= 100,
	COMMIT_TRANSACTION		= 101,
	ROLLBACK_TRANSACTION	= 102,
	END_TRANSACTION			= 103,
};


#pragma pack(1)
///////////////////////////////////////////////////////////////////////////////
// Client Request Structures
///////////////////////////////////////////////////////////////////////////////

struct LOGIN_PARAMS {
	LOGIN_PARAMS() {
		major_version = SAFMQ_PROTOCOL_MAJOR_VERSION;
		minor_version = SAFMQ_PROTOCOL_MINOR_VERSION;
	}
	char	username[SAFMQ_USER_NAME_LENGTH];
	char	password[SAFMQ_PASSWORD_LENGTH];
	SAFMQ_INT32	major_version;
	SAFMQ_INT32	minor_version;
};

struct LOGIN_RESPONSE {
	LOGIN_RESPONSE() {
		major_version = SAFMQ_PROTOCOL_MAJOR_VERSION;
		minor_version = SAFMQ_PROTOCOL_MINOR_VERSION;
	}
	SAFMQ_INT32 major_version;
	SAFMQ_INT32 minor_version;
};

struct USER_SET_PASSWORD_PARAMS {
	char	username[SAFMQ_USER_NAME_LENGTH];
	char	password[SAFMQ_PASSWORD_LENGTH];
};

struct USER_CREATE_PARAMS {
	char	username[SAFMQ_USER_NAME_LENGTH];
	char	password[SAFMQ_PASSWORD_LENGTH];
	char	description[SAFMQ_DESCRIPTION_LENGTH];
};

struct USER_SET_PERMS_PARAMS {
	char	username[SAFMQ_USER_NAME_LENGTH];
	char	modify_queues;
	char	modify_users;
	char	modify_groups;
};


struct USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS {
	char	username[SAFMQ_USER_NAME_LENGTH];
};

struct GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS {
	char	groupname[SAFMQ_GROUP_NAME_LENGTH];
};

struct GROUP_SET_PERMS_PARAMS {
	char	groupname[SAFMQ_GROUP_NAME_LENGTH];
	char	modify_queues;
	char	modify_users;
	char	modify_groups;
};

struct GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS {
	char	groupname[SAFMQ_GROUP_NAME_LENGTH];
	char	username[SAFMQ_USER_NAME_LENGTH];
};

struct QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS {
	char	queuename[SAFMQ_QNAME_LENGTH];
};

struct QUEUE_SET_USER_PERMS_PARAMS {
	char	queuename[SAFMQ_QNAME_LENGTH];
	char	username[SAFMQ_USER_NAME_LENGTH];
	char	read;
	char	write;
	char	destroy;
	char	change_security;
};

struct QUEUE_DEL_USER_PERMS_PARAM {
	char queuename[SAFMQ_QNAME_LENGTH];
	char username[SAFMQ_USER_NAME_LENGTH];
};

struct QUEUE_SET_GROUP_PERMS_PARAMS {
	char	queuename[SAFMQ_QNAME_LENGTH];
	char	groupname[SAFMQ_GROUP_NAME_LENGTH];
	char	read;
	char	write;
	char	destroy;
	char	change_security;
};

struct QUEUE_DEL_GROUP_PERMS_PARAM {
	char queuename[SAFMQ_QNAME_LENGTH];
	char groupname[SAFMQ_GROUP_NAME_LENGTH];
};

struct QUEUE_CLOSE_OPEN_CURSOR_PARAMS {
	/*QueueHandle*/SAFMQ_INT32		queueID;
};

struct ENQUEUE_PARAMS {
	/*QueueHandle*/SAFMQ_INT32		queueID;
	QueueMessage			*msg;
};

struct RETRIEVE_PEEK_FRONT_PARAMS {
	/*QueueHandle*/SAFMQ_INT32		queueID;
	bool					retrievebody;
	SAFMQ_INT32					timeoutseconds;
};

struct RETRIEVE_ACK_PARAMS {
	/*QueueHandle*/SAFMQ_INT32		queueID;
	uuid					msgID;
};

struct RETRIEVE_ID_PEEK_ID_PARAMS {
	/*QueueHandle*/SAFMQ_INT32		queueID;
	bool					retrievebody;
	uuid					reciptID;
	SAFMQ_INT32					timeoutseconds;
};

struct PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS {
	/*QueueHandle*/SAFMQ_INT32		queueID;
	bool					retrievebody;
	/*CursorHandle*/SAFMQ_INT32	cursorID;
};

struct CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS {
	/*QueueHandle*/SAFMQ_INT32		queueID;
	/*CursorHandle*/SAFMQ_INT32	cursorID;
};

struct SEEK_ID_PARAMS {
	/*QueueHandle*/SAFMQ_INT32		queueID;
	/*CursorHandle*/SAFMQ_INT32	cursorID;
	uuid					reciptID;
	SAFMQ_INT32					timeoutseconds;
};

struct ADD_USER_IDENTITY_PARAMS {
	std::string	subjectDN;
	std::string	issuerDN;
	std::string	username;
};

struct REMOVE_USER_IDENTITY {
	std::string	subjectDN;
	std::string	issuerDN;
};

struct ENUM_USER_IDENTITY {
	std::string	username;
};

///////////////////////////////////////////////////////////////////////////////
// Server Response Structures
///////////////////////////////////////////////////////////////////////////////



struct RESPONSE_RESPONSE {
	RESPONSE_RESPONSE()					: errorcode(EC_NOERROR) {}
	RESPONSE_RESPONSE(ErrorCode err)	: errorcode(err) {}

	/*ErrorCode*/SAFMQ_UINT32	errorcode;
};

struct QUEUE_CONFIG_ENTRY {
	char	queuename[SAFMQ_QNAME_LENGTH];
	char	owner[SAFMQ_USER_NAME_LENGTH];
};

struct ENUM_QUEUES_RESPONSE {
	ENUM_QUEUES_RESPONSE()				: errorcode(EC_NOERROR) {}
	ENUM_QUEUES_RESPONSE(ErrorCode err)	: errorcode(err) {}

	/*ErrorCode*/SAFMQ_UINT32	errorcode;
	SAFMQ_INT32						nQueues;
	//QUEUE_CONFIG_ENTRY queue_data[nQueues];
};

struct ENUM_USERS_RESPONSE {
	ENUM_USERS_RESPONSE(SAFMQ_INT32 nUsers=0) : errorcode(EC_NOERROR) { this->nUsers = nUsers; }

	/*ErrorCode*/SAFMQ_UINT32	errorcode;
	SAFMQ_INT32						nUsers;
	// Followed by up to nUsers: char username[SAFMQ_USER_NAME_LENGTH];
	// Followed by up to nUsers: char description[SAFMQ_DESCRIPTION_LENGTH];
};

struct ENUM_GROUPS_RESPONSE {
	ENUM_GROUPS_RESPONSE(SAFMQ_INT32 nGroups=0) : errorcode(EC_NOERROR) { this->nGroups = nGroups; }

	/*ErrorCode*/SAFMQ_UINT32	errorcode;
	SAFMQ_INT32						nGroups;
	// Followed by up to nGroups: char username[SAFMQ_USER_NAME_LENGTH];
};


struct GROUP_GET_USERS_RESPONSE {
	GROUP_GET_USERS_RESPONSE(SAFMQ_INT32 nUsers) : errorcode(EC_NOERROR) { this->nUsers = nUsers; } 

	/*ErrorCode*/SAFMQ_UINT32	errorcode;
	SAFMQ_INT32		nUsers;
	//char	username[1][SAFMQ_USER_NAME_LENGTH]; // repeats upto nUsers times
};

struct USER_GET_GROUPS_RESPONSE {
	USER_GET_GROUPS_RESPONSE(SAFMQ_INT32 nGroups) : errorcode(EC_NOERROR) { this->nGroups = nGroups; } 

	/*ErrorCode*/SAFMQ_UINT32	errorcode;
	SAFMQ_INT32		nGroups;
	//Followed By: char	groupname[1][SAFMQ_GROUP_NAME_LENGTH]; // repeats upto nGroups times
};


struct OPEN_QUEUE_RESPOSNE {
	OPEN_QUEUE_RESPOSNE() : errorcode(EC_NOERROR) {} 

	/*ErrorCode*/SAFMQ_UINT32						errorcode;
	/*ServiceThread::QueueHandle*/SAFMQ_UINT32	queueID;
};

struct ENQUEUE_RESPONSE_DATA {
	ENQUEUE_RESPONSE_DATA() {}
	ENQUEUE_RESPONSE_DATA(const uuid& id, /*time_t*/SAFMQ_INT32 ts) : msgID(id), timestamp(ts) { }
	uuid						msgID;
	/*time_t*/SAFMQ_INT32		timestamp;
};

struct ENQUEUE_RESPONSE {
	ENQUEUE_RESPONSE() : errorcode(EC_NOERROR) { }
	ENQUEUE_RESPONSE(const uuid& id, /*time_t*/SAFMQ_INT32 ts) : errorcode(EC_NOERROR), data(id,ts) { }

	/*ErrorCode*/SAFMQ_UINT32	errorcode;
	ENQUEUE_RESPONSE_DATA		data;
};

struct RETRIEVE_RESPONSE {
	RETRIEVE_RESPONSE() : errorcode(EC_NOERROR) {} 
	/*ErrorCode*/SAFMQ_UINT32	errorcode;
	QueueMessage				msg;
};

struct OPEN_CURSOR_RESPONSE {
	OPEN_CURSOR_RESPONSE() : errorcode(EC_NOERROR) {} 
	OPEN_CURSOR_RESPONSE(/*QStorage::CursorHandle*/SAFMQ_INT32 cursor) : errorcode(EC_NOERROR), cursorID(cursor) {} 

	/*ErrorCode*/SAFMQ_UINT32					errorcode;
	/*QStorage::CursorHandle*/SAFMQ_UINT32		cursorID;
};

struct USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data {
	char	modify_queues;
	char	modify_users;
	char	modify_groups;
};

struct USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE {
	USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE() : errorcode(EC_NOERROR) {}

	ErrorCode										errorcode;
	USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data	data;
};

#define MAX_ENTITY_LENGTH	20

struct QUEUE_PERM_DATA {
	char	entityname[MAX_ENTITY_LENGTH];
	char	isgroup;
	char	read;
	char	write;
	char	destroy;
	char	change_security;
};

struct QUEUE_ENUM_PERMS_RESPOSNE {
	QUEUE_ENUM_PERMS_RESPOSNE(SAFMQ_INT32 nPerms) : errorcode(EC_NOERROR) { this->nPerms = nPerms;}

	ErrorCode	errorcode;
	SAFMQ_INT32		nPerms;
//	QUEUE_PERM_DATA	perms[0];
};

/*
Structure of output
	int32	errorCode
	int32	queueID
	int32	nameLength
	char	name[]	-- Equal to name length
*/
struct QUEUE_CREATE_TEMP_RESPONSE {
	SAFMQ_INT32/*ErrorCode*/	errorcode;
	SAFMQ_INT32/*QueueHandle*/	queueID;
	std::string					queueName;
};

struct QUEUE_STATS_RESPONSE {
	SAFMQ_INT32/*ErrorCode*/	errorcode;
	QueueStatistics				stats;
};

struct QUEUE_STATS_PARAMS {
	/*QueueHandle*/SAFMQ_INT32		queueID;
	unsigned char includeStorageBytes;
	unsigned char includeMessageBytes;
};

struct CLIENT_INFO_RESPONSE {
	SAFMQ_INT32/*ErrorCode*/	errorcode;
	std::list<ClientInfo>		clientInfo;
};


#pragma pack()


///////////////////////////////////////////////////////////////////////////////

#define NUMELEM(s) (sizeof(s)/sizeof(*s))

#define ntohwstr(sz, l)
#define htonwstr(sz, l)

/*
inline void ntohwstr(char *str, int len)
{
#ifdef TWOBYTE_WIDE_CHARACTERS
	int i;
	for(i=0; str[i]!=0 && i<len; i++)
		str[i] = ntohs(str[i]);
#endif
}

inline void htonwstr(char *str, int len)
{
#ifdef TWOBYTE_WIDE_CHARACTERS
	int i;
	for(i=0; str[i]!=0 && i<len;i++)
		str[i] = htons(str[i]);
#endif
}
*/

inline std::istream& operator>>(std::istream& i, struct QMessage& msg) {
	i.read((char*)&(msg), sizeof(msg));
	if (i.good()) {
		ntohwstr(msg.Label,				NUMELEM(msg.Label));
		msg.TimeStamp					= ntohl(msg.TimeStamp);
		msg.TimeToLiveSeconds			= ntohl(msg.TimeToLiveSeconds);
		ntohwstr(msg.ResponseQueueName,	NUMELEM(msg.ResponseQueueName));
		msg.BodySize					= ntohl(msg.BodySize);
	}
	return i;
}

inline std::ostream& operator<<(std::ostream& o, const struct QMessage& param) {
	QMessage		msg = param;
	
	// Ensure the body size is set correctly
	htonwstr(msg.Label,				NUMELEM(msg.Label));
	msg.TimeStamp					= htonl(msg.TimeStamp);
	msg.TimeToLiveSeconds			= htonl(msg.TimeToLiveSeconds);
	htonwstr(msg.ResponseQueueName,	NUMELEM(msg.ResponseQueueName));
	msg.BodySize					= htonl(msg.BodySize);
	o.write((const char*)&(msg), sizeof(msg));
	return o;
}


} // end of safmq namespace

#endif

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
#pragma warning(disable: 4786)

#include <iostream>
#include <sstream>
#include <string>
#include <exception>
#include <time.h>
#include <stdio.h>

#include "safmq.h"
#include "lib/MessageQueue.h"
#include "lib/MQConnection.h"
#include "lib/MQFactory.h"
#include "url/urldecode.h"

#if _WIN32
	#define STRICMP _stricmp
	#include "_getopt.h"
#elif unix || (__APPLE__ && __MACH__) || __FreeBSD__

	#define STRICMP strcasecmp
	#include <termios.h>
	#include <unistd.h>
	#include <pwd.h>

	#if !defined(SAFMQ_GETOPT)
		#include <getopt.h>
	#else
		#include "utilities/_getopt_long.c"
	#endif
#else
	#error Unknown OS	
#endif

#if defined(SAFMQ_SSL)
#include "tcpsocket/sslsocket.h"
#endif

using namespace std;
using namespace safmq;
using namespace tcpsocket;

string	user;
string	passwd;

int		timeout=0;
int		ttl=0;
string	label;
string	response_q;
uuid	recipt_id;
bool	ttlerr=false;
bool	body=false;
bool	bodyonly=false;
int		priority=MP_STANDARD;

SSLContext ctx;

enum OPID {
	OPID_NOOP,
	OPID_Enqueue,
	OPID_Retrieve,
	OPID_RetrieveID,
	OPID_PeekFront,
	OPID_PeekID,
	OPID_PeekOffset,

	OPID_EnumerateQueues,
	OPID_EnumerateUsers,
	OPID_EnumerateGroups,

	OPID_CreateUser,
	OPID_DeleteUser,
	OPID_UserSetPermissions,
	OPID_UserGetPermissions,
	OPID_Setpasswd,
	OPID_CreateGroup,
	OPID_DeleteGroup,
	OPID_GroupSetPermissions,
	OPID_GroupGetPermissions,

	OPID_GroupAddUser,
	OPID_GroupDeleteUser,
	OPID_GroupGetUsers,
	OPID_UserGetGroups,
	OPID_CreateQueue,
	OPID_DeleteQueue,
	OPID_QueueSetUserPermission,
	OPID_QueueDeleteUserPermission,
	OPID_QueueSetGroupPermission,
	OPID_QueueDeleteGroupPermission,
	OPID_QueueEnumeratePermissions,

	OPID_QueueStatistics,

	OPID_HELP = 9900,

	OPID_USERID = 9000,
	OPID_PASSWD = 9001,
	OPID_TIMEOUT = 9002,
	OPID_TTL = 9003,
	OPID_LABEL = 9004,
	OPID_RESPONSE = 9005,
	OPID_RECIPTID = 9006,
	OPID_TTLERROR = 9007,
	OPID_PRIORITY = 9008,
	OPID_BODY = 9009,
	OPID_BODYONLY = 9010,
	OPID_KEY = 9011,
	OPID_CERT = 9012,
};



struct option	options[] = {
	{"user",1,0, OPID_USERID},
	{"passwd",1,0, OPID_PASSWD},
	{"help",0,0,OPID_HELP},

	{"timeout",1,0,OPID_TIMEOUT},
	{"ttl",1,0,OPID_TTL},
	{"label",1,0,OPID_LABEL},
	{"rq",1,0,OPID_RESPONSE},
	{"rid",1,0,OPID_RECIPTID},
	{"ttlerr",0,0,OPID_TTLERROR},
	{"priority",1,0,OPID_PRIORITY},
	{"body",0,0,OPID_BODY},
	{"bodyonly",0,0,OPID_BODYONLY},

#ifdef SAFMQ_SSL
	{"key",1,0,OPID_KEY},
	{"cert",1,0,OPID_CERT},
#endif

	{"enqueue",0,0,OPID_Enqueue},
	{"retrieve",0,0,OPID_Retrieve},
	{"retrieveid",0,0,OPID_RetrieveID},
	{"peekfront",0,0,OPID_PeekFront},
	{"peekid",0,0,OPID_PeekID},
	{"peekoffset",0,0,OPID_PeekOffset},

	{"enumeratequeues",0,0,OPID_EnumerateQueues},
	{"enumerateusers",0,0,OPID_EnumerateUsers},
	{"enumerategroups",0,0,OPID_EnumerateGroups},

	{"createuser",0,0,OPID_CreateUser},
	{"deleteuser",0,0,OPID_DeleteUser},
	{"usersetpermissions",0,0,OPID_UserSetPermissions},
	{"usergetpermissions",0,0,OPID_UserGetPermissions},
	{"setpassword",0,0,OPID_Setpasswd},
	{"creategroup",0,0,OPID_CreateGroup},
	{"deletegroup",0,0,OPID_DeleteGroup},
	{"groupsetpermissions",0,0,OPID_GroupSetPermissions},
	{"groupgetpermissions",0,0,OPID_GroupGetPermissions},

	{"groupadduser",0,0,OPID_GroupAddUser},
	{"groupdeleteuser",0,0,OPID_GroupDeleteUser},
	{"groupgetusers",0,0,OPID_GroupGetUsers},
	{"usergetgroups",0,0,OPID_UserGetGroups},
	{"createqueue",0,0,OPID_CreateQueue},
	{"deletequeue",0,0,OPID_DeleteQueue},
	{"queuesetuserpermission",0,0,OPID_QueueSetUserPermission},
	{"queuedeleteuserpermission",0,0,OPID_QueueDeleteUserPermission},
	{"queuesetgrouppermission",0,0,OPID_QueueSetGroupPermission},
	{"queuedeletegrouppermission",0,0,OPID_QueueDeleteGroupPermission},
	{"queueenumeratepermissions",0,0,OPID_QueueEnumeratePermissions},

	{"queuestatistics",0,0,OPID_QueueStatistics},

	{0,0,0,0}
};


const char* EC_Decode(ErrorCode ec) {
#define ECOUT(v) case v: return #v;
	switch(ec) {
		ECOUT(EC_NOERROR)
		ECOUT(EC_TIMEDOUT)
		ECOUT(EC_NOTAUTHORIZED)
		ECOUT(EC_ALREADYCLOSED)
		ECOUT(EC_DOESNOTEXIST)
		ECOUT(EC_NOTOPEN)
		ECOUT(EC_NETWORKERROR)
		ECOUT(EC_TTLEXPIRED)
		ECOUT(EC_CURSORINVALIDATED)
		ECOUT(EC_DUPLICATEMSGID)
		ECOUT(EC_SERVERUNAVAIL)
		ECOUT(EC_NOMOREMESSAGES)
		ECOUT(EC_FILESYSTEMERROR)
		ECOUT(EC_CANNOTCHANGEGROUP)
		ECOUT(EC_CANNOTCHANGEUSER)
		ECOUT(EC_ALREADYDEFINED)
		ECOUT(EC_NOTIMPLEMENTED)
		ECOUT(EC_QUEUEOPEN)
		ECOUT(EC_NOTLOGGEDIN)
		ECOUT(EC_ERROR)
		default: return "UNKNOWN";
	}
#undef ECOUT
}

#ifndef _WIN32
static bool				settings_stored=false;
static struct termios	stored_settings;

static void echo_off()
{
	struct termios new_settings;
	if (!settings_stored)
		tcgetattr(0,&stored_settings);
	new_settings = stored_settings;
	new_settings.c_lflag &= (~ECHO);
	tcsetattr(0,TCSANOW,&new_settings);
}

static void echo_on()
{
	tcsetattr(0,TCSANOW,&stored_settings);
}
#elif _WIN32
static DWORD	console_mode;
static bool		mode_set = false;

static void echo_off() 
{
	if (!mode_set)
		GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &console_mode);
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), console_mode & ~(ENABLE_ECHO_INPUT));
}

static void echo_on()
{
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), console_mode);
}
#endif



string getPasswd()
{
	string passwd;
	cout << "passwd: " << flush;

	echo_off();
	int c;
	while ( (c=fgetc(stdin)) != EOF && c!='\n') {
		passwd.append(1,(char)c);
	}
	echo_on();

	cout << endl;
	
	return passwd;
}

string getUser()
{
	char	username[128];
	*username = '\0';

#ifdef _WIN32
	DWORD	len = sizeof(username);
	GetUserName(username, &len);
#else
//	strncpy(username,getlogin(),sizeof(username));
//	username[sizeof(username)-1] = '\0';
//	cuserid(username);

	strncpy(username,getpwuid(geteuid())->pw_name,sizeof(username));
	username[sizeof(username)-1] = '\0';

#endif

	if (!*username) {
		int c;
		std::string	uname;

		cout << "login: " << flush;
		while ( (c=fgetc(stdin)) != EOF && c!='\n') {
			uname.append(1,(char)c);
		}
		return uname;
	}

	return username;
}

const char *priorities[] = {
	"STANDARD = 0",
	"MEDIUMLOW=1",
	"MEDIUM=2",
	"MEDIUMHIGH=3",
	"HIGH=4",
	"HIGHEST=5"
};


ErrorCode Enqueue(int argc, char* argv[])
{
	ErrorCode		result;
	char* url;
	if (argc < 1)
		return EC_ERROR;
	else {
		url = argv[0];
	}

	try {
		MessageQueue	*que = MQFactory::BuildQueueConnection(url,user,passwd);
		QueueMessage	msg;

		msg.setMessagePriority((MessagePriorityEnum)priority);
		msg.setLabel(label);
		msg.setTimeToLiveSeconds(ttl);
		msg.setTTLErrorWanted(ttlerr);
		msg.setResponseQueueName(response_q);
		msg.setReciptID(recipt_id);
		msg.setBodyType(BT_TEXT);

//		char	buffer[1024*20];
//		while (!cin.eof()) {
//			cin.read(buffer,sizeof(buffer));
//			msg.getBufferStream()->write(buffer,cin.gcount());
//		}
//		*msg.getBufferStream() << cin.rdbuf();

		cin >> msg.getBufferStream()->rdbuf();


		result = que->Enqueue(msg);
		if (result == EC_NOERROR) {
			cout << "Enqueue Message ID: " << msg.getMessageID() << endl;
			time_t t = msg.getTimeStamp();
			cout << "Enqueue Timestamp:  " << ctime(&t) << flush;
		} else {
			cerr << "Enqueue: " << EC_Decode(result) << endl;
		}
		delete que;
	} catch (std::exception& e) {
		cerr << "BuildQueueConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildQueueConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result; 
}


void printMessage(QueueMessage& msg)
{
	time_t t = msg.getTimeStamp();
	string	tmp;

	if (!bodyonly) {
		cout << "Label:            " << msg.getLabel() << endl;
		cout << "Message Class:    " << (msg.getMessageClass() == MC_SYSTEMERRMSG ? "SYSTEM" : "USER") << endl;
		cout << "Message Priority: " << priorities[msg.getMessagePriority()] << endl;
		cout << "Timestamp:        " << ctime(&t) << flush;
		cout << "Message ID:       " << msg.getMessageID() << endl;
		cout << "Recipt ID:        " << msg.getReciptID() << endl;
		msg.getResponseQueueName(tmp);
		cout << "Response Queue:   " << tmp << endl;
	}
	if (bodyonly || body) {
		if (!bodyonly) cout << "Body:" << endl;
		cout << msg.getBufferStream()->rdbuf() << endl;
	}
}

ErrorCode Retrieve(int argc, char* argv[])
{ 
	ErrorCode		result;
	char* url;
	if (argc < 1) {
		cerr << "Retrieve requires a safmq URL" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
	}

	try {
		MessageQueue	*que = MQFactory::BuildQueueConnection(url,user,passwd);
		QueueMessage	msg;
	
		result = que->Retrieve(body,timeout,msg);

		if (result == EC_NOERROR) {
			printMessage(msg);
		} else if (result == EC_NOMOREMESSAGES) {
			cout << "Retrieve: No messages available" << endl;
		} else if (result == EC_TIMEDOUT) {
			cout << "Retrieve: Timedout" << endl;
		}
		delete que;
	} catch (std::exception& e) {
		cerr << "BuildQueueConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildQueueConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result; 
}

ErrorCode RetrieveID(int argc, char* argv[])
{ 
	ErrorCode		result;
	char*	url;
	uuid	id;
	if (argc < 2) {
		cerr << "RetrieveID requires a safmq URL and UUID" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		stringstream	s(argv[1]);
		s >> id;
	}

	try {
		MessageQueue	*que = MQFactory::BuildQueueConnection(url,user,passwd);
		QueueMessage	msg;
	
		result = que->RetrieveID(body,id,timeout,msg);

		if (result == EC_NOERROR) {
			printMessage(msg);
		} else if (result == EC_NOMOREMESSAGES) {
			cout << "RetrieveID: No messages available" << endl;
		} else if (result == EC_TIMEDOUT) {
			cout << "RetrieveID: Timedout" << endl;
		}
		delete que;
	} catch (std::exception& e) {
		cerr << "BuildQueueConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildQueueConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result; 
}

ErrorCode PeekFront(int argc, char* argv[])
{ 
	ErrorCode		result;
	char* url;
	if (argc < 1) {
		cerr << "PeekFront requires a safmq URL" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
	}

	try {
		MessageQueue	*que = MQFactory::BuildQueueConnection(url,user,passwd);
		QueueMessage	msg;
	
		result = que->PeekFront(body,timeout,msg);

		if (result == EC_NOERROR) {
			printMessage(msg);
		} else if (result == EC_NOMOREMESSAGES) {
			cout << "PeekFront: No messages available" << endl;
		} else if (result == EC_TIMEDOUT) {
			cout << "PeekFront: Timedout" << endl;
		}
		delete que;
	} catch (std::exception& e) {
		cerr << "BuildQueueConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildQueueConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result; 
}

ErrorCode PeekID(int argc, char* argv[])
{ 
	ErrorCode		result;
	char*	url;
	uuid	id;
	if (argc < 2) {
		cerr << "PeekID requires a safmq URL and UUID" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		stringstream	s(argv[1]);
		s >> id;
	}

	try {
		MessageQueue	*que = MQFactory::BuildQueueConnection(url,user,passwd);
		QueueMessage	msg;
	
		result = que->PeekID(body,id,timeout,msg);

		if (result == EC_NOERROR) {
			printMessage(msg);
		} else if (result == EC_NOMOREMESSAGES) {
			cout << "PeekID: No messages available" << endl;
		} else if (result == EC_TIMEDOUT) {
			cout << "PeekID: Timedout" << endl;
		}
		delete que;
	} catch (std::exception& e) {
		cerr << "BuildQueueConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildQueueConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result; 
}

ErrorCode PeekOffset(int argc, char* argv[])
{ 
	ErrorCode		result = EC_NOERROR;
	char*	url;
	int		offset;

	if (argc < 2) {
		cerr << "PeekOffset requires a safmq URL and OFFSET" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		offset = atoi(argv[1]);
	}

	if (offset < 0) {
		cerr << "Offset must be greater than 0" << endl;
		return EC_ERROR;
	}

	try {
		MQConnection::CursorHandle	cur;
		MessageQueue				*que = MQFactory::BuildQueueConnection(url,user,passwd);
		QueueMessage				msg;
		
		que->OpenCursor(cur);

		for(int x=0; x<offset && result == EC_NOERROR; x++)
			result = que->AdvanceCursor(cur);

		if (result == EC_NOERROR) {
			result = que->PeekCursor(body,cur,msg);
			if (result == EC_NOERROR)
				printMessage(msg);
		}
		
		if (result == EC_NOMOREMESSAGES) {
			cout << "PeekOffset: No messages available" << endl;
		} else if (result == EC_TIMEDOUT) {
			cout << "PeekOffset: Timedout" << endl;
		}
		delete que;
	} catch (std::exception& e) {
		cerr << "BuildQueueConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildQueueConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result; 
}


ErrorCode EnumerateQueues(int argc, char* argv[])
{
	ErrorCode									result;
	std::vector<MQConnection::QData>			qnames;
	std::vector<MQConnection::QData>::iterator	i;
	char*										url;

	if (argc >= 1)
		url = argv[0];
	else {
		cerr << "EnumerateQueues requires a safmq URL" << endl;
		return EC_ERROR;
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->EnumerateQueues(qnames);
		if (result == EC_NOERROR) {
			for(i=qnames.begin(); i!=qnames.end();i++) {
				cout << "queue: "<< i->queuename << endl;
			}
		} else {
			cerr << "EnuemrateQueues: " << EC_Decode(result) << endl;
		}
		delete con;
	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}

	return result;
}

ErrorCode EnumerateUsers(int argc, char* argv[])
{
	ErrorCode							result;
	MQConnection::USER_VECTOR			users;
	MQConnection::USER_VECTOR::iterator	i;
	char*								url;

	if (argc >= 1)
		url = argv[0];
	else {
		cerr << "EnumerateQueues requires a safmq URL" << endl;
		return EC_ERROR;
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->EnumerateUsers(users);

		if (result == EC_NOERROR) {
			for(i=users.begin(); i!=users.end(); i++) {
				cout << "user: "<< i->name << "/" << i->description << endl;
			}
		} else {
			cerr << "EnuemrateQueues: " << EC_Decode(result) << endl;
		}
		delete con;
	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}

	return result;
}

ErrorCode EnumerateGroups(int argc, char* argv[])
{
	ErrorCode							result;
	MQConnection::NAME_VECTOR			names;
	MQConnection::NAME_VECTOR::iterator	i;
	char*								url;

	if (argc >= 1)
		url = argv[0];
	else {
		cerr << "EnumerateQueues requires a safmq URL" << endl;
		return EC_ERROR;
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->EnumerateGroups(names);

		if (result == EC_NOERROR) {
			for(i=names.begin(); i!=names.end(); i++) {
				cout << "group: "<< *i << endl;
			}
		} else {
			cerr << "EnuemrateGroups: " << EC_Decode(result) << endl;
		}
		delete con;
	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}

	return result;
}

ErrorCode CreateUser(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *username, *password, *description;

	if (argc < 4) {
		cerr << "CreateUser requires safmq URL, username, passwd, and description" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		username = argv[1];
		password = argv[2];
		description = argv[3];
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->CreateUser(username,password,description);
						
		if (result != EC_NOERROR) {
			cerr << "CreateUser: " << EC_Decode(result) << endl;
		}
		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode DeleteUser(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *username;

	if (argc < 2) {
		cerr << "DeleteUser requires safmq URL, and username" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		username = argv[1];
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->DeleteUser(username);
						
		if (result != EC_NOERROR) {
			cerr << "DeleteUser: " << EC_Decode(result) << endl;
		}
		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode UserSetPermissions(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *username;
	bool		queues=false, users=false, groups=false;

	if (argc < 2) {
		cerr << "UserSetPermissions requires safmq URL, and username with optional queues, users, and/or groups" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		username = argv[1];

		for(int x=2; x<argc; x++) {
			if (STRICMP(argv[x],"queues") == 0)
				queues = true;
			else if (STRICMP(argv[x],"users") == 0)
				users = true;
			else if (STRICMP(argv[x],"groups") == 0)
				groups = true;
		}
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->UserSetPermissions(username,queues,users,groups);
						
		if (result != EC_NOERROR) {
			cerr << "UserSetPermissions: " << EC_Decode(result) << endl;
		}
		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode UserGetPermissions(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *username;
	bool		queues=false, users=false, groups=false;

	if (argc < 2) {
		cerr << "UserGetPermissions requires safmq URL, and username" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		username = argv[1];
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->UserGetPermissions(username,queues,users,groups);
						
		if (result != EC_NOERROR) {
			cerr << "UserGetPermissions: " << EC_Decode(result) << endl;
		}
		
		cout << "user permissions: " ;
		if (queues) cout << " queues";
		if (users) cout << " users";
		if (groups) cout << " groups";
		cout << endl;

		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode Setpasswd(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *username, *password;
	bool		queues=false, users=false, groups=false;

	if (argc < 3) {
		cerr << "Setpasswd requires safmq URL, username, and passwd" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		username = argv[1];
		password = argv[2];
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->SetPassword(username,password);
						
		if (result != EC_NOERROR) {
			cerr << "Setpasswd: " << EC_Decode(result) << endl;
		}
		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode CreateGroup(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *groupname;

	if (argc < 2) {
		cerr << "CreateGroup requires safmq URL, groupname" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		groupname = argv[1];
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->CreateGroup(groupname);
						
		if (result != EC_NOERROR) {
			cerr << "CreateGroup: " << EC_Decode(result) << endl;
		}
		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode DeleteGroup(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *groupname;

	if (argc < 2) {
		cerr << "DeleteGroup requires safmq URL, and groupname" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		groupname = argv[1];
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->DeleteGroup(groupname);
						
		if (result != EC_NOERROR) {
			cerr << "DeleteGroup: " << EC_Decode(result) << endl;
		}
		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode GroupSetPermissions(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *groupname;
	bool		queues=false, users=false, groups=false;

	if (argc < 2) {
		cerr << "GroupSetPermissions requires safmq URL, and groupname with optional queues, users, and/or groups" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		groupname = argv[1];

		for(int x=2; x<argc; x++) {
			if (STRICMP(argv[x],"queues") == 0)
				queues = true;
			else if (STRICMP(argv[x],"users") == 0)
				users = true;
			else if (STRICMP(argv[x],"groups") == 0)
				groups = true;
		}
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->GroupSetPermissions(groupname,queues,users,groups);
						
		if (result != EC_NOERROR) {
			cerr << "GroupSetPermissions: " << EC_Decode(result) << endl;
		}
		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode GroupGetPermissions(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *groupname;
	bool		queues=false, users=false, groups=false;

	if (argc < 2) {
		cerr << "GroupGetPermissions requires safmq URL, and groupname" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		groupname = argv[1];
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->GroupGetPermissions(groupname,queues,users,groups);
						
		if (result != EC_NOERROR) {
			cerr << "GroupGetPermissions: " << EC_Decode(result) << endl;
		}
		
		cout << "group permissions: " ;
		if (queues) cout << " queues";
		if (users) cout << " users";
		if (groups) cout << " groups";
		cout << endl;

		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode GroupAddUser(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *groupname, *username;
	bool		queues=false, users=false, groups=false;

	if (argc < 3) {
		cerr << "GroupAddUser requires safmq URL, groupname, and username" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		groupname = argv[1];
		username = argv[2];
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->GroupAddUser(groupname,username);
						
		if (result != EC_NOERROR) {
			cerr << "GroupAddUser: " << EC_Decode(result) << endl;
		}

		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode GroupDeleteUser(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *groupname, *username;
	bool		queues=false, users=false, groups=false;

	if (argc < 3) {
		cerr << "GroupDeleteUser requires safmq URL, groupname, and username" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		groupname = argv[1];
		username = argv[2];
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->GroupDeleteUser(groupname,username);
						
		if (result != EC_NOERROR) {
			cerr << "GroupDeleteUser: " << EC_Decode(result) << endl;
		}

		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode GroupGetUsers(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *groupname;

	if (argc < 2) {
		cerr << "GroupGetUsers requires safmq URL, and groupname" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		groupname = argv[1];
	}

	try {
		MQConnection::NAME_VECTOR	names;
		MQConnection::NAME_VECTOR::iterator i;

		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->GroupGetUsers(groupname, names);
						
		if (result != EC_NOERROR) {
			cerr << "GroupGetUsers: " << EC_Decode(result) << endl;
		}

		for(i=names.begin(); i!=names.end(); i++) {
			cout << "user: " << *i << endl;
		}

		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode UserGetGroups(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *username;

	if (argc < 2) {
		cerr << "UserGetGroups requires safmq URL, and username" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		username = argv[1];
	}

	try {
		std::vector<std::string>			groups;
		std::vector<std::string>::iterator	i;

		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->UserGetGroups(username, groups);
						
		if (result != EC_NOERROR) {
			cerr << "UserGetGroups: " << EC_Decode(result) << endl;
		}

		for(i=groups.begin(); i!=groups.end(); i++) {
			cout << "group: " << *i << endl;
		}

		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode CreateQueue(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *queuename;

	if (argc < 2) {
		cerr << "CreateQueue requires safmq URL, queuename" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		queuename = argv[1];
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->CreateQueue(queuename);
						
		if (result != EC_NOERROR) {
			cerr << "CreateQueue: " << EC_Decode(result) << endl;
		}
		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode DeleteQueue(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *queuename;

	if (argc < 2) {
		cerr << "DeleteQueue requires safmq URL, queuename" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		queuename = argv[1];
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->DeleteQueue(queuename);
						
		if (result != EC_NOERROR) {
			cerr << "DeleteQueue: " << EC_Decode(result) << endl;
		}
		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode QueueSetUserPermission(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *queuename, *username;
	bool		bread = false, bwrite = false, bdestroy=false, bchange=false;

	if (argc < 3) {
		cerr << "QueueSetUserPermission requires safmq URL, queuename, and username with optional read,write,destroy,change" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		queuename = argv[1];
		username = argv[2];

		for(int x=2; x<argc; x++) {
			if (STRICMP(argv[x],"read") == 0)
				bread = true;
			else if (STRICMP(argv[x],"write") == 0)
				bwrite = true;
			else if (STRICMP(argv[x],"destroy") == 0)
				bdestroy = true;
			else if (STRICMP(argv[x],"change") == 0)
				bchange = true;
		}
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->QueueSetUserPermission(queuename,username,bread,bwrite,bdestroy,bchange);
						
		if (result != EC_NOERROR) {
			cerr << "UserSetPermissions: " << EC_Decode(result) << endl;
		}
		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode QueueDeleteUserPermission(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *queuename, *username;

	if (argc < 3) {
		cerr << "QueueDeleteUserPermission requires safmq URL, queuename, and username" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		queuename = argv[1];
		username = argv[2];
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->QueueDeleteUserPermission(queuename,username);
						
		if (result != EC_NOERROR) {
			cerr << "QueueDeleteUserPermission: " << EC_Decode(result) << endl;
		}
		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode QueueSetGroupPermission(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *queuename, *groupname;
	bool		bread = false, bwrite = false, bdestroy=false, bchange=false;

	if (argc < 3) {
		cerr << "QueueSetGroupPermission requires safmq URL, queuename, and groupname with optional read,write,destroy,bchange" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		queuename = argv[1];
		groupname = argv[2];

		for(int x=2; x<argc; x++) {
			if (STRICMP(argv[x],"read") == 0)
				bread = true;
			else if (STRICMP(argv[x],"write") == 0)
				bwrite = true;
			else if (STRICMP(argv[x],"destroy") == 0)
				bdestroy = true;
			else if (STRICMP(argv[x],"change") == 0)
				bchange = true;
		}
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->QueueSetGroupPermission(queuename,groupname,bread,bwrite,bdestroy,bchange);
						
		if (result != EC_NOERROR) {
			cerr << "GroupSetPermissions: " << EC_Decode(result) << endl;
		}
		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode QueueDeleteGroupPermission(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *queuename, *groupname;

	if (argc < 3) {
		cerr << "QueueDeleteGroupPermission requires safmq URL, queuename, and groupname" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		queuename = argv[1];
		groupname = argv[2];
	}

	try {
		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->QueueDeleteGroupPermission(queuename,groupname);
						
		if (result != EC_NOERROR) {
			cerr << "QueueDeleteGroupPermission: " << EC_Decode(result) << endl;
		}
		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode QueueEnumeratePermissions(int argc, char* argv[])
{
	ErrorCode	result;
	char		*url, *queuename;

	if (argc < 2) {
		cerr << "QueueEnumeratePermissions requires safmq URL, and queuename" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
		queuename = argv[1];
	}

	try {
		std::vector<QueuePermissions>	perms;
		std::vector<QueuePermissions>::iterator i;

		MQConnection *con = MQFactory::BuildConnection(url,user,passwd,&ctx);
		result = con->QueueEnumeratePermissions(queuename, perms);
						
		if (result != EC_NOERROR) {
			cerr << "QueueEnumeratePermissions: " << EC_Decode(result) << endl;
		}

		for(i=perms.begin(); i!=perms.end(); i++) {
			cout << "queue perm: ";
			cout << (i->isgroup?"group/":"user/") << i->entity;
			if (i->read) cout << " read";
			if (i->write) cout << " write";
			if (i->destroy) cout << " destroy";
			if (i->change_security) cout << " change";
			cout << endl;
		}

		delete con;

	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}
	return result;
}

ErrorCode GetQueueStatistics(int argc, char* argv[])
{
	ErrorCode result = EC_NOERROR;
	std::string url;
	if (argc < 1) {
		cerr << "QueueStatistics requires safmq URL" << endl;
		return EC_ERROR;
	} else {
		url = argv[0];
	}

	try {
		MessageQueue* queue = MQFactory::BuildQueueConnection(url, user, passwd, &ctx);
		QueueStatistics stats;
		
		result = queue->GetQueueStatistics(true, true, stats);
			

		if (result != EC_NOERROR) {
			cerr << "QueueStatistics: " << EC_Decode(result);
		} else {
			cout << "Queue:          " << url << endl;
			cout << "Message Count:  " << setw(10) << setfill(' ') << stats.messageCount << endl;
			cout << "Bytes on Disk:  " << setw(10) << setfill(' ') << stats.storageBytes << endl;
			cout << "Bytes in Queue: " << setw(10) << setfill(' ') << stats.messageBytes << endl;
			cout << "                    10 Sec    60 Sec    300 Sec" << endl;
			cout << "Enqueued:       ";
			cout << setw(10) << setfill(' ') << stats.enqueued10second;
			cout << setw(10) << setfill(' ') << stats.enqueued60second;
			cout << setw(10) << setfill(' ') << stats.enqueued300second;
			cout << endl << "Retrieved:      ";
			cout << setw(10) << setfill(' ') << stats.retrieved10second;
			cout << setw(10) << setfill(' ') << stats.retrieved60second;
			cout << setw(10) << setfill(' ') << stats.retrieved300second;
			cout << endl << "Peeked:         ";
			cout << setw(10) << setfill(' ') << stats.peeked10second;
			cout << setw(10) << setfill(' ') << stats.peeked60second;
			cout << setw(10) << setfill(' ') << stats.peeked300second;
			cout << endl;
		}

		delete queue;
	} catch (std::exception& e) {
		cerr << "BuildConnection: " << e.what() << endl;
		result = EC_ERROR;
	} catch (ErrorCode e) {
		cerr << "BuildConnection: received error code: " << EC_Decode(e) << endl;
		result = e;
	}

	return result;
}


void usage()
{
	cerr << "safmqc usage:\n" << endl;

	cerr << "safmq [options] <command> <command parameters>" <<endl;
	cerr << "   Options:" << endl;
	cerr << "        --help,-h           : Displays this screen" << endl;
	cerr << "        --user=login        : Sets the login user to 'login'"<< endl;
	cerr << "        --passwd=passwd     : Sets the login passwd to 'passwd'" << endl;
	cerr << "        --body              : Flags safmq to retrieve the message body" << endl;
	cerr << "        --timeout=seconds   : Sets the timeout to 'seconds' -1 for infinite" << endl;
	cerr << "        --ttl=seconds       : Sets the message time to live to 'seconds'" << endl;
	cerr << "        --ttlerr            : Flags ttl errors to be returned on 'response'" << endl;
	cerr << "        --label=lbl         : Sets the message label to 'lbl'" << endl;
	cerr << "        --rq=response       : Sets the response queue to 'response'" << endl;
	cerr << "        --rid=id            : Sets the recipt id to 'id'" << endl;
	cerr << "        --priority=num      : Sets the priority to 'num' in [0..5]" << endl;
#ifdef SAFMQ_SSL
	cerr << "        --key=file.pem      : Sets 'file.pem' as private key pem file" << endl;
	cerr << "        --cert=file.pem     : Sets 'file.pem' as certificate pem file" << endl;
#endif
	cerr << endl;
	cerr << "   Commands:" << endl;
	cerr << "        --enqueue URL           : Enqueues a message" << endl;
	cerr << "        --retrieve URL          : Retrieves a message" << endl;
	cerr << "        --retrieveid URL ID     : Retrieves a message by ID" << endl;
	cerr << "        --peekfront URL         : Peeks the first message" << endl;
	cerr << "        --peekid URL ID         : Peeks a message reffered by ID" << endl;
	cerr << "        --peekoffset URL OFFSET : Peeks a message OFFSET from front" << endl;
	cerr << endl;
	cerr << "        --enumeratequeues URL : Lists all queues" << endl;
	cerr << "        --enumerateusers URL  : Lists all users" << endl;
	cerr << "        --enumerategroups URL : Lists all groups" << endl;
	cerr << endl;
	cerr << "   User Management Commands:" << endl;
	cerr << "        --createuser URL USERNAME passwd DESCRIPTION" << endl;
	cerr << "        --deleteuser URL USERNAME" << endl;
	cerr << "        --usersetpermissions URL USERNAME [users] [groups] [queues]" << endl;
	cerr << "        --usergetpermissions URL USERNAME" << endl;
	cerr << "        --setpassword URL USERNAME passwd" << endl;
	cerr << "        --usergetgroups URL USERNAME" << endl;
	cerr << endl;
	cerr << "   Group Management Commands:" << endl;
	cerr << "        --creategroup URL GROUPNAME" << endl;
	cerr << "        --deletegroup URL GROUPANME" << endl;
	cerr << "        --groupsetpermissions URL GROUPNAME [users] [groups] [queues]" << endl;
	cerr << "        --groupgetpermissions URL GROUPNAME" << endl;
	cerr << "        --groupadduser URL GROUPNAME USERNAME" << endl;
	cerr << "        --groupdeleteuser URL GROUPNAME USERNAME" << endl;
	cerr << "        --groupgetusers URL GROUPNAME" << endl;
	cerr << endl;
	cerr << "   Queue Management Commands:" << endl;
	cerr << "        --createqueue URL QUEUENAME" << endl;
	cerr << "        --deletequeue URL QUEUENAME" << endl;
	cerr << "        --queuesetuserpermission URL QUEUENAME USERNAME [read] [write] [destroy] [change]" << endl;
	cerr << "        --queuedeleteuserpermission URL QUEUENAME USERNAME " << endl;
	cerr << "        --queuesetgrouppermission URL QUEUENAME GROUPNAME [read] [write] [destroy] [change]" << endl;
	cerr << "        --queuedeletegrouppermission URL QUEUENAME GROUPNAME " << endl;
	cerr << "        --queueenumeratepermissions URL QUEUENAME" << endl;
	cerr << "        --queuestatistics URL" << endl;
	cerr << endl;
	cerr << "URL Format:  safmq://user:passwd@host:port/queue" << endl;
#ifdef SAFMQ_SSL
	cerr << "             safmqs://user:passwd@host:port/queue -- For SSL Connections" << endl;
#endif
	cerr << endl;
	cerr << "Note: queue, port, user and passwd are optional, if not supplied\n"
			"      defaults will be used and the User name will be gathered\n"
			"      from the system.\n" << endl;
	cerr << endl;
}


int main (int argc, char* argv[])
{
	bool	passwdset = false;
	OPID	opid = OPID_NOOP;
	int		theopt;

	string	private_key, certificate;

	// Load the command line options
	while ( (theopt=getopt_long(argc,argv,"h",options,NULL)) != -1) {
		switch (theopt) {
			case '?': {
				cerr << "Unknown option: " << argv[optind-1] << endl;
				cerr << endl;
				usage();
				return -1;
			}

			case OPID_USERID:
				user = optarg;
				break;

			case OPID_PASSWD:
				passwd = optarg;
				passwdset = true;
				break;

			case 'h':
			case OPID_HELP:
				usage();
				return 0;

			case OPID_TIMEOUT:
				timeout = atoi(optarg);
				break;

			case OPID_TTL:
				ttl = atoi(optarg);
				break;

			case OPID_LABEL:
				label = optarg;
				break;

			case OPID_RESPONSE:
				response_q = optarg;
				break;

			case OPID_RECIPTID: {
				stringstream	s(optarg);
				s >> recipt_id;
				break;
			}

			case OPID_TTLERROR:
				ttlerr = true;
				break;

			case OPID_PRIORITY:
				priority = atoi(optarg);
				if (priority < MP_LOW || priority > MP_HIGHEST)
					priority = MP_STANDARD;
				break;

			case OPID_BODY:
				body = true;
				break;

			case OPID_BODYONLY:
				body = true;
				bodyonly = true;
				break;
			
			case OPID_KEY:
				private_key = optarg;
				break;

			case OPID_CERT:
				certificate = optarg;
				break;

			case -1:
				break;

			default:
				opid = (OPID)theopt;
				break;
		}
	}

	if (opid == OPID_NOOP) {
		usage();
		return -1;
	}

	// Check to see if the passwd was set in the URL.
	try {
		if (!passwdset && optind < argc) {
			URL::urldecode	url(argv[optind]);
			passwdset = url.isPasswordSpecified();
		}
	} catch (URL::urlexception) {}

	// Obtain the user name if it wasn't set
	if (!user.length())
		user = getUser();

	// Obtain the passwd if it wasn't specified
	if (!passwdset)
		passwd = getPasswd();


#ifdef SAFMQ_SSL
	PrivateKey		pk;
	X509Certificate	cert;
	if (private_key.length()) {
		pk.loadPEMFile(private_key);
		ctx.setPrivateKey(&pk);
	}
	if (certificate.length()) {
		cert.loadPEMFile(certificate);
		ctx.setX509Certificate(&cert);
	}
#endif


	// Switch on the operation
	switch (opid) {
		case OPID_Enqueue: return Enqueue(argc-optind,argv+optind);
		case OPID_Retrieve: return Retrieve(argc-optind,argv+optind);
		case OPID_RetrieveID: return RetrieveID(argc-optind,argv+optind);
		case OPID_PeekFront: return PeekFront(argc-optind,argv+optind);
		case OPID_PeekID: return PeekID(argc-optind,argv+optind);
		case OPID_PeekOffset: return PeekOffset(argc-optind,argv+optind);

		case OPID_EnumerateQueues: return EnumerateQueues(argc-optind,argv+optind);
		case OPID_EnumerateUsers: return EnumerateUsers(argc-optind,argv+optind);
		case OPID_EnumerateGroups: return EnumerateGroups(argc-optind,argv+optind);

		case OPID_CreateUser: return CreateUser(argc-optind,argv+optind);
		case OPID_DeleteUser: return DeleteUser(argc-optind,argv+optind);

		case OPID_UserSetPermissions: return UserSetPermissions(argc-optind,argv+optind);
		case OPID_UserGetPermissions: return UserGetPermissions(argc-optind,argv+optind);
		case OPID_Setpasswd: return Setpasswd(argc-optind,argv+optind);
		case OPID_CreateGroup: return CreateGroup(argc-optind,argv+optind);
		case OPID_DeleteGroup: return DeleteGroup(argc-optind,argv+optind);
		case OPID_GroupSetPermissions: return GroupSetPermissions(argc-optind,argv+optind);
		case OPID_GroupGetPermissions: return GroupGetPermissions(argc-optind,argv+optind);

		case OPID_GroupAddUser: return GroupAddUser(argc-optind,argv+optind);
		case OPID_GroupDeleteUser: return GroupDeleteUser(argc-optind,argv+optind);
		case OPID_GroupGetUsers: return GroupGetUsers(argc-optind,argv+optind);
		case OPID_UserGetGroups: return UserGetGroups(argc-optind,argv+optind);
		case OPID_CreateQueue: return CreateQueue(argc-optind,argv+optind);
		case OPID_DeleteQueue: return DeleteQueue(argc-optind,argv+optind);
		case OPID_QueueSetUserPermission: return QueueSetUserPermission(argc-optind,argv+optind);
		case OPID_QueueDeleteUserPermission: return QueueDeleteUserPermission(argc-optind,argv+optind);
		case OPID_QueueSetGroupPermission: return QueueSetGroupPermission(argc-optind,argv+optind);
		case OPID_QueueDeleteGroupPermission: return QueueDeleteGroupPermission(argc-optind,argv+optind);
		case OPID_QueueEnumeratePermissions: return QueueEnumeratePermissions(argc-optind,argv+optind);
		case OPID_QueueStatistics: return GetQueueStatistics(argc-optind,argv+optind);
		default:
			;
	}

	return 0;
}

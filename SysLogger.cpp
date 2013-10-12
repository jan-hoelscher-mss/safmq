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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include "SysLogger.h"
#include <syslog.h>

using namespace safmq;
using namespace std;

SysLogger::SysLogger(const string& ident)
{
	this->ident = ident;
	::openlog(this->ident.c_str(),  LOG_ODELAY|LOG_PID, LOG_USER);
}

SysLogger::~SysLogger()
{
	::closelog();
}

void SysLogger::Startup(Log::EventModifier modifier, va_list args)
{
	if (modifier == Log::error) {
		const char*	szErr = va_arg(args, const char*);
		::syslog(LOG_ERR, "Startup:%s", szErr);
	} else  {
		::syslog(LOG_INFO, "Startup");
	}
}

void SysLogger::Shutdown(Log::EventModifier modifier, va_list args)
{
	if (modifier == Log::error) {
		const char*	szErr = va_arg(args, const char*);
		::syslog(LOG_ERR, "Shutdown:%s", szErr);
	} else  {
		::syslog(LOG_INFO, "Shutdown");
	}
}

void SysLogger::SignOn(Log::EventModifier modifier, const char* userName, const struct sockaddr_in* paddr, va_list args)
{
	if (modifier == Log::error) {
		char*	szErr = va_arg(args, char*);
		syslog(LOG_WARNING, "SignOn:error:%s:%s:%s:%ld", userName, szErr, inet_ntoa(paddr->sin_addr), ntohs(paddr->sin_port));
	} else {
		syslog(LOG_NOTICE, "SignOn:success:%s:%s:%ld", userName, inet_ntoa(paddr->sin_addr), ntohs(paddr->sin_port));
	}
}

void SysLogger::Info(const char* format, va_list args)
{
	char buf[1024];

	strcpy(buf, "Info:");
	strcat(buf, format);
	vsyslog(LOG_INFO, format, args);
}

void SysLogger::User(Log::EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* userName, va_list args)
{
	if (modifier == Log::created) {
		const char* desc = va_arg(args, const char*);
		syslog(LOG_NOTICE,"User:create:success:%s:%s:%s:%s:%ld", userName, desc, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
	} else if (modifier == Log::deleted) {
		syslog(LOG_NOTICE,"User:deleted:success:%s:%s:%s:%ld", userName, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
	} else if (modifier == Log::acl) {
		int		queues = va_arg(args, int);
		int		users = va_arg(args, int);
		int		groups = va_arg(args, int);
		syslog(LOG_NOTICE,"User:acl:success:%s:%s %s %s:%s:%s:%ld", userName, 
			queues ? "queues":"",
			users ? "users":"",
			groups ? "groups":"",
			actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
	} else if (modifier == Log::error) {
		const char* action = va_arg(args, const char*);
		const char* code = va_arg(args, const char*);
		syslog(LOG_NOTICE,"User:%s:error:%s:%s:%s:%s:%ld", action, userName, code, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
	}
}

void SysLogger::Group(Log::EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* groupName, va_list args)
{
	if (modifier == Log::created) {
		syslog(LOG_NOTICE,"Group:create:success:%s:%s:%s:%ld", groupName, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
	} else if (modifier == Log::deleted) {
		syslog(LOG_NOTICE,"Group:deleted:success:%s:%s:%s:%ld", groupName, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
	} else if (modifier == Log::useradd) {
		const char* username = va_arg(args, const char*);
		syslog(LOG_NOTICE,"Group:add user:success:%s:%s:%s:%s:%ld", groupName, username, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
	} else if (modifier == Log::userdel) {
		const char* username = va_arg(args, const char*);
		syslog(LOG_NOTICE,"Group:delete user:success:%s:%s:%s:%s:%ld", groupName, username, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
	} else if (modifier == Log::acl) {
		int		queues = va_arg(args, int);
		int		users = va_arg(args, int);
		int		groups = va_arg(args, int);
		syslog(LOG_NOTICE,"Group:acl:success:%s:%s %s %s:%s:%s:%ld", groupName, 
			queues ? "queues":"",
			users ? "users":"",
			groups ? "groups":"",
			actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
	} else if (modifier == Log::error) {
		const char* action = va_arg(args, const char*);
		const char* code = va_arg(args, const char*);
		syslog(LOG_NOTICE,"Group:%s:error:%s:%s:%s:%s:%ld", action, groupName, code, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
	}
}

void SysLogger::Queue(Log::EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* queueName, va_list args)
{
	if (modifier == Log::created) {
		syslog(LOG_NOTICE,"Queue:create:success:%s:%s:%s:%ld", queueName, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
	} else if (modifier == Log::deleted) {
		syslog(LOG_NOTICE,"Queue:deleted:success:%s:%s:%s:%ld", queueName, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
	} else if (modifier == Log::acl) {
		const char* type = va_arg(args, const char*);
		const char* name = va_arg(args, const char*);
		int read = va_arg(args, int);
		int write = va_arg(args, int);
		int destroy = va_arg(args, int);
		int security = va_arg(args, int);
		syslog(LOG_NOTICE,"Queue:acl:success:%s:%s:%s:%s %s %s %s:%s:%s:%ld",
			queueName, 
			type,
			name,
			read?"read":"",
			write?"write":"",
			destroy?"destroy":"",
			security?"security":"",
			actor,
			inet_ntoa(peer->sin_addr),
			ntohs(peer->sin_port));
	} else if (modifier == Log::aclclear) {
		const char* type = va_arg(args, const char*);
		const char* name = va_arg(args, const char*);
		syslog(LOG_NOTICE,"Queue:acl clear:success:%s:%s:%s:%s:%s:%ld",
			queueName, 
			type,
			name,
			actor,
			inet_ntoa(peer->sin_addr),
			ntohs(peer->sin_port));
	} else if (modifier == Log::error) {
		const char* action = va_arg(args, const char*);
		const char* code = va_arg(args, const char*);
		syslog(LOG_NOTICE,"Queue:%s:error:%s:%s:%s:%s:%ld", action, queueName, code, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
	}
}

void SysLogger::Message(Log::EventModifier modifier, va_list args)
{
	if (modifier == Log::error) {
		const char* msg = va_arg(args, const char*);
		const char* url = va_arg(args, const char*);
		uuid*		u = va_arg(args, uuid*);

		stringstream	guid;
		guid << *u << ends;
		syslog(LOG_ERR,"Message:error:%s:%s:%s", msg, url, guid.str().c_str());
	} else if (modifier == Log::ttl) {
		const char*		queuename = va_arg(args, const char*);
		QueueMessage	*msg = va_arg(args, QueueMessage*);
		stringstream	guid;

		guid << msg->getMessageID() << ends;
		syslog(LOG_ERR,"Message:ttl:%s:%s:%s", queuename, msg->getLabel().c_str(), guid.str().c_str());
	}
}

void SysLogger::Forward(Log::EventModifier modifier, va_list args)
{
	if (modifier == Log::error) {
		const char* code = va_arg(args, const char*);
		const char* url = va_arg(args, const char*);
		syslog(LOG_ERR,"Forward:error:%s:%s", code, url);
	}
}

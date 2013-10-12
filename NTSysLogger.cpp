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
#include "NTSysLogger.h"
#include <sstream>
#pragma warning(disable:4996) // Disable warnings about deprecated functions

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace safmq;
using namespace std;

namespace safmq {

	struct NTSysLoggerData {
		NTSysLoggerData() {

		}

		HANDLE	log;
	};
}

NTSysLogger::NTSysLogger()
{
	pdata = new NTSysLoggerData;
	pdata->log = ::RegisterEventSource(NULL, "SAFMQ");
}

NTSysLogger::~NTSysLogger()
{
	::DeregisterEventSource(pdata->log);

	delete pdata;
}

void NTSysLogger::Startup(Log::EventModifier modifier, va_list args)
{
	if (pdata->log) {
		if (modifier == Log::error) {
			const char*  strings[1];

			strings[0] = va_arg(args, const char*);

			::ReportEvent(pdata->log,
					EVENTLOG_ERROR_TYPE,
					Log::startup,
					(Log::startup << 8) | modifier,
					NULL,
					1,
					0,
					strings,
					NULL);
		} else {
			::ReportEvent(pdata->log,
					EVENTLOG_SUCCESS,
					Log::startup,
					(Log::startup << 8) | modifier,
					NULL,
					0,
					0,
					NULL,
					NULL);
		}
	}
}

void NTSysLogger::Shutdown(Log::EventModifier modifier, va_list args)
{
	if (pdata->log) {
		if (modifier == Log::error) {
			const char*  strings[1];

			strings[0] = va_arg(args, const char*);

			::ReportEvent(pdata->log,
					EVENTLOG_ERROR_TYPE,
					Log::shutdown,
					(Log::shutdown << 8) | modifier,
					NULL,
					1,
					0,
					strings,
					NULL);
		} else {
			::ReportEvent(pdata->log,
					EVENTLOG_SUCCESS,
					Log::shutdown,
					(Log::shutdown << 8) | modifier,
					NULL,
					0,
					0,
					NULL,
					NULL);
		}
	}
}

void NTSysLogger::SignOn(Log::EventModifier modifier, const char* userName, const struct sockaddr_in* peer, va_list args)
{
	if (pdata->log) {
		const char* strings[4];
		int	count = 0;
		char	portbuf[10];


		strings[count++] = userName;
		strings[count++] = inet_ntoa(peer->sin_addr);
		strings[count++] = itoa(ntohs(peer->sin_port), portbuf, 10);

		if (modifier == Log::error) {
			strings[count++] = va_arg(args, const char*);
		}
		::ReportEvent(pdata->log,
				(modifier == Log::error)?EVENTLOG_WARNING_TYPE:EVENTLOG_SUCCESS,
				Log::sign_on,
				(Log::sign_on << 8) | modifier,
				NULL,
				count,
				0,
				strings,
				NULL);
	}
}

void NTSysLogger::Info(const char* format, va_list args)
{
	if (pdata->log) {
		const char*	strings[1];
		char		buffer[2048];

		strings[0] = buffer;
		_vsnprintf(buffer, sizeof(buffer), format, args);
		::ReportEvent(pdata->log,
				EVENTLOG_INFORMATION_TYPE,
				Log::info,
				(Log::info) << 8,
				NULL,
				1,
				0,
				strings,
				NULL);
	}
}

void NTSysLogger::User(Log::EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* userName, va_list args)
{
	if (pdata->log) {
		const char* strings[6];
		int			count = 0;
		char		portbuf[10];
		char		aclbuf[64];

		strings[count++] = actor;
		strings[count++] = userName;
		strings[count++] = inet_ntoa(peer->sin_addr);
		strings[count++] = itoa(ntohs(peer->sin_port), portbuf, 10);
		if (modifier == Log::created) {
			strings[count++] = va_arg(args, const char*); // descrption
		} else if (modifier == Log::acl) {
			int		queues = va_arg(args, int);
			int		users = va_arg(args, int);
			int		groups = va_arg(args, int);
			_snprintf(aclbuf, sizeof(aclbuf), "%s %s %s",
				queues ? "queues":"",
				users ? "users":"",
				groups ? "groups":"");
			strings[count++] = aclbuf;
		} else if (modifier == Log::error) {
			strings[count++] = va_arg(args, const char*);
			strings[count++] = va_arg(args, const char*);
		}

		::ReportEvent(pdata->log,
				(modifier == Log::error)?EVENTLOG_WARNING_TYPE:EVENTLOG_SUCCESS,
				Log::user,
				(Log::user << 8) | modifier,
				NULL,
				count,
				0,
				strings,
				NULL);
	}
}

void NTSysLogger::Group(Log::EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* groupName, va_list args)
{
	if (pdata->log) {
		const char* strings[6];
		int			count = 0;
		char		portbuf[10];
		char		aclbuf[64];

		strings[count++] = actor;
		strings[count++] = groupName;
		strings[count++] = inet_ntoa(peer->sin_addr);
		strings[count++] = itoa(ntohs(peer->sin_port), portbuf, 10);
		if (modifier == Log::useradd || modifier == Log::userdel) {
			strings[count++] = va_arg(args, const char*); // userID
		} else if (modifier == Log::acl) {
			int		queues = va_arg(args, int);
			int		users = va_arg(args, int);
			int		groups = va_arg(args, int);
			_snprintf(aclbuf, sizeof(aclbuf), "%s %s %s",
				queues ? "queues":"",
				users ? "users":"",
				groups ? "groups":"");
			strings[count++] = aclbuf;
		} else if (modifier == Log::error) {
			strings[count++] = va_arg(args, const char*);
			strings[count++] = va_arg(args, const char*);
		}

		::ReportEvent(pdata->log,
				(modifier == Log::error)?EVENTLOG_WARNING_TYPE:EVENTLOG_SUCCESS,
				Log::group,
				(Log::group << 8) | modifier,
				NULL,
				count,
				0,
				strings,
				NULL);
	}
}

void NTSysLogger::Queue(Log::EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* queueName, va_list args)
{
	if (pdata->log) {
		const char* strings[7];
		int			count = 0;
		char		portbuf[10];
		char		aclbuf[64];

		strings[count++] = actor;
		strings[count++] = queueName;
		strings[count++] = inet_ntoa(peer->sin_addr);
		strings[count++] = itoa(ntohs(peer->sin_port), portbuf, 10);

		if (modifier == Log::acl) {
			strings[count++] = va_arg(args, const char*); // type
			strings[count++] = va_arg(args, const char*); // name

			int read = va_arg(args, int);
			int write = va_arg(args, int);
			int destroy = va_arg(args, int);
			int security = va_arg(args, int);

			_snprintf(aclbuf, sizeof(aclbuf), "%s %s %s %s",
				read ? "read":"",
				write ? "write":"",
				destroy ? "destroy":"",
				security ? "security":"");

			strings[count++] = aclbuf; // security options
		} else if (modifier == Log::error || modifier == Log::aclclear) {
			strings[count++] = va_arg(args, const char*); // type -- aclclear, action -- error
			strings[count++] = va_arg(args, const char*); // name -- aclclear, code -- error
		}

		::ReportEvent(pdata->log,
				(modifier == Log::error)?EVENTLOG_WARNING_TYPE:EVENTLOG_SUCCESS,
				Log::queue,
				(Log::queue << 8) | modifier,
				NULL,
				count,
				0,
				strings,
				NULL);
	}
}

void NTSysLogger::Message(Log::EventModifier modifier, va_list args)
{
	if (pdata->log) {
		const char* strings[3];
		int			count = 0;

		if (modifier == Log::error) {
			strings[count++] = va_arg(args, const char*); // notification
			strings[count++] = va_arg(args, const char*); // url
			uuid*		u = va_arg(args, uuid*);

			stringstream	guid;
			guid << *u << ends;

			strings[count++] = guid.str().c_str();	// message id

		} else if (modifier == Log::ttl) {
			strings[count++] = va_arg(args, const char*); // queuename
			
			QueueMessage	*msg = va_arg(args, QueueMessage*);
			stringstream	guid;

			guid << msg->getMessageID() << ends;

			strings[count++] = msg->getLabel().c_str();
			strings[count++] = guid.str().c_str();
		}

		::ReportEvent(pdata->log,
				(modifier == Log::error)?EVENTLOG_ERROR_TYPE:EVENTLOG_SUCCESS,
				Log::message,
				(Log::message << 8) | modifier,
				NULL,
				count,
				0,
				strings,
				NULL);
	}
}

void NTSysLogger::Forward(Log::EventModifier modifier, va_list args)
{
	if (pdata->log) {
		const char* strings[3];
		int			count = 0;

		if (modifier == Log::error) {
			strings[count++] = va_arg(args, const char*); // code
			strings[count++] = va_arg(args, const char*); // url
		}

		::ReportEvent(pdata->log,
				(modifier == Log::error)?EVENTLOG_ERROR_TYPE:EVENTLOG_SUCCESS,
				Log::forward,
				(Log::forward << 8) | modifier,
				NULL,
				count,
				0,
				strings,
				NULL);
	}
}

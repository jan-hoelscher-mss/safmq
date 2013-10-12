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
#pragma warning(disable:4996) // Disable warnings about deprecated functions

#include "Log.h"
#include "Mutex.h"
#include "main.h"
#include <stdio.h>
#include <stdarg.h>

using namespace safmq;

#include "logger.h"

#if defined(_WIN32)
#include "NTSysLogger.h"
NTSysLogger	syslogger;
#else
#include "SysLogger.h"
SysLogger syslogger("safmq");
#endif

namespace safmq {
	struct LogInfo {
		LogInfo() { 
			startupLog = NULL;
			shutdownLog = NULL;
			signOnLog = NULL;
			infoLog = NULL;
			userLog = NULL;
			groupLog = NULL;
			queueLog = NULL;
			messageLog = NULL;
			forwardLog = NULL;
		}

		Mutex	mtx;
		logger*	startupLog;
		logger*	shutdownLog;
		logger* signOnLog;
		logger* infoLog;
		logger* userLog;
		logger* groupLog;
		logger* queueLog;
		logger* messageLog;
		logger* forwardLog;
	};
}

const char* Log::SYSTEM_LOG = "SYSTEM_LOG";
const char* Log::STDOUT = "STDOUT";
const char* Log::STDERR = "STDERR";
const char* Log::NONE = "NONE";
Log* Log::theLog = NULL;

Log::Log()
{
	pinfo = new LogInfo;
	pinfo->startupLog = new logger(stdout);
	pinfo->shutdownLog = new logger(stdout);
	pinfo->signOnLog = new logger(stdout);
	pinfo->infoLog = new logger(stdout);
	pinfo->userLog = new logger(stdout);
	pinfo->groupLog = new logger(stdout);
	pinfo->queueLog = new logger(stdout);
	pinfo->forwardLog = new logger(stdout);
	pinfo->messageLog = new logger(stdout);
}

Log::~Log()
{
	delete pinfo;
}

Log* Log::getLog()
{
	static Mutex mtx;
	MutexLock	lock(&mtx);
	if (theLog == NULL)
		theLog = new Log();
	return theLog;
}

void Log::Startup(EventModifier modifier, ...)
{
	if (pinfo->startupLog) {
		MutexLock	lock(&pinfo->mtx);
		va_list		args;
		va_start(args, modifier);
		pinfo->startupLog->Startup(modifier, args);
		va_end(args);
	}
}

void Log::Shutdown(EventModifier modifier, ...)
{
	if (pinfo->shutdownLog) {
		MutexLock	lock(&pinfo->mtx);
		va_list		args;
		va_start(args, modifier);
		pinfo->shutdownLog->Shutdown(modifier, args);
		va_end(args);
	}
}

void Log::SignOn(EventModifier modifier, const char* userID, const struct sockaddr_in* peer, ...)
{
	if (pinfo->signOnLog) {
		MutexLock	lock(&pinfo->mtx);
		va_list		args;
		va_start(args, peer);
		pinfo->signOnLog->SignOn(modifier, userID, peer, args);
		va_end(args);
	}
}

void Log::Info(const char* format, ...)
{
	if (pinfo->infoLog) {
		MutexLock	lock(&pinfo->mtx);
		va_list		args;
		va_start(args, format);
		pinfo->infoLog->Info(format, args);
		va_end(args);
	}
}

void Log::User(EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* userName, ...)
{
	if (pinfo->userLog) {
		MutexLock	lock(&pinfo->mtx);
		va_list		args;
		va_start(args, userName);
		pinfo->userLog->User(modifier, actor, peer, userName, args);
		va_end(args);
	}
}

void Log::Group(EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* groupName, ...)
{
	if (pinfo->groupLog) {
		MutexLock	lock(&pinfo->mtx);
		va_list		args;
		va_start(args, groupName);
		pinfo->groupLog->Group(modifier, actor, peer, groupName, args);
		va_end(args);
	}
}

void Log::Queue(EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* queueName, ...)
{
	if (pinfo->queueLog) {
		MutexLock	lock(&pinfo->mtx);
		va_list		args;
		va_start(args, queueName);
		pinfo->queueLog->Queue(modifier, actor, peer, queueName, args);
		va_end(args);
	}
}

void Log::Message(EventModifier modifier, ...)
{
	if (pinfo->messageLog) {
		MutexLock	lock(&pinfo->mtx);
		va_list		args;
		va_start(args, modifier);
		pinfo->messageLog->Message(modifier, args);
		va_end(args);
	}
}

void Log::Forward(EventModifier modifier, ...)
{
	if (pinfo->forwardLog) {
		MutexLock	lock(&pinfo->mtx);
		va_list		args;
		va_start(args, modifier);
		pinfo->forwardLog->Forward(modifier, args);
		va_end(args);
	}
}

void Log::SetLogDestination(Event event, const char* dest)
{
	MutexLock	lock(&pinfo->mtx);
	
	logger**	hlogger;

	switch (event) {
		case startup:
			hlogger = &pinfo->startupLog;
			break;
		case shutdown:
			hlogger = &pinfo->shutdownLog;
			break;
		case sign_on:
			hlogger = &pinfo->signOnLog;
			break;
		case user:
			hlogger = &pinfo->userLog;
			break;
		case group:
			hlogger = &pinfo->groupLog;
			break;
		case queue:
			hlogger = &pinfo->queueLog;
			break;
		case message:
			hlogger = &pinfo->messageLog;
			break;
		case forward:
			hlogger = &pinfo->forwardLog;
			break;
		case info:
			hlogger = &pinfo->infoLog;
			break;
	}

	if (!hlogger)
		return;

	if (*hlogger && *hlogger != &syslogger)
		delete *hlogger;
	*hlogger = NULL;

	if (dest == NULL || stricmp(dest,NONE) == 0 || *dest == '\0')
		return;

	if (stricmp(dest, STDERR)==0) {
		*hlogger = new logger(stderr);
	} else if (stricmp(dest, STDOUT)==0) {
		*hlogger = new logger(stdout);
	} else if (stricmp(dest, SYSTEM_LOG)==0) {
		*hlogger = &syslogger;
	} else {
		*hlogger = new logger(dest);
	}
}


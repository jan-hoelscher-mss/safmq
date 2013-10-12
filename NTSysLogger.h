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
#if !defined(_SAFMQ_NTSYSLOGGER_H_)
#define _SAFMQ_NTSYSLOGGER_H_

#include "logger.h"

namespace safmq {

struct NTSysLoggerData;

class NTSysLogger : public logger  
{
public:
	NTSysLogger();
	virtual ~NTSysLogger();

	virtual void Startup(Log::EventModifier modifier, va_list args);
	virtual void Shutdown(Log::EventModifier modifier, va_list args);
	virtual void SignOn(Log::EventModifier modifier, const char* userName, const struct sockaddr_in* peer, va_list args);
	virtual void Info(const char* format, va_list args);
	virtual void User(Log::EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* userName, va_list args);
	virtual void Group(Log::EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* groupName, va_list args);
	virtual void Queue(Log::EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* queueName, va_list args);
	virtual void Message(Log::EventModifier modifier, va_list args);
	virtual void Forward(Log::EventModifier modifier, va_list args);

protected:
	NTSysLoggerData*	pdata;
};

}

#endif 

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
#if !defined(_SAFMQ_LOG_H)
#define _SAFMQ_LOG_H

struct sockaddr_in;

namespace safmq {

struct LogInfo;

class Log {
protected:
	Log();

public:
	enum Event {	startup=0x01, 
					shutdown=0x02,
					sign_on=0x03,
					user=0x04,
					group=0x05,
					queue=0x06,
					message=0x07,
					forward=0x08,
					info=0x09 };

	enum EventModifier {	none=0x00,
							created=0x01,
							deleted=0x02,
							acl=0x03,
							aclclear=0x04,
							ttl=0x05,
							useradd=0x06,
							userdel=0x07,
							error=0x08};

	static const char* SYSTEM_LOG;
	static const char* STDOUT;
	static const char* STDERR;
	static const char* NONE;

	static Log* getLog();
	virtual ~Log();

	void Startup(EventModifier modifier = none, ...);
	void Shutdown(EventModifier modifier = none, ...);
	void SignOn(EventModifier modifier, const char* userName, const struct sockaddr_in* peer, ...);
	void Info(const char* format, ...);
	void User(EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* userName, ...);
	void Group(EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* groupName, ...);
	void Queue(EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* queueName, ...);

	void Message(EventModifier modifier, ...);
	void Forward(EventModifier modifier, ...);

	void SetLogDestination(Event event, const char* dest);
protected:
	LogInfo			*pinfo;
	static Log		*theLog;
};

}

#endif

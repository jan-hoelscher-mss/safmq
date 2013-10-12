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




This software implements store and forward message queue
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include "logger.h"

using namespace safmq;
using namespace std;

const char* logger::df = "%Y-%m-%d %H:%M:%S";

logger::logger()
{
	pfile = stdout;
}

logger::logger(const string& dest)
{
	this->dest = dest;
	pfile = NULL;
}

logger::logger(FILE* pfile)
{
	this->pfile = pfile;
}

logger::~logger()
{

}

void logger::Startup(Log::EventModifier modifier, va_list args)
{
	FILE*		f = pfile;
	time_t		now = time(NULL);
	char		buf[1024];

	if (!f) {
		// allows file names to have strftime escapes in the name for dated files.
		::strftime(buf, sizeof(buf), dest.c_str(), localtime(&now));
		f = fopen(buf, "a");
	}
	if (f) {
		::strftime(buf, sizeof(buf), df, localtime(&now));
		fprintf(f, "[%s]:Startup:", buf);
		if (modifier == Log::error) {
			const char*	szErr = va_arg(args, const char*);
			fprintf(f, "%s", szErr);
		}
		fprintf(f, "\n");
		fflush(f);
		if (!pfile) {
			fclose(f);
		}
	}
}

void logger::Shutdown(Log::EventModifier modifier, va_list args)
{
	FILE*		f = pfile;
	time_t		now = time(NULL);
	char		buf[1024];

	if (!f) {
		// allows file names to have strftime escapes in the name for dated files.
		::strftime(buf, sizeof(buf), dest.c_str(), localtime(&now));
		f = fopen(buf, "a");
	}
	if (f) {
		::strftime(buf, sizeof(buf), df, localtime(&now));
		fprintf(f, "[%s]:Shutdown:", buf);
		if (modifier == Log::error) {
			char*	szErr = va_arg(args, char*);
			fprintf(f, "%s", szErr);
		}
		fprintf(f, "\n");
		fflush(f);
		if (!pfile) {
			fclose(f);
		}
	}
}

void logger::SignOn(Log::EventModifier modifier, const char* userName, const struct sockaddr_in* paddr, va_list args)
{
	FILE*		f = pfile;
	time_t		now = time(NULL);
	char		buf[1024];

	if (!f) {
		// allows file names to have strftime escapes in the name for dated files.
		::strftime(buf, sizeof(buf), dest.c_str(), localtime(&now));
		f = fopen(buf, "a");
	}
	if (f) {
		::strftime(buf, sizeof(buf), df, localtime(&now));
		fprintf(f, "[%s]:SignOn:", buf);

		if (modifier == Log::error) {
			char*	szErr = va_arg(args, char*);
			fprintf(f, "error:%s:%s:", userName, szErr);
		} else {
			fprintf(f, "success:%s:", userName);
		}
		fprintf(f, "%s:%ld", inet_ntoa(paddr->sin_addr), ntohs(paddr->sin_port));
		fprintf(f, "\n");
		fflush(f);
		if (!pfile) {
			fclose(f);
		}
	}
}

void logger::Info(const char* format, va_list args)
{
	FILE*		f = pfile;
	time_t		now = time(NULL);
	char		buf[1024];

	if (!f) {
		// allows file names to have strftime escapes in the name for dated files.
		::strftime(buf, sizeof(buf), dest.c_str(), localtime(&now));
		f = fopen(buf, "a");
	}
	if (f) {
		::strftime(buf, sizeof(buf), df, localtime(&now));
		fprintf(f, "[%s]:Info:", buf);
		vfprintf(f, format, args);
		fprintf(f, "\n");
		fflush(f);
		if (!pfile) {
			fclose(f);
		}
	}
}

void logger::User(Log::EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* userName, va_list args)
{
	FILE*		f = pfile;
	time_t		now = time(NULL);
	char		buf[1024];

	if (!f) {
		// allows file names to have strftime escapes in the name for dated files.
		::strftime(buf, sizeof(buf), dest.c_str(), localtime(&now));
		f = fopen(buf, "a");
	}
	if (f) {
		::strftime(buf, sizeof(buf), df, localtime(&now));
		fprintf(f, "[%s]:User:", buf);

		if (modifier == Log::created) {
			const char* desc = va_arg(args, const char*);
			fprintf(f,"create:success:%s:%s:%s:%s:%ld", userName, desc, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
		} else if (modifier == Log::deleted) {
			fprintf(f,"deleted:success:%s:%s:%s:%ld", userName, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
		} else if (modifier == Log::acl) {
			int		queues = va_arg(args, int);
			int		users = va_arg(args, int);
			int		groups = va_arg(args, int);
			fprintf(f,"acl:success:%s:%s %s %s:%s:%s:%ld", userName, 
				queues ? "queues":"",
				users ? "users":"",
				groups ? "groups":"",
				actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
		} else if (modifier == Log::error) {
			const char* action = va_arg(args, const char*);
			const char* code = va_arg(args, const char*);
			fprintf(f,"%s:error:%s:%s:%s:%s:%ld", action, userName, code, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
		}

		fprintf(f, "\n");
		fflush(f);
		if (!pfile) {
			fclose(f);
		}
	}
}

void logger::Group(Log::EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* groupName, va_list args)
{
	FILE*		f = pfile;
	time_t		now = time(NULL);
	char		buf[1024];

	if (!f) {
		// allows file names to have strftime escapes in the name for dated files.
		::strftime(buf, sizeof(buf), dest.c_str(), localtime(&now));
		f = fopen(buf, "a");
	}
	if (f) {
		::strftime(buf, sizeof(buf), df, localtime(&now));
		fprintf(f, "[%s]:Group:", buf);

		if (modifier == Log::created) {
			fprintf(f,"create:success:%s:%s:%s:%ld", groupName, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
		} else if (modifier == Log::deleted) {
			fprintf(f,"deleted:success:%s:%s:%s:%ld", groupName, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
		} else if (modifier == Log::useradd) {
			const char* username = va_arg(args, const char*);
			fprintf(f,"add user:success:%s:%s:%s:%s:%ld", groupName, username, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
		} else if (modifier == Log::userdel) {
			const char* username = va_arg(args, const char*);
			fprintf(f,"delete user:success:%s:%s:%s:%s:%ld", groupName, username, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
		} else if (modifier == Log::acl) {
			int		queues = va_arg(args, int);
			int		users = va_arg(args, int);
			int		groups = va_arg(args, int);
			fprintf(f,"acl:success:%s:%s %s %s:%s:%s:%ld", groupName, 
				queues ? "queues":"",
				users ? "users":"",
				groups ? "groups":"",
				actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
		} else if (modifier == Log::error) {
			const char* action = va_arg(args, const char*);
			const char* code = va_arg(args, const char*);
			fprintf(f,"%s:error:%s:%s:%s:%s:%ld", action, groupName, code, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
		}

		fprintf(f, "\n");
		fflush(f);
		if (!pfile) {
			fclose(f);
		}
	}
}

void logger::Queue(Log::EventModifier modifier, const char* actor, const struct sockaddr_in* peer, const char* queueName, va_list args)
{
	FILE*		f = pfile;
	time_t		now = time(NULL);
	char		buf[1024];

	if (!f) {
		// allows file names to have strftime escapes in the name for dated files.
		::strftime(buf, sizeof(buf), dest.c_str(), localtime(&now));
		f = fopen(buf, "a");
	}
	if (f) {
		::strftime(buf, sizeof(buf), df, localtime(&now));
		fprintf(f, "[%s]:Queue:", buf);

		if (modifier == Log::created) {
			fprintf(f,"create:success:%s:%s:%s:%ld", queueName, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
		} else if (modifier == Log::deleted) {
			fprintf(f,"deleted:success:%s:%s:%s:%ld", queueName, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
		} else if (modifier == Log::acl) {
			const char* type = va_arg(args, const char*);
			const char* name = va_arg(args, const char*);
			int read = va_arg(args, int);
			int write = va_arg(args, int);
			int destroy = va_arg(args, int);
			int security = va_arg(args, int);
			fprintf(f,"acl:success:%s:%s:%s:%s %s %s %s:%s:%s:%ld",
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
			fprintf(f,"acl clear:success:%s:%s:%s:%s:%s:%ld",
				queueName, 
				type,
				name,
				actor,
				inet_ntoa(peer->sin_addr),
				ntohs(peer->sin_port));
		} else if (modifier == Log::error) {
			const char* action = va_arg(args, const char*);
			const char* code = va_arg(args, const char*);
			fprintf(f,"%s:error:%s:%s:%s:%s:%ld", action, queueName, code, actor, inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));
		}

		fprintf(f, "\n");
		fflush(f);
		if (!pfile) {
			fclose(f);
		}
	}
}

void logger::Message(Log::EventModifier modifier, va_list args)
{
	FILE*		f = pfile;
	time_t		now = time(NULL);
	char		buf[1024];

	if (!f) {
		// allows file names to have strftime escapes in the name for dated files.
		::strftime(buf, sizeof(buf), dest.c_str(), localtime(&now));
		f = fopen(buf, "a");
	}
	if (f) {
		::strftime(buf, sizeof(buf), df, localtime(&now));
		fprintf(f, "[%s]:Message:", buf);

		if (modifier == Log::error) {
			const char* msg = va_arg(args, const char*);
			const char* url = va_arg(args, const char*);
			uuid*		u = va_arg(args, uuid*);

			stringstream	guid;
			guid << *u << ends;
			fprintf(f,"error:%s:%s:%s", msg, url, guid.str().c_str());
		} else if (modifier == Log::ttl) {
			const char*		queuename = va_arg(args, const char*);
			QueueMessage	*msg = va_arg(args, QueueMessage*);
			stringstream	guid;

			guid << msg->getMessageID() << ends;
			fprintf(f,"ttl:%s:%s:%s", queuename, msg->getLabel().c_str(), guid.str().c_str());
		}

		
		fprintf(f, "\n");
		fflush(f);
		if (!pfile) {
			fclose(f);
		}
	}
}

void logger::Forward(Log::EventModifier modifier, va_list args)
{
	FILE*		f = pfile;
	time_t		now = time(NULL);
	char		buf[1024];

	if (!f) {
		// allows file names to have strftime escapes in the name for dated files.
		::strftime(buf, sizeof(buf), dest.c_str(), localtime(&now));
		f = fopen(buf, "a");
	}
	if (f) {
		::strftime(buf, sizeof(buf), df, localtime(&now));
		fprintf(f, "[%s]:Forward:", buf);

		if (modifier == Log::error) {
			const char* code = va_arg(args, const char*);
			const char* url = va_arg(args, const char*);
			fprintf(f, "%s:%s", code, url);
		}
		
		fprintf(f, "\n");
		fflush(f);
		if (!pfile) {
			fclose(f);
		}
	}
}

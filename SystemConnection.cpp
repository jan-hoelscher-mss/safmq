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
#include <iostream>
#include <stdio.h>

#include "SystemConnection.h"
#include "comdefs.h"
#include "lib/client_operators.h"

using namespace safmq;
using namespace std;

namespace safmq {
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
}

ErrorCode SystemConnection::EnqueueForward(MQConnection::QueueHandle que, QueueMessage& msg)
{
	ErrorCode ret = EC_NETWORKERROR;
	ENQUEUE_PARAMS	params;
	params.queueID = que;
	params.msg = &msg;

	server->put((char)ENQUEUE_FORWARD) << params << flush;
	if (server->good()) {
		ret = getResponseCode();
		if (ret == EC_NOERROR) {
			ENQUEUE_RESPONSE_DATA	data;
			*(server) >> data;
			if (server->good()) {
				msg.msg.MessageID = data.msgID;
				msg.msg.TimeStamp = data.timestamp;
			} else {
				ret = EC_NETWORKERROR;
			}
		}
	}
	return ret;
}

void SystemConnection::BuildErrorMsg(QueueMessage& msg, ErrorCode ec, const uuid& recipt_id)
{
	char			buffer[128];

	sprintf(buffer,"Error:%s(%ld)", EC_Decode(ec), ec);
	
	msg.setReciptID(recipt_id);
	msg.setLabel(buffer);
	msg.setTimeToLiveSeconds(-1);
	msg.setBodyType(BT_NONE);
	msg.setTTLErrorWanted(false);
}


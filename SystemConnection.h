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
#if !defined (_SYSTEMCONNECTION_H_)
#define _SYSTEMCONNECTION_H_

#include "safmq.h"
#include "lib/MQConnection.h"

namespace safmq {

class SystemMessage : public QueueMessage
{
public:
	SystemMessage() {
		msg.MessageClass = MC_SYSTEMERRMSG;
		msg.BodySize = 0;
		msg.BodyType = BT_NONE;
	}

	SystemMessage(const SystemMessage& src) {
		operator=(src);
	}

	SystemMessage& operator=(const SystemMessage& src) {
		memcpy(&msg, &src.msg, sizeof(msg));
		return *this;
	}
};

class SystemConnection : public safmq::MQConnection
{
protected:
	SystemConnection(tcpsocket::socstream* server, const std::string& username, const std::string& password) throw (ErrorCode) :
		safmq::MQConnection(server,username,password) {}
public:
	ErrorCode EnqueueForward(MQConnection::QueueHandle que, QueueMessage& msg);

	static void BuildErrorMsg(QueueMessage& msg, ErrorCode ec, const uuid& recipt_id);
};

}

#endif


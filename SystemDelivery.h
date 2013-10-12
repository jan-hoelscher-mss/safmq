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
#if !defined(_TTLDELIVERY_H_)
#define _TTLDELIVERY_H_

#include "thdlib.h"
#include "Mutex.h"
#include "safmq-Signal.h"
#include <queue>
#include "SystemConnection.h"

using namespace safmq;


class SystemDelivery : public Thread  
{
public:
	SystemDelivery();
	virtual ~SystemDelivery();

	static void Enqueue(SystemMessage& msg);
	virtual void stop();

protected:
	virtual void* run();
	virtual void sendMessages();
protected:
	std::string							ttl_user;
	std::string							ttl_passwd;

	static std::queue<SystemMessage>	que;
	static Mutex						qmtx;
	static Signal						qsignal;
};

#endif // !defined(AFX_TTLDELIVERY_H__A729EA2F_0124_446B_B9BE_48AF1ABD4286__INCLUDED_)

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
#if !defined(_SERVERTHREAD_H_)
#define _SERVERTHREAD_H_

#include "thdlib.h"
#include <list>

namespace tcpsocket {
	class ServerSocket;
}

class ServerThread : public Thread
{
public:
	ServerThread();
	virtual ~ServerThread();

	
	virtual void stop();

protected:
	virtual void* run();

	std::list<tcpsocket::ServerSocket *>	servers;
};

#endif // !defined(_SERVERTHREAD_H_)

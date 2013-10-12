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
#if !defined(_MQFACTORY_H_)
#define _MQFACTORY_H_

//#include <string>
#include "tcpsocket/tcpsocket.h"
#include <string>
#include "safmq.h"

#ifdef SAFMQ_SSL
#include "tcpsocket/sslsocket.h"
#else
namespace tcpsocket { struct SSLContext {}; }
#endif


namespace safmq {

class MQConnection;
class MessageQueue;


/**
An exception thrown by the MQFactory
@see MQFactory
*/
class MQFactoryException : public std::exception {
public:
	MQFactoryException() throw() { }
	MQFactoryException(const std::string& err) {
		error = err.c_str();
	}
	MQFactoryException(const MQFactoryException& rhs) throw() { 
		operator=(rhs);
	}
	exception& operator=(const MQFactoryException& rhs) throw() {
		error = rhs.what();
		return *this;
	}
	virtual ~MQFactoryException() throw() { }
	virtual const char *what() const throw() {
		return error.c_str();
	}
protected:
	std::string	error;
};

/**
A factory class which is used to construct connections to safmq servers and message queues.
*/
class MQFactory  
{
public:
	static MQConnection* BuildConnection(const std::string& serverurl, const std::string& user, const std::string& password, tcpsocket::SSLContext* pCtx = NULL) throw (ErrorCode, MQFactoryException, tcpsocket::SocketException);
	static MessageQueue* BuildQueueConnection(const std::string& queueurl, const std::string& user, const std::string& password, tcpsocket::SSLContext* pCtx = NULL)throw (ErrorCode, MQFactoryException, tcpsocket::SocketException);
};

}

#endif // !defined(_MQFACTORY_H_)

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
#include <stdlib.h>
#include "url/urldecode.h"
#include "tcpsocket/socstream.h"
#ifdef SAFMQ_SSL
#include "tcpsocket/sslsocket.h"
#endif
#include "safmq.h"
#include "MQFactory.h"
#include "MQConnection.h"
#include "MessageQueue.h"

using namespace safmq;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const short DEF_PORT = 9000;
const short DEF_SSL_PORT = 9001;
const char* SAFMQ_PROTOCOL = "safmq";

#ifdef SAFMQ_SSL

const char* SAFMQ_SSL_PROTOCOL = "safmqs";

#endif

/**
Establishes a connection to a safmq server.  The parameter <code>serverurl</code> has the form:<br>
<code> safmq://user:password@server:port </code> -or- <br>
<code> safmqs://user:password@server:port </code> <br>

<p>Note that the protocol specification "safmq" and "safmqs" are optional as well as the user, password and password portions, 
leaving the minimum url to be "//server".  If the user and password are not specified in the url the parameters <code>user</code>
and <code>password</code> will used instead.  Defaults are used in cases where they are not specified.</p>

<p>Note: if ssl has not been compiled into the safmq library, using the safmqs protocol will result in an error.</p>

@param serverurl [in] The url locating the safmq server
@param user [in,optional] The user name to login with, if not specified in the url
@param password [in,optional] The password to login with, if not specified in the url
@param pCtx [in,optional] The tcpsocket::SSLContext specifying the SSL environment

@return A MQConnection connected to the safmq server

@exception ErrorCode - thrown if an error occurs logging in
@exception MQFactorException - Thrown if the url cannot be understood 
@exception tcpsocket::SocketException - Thrown if there is a network error
*/
MQConnection* MQFactory::BuildConnection(const std::string& serverurl, const std::string& user, const std::string& password, tcpsocket::SSLContext* pCtx) throw (ErrorCode, MQFactoryException, tcpsocket::SocketException)
{
	try {
		URL::urldecode			url(serverurl);
		int						port = DEF_PORT;
		tcpsocket::socstream	*stm;

		std::string					u = user, p = password;

		if (url.getPort().length() != 0 ) {
			int tmp = atoi(url.getPort().c_str());
			if (tmp > 0)
				port = tmp;
		}
#ifdef SAFMQ_SSL
		else if (url.getProtocol() == SAFMQ_SSL_PROTOCOL) {
			port = DEF_SSL_PORT;
		}
#endif
		if (url.isPasswordSpecified()) {
			u = url.getUser();
			p = url.getPassword();
		} else {
			if (url.getUser().length())
				u = url.getUser();
		}

		if (url.getProtocol() == SAFMQ_PROTOCOL || url.getProtocol().length() ==0 ) {
			stm = new tcpsocket::socstream(tcpsocket::Socket(url.getDestination().c_str(),port));
#ifdef SAFMQ_SSL
		} else if (url.getProtocol() == SAFMQ_SSL_PROTOCOL) {
			tcpsocket::SSLSocket	soc(url.getDestination().c_str(), port, pCtx ? pCtx : tcpsocket::SSLContext::getDefaultContext());
			stm = new tcpsocket::socstream(soc);
#endif
		} else
			throw MQFactoryException("protocol not supported");

		return new MQConnection(stm,u,p);
	} catch (URL::urlexception& e) {
		throw MQFactoryException(e.what());
	}
}

/**
Establishes a connection to a safmq server and opens a queue.  The parameter <code>serverurl</code> has the form:<br>
<code> safmq://user:password@server:port/queue </code> -or- <br>
<code> safmqs://user:password@server:port/queue </code> <br>

<p>Note that the protocol specification "safmq" and "safmqs" are optional as well as the user, password and password portions, 
leaving the minimum url to be "//server/queue".  If the user and password are not specified in the url the parameters <code>user</code>
and <code>password</code> will used instead.  Defaults are used in cases where they are not specified.</p>

<p>Note: if ssl has not been compiled into the safmq library, using the safmqs protocol will result in an error.</p>

@param queueurl [in] The url locating the safmq server and queue
@param user [in,optional] The user name to login with, if not specified in the url
@param password [in,optional] The password to login with, if not specified in the url
@param pCtx [in,optional] The tcpsocket::SSLContext specifying the SSL environment

@return A MQConnection connected to the safmq server

@exception ErrorCode - thrown if an error occurs logging in, or if the user cannot access the queue
@exception MQFactorException - Thrown if the url cannot be understood 
@exception tcpsocket::SocketException - Thrown if there is a network error
*/
MessageQueue* MQFactory::BuildQueueConnection(const std::string& queueurl, const std::string& user, const std::string& password, tcpsocket::SSLContext* pCtx)throw (ErrorCode, MQFactoryException, tcpsocket::SocketException)
{
	try {
		URL::urldecode			url(queueurl);
		int						port = DEF_PORT;
		tcpsocket::socstream	*stm;

		std::string					u = user, p = password, q;

		if (url.getPort().length() != 0 ) {
			int tmp = atoi(url.getPort().c_str());
			if (tmp > 0)
				port = tmp;
		}
#ifdef SAFMQ_SSL
		else if (url.getProtocol() == SAFMQ_SSL_PROTOCOL) {
			port = DEF_SSL_PORT;
		}
#endif

		if (url.isPasswordSpecified()) {
			u = url.getUser();
			p = url.getPassword();
		} else {
			if (url.getUser().length())
				u = url.getUser();
		}

		if (url.getProtocol() == SAFMQ_PROTOCOL || url.getProtocol().length() ==0 ) {
			stm = new tcpsocket::socstream(tcpsocket::Socket(url.getDestination().c_str(),port));
#ifdef SAFMQ_SSL
		} else if (url.getProtocol() == SAFMQ_SSL_PROTOCOL) {
			tcpsocket::SSLSocket	soc(url.getDestination().c_str(),port, pCtx ? pCtx : tcpsocket::SSLContext::getDefaultContext());
			stm = new tcpsocket::socstream(soc);
#endif
		} else
			throw MQFactoryException("protocol not supported");

		q = url.getResource();
		std::string::size_type	e = q.find_first_not_of("/");
		if (e != std::string::npos && e < q.length())
			q = q.substr(e);
		
		MQConnection *con = new MQConnection(stm,u,p);

		return new MessageQueue(con, q, true);
	} catch (URL::urlexception& e) {
		throw MQFactoryException(e.what());
	}
}

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
#include "main.h"
#include "SystemDelivery.h"
#include "lib/MQFactory.h"
#include "lib/MQConnection.h"
#include "url/urldecode.h"
#include "comdefs.h"
#include "Log.h"

using namespace std;
using namespace safmq;


namespace safmq {
	const char* EC_Decode(ErrorCode ec);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

std::queue<SystemMessage>	SystemDelivery::que;
Mutex						SystemDelivery::qmtx;
Signal						SystemDelivery::qsignal;

SystemDelivery::SystemDelivery()
{
	// TODO: fetch the login id's
	ttl_user = system_user;
	ttl_passwd = "";
}

SystemDelivery::~SystemDelivery()
{

}

void SystemDelivery::Enqueue(SystemMessage &msg)
{
	MutexLock	lock(&qmtx);
	que.push(msg);
}

void* SystemDelivery::run()
{
	while (!m_bStop) {
		qsignal.WaitFor(500);
		if (!m_bStop) 
			sendMessages();
	}
	return 0;
}

void SystemDelivery::stop()
{
	Thread::stop();
	qsignal.Set();
}

void SystemDelivery::sendMessages()
{
	qmtx.Lock();
	bool	empty = que.empty();
	qmtx.Unlock();

	std::string cert = pcfg->getParam(SSL_CLIENT_CERT_PARAM,"");
	std::string key = pcfg->getParam(SSL_CLIENT_KEY_PARAM,"");

	tcpsocket::SSLContext		ctx;

#if defined(SAFMQ_SSL)
	tcpsocket::PrivateKey		pkey;
	tcpsocket::X509Certificate	x509;
	if (pkey.loadPEMFile(key) == tcpsocket::PrivateKey::noError) {
		ctx.setPrivateKey(&pkey);
	}

	if (x509.loadPEMFile(cert) == tcpsocket::X509Certificate::noError) {
		ctx.setX509Certificate(&x509);
	}
#endif

	SystemMessage	msg;
	while (!empty) {
		qmtx.Lock();
		msg = que.front();
		que.pop();
		empty = que.empty();
		qmtx.Unlock();
		
		std::string	url_nuevo;
		std::string	queuename;

		uuid guid = msg.getReciptID();

		try {
			std::string		qname;
			msg.getResponseQueueName(qname);

			try {
				URL::urldecode	url(qname);
				std::string	resource = url.getResource();

				if (url.getProtocol().length())
					url_nuevo = url.getProtocol() + ":";
				url_nuevo += "//";
					
				url_nuevo += url.getDestination();
				if (url.getPort().length())
					url_nuevo += ":" + url.getPort();

				/*
				cout << "System Message Delivery:" << msg.getLabel() << endl;
				cout << "Destination: " << url_nuevo << endl;
				cout << "ttl_user:" << ttl_user << endl;
				cout << "ttl_passwd: " << ttl_passwd << endl;
				*/

				// TODO: Is this forwarding? Or is this TTL expiration?
				// TODO: System Delivery Logging
				//Log::getLog()->Forward(Log::none, &msg, url_nuevo.c_str(), ttl_user.c_str());

				SystemConnection			*con = (SystemConnection*)MQFactory::BuildConnection(url_nuevo, ttl_user.c_str(), ttl_passwd.c_str(),&ctx);
				MQConnection::QueueHandle	que;
				
				if (resource.length() && resource[0] == '/')
					resource.erase(0,1);

				if (resource.length()) {
					queuename = resource;
					ErrorCode ec = con->OpenQueue(resource,que);
					if (ec == EC_NOERROR) {
						ec = con->Enqueue(que,msg);
					}

					if (ec != EC_NOERROR) {
						Log::getLog()->Message(Log::error, EC_Decode(ec), (url_nuevo+"/"+queuename).c_str(), &guid);
					}
				}
		
				delete con;
			} catch (URL::urlexception& e) {
				Log::getLog()->Message(Log::error, e.what(), (url_nuevo+"/"+queuename).c_str(), &guid);
			}
		} catch (ErrorCode c) {
			Log::getLog()->Message(Log::error, EC_Decode(c), (url_nuevo+"/"+queuename).c_str(), &guid);
		} catch (MQFactoryException e) {
			Log::getLog()->Message(Log::error, e.what(), (url_nuevo+"/"+queuename).c_str(), &guid);
		} catch (tcpsocket::SocketException e) {
			Log::getLog()->Message(Log::error, e.what(), (url_nuevo+"/"+queuename).c_str(), &guid);
		}
	}
}

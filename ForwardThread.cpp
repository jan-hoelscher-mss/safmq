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
#pragma warning(disable:4996) // Disable warnings about deprecated functions

#include "main.h"
#include <iostream>
#include "ForwardThread.h"
#include "QManager.h"
#include "QAccessControl.h" 
#include "QStorage.h"
#include "url/urldecode.h"
#include "lib/MessageQueue.h"
#include "lib/MQFactory.h"
#include "SystemConnection.h"
#include "SystemDelivery.h"
#include "Log.h"

using namespace std;

namespace safmq {
	const char* EC_Decode(ErrorCode ec);
}

extern QManager*	theQueueManager;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ForwardThread::ForwardThread()
{

}

ForwardThread::~ForwardThread()
{

}

void ForwardThread::stop()
{
	Thread::stop();
	quitsignal.Set();
}

void ForwardThread::SendError(safmq::QueueMessage& src, safmq::ErrorCode err)
{
	std::string	rq;

	src.getResponseQueueName(rq);

	if (rq.length()) {
		SystemMessage	msg;
		SystemConnection::BuildErrorMsg(msg,err,src.getMessageID());
		msg.setResponseQueueName(rq);
		SystemDelivery::Enqueue(msg);
	}
}

void* ForwardThread::run()
{
	std::string allow_forwarding = pcfg->getParam(ENABLE_FOWARDING_PARAM,"no");
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


	if (stricmp(allow_forwarding.c_str(),"yes")==0 || allow_forwarding=="1") {
		QStorage*				forward;
		forward = theQueueManager->GetQueue(FORWARD_QUEUE_NAME);
		if (forward) {
			while (!m_bStop) {{
				// NOTE: double block here to reallocate the MSG every time.
				QueueMessage			msg;
				QStorage::CursorHandle	retrieveID;
				ErrorCode				err;
				
				// TODO: set a maximum time to re-attempt delivery
				// TODO: set a retry-timeout capability.

				if ((err=forward->BeginRetrieve(true, msg, retrieveID)) == EC_NOERROR) {
					forward->FinalizeRetrieve(retrieveID);

					std::string	urlLabel = msg.getLabel();
					bool			success = false;

					try {
						URL::urldecode	url(urlLabel);
						
						Log::getLog()->Info("Forwarding to %s", urlLabel.c_str());

						MQConnection*				con = MQFactory::BuildConnection(urlLabel,"","",&ctx);
						MQConnection::QueueHandle	que;
						ErrorCode					ec;
						std::string					qname = url.getResource();

						if (qname.length() && qname[0] == '/')
							qname.erase(0,1);

						ec = con->OpenQueue(qname, que);
			
						if (ec == EC_NOERROR) {
							URL::urldecode::ParameterMultimap::const_iterator plabel = url.getParameters().find("label");
							if (plabel != url.getParameters().end()) {
								msg.setLabel(plabel->second);
							} else {
								msg.setLabel("");
							}
							ec = ((SystemConnection*)con)->EnqueueForward(que,msg);
						}

						switch (ec) {
							case EC_NOERROR:
								success = true;
								break;

							case EC_NETWORKERROR:
							case EC_FILESYSTEMERROR:
								Log::getLog()->Forward(Log::error, EC_Decode(ec), urlLabel.c_str());
								success = false;
								break;

							case EC_DOESNOTEXIST: // Will get this error if the dest queue does not exits
							case EC_NOTAUTHORIZED:
							case EC_WRONGMESSAGETYPE:
							case EC_NOTOPEN: // NOTE: Shouldn't ever receive this error.
							case EC_FORWARDNOTALLOWED:
							case EC_DUPLICATEMSGID:
								// send an error and throw out the message
								Log::getLog()->Forward(Log::error, EC_Decode(ec), urlLabel.c_str());
								SendError(msg,ec);
								success = true;
								break;

							//NOTE: Errors not produced by OpenQueue or EnqueueForward
							//case EC_TIMEDOUT,
							//case EC_ALREADYCLOSED:
							//case EC_TTLEXPIRED:
							//case EC_CURSORINVALIDATED:
							//case EC_SERVERUNAVAIL:
							//case EC_NOMOREMESSAGES:
							//case EC_CANNOTCHANGEGROUP:
							//case EC_CANNOTCHANGEUSER:
							//case EC_ALREADYDEFINED:
							//case EC_NOTIMPLEMENTED:
							//case EC_QUEUEOPEN:
							//case EC_NOTLOGGEDIN:
							//case EC_INVALIDNAME:
						}

						delete con;
					} catch (ErrorCode ec) {
						
						Log::getLog()->Forward(Log::error, EC_Decode(ec), urlLabel.c_str());
						
						switch (ec) {
							// TODO: Other Error Codes thrown?
							case EC_NOTAUTHORIZED:
							case EC_UNSUPPORTED_PROTOCOL:
							case EC_NOTIMPLEMENTED:
								// Connected but not allowed, do not reattemp
								SendError(msg,ec);
								success = true;
								break;
							default:
								success = false;
								break;
						}

					} catch (std::exception& e) {
						Log::getLog()->Forward(Log::error, e.what(), urlLabel.c_str());
						success = false;
					}

					if (!success) {
						// requeue the message
						Log::getLog()->Info("Requeueing Message: %s", urlLabel.c_str());

						msg.setLabel(urlLabel);
						QStorage::CursorHandle enq;
						if (forward->Enqueue(msg,enq) == EC_NOERROR)
							forward->FinalizeEnqueue(enq);
					}

				} else {
					forward->WaitFront(1);
				}
			}}
			theQueueManager->ReleaseQueue(forward);
		} else {
			Log::getLog()->Forward(Log::error,"Forwarding Disabled, queue does not exist", FORWARD_QUEUE_NAME);
		}
	}
	return NULL;
}

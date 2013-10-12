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
#pragma warning(disable:4786)
#pragma warning(disable:4996) // Disable warnings about deprecated functions
#include "SSLServerThread.h"
#include "ServiceThread.h"
#include "tcpsocket/socstream.h"
#include "main.h"
#include "Log.h"
#include "QAccessControl.h"
#include "AddressParser.h"

#include <iostream>

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef _WIN32
#include <openssl/applink.c>
#endif

using namespace std;
using namespace tcpsocket;

namespace safmq {
	const char* EC_Decode(ErrorCode ec);
}


const unsigned char SAFMQ_SESSION_ID[]="safmq ssl server";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SSLServerThread::SSLServerThread()
{

}

SSLServerThread::~SSLServerThread()
{
	for(std::list<tcpsocket::SSLServerSocket *>::iterator i = servers.begin(); i != servers.end(); ++i) {
		delete *i;
	}
}

void* SSLServerThread::run()
{
	int						port = pcfg->getIntParam(SSL_PORT_PARAM,9001);
	std::string				cert = pcfg->getParam(SSL_CERT_PARAM);
	std::string				key = pcfg->getParam(SSL_KEY_PARAM);
	std::string				caFile = pcfg->getParam(SSL_CA_PARAM);
	std::string				caDir = pcfg->getParam(SSL_CA_DIR_PARAM);
	bool					require_cert = stricmp(pcfg->getParam(SSL_REQUIRE_CLIENT_CERT, "no").c_str(), "yes") == 0;
	std::string				address_list = pcfg->getParam(SSL_BIND_ADDRESS, "");
	AddressParser			ap(address_list);

	SSLSocket				client;
	X509Certificate*		pcert;

	X509Certificate			serverCert;
	PrivateKey				serverPkey;
	CertificateAuthority	ca(caFile, caDir);
	SSLContext				ctx;
	bool					trusted = false;
	std::string				user;

	if (serverCert.loadPEMFile(cert) != X509Certificate::noError) {
		Log::getLog()->Info("Unable to load the SSL server certificate, disabling SSL");
		return NULL;
	}

	if (serverPkey.loadPEMFile(key) != PrivateKey::noError) {
		Log::getLog()->Info("Unable to load the SSL server private key, disabling SSL");
		return NULL;
	}

	Log::getLog()->Info("Reporting SSL Identity as: %s", serverCert.getSubjectName().c_str());

	ctx.setPrivateKey(&serverPkey);
	ctx.setX509Certificate(&serverCert);
	ctx.setCertificateAuthority(&ca);
	ctx.enableVerification(require_cert); // TODO: Get a parameter to determine if client certificates are required
	ctx.setSessionID(SAFMQ_SESSION_ID, sizeof(SAFMQ_SESSION_ID)-1);

	if (port < 1)
		port = 9001;

	if (ap.addresses.size() == 0) {
		try {
			SSLServerSocket* svr = new SSLServerSocket(port,&ctx);
			servers.push_back(svr);
			Log::getLog()->Info("Starting SAFMQ Server at address safmqs://*:%ld", port);
		} catch (tcpsocket::SocketException e) {
			Log::getLog()->Info("Error:Binding Address: safmqs://*:%ld -- %s", port, e.what());
		}
	} else {
		for(AddressParser::AddressList::size_type x=0;x<ap.addresses.size();x++) {
			unsigned short tmpport;
			try {
				tmpport = (ap.addresses[x].port == 0xFFFF ) ? port : ap.addresses[x].port;

				SSLServerSocket* svr = new SSLServerSocket(tmpport, &ctx, ap.addresses[x].address);
				servers.push_back(svr);
				Log::getLog()->Info("Starting SAFMQ Server at address safmqs://%s:%ld", ap.addresses[x].name.c_str(),tmpport);
			} catch (tcpsocket::SocketException e) {
				Log::getLog()->Info("Error:Binding Address: safmqs://%s:%ld -- %s", ap.addresses[x].name.c_str(),tmpport, e.what());
			}
		}
	}

	try {
		std::list<SSLServerSocket*>	avail;
		tcpsocket::Selector<tcpsocket::SSLServerSocket>	selector;
		std::list<tcpsocket::SSLServerSocket*>::iterator	i;

		while (!m_bStop) {
			trusted = false;
			user = "";

			avail.clear();
			avail.assign(servers.begin(), servers.end());

			selector.selectReaders(avail);
			for(i = avail.begin(); !m_bStop && i != avail.end(); ++i) {
				client = (*i)->acceptConnection();
				Log::getLog()->Info("New conneciton accepted");
				
				// Force the connection negotiation, this will call verify()
				pcert = client.getPeerCertificate();
				Log::getLog()->Info("Peer certificate obtained 0x%lx", pcert);

				int res = client.getVerifyResult();
				Log::getLog()->Info("Peer Verify Result: %ld", res);

				if (pcert) {
					Log::getLog()->Info("Client Cert Issuer: %s", pcert->getIssuerName().c_str());
					Log::getLog()->Info("Client Cert Subject: %s", pcert->getSubjectName().c_str());
				} else {
					Log::getLog()->Info("SSLServerThread::run()-pcert: %p", pcert);
				}

				// determine if a trusted user certificate has been presented
				if (res == X509_V_OK && pcert) {
					trusted = (SecurityControl::getSecurityControl()->identifyUser(pcert->getSubjectName(), pcert->getIssuerName(), user) == SecurityControl::GRANTED);
				} 
				Log::getLog()->Info("Certificate Trusted: %ld require_cert: %ld", trusted, require_cert);			

	
				if (require_cert && (res != X509_V_OK || !pcert) ) {
					// reject the client certificate not allowed
					// since valid certificates are required, enforce the requirement

					struct sockaddr_in	peername;

					client.getPeerName(&peername);
					Log::getLog()->SignOn(Log::error, "Digital Certificate", &peername, EC_Decode(EC_NOTAUTHORIZED));
					if (pcert) {
						Log::getLog()->Info("X509 Cert rejected, subject: %s -- reason: %s",
							pcert->getSubjectName().c_str(), X509_verify_cert_error_string(res));
					}
					client.close();
				} else {
					Log::getLog()->Info("Establishing a connection with client user:%s", user.c_str());

					// Establis a connection for the user
					socstream *stm = new tcpsocket::socstream(client);
					ServiceThread *thd = new ServiceThread(stm,trusted,user);
					thd->start();
				}

				if (pcert) delete pcert;
			}
		}
	} catch (tcpsocket::SocketException e) {
		Log::getLog()->Info("SSLServer exception: %s", e.what());
	}

	return NULL;
}

void SSLServerThread::stop()
{
	Thread::stop();
	for(std::list<tcpsocket::SSLServerSocket *>::iterator i = servers.begin(); i != servers.end(); ++i) {
		(*i)->close();
	}
}


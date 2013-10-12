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
#pragma warning(disable: 4786)
#include <iostream>
#include "ServerThread.h"
#include "tcpsocket/socstream.h"
#include "ServiceThread.h"
#include "main.h"
#include "Log.h"
#include "AddressParser.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ServerThread::ServerThread() : Thread(Thread::joinable)
{

}

ServerThread::~ServerThread()
{
	for(std::list<tcpsocket::ServerSocket *>::iterator i = servers.begin(); i != servers.end(); ++i) {
		delete *i;
	}
}

void* ServerThread::run()
{
	std::string		address_list = pcfg->getParam(BIND_ADDRESS, "");
	AddressParser	ap(address_list);
	tcpsocket::ServerSocket*	svr;

	int	port = pcfg->getIntParam(PORT_PARAM,9000);
	if (port < 1)
		port = 9000;

	//  Bind to each address listed in ap

	if (ap.addresses.size() == 0) {
		try {
			svr = new tcpsocket::ServerSocket(port);
			servers.push_back(svr);
			Log::getLog()->Info("Starting SAFMQ Server at address safmq://*:%ld", port);
		} catch (tcpsocket::SocketException e) {
			Log::getLog()->Info("Error:Binding Address: safmq://*:%ld -- %s", port, e.what());
		}
	} else {
		for(AddressParser::AddressList::size_type x=0;x<ap.addresses.size();x++) {
			unsigned short tmpport;
			try {
				tmpport = (ap.addresses[x].port == 0xFFFF) ? port : ap.addresses[x].port;

				svr = new tcpsocket::ServerSocket(tmpport, ap.addresses[x].address);
				servers.push_back(svr);
				Log::getLog()->Info("Starting SAFMQ Server at address safmq://%s:%ld", ap.addresses[x].name.c_str(),tmpport);
			} catch (tcpsocket::SocketException e) {
				Log::getLog()->Info("Error:Binding Address: safmq://%s:%ld -- %s", ap.addresses[x].name.c_str(),tmpport, e.what());
			}
		}
	}


	try {
		std::list<tcpsocket::ServerSocket*>				avail;
		tcpsocket::Selector<tcpsocket::ServerSocket>	selector;
		std::list<tcpsocket::ServerSocket*>::iterator	i;

		while (!m_bStop) {
			avail.clear();
			avail.assign(servers.begin(), servers.end());
		
			selector.selectReaders(avail);
			for(i = avail.begin(); !m_bStop && i != avail.end(); ++i) {
				tcpsocket::socstream *stm = new tcpsocket::socstream((*i)->acceptConnection());
				ServiceThread *thd = new ServiceThread(stm);
				thd->start();
			}
		}
	} catch (tcpsocket::SocketException e) {
		Log::getLog()->Info("TCP ServerThread exception: %s", e.what());
	}

	return NULL;
}

void ServerThread::stop()
{
	Thread::stop();
	for(std::list<tcpsocket::ServerSocket *>::iterator i = servers.begin(); i != servers.end(); ++i) {
		(*i)->close();
	}
}

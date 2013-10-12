/*
 Copyright 2005 Matthew J. Battey

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
// SafmqServerNode.cpp: implementation of the CSafmqServerNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SAFMQManagerSnapin.h"
#include "SAFMQManagerRoot.h"
#include "SafmqServerNode.h"

#include "Folder.h"

#include "MQConnection.h"
#include "MQFactory.h"
#include "ServerList.h"
#include "ServerLoginDlg.h"

using namespace safmq;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// {C5854514-1C30-49d0-8EB1-65586E8BC2F7}
static const GUID CSafmqServerNodeGUID_NODETYPE = 
{ 0xc5854514, 0x1c30, 0x49d0, { 0x8e, 0xb1, 0x65, 0x58, 0x6e, 0x8b, 0xc2, 0xf7 } };

const GUID*		CSafmqServerNode::m_NODETYPE		= &CSafmqServerNodeGUID_NODETYPE;
const OLECHAR*	CSafmqServerNode::m_SZNODETYPE		= OLESTR("C5854514-1C30-49d0-8EB1-65586E8BC2F7");
const OLECHAR*	CSafmqServerNode::m_SZDISPLAY_NAME	= OLESTR("Server Node");
const CLSID*	CSafmqServerNode::m_SNAPIN_CLASSID	= &CLSID_SAFMQManagerRoot;


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


CSafmqServerNode::CSafmqServerNode(CServerList* serverList, const string& url, const string& user, const string& password, bool savePasswd)
		: CNode<CSafmqServerNode, CSAFMQManagerRoot, CSAFMQManagerRootComponent>(serverList)
{
	this->url = url;
	this->user = user;
	this->password = password;

	this->serverList = serverList;

	m_bstrDisplayName.Empty();
	m_bstrDisplayName = url.c_str();

	this->savePasswd = savePasswd;
	validPasswd = true; // attempt to use the entered password
	
	init();
}

CSafmqServerNode::CSafmqServerNode(CServerList* serverList)
		: CNode<CSafmqServerNode, CSAFMQManagerRoot, CSAFMQManagerRootComponent>(serverList)
{
	this->serverList = serverList;

	init();
}


CSafmqServerNode::~CSafmqServerNode()
{
	cleanup();
}

void CSafmqServerNode::cleanup()
{
	delete con;
	delete queues;
	delete users;
	delete groups;

	con = NULL;
	queues = NULL;
	users = NULL;
	groups = NULL;

}


void CSafmqServerNode::init()
{
	memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
	m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
	m_scopeDataItem.displayname = MMC_CALLBACK;
	m_scopeDataItem.nImage = IMG_SERVER_U; 		// May need modification
	m_scopeDataItem.nOpenImage = IMG_SERVER_U; 	// May need modification
	m_scopeDataItem.lParam = (LPARAM) this;
	m_scopeDataItem.cChildren = 0;

	memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));

	con = NULL;
	queues = NULL;
	users = NULL;
	groups = NULL;
}

HRESULT CSafmqServerNode::OnExpand(BOOL bExpand, IConsole *pConsole, HSCOPEITEM me)
{
	CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(pConsole);

	if (me > 0)
		m_scopeDataItem.ID = me;


	if (con) {
		SCOPEDATAITEM	*item;

		
		m_scopeDataItem.nImage = IMG_SERVER_C;
		m_scopeDataItem.nOpenImage = IMG_SERVER_C; 	// May need modification
		m_scopeDataItem.cChildren = 3;
		spConsoleNameSpace->SetItem(&m_scopeDataItem);

		queues->GetScopeData(&item);
		item->relativeID = m_scopeDataItem.ID;
		spConsoleNameSpace->InsertItem(item);

		users->GetScopeData(&item);
		item->relativeID = m_scopeDataItem.ID;
		spConsoleNameSpace->InsertItem(item);

		groups->GetScopeData(&item);
		item->relativeID = m_scopeDataItem.ID;
		spConsoleNameSpace->InsertItem(item);
	}


	return S_OK;
}

HRESULT CSafmqServerNode::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me)
{
	if (me > 0)
		m_scopeDataItem.ID = me;

	CComQIPtr<IResultData, &IID_IResultData> spResultData(pConsole);
	CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeaderCtrl(pConsole);

	spResultData->DeleteAllRsltItems();

	if (bShow) {
		if (me != -1) {
			// Don't add columns if flagged as a refresh
			spHeaderCtrl->InsertColumn( 0, L"Option Group", 0,  MMCLV_AUTO);
		}
	}

	return S_OK;
}

HRESULT CSafmqServerNode::OnViewChange(IConsole* pConsole, long arg, long param)
{
	return OnExpand(true, pConsole, -1);
}

safmq::MQConnection* CSafmqServerNode::getConnection()
{
	return con;
}

void CSafmqServerNode::setBad()
{
	cleanup();

	CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(getConsole());

	m_scopeDataItem.nImage = IMG_SERVER_DC;
	m_scopeDataItem.nOpenImage = IMG_SERVER_DC; 	// May need modification
	m_scopeDataItem.cChildren = 0;
	spConsoleNameSpace->SetItem(&m_scopeDataItem);

	spConsoleNameSpace->DeleteItem(m_scopeDataItem.ID, FALSE);

	CComPtr<IDataObject>	pdata;
	GetDataObject(&pdata, CCT_SCOPE);
	getConsole()->UpdateAllViews(pdata, 0, 0);

}


HRESULT CSafmqServerNode::OnNewGroup(bool& handled, CSnapInObjectRootBase* pBase)
{
	return groups->OnNewGroup(handled, pBase);
}

HRESULT CSafmqServerNode::OnNewUser(bool& handled, CSnapInObjectRootBase* pBase)
{
	return users->OnNewUser(handled, pBase);
}

HRESULT CSafmqServerNode::OnNewQueue(bool& handled, CSnapInObjectRootBase* pBase)
{
	return queues->OnNewQueue(handled, pBase);
}

HRESULT CSafmqServerNode::OnDelete(bool& handled, CSnapInObjectRootBase* pBase)
{
	handled = true;
	
	serverList->DeleteServer(this);

	return S_OK; // TODO: Delete the server
}

HRESULT CSafmqServerNode::OnConnect(bool& handled, CSnapInObjectRootBase* pBase)
{
	if (con == NULL) {
		while (true) {
			if (!validPasswd) {
				// Query for the password
				CServerLoginDlg	dlg;

				dlg.setServerName(url);
				dlg.setUserName(user);


				int rc = dlg.DoModal();
				if (rc == IDCANCEL) {
			 		savePasswd = false;
					break;
				} else if (rc == IDOK) {
					user = dlg.getUsername();
					password = dlg.getPassword();
					savePasswd = dlg.getSavePassword();
				}
			}
			try {
				con = MQFactory::BuildConnection(getURL(), user, password);
				validPasswd = true;


				queues = new CQueueFolder(this);
				users = new CUserFolder(this);
				groups = new CGroupFolder(this);


				CComPtr<IDataObject>	pdata;
				GetDataObject(&pdata, CCT_SCOPE);
				getConsole()->UpdateAllViews(pdata, 0, 0);

				handled = true;
				return S_OK;
			} catch (ErrorCode ec) {
				ATLTRACE("Caught Errorcode ex: %ld\n", ec);
				HWND hwnd;

				std::string msg = "Sorry, unable to connect\n";
				msg += "Error: ";
				msg += EC_Decode(ec);

				getConsole()->GetMainWindow(&hwnd);
				::MessageBox(hwnd, msg.c_str(), "Server Connect", MB_ICONERROR | MB_OK);
			} catch (tcpsocket::SocketException& ex) {
				std::string msg = "Sorry, unable to connect\n";
				HWND		hwnd;

				msg += ex.what();

				getConsole()->GetMainWindow(&hwnd);
				::MessageBox(hwnd, msg.c_str(), "Server Connect", MB_ICONERROR | MB_OK);
			} catch (safmq::MQFactoryException& ex) {
				std::string msg = "Sorry, unable to connect\n";
				HWND		hwnd;

				msg += ex.what();

				getConsole()->GetMainWindow(&hwnd);
				::MessageBox(hwnd, msg.c_str(), "Server Connect", MB_ICONERROR | MB_OK);
			}
			validPasswd = false;
		}

	}

	setBad();
	return S_OK;
}

HRESULT CSafmqServerNode::OnDisconnect(bool& handled, CSnapInObjectRootBase* pBase)
{

	CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(getConsole());

	m_scopeDataItem.nImage = IMG_SERVER_U;
	m_scopeDataItem.nOpenImage = IMG_SERVER_U; 	// May need modification
	m_scopeDataItem.cChildren = 0;
	spConsoleNameSpace->SetItem(&m_scopeDataItem);

	getConsole()->SelectScopeItem(serverList->m_scopeDataItem.ID);

	spConsoleNameSpace->DeleteItem(m_scopeDataItem.ID, FALSE);

	cleanup();

	CComPtr<IDataObject>	pdata;
	GetDataObject(&pdata, CCT_SCOPE);
	getConsole()->UpdateAllViews(pdata, 0, 0);
	
	handled = true;
	return S_OK;
}


string CSafmqServerNode::getURL()
{
	if (url.find("safmq://") == 0 || url.find("safmqs://") == 0 || url.find("//") == 0)
		return url;
	return "safmq://" + url;
}

void CSafmqServerNode::DeleteQueue(BSTR queueName, CSnapInItem* queueItem)
{
	queues->DeleteQueue(queueName, queueItem);
}

void CSafmqServerNode::DeleteUser(BSTR userName, CSnapInItem* userItem)
{
	users->DeleteUser(userName, userItem);
}

void CSafmqServerNode::DeleteGroup(BSTR groupName, CSnapInItem* groupItem)
{
	groups->DeleteGroup(groupName, groupItem);
}

HRESULT CSafmqServerNode::Load(IStream *pStm)
{
	char	buffer[1024];
	long	size;
	ULONG	read;

	memset(buffer,0,sizeof(buffer));
	size = 0;
	pStm->Read(&size, sizeof(size), &read);
	pStm->Read(buffer, size, &read);
	buffer[size] = 0;
	url = buffer;

	memset(buffer,0,sizeof(buffer));
	size = 0;
	pStm->Read(&size, sizeof(size), &read);
	pStm->Read(buffer, size, &read);
	buffer[size] = 0;
	user = buffer;

	char	tmp = 0;
	pStm->Read(&tmp, 1, &read);
	validPasswd = savePasswd = tmp != 0;

	if (savePasswd) {
		memset(buffer,0,sizeof(buffer));
		size = 0;
		pStm->Read(&size, sizeof(size), &read);
		pStm->Read(buffer, size, &read);
		buffer[size] = 0;
		password = buffer;
	}

	m_bstrDisplayName.Empty();
	m_bstrDisplayName = url.c_str();

	return S_OK;
}

HRESULT CSafmqServerNode::Save(IStream *pStm)
{
	long	size;
	ULONG	written;

	size = url.length();
	pStm->Write(&size, sizeof(size), &written);
	pStm->Write(url.c_str(), size, &written);

	size = user.length();
	pStm->Write(&size, sizeof(size), &written);
	pStm->Write(user.c_str(), size, &written);

	char tmp = savePasswd;
	pStm->Write(&tmp, 1, &written);

	if (savePasswd) {
		size = password.length();
		pStm->Write(&size, sizeof(size), &written);
		pStm->Write(password.c_str(), size, &written);
	}
	
	return S_OK;
}

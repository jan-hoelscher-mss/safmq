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
// UserFolder.cpp: implementation of the CUserFolder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SAFMQManagerSnapin.h"
#include "SAFMQManagerRoot.h"
#include "Folder.h"

#include "SafmqServerNode.h"

#include "Actor.h"

#include "MQConnection.h"
#include "MQFactory.h"
#include "NewUserDlg.h"
#include "NewGroupDlg.h"
#include "NewQueueDlg.h"

using namespace safmq;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// {EFF1DCBE-A584-489b-8CBF-3FD52CEDA4CA}
static const GUID CUserFolder_NODETYPE = 
{ 0xeff1dcbe, 0xa584, 0x489b, { 0x8c, 0xbf, 0x3f, 0xd5, 0x2c, 0xed, 0xa4, 0xca } };

const GUID*		CUserFolder::m_NODETYPE		= &CUserFolder_NODETYPE;
const OLECHAR*	CUserFolder::m_SZNODETYPE		= OLESTR("EFF1DCBE-A584-489b-8CBF-3FD52CEDA4CA");
const OLECHAR*	CUserFolder::m_SZDISPLAY_NAME	= OLESTR("Users");
const CLSID*	CUserFolder::m_SNAPIN_CLASSID	= &CLSID_SAFMQManagerRoot;

CUserFolder::CUserFolder(CSafmqServerNode* server)
		: CNode<CUserFolder, CSAFMQManagerRoot, CSAFMQManagerRootComponent>(server)
{
	memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
	m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
	m_scopeDataItem.displayname = MMC_CALLBACK;
	m_scopeDataItem.nImage = -1; 		// May need modification
	m_scopeDataItem.nOpenImage = -1; 	// May need modification
	m_scopeDataItem.lParam = (LPARAM) this;
	m_scopeDataItem.cChildren = 99;


	m_bstrDisplayName.Empty();
	m_bstrDisplayName = m_SZDISPLAY_NAME;

	memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	m_resultDataItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
	m_resultDataItem.str = MMC_CALLBACK;
	m_resultDataItem.nImage = -1;
	m_resultDataItem.lParam = (LPARAM) this;

	this->server = server;

}

CUserFolder::~CUserFolder()
{
	FreeUsers();
}

void CUserFolder::FreeUsers()
{
	vector<CSnapInItem*>::iterator i;
	for(i=children.begin();i!=children.end();i++) {
		delete *i;
	}
	children.clear();
}


void CUserFolder::LoadUsers()
{
	FreeUsers();

	MQConnection *con = server->getConnection();
	if (con) {
		MQConnection::USER_VECTOR	users;
		MQConnection::USER_VECTOR::iterator user;
		ErrorCode	ec;

		if ((ec=con->EnumerateUsers(users)) == EC_NOERROR) {
			for(user = users.begin(); user != users.end(); ++user) {
				children.push_back(new CUserNode(server, user->name, user->description));
			}
		}
		if (ec == EC_NETWORKERROR)
			server->setBad();
	}
}


HRESULT CUserFolder::OnExpand(BOOL bExpand, IConsole *pConsole, HSCOPEITEM me)
{
	CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(pConsole);

	if (me > 0)
		m_scopeDataItem.ID = me;

	SCOPEDATAITEM	*item;
	HSCOPEITEM		child;
	long			cookie;

	while (spConsoleNameSpace->GetChildItem(m_scopeDataItem.ID, &child, &cookie) == S_OK) {
		spConsoleNameSpace->DeleteItem(child, TRUE);
	}

	LoadUsers();

	vector<CSnapInItem*>::iterator i;
	for(i=children.begin(); i!=children.end(); ++i) {
		((CUserNode*)*i)->GetScopeData(&item);
		if (spConsoleNameSpace->GetItem(item) != S_OK) {
			item->relativeID = m_scopeDataItem.ID;
			spConsoleNameSpace->InsertItem(item);
		}
	}

	return S_OK;
}

HRESULT CUserFolder::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me)
{
	CComQIPtr<IResultData, &IID_IResultData> spResultData(pConsole);
	CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeaderCtrl(pConsole);

	if (me > 0)
		m_resultDataItem.itemID = me;

	spResultData->DeleteAllRsltItems();

	if (bShow) {
		if (me != -1) {
			// Don't add columns if flagged as a refresh
			spHeaderCtrl->InsertColumn( 0, L"User Name", 0,  MMCLV_AUTO);
			spHeaderCtrl->InsertColumn( 1, L"Description", 0,  MMCLV_AUTO);
		}
	}
	return S_OK;
}

HRESULT CUserFolder::OnNewUser(bool& handled, CSnapInObjectRootBase* pBase)
{
	CNewUserDlg	dlg;
	if (dlg.DoModal() == IDOK) {
		if (dlg.getUsername().length()) {
			MQConnection* con = server->getConnection();
			if (con) {
				ErrorCode ec;
				if ((ec=con->CreateUser(dlg.getUsername(), dlg.getPassword(), dlg.getDescription())) == EC_NOERROR) {
					CComPtr<IDataObject>	pdata;
					GetDataObject(&pdata, CCT_SCOPE);
					getConsole()->UpdateAllViews(pdata, 0, 0);
				} else {
					if (ec == EC_NETWORKERROR) {
						server->setBad();
					} else if (ec == EC_NOTAUTHORIZED) {
						HWND hwnd;
						getConsole()->GetMainWindow(&hwnd);
						::MessageBox(hwnd, "Sorry, you are not authorized to create users", "New User", MB_ICONERROR | MB_OK);
					} else if (ec == EC_ALREADYDEFINED) {
						HWND hwnd;
						getConsole()->GetMainWindow(&hwnd);
						::MessageBox(hwnd, "Sorry, a user with that name already exists", "New User", MB_ICONERROR | MB_OK);
					}
				}
			} else {
				// TODO:handle the no connection
			}
		}
	}
	return S_OK;
}

void CUserFolder::DeleteUser(BSTR userName, CSnapInItem* userItem)
{
	HWND	hwnd;
	_bstr_t	msg("Are you sure you want to remove the user: ");

	msg += userName;
	getConsole()->GetMainWindow(&hwnd);

	if (::MessageBox(hwnd, msg, "Delete User", MB_ICONQUESTION | MB_YESNO) == IDYES) {
		MQConnection	*con = server->getConnection();
		if (con) {
			ErrorCode ec = con->DeleteUser((const char*)_bstr_t(userName));
			if (ec == EC_NOERROR) {
				getConsole()->SelectScopeItem(m_scopeDataItem.ID); // Select me to avoid references to the removed item

				CComPtr<IDataObject>	pdata;
				GetDataObject(&pdata, CCT_SCOPE);
				getConsole()->UpdateAllViews(pdata, 0, 0);
			} else {
				if (ec == EC_NETWORKERROR) {
					server->setBad();
				} else if (ec == EC_NOTAUTHORIZED) {
					HWND hwnd;
					getConsole()->GetMainWindow(&hwnd);
					::MessageBox(hwnd, "Sorry, you are not authorized to delete users", "Delete User", MB_ICONERROR | MB_OK);
				}
			}
		} else {
			// TODO: Handle no connection
		}
	}
}


HRESULT CUserFolder::OnViewChange(IConsole* pConsole, long arg, long param)
{
	OnExpand(true, pConsole, -1);
	return S_FALSE;
}


// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////

// ////////////////////////////////////////////////////////////////////////////
// CGroupFolder Implementation

// {D7E25C8A-9656-445f-AC30-866B7DDB80E8}
static const GUID CGroupFolder_NODETYPE = 
{ 0xd7e25c8a, 0x9656, 0x445f, { 0xac, 0x30, 0x86, 0x6b, 0x7d, 0xdb, 0x80, 0xe8 } };

const GUID*		CGroupFolder::m_NODETYPE		= &CGroupFolder_NODETYPE;
const OLECHAR*	CGroupFolder::m_SZNODETYPE		= OLESTR("D7E25C8A-9656-445f-AC30-866B7DDB80E8");
const OLECHAR*	CGroupFolder::m_SZDISPLAY_NAME	= OLESTR("Groups");
const CLSID*	CGroupFolder::m_SNAPIN_CLASSID	= &CLSID_SAFMQManagerRoot;

CGroupFolder::CGroupFolder(CSafmqServerNode* server)
		: CNode<CGroupFolder, CSAFMQManagerRoot, CSAFMQManagerRootComponent>(server)
{
	memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
	m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
	m_scopeDataItem.displayname = MMC_CALLBACK;
	m_scopeDataItem.nImage = -1; 		// May need modification
	m_scopeDataItem.nOpenImage = -1; 	// May need modification
	m_scopeDataItem.lParam = (LPARAM) this;
	m_scopeDataItem.cChildren = 99;


	m_bstrDisplayName.Empty();
	m_bstrDisplayName = m_SZDISPLAY_NAME;

	memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	m_resultDataItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
	m_resultDataItem.str = MMC_CALLBACK;
	m_resultDataItem.nImage = -1;
	m_resultDataItem.lParam = (LPARAM) this;

	this->server = server;
}

CGroupFolder::~CGroupFolder()
{
	FreeGroups();
}

void CGroupFolder::FreeGroups()
{
	vector<CSnapInItem*>::iterator i;
	for(i=children.begin();i!=children.end();i++) {
		delete *i;
	}
	children.clear();
}

void CGroupFolder::LoadGroups()
{
	FreeGroups();

	MQConnection *con = server->getConnection();
	if (con) {
		MQConnection::NAME_VECTOR			groups;
		MQConnection::NAME_VECTOR::iterator group;
		ErrorCode	ec;

		if ((ec=con->EnumerateGroups(groups)) == EC_NOERROR) {
			for(group = groups.begin(); group != groups.end(); ++group) {
				children.push_back(new CGroupNode(server, *group));
			}
		}
		if (ec == EC_NETWORKERROR)
			server->setBad();
	}
}

HRESULT CGroupFolder::OnExpand(BOOL bExpand, IConsole *pConsole, HSCOPEITEM me)
{
	CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(pConsole);

	if (me > 0)
		m_scopeDataItem.ID = me;

	SCOPEDATAITEM	*item;
	HSCOPEITEM		child;
	long			cookie;

	while (spConsoleNameSpace->GetChildItem(m_scopeDataItem.ID, &child, &cookie) == S_OK) {
		spConsoleNameSpace->DeleteItem(child, TRUE);
	}

	LoadGroups();

	vector<CSnapInItem*>::iterator i;
	for(i=children.begin(); i!=children.end(); ++i) {
		((CGroupNode*)*i)->GetScopeData(&item);
		item->relativeID = m_scopeDataItem.ID;
		spConsoleNameSpace->InsertItem(item);
	}

	return S_OK;
}

HRESULT CGroupFolder::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me)
{
	CComQIPtr<IResultData, &IID_IResultData> spResultData(pConsole);
	CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeaderCtrl(pConsole);

	if (me > 0)
		m_resultDataItem.itemID = me;

	spResultData->DeleteAllRsltItems();

	if (bShow) {
		if (me != -1) {
			// Don't add columns if flagged as a refresh
			spHeaderCtrl->InsertColumn( 0, L"Group Name", 0,  MMCLV_AUTO);
		}
	}
	return S_OK;
}

HRESULT CGroupFolder::OnNewGroup(bool& handled, CSnapInObjectRootBase* pBase)
{
	CNewGroupDlg	dlg;

	if (dlg.DoModal() == IDOK) {
		MQConnection* con = server->getConnection();
		if (con) {
			ErrorCode ec = con->CreateGroup(dlg.getGroupName());
			if (ec == EC_NOERROR) {
				CComPtr<IDataObject>	pdata;
				GetDataObject(&pdata, CCT_SCOPE);
				getConsole()->UpdateAllViews(pdata, 0, 0);
			} else {
				if (ec == EC_NETWORKERROR) {
					server->setBad();
				} else if (ec == EC_NOTAUTHORIZED) {
					HWND hwnd;
					getConsole()->GetMainWindow(&hwnd);
					::MessageBox(hwnd, "Sorry, you are not authorized to create groups", "New Group", MB_ICONERROR | MB_OK);
				} else if (ec == EC_ALREADYDEFINED) {
					HWND hwnd;
					getConsole()->GetMainWindow(&hwnd);
					::MessageBox(hwnd, "Sorry, a group with that name already exists", "New Group", MB_ICONERROR | MB_OK);
				}
			}
		} else {
			// TODO: handle no connection
		}
	}
	return S_OK;
}

void CGroupFolder::DeleteGroup(BSTR groupName, CSnapInItem* groupItem)
{
	HWND	hwnd;
	_bstr_t	msg("Are you sure you want to remove the group: ");

	msg += groupName;
	getConsole()->GetMainWindow(&hwnd);

	if (::MessageBox(hwnd, msg, "Delete Group", MB_ICONQUESTION | MB_YESNO) == IDYES) {
		MQConnection	*con = server->getConnection();
		if (con) {
			ErrorCode ec = con->DeleteGroup((const char*)_bstr_t(groupName));
			if (ec == EC_NOERROR) {
				getConsole()->SelectScopeItem(m_scopeDataItem.ID); // Select me to avoid references to the removed item
				CComPtr<IDataObject>	pdata;
				GetDataObject(&pdata, CCT_SCOPE);
				getConsole()->UpdateAllViews(pdata, 0, 0);
			} else {
				if (ec == EC_NETWORKERROR) {
					server->setBad();
				} else if (ec == EC_NOTAUTHORIZED) {
					HWND hwnd;
					getConsole()->GetMainWindow(&hwnd);
					::MessageBox(hwnd, "Sorry, you are not authorized to delete groups", "Delete Group", MB_ICONERROR | MB_OK);
				}
			}
		} else {
			// TODO: Handle no connection
		}
	}
}

HRESULT CGroupFolder::OnViewChange(IConsole* pConsole, long arg, long param)
{
	OnExpand(true, pConsole, -1);
	return S_FALSE;
}


// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////

// ////////////////////////////////////////////////////////////////////////////
// CQueueFolder Implementation

// {06E4E3CD-4C55-4b69-9C99-C0C0D84360FA}
static const GUID CQueueFolder_NODETYPE = 
{ 0x6e4e3cd, 0x4c55, 0x4b69, { 0x9c, 0x99, 0xc0, 0xc0, 0xd8, 0x43, 0x60, 0xfa } };

const GUID*		CQueueFolder::m_NODETYPE		= &CQueueFolder_NODETYPE;
const OLECHAR*	CQueueFolder::m_SZNODETYPE		= OLESTR("06E4E3CD-4C55-4b69-9C99-C0C0D84360FA");
const OLECHAR*	CQueueFolder::m_SZDISPLAY_NAME	= OLESTR("Queues");
const CLSID*	CQueueFolder::m_SNAPIN_CLASSID	= &CLSID_SAFMQManagerRoot;

CQueueFolder::CQueueFolder(CSafmqServerNode* server)
		: CNode<CQueueFolder, CSAFMQManagerRoot, CSAFMQManagerRootComponent>(server)
{
	memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
	m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
	m_scopeDataItem.displayname = MMC_CALLBACK;
	m_scopeDataItem.nImage = -1; 		// May need modification
	m_scopeDataItem.nOpenImage = -1; 	// May need modification
	m_scopeDataItem.lParam = (LPARAM) this;
	m_scopeDataItem.cChildren = 99;


	m_bstrDisplayName.Empty();
	m_bstrDisplayName = m_SZDISPLAY_NAME;

	memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	m_resultDataItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
	m_resultDataItem.str = MMC_CALLBACK;
	m_resultDataItem.nImage = -1;
	m_resultDataItem.lParam = (LPARAM) this;

	this->server = server;
}

CQueueFolder::~CQueueFolder()
{
	FreeQueues();
}

void CQueueFolder::FreeQueues()
{
	vector<CSnapInItem*>::iterator i;
	for(i=children.begin();i!=children.end();i++) {
		delete *i;
	}
	children.clear();
}

void CQueueFolder::LoadQueues()
{
	FreeQueues();

	MQConnection *con = server->getConnection();
	if (con) {
		MQConnection::QDATA_VECTOR				queues;
		MQConnection::QDATA_VECTOR::iterator	q;
		ErrorCode	ec;

		if ((ec=con->EnumerateQueues(queues)) == EC_NOERROR) {
			for(q = queues.begin(); q != queues.end(); ++q) {
				children.push_back(new CQueueNode(server, q->queuename));
			}
		}
		if (ec == EC_NETWORKERROR)
			server->setBad();
	}
}

HRESULT CQueueFolder::OnExpand(BOOL bExpand, IConsole *pConsole, HSCOPEITEM me)
{
	CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(pConsole);

	if (me > 0)
		m_scopeDataItem.ID = me;

	SCOPEDATAITEM	*item;
	HSCOPEITEM		child;
	long			cookie;

	while (spConsoleNameSpace->GetChildItem(m_scopeDataItem.ID, &child, &cookie) == S_OK) {
		spConsoleNameSpace->DeleteItem(child, TRUE);
	}

	LoadQueues();

	vector<CSnapInItem*>::iterator i;
	for(i=children.begin(); i!=children.end(); ++i) {
		((CQueueNode*)*i)->GetScopeData(&item);
		item->relativeID = m_scopeDataItem.ID;
		spConsoleNameSpace->InsertItem(item);
	}

	return S_OK;
}

HRESULT CQueueFolder::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me)
{
	CComQIPtr<IResultData, &IID_IResultData> spResultData(pConsole);
	CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeaderCtrl(pConsole);

	if (me > 0)
		m_resultDataItem.itemID = me;

	spResultData->DeleteAllRsltItems();

	if (bShow) {
		if (me != -1) {
			// Don't add columns if flagged as a refresh
			spHeaderCtrl->InsertColumn( 0, L"Queue Name", 0,  MMCLV_AUTO);
		}
	}
	return S_OK;
}

HRESULT CQueueFolder::OnNewQueue(bool& handled, CSnapInObjectRootBase* pBase)
{
	CNewQueueDlg	dlg;

	if (dlg.DoModal() == IDOK) {
		MQConnection* con = server->getConnection();
		if (con) {
			ErrorCode ec = con->CreateQueue(dlg.getQueueName());
			if (ec == EC_NOERROR) {
				CComPtr<IDataObject>	pdata;
				GetDataObject(&pdata, CCT_SCOPE);
				getConsole()->UpdateAllViews(pdata, 0, 0);
			} else {
				if (ec == EC_NETWORKERROR) {
					server->setBad();
				} else if (ec == EC_NOTAUTHORIZED) {
					HWND hwnd;
					getConsole()->GetMainWindow(&hwnd);
					::MessageBox(hwnd, "Sorry, you are not authorized to create queues", "New Queue", MB_ICONERROR | MB_OK);
				} else if (ec == EC_ALREADYDEFINED) {
					HWND hwnd;
					getConsole()->GetMainWindow(&hwnd);
					::MessageBox(hwnd, "Sorry, a queue with that name already exists", "New Queue", MB_ICONERROR | MB_OK);
				}
			}
		} else {
			// TODO: handle the error
		}
	}
	return S_OK;
}

void CQueueFolder::DeleteQueue(BSTR queueName, CSnapInItem* queueItem)
{
	HWND	hwnd;
	_bstr_t	msg("Are you sure you want to remove the queue: ");

	msg += queueName;
	getConsole()->GetMainWindow(&hwnd);

	if (::MessageBox(hwnd, msg, "Delete Queue", MB_ICONQUESTION | MB_YESNO) == IDYES) {
		MQConnection	*con = server->getConnection();
		if (con) {
			ErrorCode ec = con->DeleteQueue((const char*)_bstr_t(queueName));
			if (ec == EC_NOERROR) {
				getConsole()->SelectScopeItem(m_scopeDataItem.ID); // Select me to avoid references to the removed item

				CComPtr<IDataObject>	pdata;
				GetDataObject(&pdata, CCT_SCOPE);
				getConsole()->UpdateAllViews(pdata, 0, 0);
			} else {
				if (ec == EC_NETWORKERROR) {
					server->setBad();
				} else if (ec == EC_NOTAUTHORIZED) {
					HWND hwnd;
					getConsole()->GetMainWindow(&hwnd);
					::MessageBox(hwnd, "Sorry, you are not authorized to create queues", "New Queue", MB_ICONERROR | MB_OK);
				}
			}
		} else {
			// TODO: Handle no connection
		}
	}
}

HRESULT CQueueFolder::OnViewChange(IConsole* pConsole, long arg, long param)
{
	OnExpand(true, pConsole, -1);
	return S_FALSE;
}

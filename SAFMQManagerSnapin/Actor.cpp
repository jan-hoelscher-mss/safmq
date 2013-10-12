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
// Actor.cpp: implementation of the CUserNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "SAFMQManagerSnapin.h"
#include "SAFMQManagerRoot.h"
#include "SafmqServerNode.h"

#include "Actor.h"

#include "MQConnection.h"
#include "MQFactory.h"
#include "ActorPermsDlg.h"
#include "AddActorDlg.h"

using namespace safmq;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// {12DDD54A-BE4A-4606-AAF7-2586EBEECD8D}
static const GUID CUserNode_NODETYPE = 
{ 0x12ddd54a, 0xbe4a, 0x4606, { 0xaa, 0xf7, 0x25, 0x86, 0xeb, 0xee, 0xcd, 0x8d } };


const GUID*		CUserNode::m_NODETYPE = &CUserNode_NODETYPE;
const OLECHAR*	CUserNode::m_SZNODETYPE = OLESTR("12DDD54A-BE4A-4606-AAF7-2586EBEECD8D");
const OLECHAR*	CUserNode::m_SZDISPLAY_NAME = OLESTR("");
const CLSID*	CUserNode::m_SNAPIN_CLASSID = &CLSID_SAFMQManagerRoot;


_ATL_FUNC_INFO CUserNode::userChangePassword = {CC_STDCALL, VT_EMPTY, 1, {VT_BSTR}};
_ATL_FUNC_INFO CUserNode::userApplyChanges = {CC_STDCALL, VT_EMPTY, 4, {VT_BOOL, VT_BOOL, VT_BOOL, VT_VARIANT}};
_ATL_FUNC_INFO CUserNode::userGetAvailableGroups = { CC_STDCALL, VT_VARIANT, 0, {VT_EMPTY}};

CUserNode::CUserNode(CSafmqServerNode* server, const string& name, const string& description)
		: CNode<CUserNode, CSAFMQManagerRoot, CSAFMQManagerRootComponent>(server)
{
	memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
	m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
	m_scopeDataItem.displayname = MMC_CALLBACK;
	m_scopeDataItem.nImage = IMG_USER; 		// May need modification
	m_scopeDataItem.nOpenImage = IMG_USER; 	// May need modification
	m_scopeDataItem.lParam = (LPARAM) this;
	m_scopeDataItem.cChildren = 0;


	m_bstrDisplayName.Empty();
	m_bstrDisplayName = name.c_str();

	bname = name.c_str();
	bdescription = description.c_str();

	memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	m_resultDataItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
	m_resultDataItem.str = MMC_CALLBACK;
	m_resultDataItem.nImage = IMG_USER;
	m_resultDataItem.lParam = (LPARAM) this;

	this->server = server;

	modifyGroups = modifyUsers = modifyQueues = false;
}

CUserNode::~CUserNode()
{
	if (editor != NULL) {
		this->DispEventUnadvise(editor);
	}
}

HRESULT CUserNode::OnDelete(bool& bHandled, CSnapInObjectRootBase* pObj)
{
	server->DeleteUser(m_bstrDisplayName, this);
	return S_OK;
}

HRESULT CUserNode::OnInitOCX(IConsole* spConsole, IUnknown* myOCX) {
	if (myOCX) {
		if (editor != NULL) {
			this->DispEventUnadvise(editor);
		}

		

		editor = NULL;
		myOCX->QueryInterface(&editor);

		if (editor) {
			MQConnection *con = server->getConnection();
			if (con) {
				string name = _bstr_t(m_bstrDisplayName);
				ErrorCode ec;
				if ((ec=con->UserGetPermissions(name, modifyQueues, modifyUsers, modifyGroups)) == EC_NOERROR) {
					ec = con->UserGetGroups(name, groups);
				}
				if (ec == EC_NETWORKERROR)
					server->setBad();
			} else {
				modifyQueues = modifyUsers = modifyGroups = false;
			}
			
			
			VariantVector<BSTR, VT_BSTR>	vgroups(groups.size());

			vgroups.lock();
			for(vector<string>::size_type x = 0; x < groups.size(); x++) {
				vgroups[x] = _bstr_t(groups[x].c_str()).copy();
			}
			vgroups.unlock();

			this->DispEventAdvise(editor);

			editor->Initialize(bname, bdescription, modifyUsers, modifyGroups, modifyQueues, vgroups);

		}
	}
	return S_FALSE;
}



HRESULT CUserNode::OnChangePassword(BSTR newPassword)
{
	_bstr_t pwd = newPassword;

	MQConnection *con = server->getConnection();
	if (con) {
		ErrorCode	ec;

		if ((ec=con->SetPassword((const char*)bname, (const char*)pwd)) != EC_NOERROR) {
			if (ec == EC_NETWORKERROR) {
				server->setBad();
			} else if (ec == EC_NOTAUTHORIZED) {
				HWND hwnd;
				getConsole()->GetMainWindow(&hwnd);
				::MessageBox(hwnd, "Sorry, you are not authorized to change the password for this user", "Password", MB_ICONERROR | MB_OK);
			}
		}
	}

	return S_OK;
}

HRESULT CUserNode::OnApplyChanges(BOOL modifyUsers, BOOL modifyGroups, BOOL modifyQueues, VARIANT groupList)
{
	MQConnection *con = server->getConnection();
	if (con) {
		string		username = (const char*)_bstr_t(bname);
		ErrorCode	ec;

		if ((ec=con->UserSetPermissions(username, modifyQueues!=0, modifyUsers!=0, modifyGroups!=0)) != EC_NOERROR) {
			if (ec == EC_NOTAUTHORIZED) {		
				HWND hwnd;

				getConsole()->GetMainWindow(&hwnd);
				::MessageBox(hwnd, "Sorry you are not authorized to change the permissions for this user", "Error", MB_ICONERROR | MB_OK);
			} else if (ec == EC_NETWORKERROR) {
				server->setBad();
			}
		} else {
			this->modifyUsers = modifyUsers != 0;
			this->modifyGroups = modifyGroups != 0;
			this->modifyQueues = modifyQueues != 0;

			VariantVector<BSTR, VT_BSTR>	vgl(groupList);
			int								count = vgl.getBounds();
			vector<string>					newgroups;

			vgl.lock();
			for(int x = 0; x < count; x++) {
				newgroups.push_back((const char*)_bstr_t(vgl[x]));
			}
			vgl.unlock();

			vector<string>				dels;
			vector<string>::iterator	i;
			vector<string>::iterator	found;

			for(i = groups.begin(); i != groups.end(); i++) {
				found = find(newgroups.begin(), newgroups.end(), *i);
				if ( found != newgroups.end() ) {
					newgroups.erase(found);
				} else {
					dels.push_back(*i);
				}
			}

			// TODO: Handle return codes			
			// delete dels
			for (i=dels.begin(); i != dels.end(); i++)
				con->GroupDeleteUser(*i, username);
			
			// add newgroups
			for (i=newgroups.begin(); i != newgroups.end(); i++)
				con->GroupAddUser(*i, username);

			con->UserGetGroups(username, groups);
		}
	}

	return S_OK;
}

HRESULT CUserNode::OnGetAvailableGroups(VARIANT* groupList)
{
	MQConnection* con = server->getConnection();
	if (con) {
		MQConnection::NAME_VECTOR				gl;
		
		ErrorCode	ec;
		if ((ec=con->EnumerateGroups(gl)) == EC_NOERROR) {
			VariantVector<BSTR, VT_BSTR>	vgs(gl.size(), false); // don't take ownership, allow the OCX to destroy

			vgs.lock();
			for(MQConnection::NAME_VECTOR::size_type x = 0; x < gl.size(); x++) {
				vgs[x] = _bstr_t(gl[x].c_str()).copy();
			}
			vgs.unlock();

			*groupList = vgs;
		}
		if (ec == EC_NETWORKERROR) {
			server->setBad();
		}
	}

	return S_OK;
}




// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////


// ////////////////////////////////////////////////////////////////////////////
// CGroupNode Implementation


// {E036605E-98AC-4a6e-9F62-701685172409}
static const GUID CGroupNode_NODETYPE = 
{ 0xe036605e, 0x98ac, 0x4a6e, { 0x9f, 0x62, 0x70, 0x16, 0x85, 0x17, 0x24, 0x9 } };


const GUID*		CGroupNode::m_NODETYPE = &CGroupNode_NODETYPE;
const OLECHAR*	CGroupNode::m_SZNODETYPE = OLESTR("E036605E-98AC-4a6e-9F62-701685172409");
const OLECHAR*	CGroupNode::m_SZDISPLAY_NAME = OLESTR("");
const CLSID*	CGroupNode::m_SNAPIN_CLASSID = &CLSID_SAFMQManagerRoot;

_ATL_FUNC_INFO CGroupNode::userApplyChanges = {CC_STDCALL, VT_EMPTY, 4, {VT_BOOL, VT_BOOL, VT_BOOL, VT_VARIANT}};
_ATL_FUNC_INFO CGroupNode::userGetAvailableGroups = { CC_STDCALL, VT_VARIANT, 0, {VT_EMPTY}};



CGroupNode::CGroupNode(CSafmqServerNode* server, const string& name)
		: CNode<CGroupNode, CSAFMQManagerRoot, CSAFMQManagerRootComponent>(server)
{
	memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
	m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
	m_scopeDataItem.displayname = MMC_CALLBACK;
	m_scopeDataItem.nImage = IMG_GROUP; 		// May need modification
	m_scopeDataItem.nOpenImage = IMG_GROUP; 	// May need modification
	m_scopeDataItem.lParam = (LPARAM) this;
	m_scopeDataItem.cChildren = 0;


	m_bstrDisplayName.Empty();
	m_bstrDisplayName = name.c_str();

	memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	m_resultDataItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
	m_resultDataItem.str = MMC_CALLBACK;
	m_resultDataItem.nImage = IMG_GROUP;
	m_resultDataItem.lParam = (LPARAM) this;

	this->server = server;

	modifyQueues = modifyUsers = modifyGroups = false;


}

CGroupNode::~CGroupNode()
{
	if (editor != NULL) {
		this->DispEventUnadvise(editor);
	}
}

HRESULT CGroupNode::OnDelete(bool& bHandled, CSnapInObjectRootBase* pObj)
{
	server->DeleteGroup(m_bstrDisplayName, this);
	return S_OK;
}

HRESULT CGroupNode::OnInitOCX(IConsole* spConsole, IUnknown* myOCX) {
	if (myOCX) {
		if (editor != NULL) {
			this->DispEventUnadvise(editor);
		}

		editor = NULL;
		myOCX->QueryInterface(&editor);

		if (editor) {
			MQConnection *con = server->getConnection();
			ErrorCode	ec;

			if (con) {
				string name = (const char*)_bstr_t(m_bstrDisplayName);
				if ((ec=con->GroupGetUsers(name, users)) == EC_NOERROR) {
					if ((ec=con->GroupGetPermissions(name, modifyQueues, modifyUsers, modifyGroups)) != EC_NOERROR) {
						if (ec != EC_DOESNOTEXIST) { // ACL Has not been set for the group.
							if (ec == EC_NETWORKERROR) {
								server->setBad();
							}	
						}
					}
				} else {
					if (ec == EC_NETWORKERROR) {
						server->setBad();
					}	
				}
			}

			VariantVector<BSTR, VT_BSTR>	vusers(users.size());

			vusers.lock();
			for(vector<string>::size_type x = 0; x < users.size(); x++) {
				vusers[x] = _bstr_t(users[x].c_str()).copy();
			}
			vusers.unlock();

			this->DispEventAdvise(editor);

			editor->Initialize(m_bstrDisplayName, modifyUsers, modifyGroups, modifyQueues, vusers);

		}
	}
	return S_FALSE;
}

HRESULT CGroupNode::OnApplyChanges(BOOL modifyUsers, BOOL modifyGroups, BOOL modifyQueues, VARIANT userList)
{
	MQConnection *con = server->getConnection();
	if (con) {
		string		groupname = (const char*)_bstr_t(m_bstrDisplayName);
		ErrorCode	ec;

		if ((ec=con->GroupSetPermissions(groupname, modifyQueues!=0, modifyUsers!=0, modifyGroups!=0)) != EC_NOERROR) {
			if (ec == EC_NETWORKERROR) {
				server->setBad();
			} else if (ec == EC_NOTAUTHORIZED) {
				HWND hwnd;
				getConsole()->GetMainWindow(&hwnd);
				::MessageBox(hwnd, "Sorry you are not authorized to set this group's permissions", "Error", MB_ICONERROR | MB_OK);
			}
		} else {
			this->modifyUsers = modifyUsers != 0;
			this->modifyGroups = modifyGroups != 0;
			this->modifyQueues = modifyQueues != 0;

			VariantVector<BSTR, VT_BSTR>	vul(userList);
			int								count = vul.getBounds();
			vector<string>					newusers;

			vul.lock();
			for(int x = 0; x < count; x++) {
				newusers.push_back((const char*)_bstr_t(vul[x]));
			}
			vul.unlock();

			vector<string>				dels;
			vector<string>::iterator	i;
			vector<string>::iterator	found;

			for(i = users.begin(); i != users.end(); i++) {
				found = find(newusers.begin(), newusers.end(), *i);
				if ( found != newusers.end() ) {
					newusers.erase(found);
				} else {
					dels.push_back(*i);
				}
			}

			// TODO: Handle return codes			
			// delete dels
			for (i=dels.begin(); i != dels.end(); i++)
				con->GroupDeleteUser(groupname, *i);
			
			// add newusers
			for (i=newusers.begin(); i != newusers.end(); i++)
				con->GroupAddUser(groupname, *i);

			con->GroupGetUsers(groupname, users);
		}
	}

	return S_OK;
}

HRESULT CGroupNode::OnGetAvailableUsers(VARIANT* groupList)
{
	MQConnection* con = server->getConnection();
	if (con) {
		MQConnection::USER_VECTOR				ul;

		ErrorCode ec;
		if ((ec=con->EnumerateUsers(ul)) == EC_NOERROR) {
			VariantVector<BSTR, VT_BSTR>	vusers(ul.size(), false); // don't take ownership, allow the OCX to destroy

			vusers.lock();
			for(MQConnection::USER_VECTOR::size_type x = 0; x < ul.size(); x++) {
				vusers[x] = _bstr_t(ul[x].name.c_str()).copy();
			}
			vusers.unlock();

			*groupList = vusers;
		}
		if (ec == EC_NETWORKERROR)
			server->setBad();
	}

	return S_OK;
}


// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////


// ////////////////////////////////////////////////////////////////////////////
// CQueueNode Implementation


// {460C2303-D0ED-4688-8698-436D3A4FCF16}
static const GUID CQueueNode_NODETYPE = 
{ 0x460c2303, 0xd0ed, 0x4688, { 0x86, 0x98, 0x43, 0x6d, 0x3a, 0x4f, 0xcf, 0x16 } };


const GUID*		CQueueNode::m_NODETYPE = &CQueueNode_NODETYPE;
const OLECHAR*	CQueueNode::m_SZNODETYPE = OLESTR("460C2303-D0ED-4688-8698-436D3A4FCF16");
const OLECHAR*	CQueueNode::m_SZDISPLAY_NAME = OLESTR("");
const CLSID*	CQueueNode::m_SNAPIN_CLASSID = &CLSID_SAFMQManagerRoot;

CQueueNode::CQueueNode(CSafmqServerNode* server, const string& name)
		: CNode<CQueueNode, CSAFMQManagerRoot, CSAFMQManagerRootComponent>(server)
{
	memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
	m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
	m_scopeDataItem.displayname = MMC_CALLBACK;
	m_scopeDataItem.nImage = IMG_QUEUE; 		// May need modification
	m_scopeDataItem.nOpenImage = IMG_QUEUE; 	// May need modification
	m_scopeDataItem.lParam = (LPARAM) this;
	m_scopeDataItem.cChildren = 0;


	m_bstrDisplayName.Empty();
	m_bstrDisplayName = name.c_str();

	memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	m_resultDataItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
	m_resultDataItem.str = MMC_CALLBACK;
	m_resultDataItem.nImage = IMG_QUEUE;
	m_resultDataItem.lParam = (LPARAM) this;

	this->server = server;

}

CQueueNode::~CQueueNode()
{
	FreePermissions();
}

void CQueueNode::LoadPermissions() {
	FreePermissions();

	MQConnection* con = server->getConnection();
	if (con) {
		vector<safmq::QueuePermissions>	qp;
		ErrorCode	ec;
		if ((ec=con->QueueEnumeratePermissions((const char*)_bstr_t(m_bstrDisplayName), qp)) == EC_NOERROR) {
			for(vector<safmq::QueuePermissions>::size_type x=0; x < qp.size(); x++) {
				perms.push_back(new CQueuePerm(this, qp[x].entity, qp[x].isgroup, qp[x].read, qp[x].write, qp[x].change_security, qp[x].destroy));
			}
		} else if (ec == EC_NOERROR) {
			server->setBad();
		}
	}
}

void CQueueNode::FreePermissions()
{
	for(vector<CQueuePerm*>::size_type x=0;x<perms.size();x++)
		delete perms[x];
	perms.clear();
}

HRESULT CQueueNode::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me) {
	if (me > 0)
		m_scopeDataItem.ID = me;

	CComQIPtr<IResultData, &IID_IResultData> spResultData(pConsole);
	CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeaderCtrl(pConsole);

	spResultData->DeleteAllRsltItems();

	LoadPermissions();

	if (bShow) {
		if (me != -1) {
			// Don't add columns if flagged as a refresh
			spHeaderCtrl->InsertColumn( 0, L"Actor Name", LVCFMT_LEFT,  MMCLV_AUTO);
			spHeaderCtrl->InsertColumn( 1, L"Read", LVCFMT_CENTER,  MMCLV_AUTO);
			spHeaderCtrl->InsertColumn( 2, L"Write", LVCFMT_CENTER,  MMCLV_AUTO);
			spHeaderCtrl->InsertColumn( 3, L"Control Security", LVCFMT_CENTER,  MMCLV_AUTO);
			spHeaderCtrl->InsertColumn( 4, L"Destroy", LVCFMT_CENTER,  MMCLV_AUTO);
		}
		RESULTDATAITEM	*item;

		for(vector<CQueuePerm*>::size_type x = 0; x < perms.size(); x++) {
			perms[x]->GetResultData(&item);
			spResultData->InsertItem(item);
		}
	}
	return S_OK;
}

HRESULT CQueueNode::OnAddActor(bool& bHandled, CSnapInObjectRootBase* pObj)
{
	ATLTRACE("CQueueNode::OnAddActor\n");
	// TODO: Add Actor
	MQConnection*	con = server->getConnection();



	if (con) {
		CAddActorDlg				dlg;
		MQConnection::USER_VECTOR	uv;



		ErrorCode					ec;

		if ((ec=con->EnumerateGroups(dlg.groups)) == EC_NOERROR) {
			if ((ec=con->EnumerateUsers(uv)) == EC_NOERROR) {
				vector<CQueuePerm*>::iterator	i;
				string							name;
				MQConnection::USER_VECTOR::iterator uvi;

				for(i = perms.begin(); i != perms.end(); ++i) {
					name = (const char*)_bstr_t((*i)->m_bstrDisplayName);
					
					if ((*i)->IsGroup()) {
						vector<string>::iterator rem = find(dlg.groups.begin(), dlg.groups.end(), name );
						if (rem != dlg.groups.end())
							dlg.groups.erase(rem);
					} else {
						for(uvi = uv.begin(); uvi != uv.end(); ++uvi) {
							if (uvi->name == name) {
								uv.erase(uvi);
								break;
							}
						}
					}
				}

				for(uvi = uv.begin(); uvi != uv.end(); ++uvi)
					dlg.users.push_back(uvi->name);


				if (dlg.DoModal() == IDOK) {
					vector<string>::iterator i;
					string qname = (const char*)_bstr_t(m_bstrDisplayName);

					for(i = dlg.groups.begin(); i != dlg.groups.end(); ++i) {
						con->QueueSetGroupPermission(qname, *i, true, true, false, false);
					}
					for(i = dlg.users.begin(); i != dlg.users.end(); ++i) {
						con->QueueSetUserPermission(qname, *i, true, true, false, false);
					}

					getConsole()->SelectScopeItem(m_scopeDataItem.ID); // Select Me to keep from putting the actors in the wrong place;

					CComPtr<IDataObject>	pdata;
					GetDataObject(&pdata, CCT_SCOPE);
					getConsole()->UpdateAllViews(pdata, 0, 0);

				}
			}
		}
	}

	return S_OK;
}

HRESULT CQueueNode::OnDelete(bool& bHandled, CSnapInObjectRootBase* pObj)
{
	server->DeleteQueue(m_bstrDisplayName, this);
	return S_OK;
}

void CQueueNode::DeletePermNode(CQueuePerm* theNode)
{
	MQConnection* con = server->getConnection();
	if (con) {
		ErrorCode ec;
		if (theNode->IsGroup())
			ec = con->QueueDeleteGroupPermission((const char*)_bstr_t(m_bstrDisplayName), (const char*)_bstr_t(theNode->m_bstrDisplayName));
		else
			ec = con->QueueDeleteUserPermission((const char*)_bstr_t(m_bstrDisplayName), (const char*)_bstr_t(theNode->m_bstrDisplayName));
		if (ec == EC_NOERROR) {
			vector<CQueuePerm*>::iterator i = find(perms.begin(), perms.end(), theNode);
			if (i != perms.end()) {
				perms.erase(i);

				CComPtr<IDataObject>	pdata;
				GetDataObject(&pdata, CCT_SCOPE);
				getConsole()->UpdateAllViews(pdata, 0, 0);
			}
		}
	}
}

HRESULT CQueueNode::OnViewChange(IConsole* pConsole, long arg, long param)
{
	return OnShow(pConsole, TRUE, -1);
}


// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////


// ////////////////////////////////////////////////////////////////////////////
// CQueuePerm Implementation




// {0B5DFF2D-9D23-432d-8BB5-6D363FD6772A}
static const GUID CQueuePerm_NODETYPE = 
{ 0xb5dff2d, 0x9d23, 0x432d, { 0x8b, 0xb5, 0x6d, 0x36, 0x3f, 0xd6, 0x77, 0x2a } };


const GUID*		CQueuePerm::m_NODETYPE = &CQueuePerm_NODETYPE;
const OLECHAR*	CQueuePerm::m_SZNODETYPE = OLESTR("0B5DFF2D-9D23-432d-8BB5-6D363FD6772A");
const OLECHAR*	CQueuePerm::m_SZDISPLAY_NAME = OLESTR("");
const CLSID*	CQueuePerm::m_SNAPIN_CLASSID = &CLSID_SAFMQManagerRoot;

CQueuePerm::CQueuePerm(CQueueNode* parent, const string& name, bool isGroup, bool read, bool write, bool security, bool destroy)
		: CNode<CQueuePerm, CSAFMQManagerRoot, CSAFMQManagerRootComponent>(parent)
{
	memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));

	m_bstrDisplayName.Empty();
	m_bstrDisplayName = name.c_str();

	memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	m_resultDataItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
	m_resultDataItem.str = MMC_CALLBACK;
	m_resultDataItem.nImage = isGroup ? IMG_GROUP : IMG_USER;
	m_resultDataItem.lParam = (LPARAM) this;

	this->read = read;
	this->write = write;
	this->security = security;
	this->destroy = destroy;
	this->isGroup = isGroup;
	this->parent = parent;
}

CQueuePerm::~CQueuePerm()
{

}

HRESULT CQueuePerm::OnDelete(bool& bHandled, CSnapInObjectRootBase* pObj)
{
	parent->DeletePermNode(this);
	return S_OK;
}

HRESULT CQueuePerm::OnSecurityProperties(bool& bHandled, CSnapInObjectRootBase* pObj)
{
	string name = (const char*)_bstr_t(m_bstrDisplayName);

	CActorPermsDlg	dlg(name, isGroup, read, write, security, destroy);

	if (dlg.DoModal() == IDOK) {
		MQConnection* con = parent->getServer()->getConnection();
		if (con) {
			ErrorCode ec;
			string qname = (const char*)_bstr_t(parent->m_bstrDisplayName);

			if (isGroup)
				ec = con->QueueSetGroupPermission(qname, name, dlg.getRead(), dlg.getWrite(), dlg.getDestroy(), dlg.getSecurity());
			else
				ec = con->QueueSetUserPermission(qname, name, dlg.getRead(), dlg.getWrite(), dlg.getDestroy(), dlg.getSecurity());
 
			if (ec == EC_NOERROR) {
				CComPtr<IDataObject>	pdata;
				parent->GetDataObject(&pdata, CCT_SCOPE);
				getConsole()->UpdateAllViews(pdata, 0, 0);
			} else if (ec == EC_NOTAUTHORIZED) {
				HWND hwnd;
				getConsole()->GetMainWindow(&hwnd);
				::MessageBox(hwnd, "Sorry, you are not authorized to change security on this queue", "Queue Permissions", MB_ICONERROR | MB_OK);
			} else if (ec == EC_NETWORKERROR) {
				parent->getServer()->setBad();
			}

		} else {
			// TODO: Handle no connection
		}

	}

	return S_OK;
}

HRESULT CQueuePerm::OnAddActor(bool& bHandled, CSnapInObjectRootBase* pObj)
{
	return parent->OnAddActor(bHandled, pObj);
}


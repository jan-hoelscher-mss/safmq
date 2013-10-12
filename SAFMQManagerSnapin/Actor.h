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
// Actor.h: interface for the CUserNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACTOR_H__FE4B666D_B0DA_460B_AB90_B39D7D09E9B0__INCLUDED_)
#define AFX_ACTOR_H__FE4B666D_B0DA_460B_AB90_B39D7D09E9B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Node.h"
#include "SafmqServerNode.h"


class CSAFMQManagerRoot;
class CSAFMQManagerRootComponent;

#define CUE_ID	100
#define CGE_ID	100

class CUserNode : public CNode<CUserNode, CSAFMQManagerRoot, CSAFMQManagerRootComponent>,
				public IDispEventSimpleImpl<CUE_ID, CUserNode, &DIID__IUserEditorEvents>
{
private:

	static _ATL_FUNC_INFO userApplyChanges;
	static _ATL_FUNC_INFO userChangePassword;
	static _ATL_FUNC_INFO userGetAvailableGroups;

public:
	BEGIN_SINK_MAP(CUserNode)
		SINK_ENTRY_INFO(CUE_ID, DIID__IUserEditorEvents, 0x01, OnChangePassword, &userChangePassword)
		SINK_ENTRY_INFO(CUE_ID, DIID__IUserEditorEvents, 0x02, OnApplyChanges, &userApplyChanges)
		SINK_ENTRY_INFO(CUE_ID, DIID__IUserEditorEvents, 0x03, OnGetAvailableGroups, &userGetAvailableGroups)
	END_SINK_MAP()

	STDMETHOD(OnChangePassword)(BSTR newPassword);
	STDMETHOD(OnApplyChanges)(BOOL modifyUsers, BOOL modifyGroups, BOOL modifyQueues, VARIANT groupList);
	STDMETHOD(OnGetAvailableGroups)(VARIANT* groupList);


private:
	CSafmqServerNode*	server;

	_bstr_t		bname;
	_bstr_t		bdescription;

	CComPtr<IUserEditor>	editor;

	vector<string>		groups;
	bool				modifyQueues;
	bool				modifyGroups;
	bool				modifyUsers;

public:
	static const GUID*		m_NODETYPE;
	static const OLECHAR*	m_SZNODETYPE;
	static const OLECHAR*	m_SZDISPLAY_NAME;
	static const CLSID*		m_SNAPIN_CLASSID;

public:
	SNAPINMENUID(IDR_DELETABLE_MENU)

	BEGIN_SNAPINCOMMAND_MAP(CUserNode, FALSE)
		SNAPINCOMMAND_ENTRY(ID_DELETE, OnDelete);
		CHAIN_SNAPINCOMMAND_DELEGATE(*server)
	END_SNAPINCOMMAND_MAP()


	CUserNode(CSafmqServerNode* server, const string& name, const string& description);
	virtual ~CUserNode();

	virtual LPOLESTR GetResultPaneColInfo(int nCol) {
		switch (nCol) {
			case 0:
				return bname;
			case 1:
				return bdescription;
		}
		return OLESTR("override CNode<I,D,C>::GetResultPaneColInfo");
	}

	HRESULT OnDelete(bool& bHandled, CSnapInObjectRootBase* pObj);

	STDMETHOD(GetResultViewType)(LPOLESTR *ppViewType,long *pViewOptions)
	{
		LPOLESTR lpOleStr = OLESTR("{0058631C-749D-41EB-86CD-11EFC20BCF06}");
		*ppViewType = static_cast<LPOLESTR>(CoTaskMemAlloc((wcslen(lpOleStr) + 1) * sizeof(WCHAR)));
		wcscpy(*ppViewType, lpOleStr);

		*pViewOptions = MMC_VIEW_OPTIONS_CREATENEW;
		return S_FALSE;
	}

	virtual HRESULT OnSelect(IConsole *pConsole, IComponent* pComponent ) {
		CComPtr<IUnknown>	pUnk;

		pConsole->QueryResultView(&pUnk);

		CComPtr<IUserEditor>	userEdit;

		if (pUnk) {
			pUnk->QueryInterface(&userEdit);
		}

		return S_FALSE;
	}

	virtual HRESULT OnInitOCX(IConsole* spConsole, IUnknown* myOCX);

};


class CGroupNode : public CNode<CGroupNode, CSAFMQManagerRoot, CSAFMQManagerRootComponent>,
				public IDispEventSimpleImpl<CGE_ID, CGroupNode, &DIID__IGroupEditorEvents>
{
private:

	static _ATL_FUNC_INFO userApplyChanges;
	static _ATL_FUNC_INFO userGetAvailableGroups;

public:
	BEGIN_SINK_MAP(CGroupNode)
		SINK_ENTRY_INFO(CUE_ID, DIID__IGroupEditorEvents, 0x01, OnApplyChanges, &userApplyChanges)
		SINK_ENTRY_INFO(CUE_ID, DIID__IGroupEditorEvents, 0x02, OnGetAvailableUsers, &userGetAvailableGroups)
	END_SINK_MAP()

	STDMETHOD(OnApplyChanges)(BOOL modifyUsers, BOOL modifyGroups, BOOL modifyQueues, VARIANT userList);
	STDMETHOD(OnGetAvailableUsers)(VARIANT* groupList);


private:
	CSafmqServerNode*	server;

	vector<string>		users;
	bool				modifyQueues;
	bool				modifyGroups;
	bool				modifyUsers;

	CComPtr<IGroupEditor>	editor;
public:
	static const GUID*		m_NODETYPE;
	static const OLECHAR*	m_SZNODETYPE;
	static const OLECHAR*	m_SZDISPLAY_NAME;
	static const CLSID*		m_SNAPIN_CLASSID;

public:
	SNAPINMENUID(IDR_DELETABLE_MENU)

	BEGIN_SNAPINCOMMAND_MAP(CUserNode, FALSE)
		SNAPINCOMMAND_ENTRY(ID_DELETE, OnDelete);
		CHAIN_SNAPINCOMMAND_DELEGATE(*server)
	END_SNAPINCOMMAND_MAP()

	CGroupNode(CSafmqServerNode* server, const string& name);
	virtual ~CGroupNode();

	HRESULT OnDelete(bool& bHandled, CSnapInObjectRootBase* pObj);

	STDMETHOD(GetResultViewType)(LPOLESTR *ppViewType,long *pViewOptions)
	{
		LPOLESTR lpOleStr = OLESTR("{D7E8D2B8-BE9C-460C-8F0D-60341708A679}");
		*ppViewType = static_cast<LPOLESTR>(CoTaskMemAlloc((wcslen(lpOleStr) + 1) * sizeof(WCHAR)));
		wcscpy(*ppViewType, lpOleStr);

		*pViewOptions = MMC_VIEW_OPTIONS_CREATENEW;
		return S_FALSE;
	}

	virtual HRESULT OnInitOCX(IConsole* spConsole, IUnknown* myOCX);

};

class CQueueNode;

class CQueuePerm : public CNode<CQueuePerm, CSAFMQManagerRoot, CSAFMQManagerRootComponent>
{
private:
	bool	isGroup, read, write, security, destroy;

	CQueueNode* parent;

public:
	static const GUID*		m_NODETYPE;
	static const OLECHAR*	m_SZNODETYPE;
	static const OLECHAR*	m_SZDISPLAY_NAME;
	static const CLSID*		m_SNAPIN_CLASSID;

public:
	SNAPINMENUID(IDR_QUEUE_PERM_MENU)

	BEGIN_SNAPINCOMMAND_MAP(CQueuePerm, FALSE)
		SNAPINCOMMAND_ENTRY(ID_DELETE, OnDelete);
		SNAPINCOMMAND_ENTRY(ID_SECURITY_PROPERTIES, OnSecurityProperties);
		SNAPINCOMMAND_ENTRY(ID_ADD_ACTOR, OnAddActor);
	END_SNAPINCOMMAND_MAP()


	CQueuePerm(CQueueNode* parent, const string& name, bool isGroup, bool read, bool write, bool security, bool destroy);
	~CQueuePerm();

	HRESULT OnDelete(bool& bHandled, CSnapInObjectRootBase* pObj);
	HRESULT OnSecurityProperties(bool& bHandled, CSnapInObjectRootBase* pObj);
	HRESULT OnAddActor(bool& bHandled, CSnapInObjectRootBase* pObj);

	bool IsGroup() { return isGroup; }

	virtual LPOLESTR GetResultPaneColInfo(int nCol) {
		switch (nCol) {
			case 0:
				return m_bstrDisplayName;
			case 1:
				return read ? OLESTR("X") : OLESTR("");
			case 2:
				return write ? OLESTR("X") : OLESTR("");
			case 3:
				return security ? OLESTR("X") : OLESTR("");
			case 4:
				return destroy ? OLESTR("X") : OLESTR("");
		}
		return OLESTR("override CNode<I,D,C>::GetResultPaneColInfo");
	}
};


class CQueueNode : public CNode<CQueueNode, CSAFMQManagerRoot, CSAFMQManagerRootComponent>
{
private:
	CSafmqServerNode*			server;
	vector<CQueuePerm*>			perms;
public:
	static const GUID*		m_NODETYPE;
	static const OLECHAR*	m_SZNODETYPE;
	static const OLECHAR*	m_SZDISPLAY_NAME;
	static const CLSID*		m_SNAPIN_CLASSID;

public:
	SNAPINMENUID(IDR_QUEUE_MENU)

	BEGIN_SNAPINCOMMAND_MAP(CQueueNode, FALSE)
		SNAPINCOMMAND_ENTRY(ID_DELETE, OnDelete);
		SNAPINCOMMAND_ENTRY(ID_ADD_ACTOR, OnAddActor);
		CHAIN_SNAPINCOMMAND_DELEGATE(*server)
	END_SNAPINCOMMAND_MAP()

	CQueueNode(CSafmqServerNode* server, const string& name);
	virtual ~CQueueNode();

	virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me);

	HRESULT OnAddActor(bool& bHandled, CSnapInObjectRootBase* pObj);
	HRESULT OnDelete(bool& bHandled, CSnapInObjectRootBase* pObj);

	void DeletePermNode(CQueuePerm* theNode);


	virtual HRESULT OnViewChange(IConsole* pConsole, long arg, long param);

	CSafmqServerNode*	getServer() { return server; }

protected:
	void LoadPermissions();
	void FreePermissions();
};



#endif // !defined(AFX_ACTOR_H__FE4B666D_B0DA_460B_AB90_B39D7D09E9B0__INCLUDED_)

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
// UserFolder.h: interface for the CUserFolder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERFOLDER_H__983C31CE_5C23_4006_82AC_457345EDF17C__INCLUDED_)
#define AFX_USERFOLDER_H__983C31CE_5C23_4006_82AC_457345EDF17C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Node.h"
#include "SafmqServerNode.h"

class CSAFMQManagerRoot;
class CSAFMQManagerRootComponent;


class CUserFolder : public CNode<CUserFolder, CSAFMQManagerRoot, CSAFMQManagerRootComponent>
{
private:
	CSafmqServerNode*		server;
	vector<CSnapInItem*>	children;

public:
	static const GUID*		m_NODETYPE;
	static const OLECHAR*	m_SZNODETYPE;
	static const OLECHAR*	m_SZDISPLAY_NAME;
	static const CLSID*		m_SNAPIN_CLASSID;

public:
	SNAPINMENUID(IDR_FOLDER_MENU)

	BEGIN_SNAPINCOMMAND_MAP(CSafmqServerNode, FALSE)
		SNAPINCOMMAND_ENTRY(ID_NEW_USER, OnNewUser);
		CHAIN_SNAPINCOMMAND_DELEGATE(*server)
	END_SNAPINCOMMAND_MAP()

	
	CUserFolder(CSafmqServerNode* server);
	virtual ~CUserFolder();


	virtual HRESULT OnExpand(BOOL bExpand, IConsole *pConsole, HSCOPEITEM me);
	virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me);

	virtual HRESULT OnNewUser(bool& handled, CSnapInObjectRootBase* pBase);
	virtual HRESULT OnViewChange(IConsole* pConsole, long arg, long param);


	void DeleteUser(BSTR userName, CSnapInItem* userItem);

private:
	virtual void LoadUsers();
	virtual void FreeUsers();
};

class CGroupFolder : public CNode<CGroupFolder, CSAFMQManagerRoot, CSAFMQManagerRootComponent>
{
private:
	CSafmqServerNode*		server;
	vector<CSnapInItem*>	children;

public:
	static const GUID*		m_NODETYPE;
	static const OLECHAR*	m_SZNODETYPE;
	static const OLECHAR*	m_SZDISPLAY_NAME;
	static const CLSID*		m_SNAPIN_CLASSID;

public:
	SNAPINMENUID(IDR_FOLDER_MENU)

	BEGIN_SNAPINCOMMAND_MAP(CSafmqServerNode, FALSE)
		SNAPINCOMMAND_ENTRY(ID_NEW_GROUP, OnNewGroup);
		CHAIN_SNAPINCOMMAND_DELEGATE(*server)
	END_SNAPINCOMMAND_MAP()

	CGroupFolder(CSafmqServerNode* server);
	virtual ~CGroupFolder();

	virtual HRESULT OnExpand(BOOL bExpand, IConsole *pConsole, HSCOPEITEM me);
	virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me);

	virtual HRESULT OnNewGroup(bool& handled, CSnapInObjectRootBase* pBase);
	virtual HRESULT OnViewChange(IConsole* pConsole, long arg, long param);

	void DeleteGroup(BSTR groupName, CSnapInItem* groupItem);

private:
	virtual void LoadGroups();
	virtual void FreeGroups();
};

class CQueueFolder : public CNode<CQueueFolder, CSAFMQManagerRoot, CSAFMQManagerRootComponent>
{
private:
	CSafmqServerNode*		server;
	vector<CSnapInItem*>	children;

public:
	static const GUID*		m_NODETYPE;
	static const OLECHAR*	m_SZNODETYPE;
	static const OLECHAR*	m_SZDISPLAY_NAME;
	static const CLSID*		m_SNAPIN_CLASSID;

public:
	SNAPINMENUID(IDR_FOLDER_MENU)

	BEGIN_SNAPINCOMMAND_MAP(CSafmqServerNode, FALSE)
		SNAPINCOMMAND_ENTRY(ID_NEW_QUEUE, OnNewQueue);
		CHAIN_SNAPINCOMMAND_DELEGATE(*server)
	END_SNAPINCOMMAND_MAP()

	CQueueFolder(CSafmqServerNode* server);
	virtual ~CQueueFolder();

	virtual HRESULT OnExpand(BOOL bExpand, IConsole *pConsole, HSCOPEITEM me);
	virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me);

	virtual HRESULT OnNewQueue(bool& handled, CSnapInObjectRootBase* pBase);
	virtual HRESULT OnViewChange(IConsole* pConsole, long arg, long param);


	void DeleteQueue(BSTR queueName, CSnapInItem* queueItem);

private:
	virtual void LoadQueues();
	virtual void FreeQueues();

};


#endif // !defined(AFX_USERFOLDER_H__983C31CE_5C23_4006_82AC_457345EDF17C__INCLUDED_)

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
// SafmqServerNode.h: interface for the CSafmqServerNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SAFMQSERVERNODE_H__235C2710_EB9A_481C_B777_CB3517302203__INCLUDED_)
#define AFX_SAFMQSERVERNODE_H__235C2710_EB9A_481C_B777_CB3517302203__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include <atlsnap.h>
#include "SAFMQManagerRoot.h"
#include "Node.h"
#include <string>

namespace safmq {
	class MQConnection;
}


class CUserFolder;
class CGroupFolder;
class CQueueFolder;

class CServerList;

class CSafmqServerNode : public CNode<CSafmqServerNode, CSAFMQManagerRoot, CSAFMQManagerRootComponent>
{
protected:
	string					url;
	string					user;
	string					password;
	bool					savePasswd;
	bool					validPasswd;

	safmq::MQConnection*	con;

	CUserFolder				*users;
	CGroupFolder			*groups;
	CQueueFolder			*queues;

	CServerList				*serverList;

public:
	static const GUID*		CSafmqServerNode::m_NODETYPE;
	static const OLECHAR*	CSafmqServerNode::m_SZNODETYPE;
	static const OLECHAR*	CSafmqServerNode::m_SZDISPLAY_NAME;
	static const CLSID*		CSafmqServerNode::m_SNAPIN_CLASSID;

public:
	SNAPINMENUID(IDR_SERVER_MENU)

	BEGIN_SNAPINCOMMAND_MAP(CSafmqServerNode, FALSE)
		SNAPINCOMMAND_ENTRY(ID_NEW_USER, OnNewUser);
		SNAPINCOMMAND_ENTRY(ID_NEW_GROUP, OnNewGroup);
		SNAPINCOMMAND_ENTRY(ID_NEW_QUEUE, OnNewQueue);
		SNAPINCOMMAND_ENTRY(ID_DELETE, OnDelete);
		SNAPINCOMMAND_ENTRY(ID_CONNECT, OnConnect);
		SNAPINCOMMAND_ENTRY(ID_DISCONNECT, OnDisconnect);
	END_SNAPINCOMMAND_MAP()


	CSafmqServerNode(CServerList* serverList, const string& url, const string& user, const string& password, bool savePasswd);
	CSafmqServerNode(CServerList* serverList);

	virtual ~CSafmqServerNode();

	void	cleanup();


	virtual HRESULT OnExpand(BOOL bExpand, IConsole *pConsole, HSCOPEITEM me);
	virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me);
	virtual HRESULT OnViewChange(IConsole* pConsole, long arg, long param);

	virtual HRESULT OnNewGroup(bool& handled, CSnapInObjectRootBase* pBase);
	virtual HRESULT OnNewUser(bool& handled, CSnapInObjectRootBase* pBase);
	virtual HRESULT OnNewQueue(bool& handled, CSnapInObjectRootBase* pBase);
	virtual HRESULT OnDelete(bool& handled, CSnapInObjectRootBase* pBase);
	virtual HRESULT OnConnect(bool& handled, CSnapInObjectRootBase* pBase);
	virtual HRESULT OnDisconnect(bool& handled, CSnapInObjectRootBase* pBase);


	void DeleteQueue(BSTR queueName, CSnapInItem* queueItem);
	void DeleteUser(BSTR userName, CSnapInItem* userItem);
	void DeleteGroup(BSTR groupName, CSnapInItem* groupItem);


	safmq::MQConnection* getConnection();
	void setBad();

	STDMETHOD(Load)(IStream *pStm);
	STDMETHOD(Save)(IStream *pStm);

private:
	string getURL();
	void init();
};

#endif // !defined(AFX_SAFMQSERVERNODE_H__235C2710_EB9A_481C_B777_CB3517302203__INCLUDED_)

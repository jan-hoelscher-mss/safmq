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
// ServerList.h: interface for the CServerList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERLIST_H__6CAD9D0F_4E27_4209_B95D_C5D7BDB864F3__INCLUDED_)
#define AFX_SERVERLIST_H__6CAD9D0F_4E27_4209_B95D_C5D7BDB864F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Node.h"
#include <vector>
#include "resource.h"

class CSAFMQManagerRoot;
class CSAFMQManagerRootComponent;

class CServerList : public CNode<CServerList, CSAFMQManagerRoot, CSAFMQManagerRootComponent>
{
	std::vector<CSnapInItem*>	nodes;

public:
	static const GUID*		CServerList::m_NODETYPE;
	static const OLECHAR*	CServerList::m_SZNODETYPE;
	static const OLECHAR*	CServerList::m_SZDISPLAY_NAME;
	static const CLSID*		CServerList::m_SNAPIN_CLASSID;

public:
	BEGIN_SNAPINCOMMAND_MAP(CSAFMQManagerRootNode, FALSE)
		SNAPINCOMMAND_ENTRY(ID_NEW_ADDSAFMQSERVER, AddSAFMQServer);
	END_SNAPINCOMMAND_MAP()

	SNAPINMENUID(IDR_SAFMQMANAGERROOT_MENU)


	CServerList(CConsoleProvider* provider);
	virtual ~CServerList();


	virtual HRESULT OnExpand(BOOL bExpand, IConsole *pConsole, HSCOPEITEM me);
	virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me);

	HRESULT AddSAFMQServer(bool& handled, CSnapInObjectRootBase* pBase);
	HRESULT DeleteServer(CSnapInItem* server);

	virtual HRESULT OnViewChange(IConsole* pConsole, long arg, long param);

	STDMETHOD(Load)(IStream *pStm);
	STDMETHOD(Save)(IStream *pStm);
};

#endif // !defined(AFX_SERVERLIST_H__6CAD9D0F_4E27_4209_B95D_C5D7BDB864F3__INCLUDED_)

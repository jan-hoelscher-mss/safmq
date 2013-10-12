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
// ServerList.cpp: implementation of the CServerList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerList.h"

#include "SAFMQManagerSnapin.h"
#include "SAFMQManagerRoot.h"
#include "SafmqServerNode.h"
#include "NewServerDlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// {2F780D0E-2FD6-4835-AB6C-D27B500D04C0}
static const GUID CServerList_NODETYPE = 
{ 0x2f780d0e, 0x2fd6, 0x4835, { 0xab, 0x6c, 0xd2, 0x7b, 0x50, 0xd, 0x4, 0xc0 } };

const GUID*		CServerList::m_NODETYPE			= &CServerList_NODETYPE;
const OLECHAR*	CServerList::m_SZNODETYPE		= OLESTR("2F780D0E-2FD6-4835-AB6C-D27B500D04C0");
const OLECHAR*	CServerList::m_SZDISPLAY_NAME	= OLESTR("Server List");
const CLSID*	CServerList::m_SNAPIN_CLASSID	= &CLSID_SAFMQManagerRoot;

CServerList::CServerList(CConsoleProvider* provider) 
		: CNode<CServerList, CSAFMQManagerRoot, CSAFMQManagerRootComponent>(provider)
{
	memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
	m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
	m_scopeDataItem.displayname = MMC_CALLBACK;
	m_scopeDataItem.nImage = -1; 		// May need modification
	m_scopeDataItem.nOpenImage = -1; 	// May need modification
	m_scopeDataItem.lParam = (LPARAM) this;
	m_scopeDataItem.cChildren = 1;

	m_bstrDisplayName.Empty();
	m_bstrDisplayName.Attach(OLESTR("SAFMQ Servers"));

	memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	m_resultDataItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
	m_resultDataItem.str = MMC_CALLBACK;
	m_resultDataItem.nImage = -1;
	m_resultDataItem.lParam = (LPARAM) this;

}

CServerList::~CServerList()
{

}


HRESULT CServerList::OnExpand(BOOL bExpand, IConsole *pConsole, HSCOPEITEM me) {
	CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(pConsole);

	if (me > 0)
		m_scopeDataItem.ID = me;
	
	SCOPEDATAITEM	*item;

	for(std::vector<CSnapInItem*>::iterator i = nodes.begin(); i != nodes.end(); i++) {
		CSafmqServerNode	*node = (CSafmqServerNode*)*i;

		node->GetScopeData(&item);

		if (spConsoleNameSpace->GetItem(item) != S_OK) { // only add ones not in the list
			item->relativeID = m_scopeDataItem.ID;
			spConsoleNameSpace->InsertItem(item);
		}
	}

	return S_OK;
}

HRESULT CServerList::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me) {
	if (me > 0)
		m_scopeDataItem.ID = me;

	CComQIPtr<IResultData, &IID_IResultData> spResultData(pConsole);
	CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeaderCtrl(pConsole);

	spResultData->DeleteAllRsltItems();

	if (bShow) {
		if (me != -1) {
			// Don't add columns if flagged as a refresh
			spHeaderCtrl->InsertColumn( 0, L"Name", 0,  MMCLV_AUTO);
		}
	}

	return S_OK;
}


HRESULT CServerList::AddSAFMQServer(bool& handled, CSnapInObjectRootBase* pBase)
{
	CNewServerDlg	dlg;

	handled = true;

	if (dlg.DoModal() == IDOK) {
		if (dlg.getURL().length()) {
			nodes.push_back(new CSafmqServerNode(this, dlg.getURL(), dlg.getUsername(), dlg.getPassword(), dlg.getSavePassword()));
		
			CComPtr<IDataObject>	pdata;
			GetDataObject(&pdata, CCT_SCOPE);
			getConsole()->UpdateAllViews(pdata, 0, 0);
			return S_OK;
		}
	}

	return S_FALSE;
}

HRESULT CServerList::DeleteServer(CSnapInItem* server)
{
	vector<CSnapInItem*>::iterator svr = find(nodes.begin(), nodes.end(), server);
	if (svr != nodes.end()) {
		nodes.erase(svr);
		CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(getConsole());

		if (spConsoleNameSpace) {
			getConsole()->SelectScopeItem(m_scopeDataItem.ID); // select me

			spConsoleNameSpace->DeleteItem(((CSafmqServerNode*)server)->m_scopeDataItem.ID, TRUE); // remove the target

			delete (CSafmqServerNode*)server;

			CComPtr<IDataObject>	pdata;		// call a update
			GetDataObject(&pdata, CCT_SCOPE);
			getConsole()->UpdateAllViews(pdata, 0, 0);
		}
	}
	return S_OK;
}


HRESULT CServerList::OnViewChange(IConsole* pConsole, long arg, long param)
{
	OnExpand(true, pConsole, -1);

	return S_FALSE;
}

HRESULT CServerList::Load(IStream *pStm)
{
	// Load the server nodes from the data stream
	ULONG	read;
	long	count;

	count = 0;
	pStm->Read(&count, sizeof(count), &read);
	for(int x = 0; x< count; x++) {
		CSafmqServerNode*	pserver = new CSafmqServerNode(this);
		pserver->Load(pStm);
		nodes.push_back(pserver);
	}

	return S_OK;
}

HRESULT CServerList::Save(IStream *pStm)
{
	// Store the server nodes to the data stream.
	ULONG	written;
	long	count;

	count = nodes.size();
	pStm->Write(&count, sizeof(count), &written);

	for(int x=0;x<(int)nodes.size();x++)
		((CSafmqServerNode*)nodes[x])->Save(pStm);

	return S_OK;
}

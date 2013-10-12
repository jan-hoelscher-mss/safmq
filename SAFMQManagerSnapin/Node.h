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
// Node.h: interface for the CNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODE_H__73E81A60_8F02_4ACC_9A06_893E19B108CD__INCLUDED_)
#define AFX_NODE_H__73E81A60_8F02_4ACC_9A06_893E19B108CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlsnap.h>

class CConsoleProvider
{
public:
	virtual IConsole*	getConsole() = 0;
};

/*
template <class _TYPE>
inline HRESULT ChainCommandToParent(ProcessCommand(UINT nID, bool& bHandled, CSnapInObjectRootBase* pObj, DATA_OBJECT_TYPES type) {
	if (m_scopeDataItem.relativeID != 0) {
		IConsole*	pConsole = getConsole();
		if (pConsole) {
			CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> pCNS(pConsole);
			SCOPEDATAITEM	item;

			memset(&item, 0,sizeof(item));
			
			item.mask = SDI_PARAM;
			item.ID = m_scopeDataItem.relativeID;
			item.lParam = (LPARAM) this;

			if (pCNS->GetItem(&item) == S_OK) {
				return ((_TYPE*)item.lParam)->ProcessCommand(nID, bHandled, pOBJ, type);
			}
		}
	}
	return S_FALSE;
}
*/


#define CHAIN_SNAPINCOMMAND_DELEGATE(_MEMBER)	\
{	hr = (_MEMBER).ProcessCommand(nID, bHandled, pObj, type); if (bHandled) return hr; }


template<class _implement, class _scope, class _result>
class  CNode : public CSnapInItemImpl<_implement>, public CConsoleProvider //, public CommandHandler
{
private:
	CConsoleProvider		*consoleProvider;

public:
	static const GUID*		m_NODETYPE;
	static const OLECHAR*	m_SZNODETYPE;
	static const OLECHAR*	m_SZDISPLAY_NAME;
	static const CLSID*		m_SNAPIN_CLASSID;


public:
	CNode(CConsoleProvider* consoleProvider) {
		this->consoleProvider = consoleProvider;
	}

	virtual ~CNode() {

	}

	virtual IConsole* getConsole() {
		if (consoleProvider)
			return consoleProvider->getConsole();
		return NULL;
	}

	STDMETHOD(GetScopePaneInfo)(SCOPEDATAITEM *pScopeDataItem) {
		if (pScopeDataItem->mask & SDI_STR)
			pScopeDataItem->displayname = m_bstrDisplayName;

		if (pScopeDataItem->mask & SDI_IMAGE)
			pScopeDataItem->nImage = m_scopeDataItem.nImage;

		if (pScopeDataItem->mask & SDI_OPENIMAGE)
			pScopeDataItem->nOpenImage = m_scopeDataItem.nOpenImage;

		if (pScopeDataItem->mask & SDI_PARAM)
			pScopeDataItem->lParam = m_scopeDataItem.lParam;

		if (pScopeDataItem->mask & SDI_STATE )
			pScopeDataItem->nState = m_scopeDataItem.nState;

		return S_OK;
	}

	STDMETHOD(GetResultPaneInfo)(RESULTDATAITEM *pResultDataItem) {
		if (pResultDataItem->bScopeItem) {
			if (pResultDataItem->mask & RDI_STR)
				pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
			if (pResultDataItem->mask & RDI_IMAGE)
				pResultDataItem->nImage = m_scopeDataItem.nImage;
			if (pResultDataItem->mask & RDI_PARAM)
				pResultDataItem->lParam = m_scopeDataItem.lParam;
			return S_OK;
		}

		if (pResultDataItem->mask & RDI_STR)
			pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
		if (pResultDataItem->mask & RDI_IMAGE)
			pResultDataItem->nImage = m_resultDataItem.nImage;
		if (pResultDataItem->mask & RDI_PARAM)
			pResultDataItem->lParam = m_resultDataItem.lParam;
		if (pResultDataItem->mask & RDI_INDEX)
			pResultDataItem->nIndex = m_resultDataItem.nIndex;

		return S_OK;
	}


	STDMETHOD(Notify)(MMC_NOTIFY_TYPE event, long arg, long param, IComponentData* pComponentData, IComponent* pComponent, DATA_OBJECT_TYPES type) {
		_ASSERTE(pComponentData != NULL || pComponent != NULL);

		CComPtr<IConsole> spConsole;

		if (pComponentData != NULL)
			spConsole = ((_scope*)pComponentData)->m_spConsole;
		else 
			spConsole = ((_result*)pComponent)->m_spConsole;

		switch (event) {
			case MMCN_VIEW_CHANGE:
				return OnViewChange(spConsole, arg, param);

//			case MMCN_CONTEXTHELP:
//				return OnShowContextHelp(((_result*)pComponent)->m_ipDisplayHelp);

			case MMCN_SHOW:
				return OnShow(spConsole, arg, param);

			case MMCN_EXPAND:
				return OnExpand(arg, spConsole, param);

			case MMCN_ADD_IMAGES:
				 return OnAddImages((IImageList*) arg);

			case MMCN_DBLCLICK:
				 return spConsole->SelectScopeItem(m_scopeDataItem.ID);

			case MMCN_SELECT:
				 return OnSelect(spConsole, pComponent);

			case MMCN_RENAME:
				 return OnRename(spConsole, (LPOLESTR)param);

			case MMCN_INITOCX:
				return OnInitOCX(spConsole, (IUnknown*)param);
		}
		return E_NOTIMPL;
	}


	STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,long handle,  IUnknown* pUnk,DATA_OBJECT_TYPES type) {
		return S_FALSE;
	}

	STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type) {
		return S_FALSE;
	}


	virtual LPOLESTR GetResultPaneColInfo(int nCol) {
		if (nCol == 0)
			return m_bstrDisplayName;
		return OLESTR("override CNode<I,D,C>::GetResultPaneColInfo");
	}

	virtual HRESULT OnViewChange(IConsole* pConsole, long arg, long param) {
		return S_FALSE;
	}

//	virtual HRESULT OnShowContextHelp(IDisplayHelp *pDisplayHelp)  { 
//		return S_FALSE;
//	}
	
	virtual HRESULT OnExpand(BOOL bExpand, IConsole *pConsole, HSCOPEITEM me) {
		return S_FALSE;
	}
	
	virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me) {
		return S_FALSE;
	}

	virtual HRESULT OnSelect(IConsole *pConsole, IComponent* pComponent ) {
		return S_FALSE;
	}

	virtual HRESULT OnAddImages(IImageList* pImageList) {
		return _scope::LoadImages(pImageList);
	}

	virtual HRESULT OnRename(IConsole* pConsole, LPOLESTR newName) {
		return S_FALSE;
	}

	virtual HRESULT OnInitOCX(IConsole* spConsole, IUnknown* myOCX) {
		return S_FALSE;
	}

//	HRESULT HandleCommand(UINT nID, bool& bHandled, CSnapInObjectRootBase* pObj, DATA_OBJECT_TYPES type) {
//		_implement* p = static_cast<_implement*>(this);
//		return p->ProcessCommand(nID, bHandled, pObj, type);
//	}
};

#endif // !defined(AFX_NODE_H__73E81A60_8F02_4ACC_9A06_893E19B108CD__INCLUDED_)

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
#ifndef __SAFMQMANAGERROOT_H_
#define __SAFMQMANAGERROOT_H_
#include "resource.h"
#include <atlsnap.h>
#include <vector>
#include "Node.h"


class CSafmqServerNode;

class CSAFMQManagerRootPage : public CSnapInPropertyPageImpl<CSAFMQManagerRootPage>
{
public :
	CSAFMQManagerRootPage(long lNotifyHandle, bool bDeleteHandle = false, TCHAR* pTitle = NULL) : 
		CSnapInPropertyPageImpl<CSAFMQManagerRootPage> (pTitle),
		m_lNotifyHandle(lNotifyHandle),
		m_bDeleteHandle(bDeleteHandle) // Should be true for only page.
	{
	}

	~CSAFMQManagerRootPage()
	{
		if (m_bDeleteHandle)
			MMCFreeNotifyHandle(m_lNotifyHandle);
	}

	enum { IDD = IDD_SAFMQMANAGERROOT };

BEGIN_MSG_MAP(CSAFMQManagerRootPage)
	CHAIN_MSG_MAP(CSnapInPropertyPageImpl<CSAFMQManagerRootPage>)
END_MSG_MAP()
// Handler prototypes:
//	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//	LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//	LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	HRESULT PropertyChangeNotify(long param)
	{
		return MMCPropertyChangeNotify(m_lNotifyHandle, param);
	}

public:
	long m_lNotifyHandle;
	bool m_bDeleteHandle;
};





// ////////////////////////////////////////////////////////////////////////////
// 

class CSAFMQManagerRoot;
class CSAFMQManagerRootComponent;

class CSAFMQManagerRootNode : public CNode<CSAFMQManagerRootNode, CSAFMQManagerRoot, CSAFMQManagerRootComponent>
{
protected:
	CSnapInItem			*pLocalSettings;
	CSnapInItem			*pServerList;
	CSAFMQManagerRoot	*root;
public:
	static const GUID* m_NODETYPE;
	static const OLECHAR* m_SZNODETYPE;
	static const OLECHAR* m_SZDISPLAY_NAME;
	static const CLSID* m_SNAPIN_CLASSID;

	CComPtr<IControlbar> m_spControlBar;



	BEGIN_SNAPINCOMMAND_MAP(CSAFMQManagerRootNode, FALSE)
		SNAPINCOMMAND_ENTRY(ID_NEW_ADDSAFMQSERVER, AddSAFMQServer);
	END_SNAPINCOMMAND_MAP()

	SNAPINMENUID(IDR_SAFMQMANAGERROOT_MENU)

	BEGIN_SNAPINTOOLBARID_MAP(CSAFMQManagerRootNode)
		// Create toolbar resources with button dimensions 16x16 
		// and add an entry to the MAP. You can add multiple toolbars
		SNAPINTOOLBARID_ENTRY(IDR_SAFMQMANAGERROOT)
	END_SNAPINTOOLBARID_MAP()


	CSAFMQManagerRootNode(CSAFMQManagerRoot* root);
	virtual ~CSAFMQManagerRootNode();

	HRESULT AddSAFMQServer(bool& handled, CSnapInObjectRootBase* pBase);
	
	STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider, long handle, IUnknown* pUnk, DATA_OBJECT_TYPES type);

	STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type) {
		if (type == CCT_SCOPE || type == CCT_RESULT)
			return S_OK;

		return S_FALSE;
	}

	virtual HRESULT OnExpand(BOOL bExpand, IConsole *pConsole, HSCOPEITEM me);
	virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM me);
	virtual HRESULT OnAddImages(IImageList* pImageList);

	LPOLESTR GetResultPaneColInfo(int nCol);


	STDMETHOD(Load)(IStream *pStm);
	STDMETHOD(Save)(IStream *pStm);
};


class CSAFMQManagerRootComponent : public CComObjectRootEx<CComSingleThreadModel>,
	public CSnapInObjectRoot<2, CSAFMQManagerRoot >,
	public IExtendPropertySheetImpl<CSAFMQManagerRootComponent>,
	public IExtendContextMenuImpl<CSAFMQManagerRootComponent>,
	public IExtendControlbarImpl<CSAFMQManagerRootComponent>,
	public IPersistStreamInit,
	public IComponentImpl<CSAFMQManagerRootComponent>
{
protected:
	bool	modified;

public:
BEGIN_COM_MAP(CSAFMQManagerRootComponent)
	COM_INTERFACE_ENTRY(IComponent)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
    COM_INTERFACE_ENTRY(IExtendControlbar)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
END_COM_MAP()

public:
	CSAFMQManagerRootComponent()
	{
		modified = true;
	}


	const char* eventDecode(MMC_NOTIFY_TYPE event) {
#define ed(s) case s: return #s;
		switch (event) {
			ed(MMCN_ACTIVATE)
			ed(MMCN_ADD_IMAGES)
			ed(MMCN_BTN_CLICK)
			ed(MMCN_CLICK)
			ed(MMCN_COLUMN_CLICK)
			ed(MMCN_CONTEXTMENU)
			ed(MMCN_CUTORMOVE)
			ed(MMCN_DBLCLICK)
			ed(MMCN_DELETE)
			ed(MMCN_DESELECT_ALL)
			ed(MMCN_EXPAND)
			ed(MMCN_HELP)
			ed(MMCN_MENU_BTNCLICK)
			ed(MMCN_MINIMIZED)
			ed(MMCN_PASTE)
			ed(MMCN_PROPERTY_CHANGE)
			ed(MMCN_QUERY_PASTE)
			ed(MMCN_REFRESH)
			ed(MMCN_REMOVE_CHILDREN)
			ed(MMCN_RENAME)
			ed(MMCN_SELECT)
			ed(MMCN_SHOW)
			ed(MMCN_VIEW_CHANGE)
			ed(MMCN_SNAPINHELP)
			ed(MMCN_CONTEXTHELP)
			ed(MMCN_INITOCX)
			ed(MMCN_FILTER_CHANGE)
//			ed(MMCN_GET_FILTER_MENU)
//			ed(MMCN_FILTER_OPERATOR)
		}
#undef ed
		return "UNKNOWN";
	}


	STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, long arg, long param)
	{
		ATLTRACE("CSAFMQManagerRootComponent::Notify(%p, %s, %ld, %ld)\n", lpDataObject, eventDecode(event), arg, param);

		// The following cases the CSAFMQManagerRootNode::Notify() to be called;
		if (lpDataObject != NULL)
			return IComponentImpl<CSAFMQManagerRootComponent>::Notify(lpDataObject, event, arg, param);
		
		// TODO : Add code to handle notifications that set lpDataObject == NULL.
		return E_NOTIMPL;
	}

	STDMETHOD(GetClassID)(CLSID *pClassID)
	{
		ATLTRACENOTIMPL(_T("CSAFMQManagerRootComponent::GetClassID"));
	}	

	STDMETHOD(IsDirty)()
	{
		return modified ? S_OK : S_FALSE;
	}

	STDMETHOD(Load)(IStream *pStm)
	{
		return S_OK; // Store the result view data
	}

	STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty)
	{
		return S_OK; // Store the result view data
	}

	STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize)
	{
		ATLTRACENOTIMPL(_T("CSAFMQManagerRootComponent::GetSizeMax"));
	}

	STDMETHOD(InitNew)()
	{
		// Initialize the Component (result view data)
		return S_OK;
	}

};

class CSAFMQManagerRoot : public CComObjectRootEx<CComSingleThreadModel>,
	public CSnapInObjectRoot<1, CSAFMQManagerRoot>,
	public IComponentDataImpl<CSAFMQManagerRoot, CSAFMQManagerRootComponent>,
	public IExtendPropertySheetImpl<CSAFMQManagerRoot>,
	public IExtendContextMenuImpl<CSAFMQManagerRoot>,
	public IPersistStreamInit,
	public CComCoClass<CSAFMQManagerRoot, &CLSID_SAFMQManagerRoot>,
	public CConsoleProvider
{
protected:
	bool	modified;
public:
	CSAFMQManagerRoot()
	{
		m_pNode = new CSAFMQManagerRootNode(this);

		_ASSERTE(m_pNode != NULL);
		m_pComponentData = this;

		modified = false;
	}

	~CSAFMQManagerRoot()
	{
		delete m_pNode;
		m_pNode = NULL;
	}

BEGIN_COM_MAP(CSAFMQManagerRoot)
	COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_SAFMQMANAGERROOT)

DECLARE_NOT_AGGREGATABLE(CSAFMQManagerRoot)

	STDMETHOD(GetClassID)(CLSID *pClassID)
	{
		ATLTRACENOTIMPL(_T("CSAFMQManagerRoot::GetClassID"));
	}	

	STDMETHOD(IsDirty)()
	{
		return modified ? S_OK : S_FALSE;
	}

	STDMETHOD(Load)(IStream *pStm);

	STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

	STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize)
	{
		ATLTRACENOTIMPL(_T("CSAFMQManagerRoot::GetSizeMax"));
	}

	STDMETHOD(InitNew)()
	{
		// Initialize the ComponentDataImpl
		return S_OK;
	}

	STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
	static HRESULT LoadImages(IImageList* pImageList);

	virtual IConsole* getConsole() {
		return m_spConsole;
	}

	static void WINAPI ObjectMain(bool bStarting)
	{
		if (bStarting)
			CSnapInItem::Init();
	}

	void SetModified() {
		modified = true;
	}
};

class ATL_NO_VTABLE CSAFMQManagerRootAbout : public ISnapinAbout,
	public CComObjectRoot,
	public CComCoClass< CSAFMQManagerRootAbout, &CLSID_SAFMQManagerRootAbout>
{
protected:
	HBITMAP	hSmallImage;
	HBITMAP hLargeImage;
	HICON	hIcon;

public:
	DECLARE_REGISTRY(CSAFMQManagerRootAbout, _T("SAFMQManagerRootAbout.1"), _T("SAFMQManagerRootAbout.1"), IDS_SAFMQMANAGERROOT_DESC, THREADFLAGS_BOTH);

	BEGIN_COM_MAP(CSAFMQManagerRootAbout)
		COM_INTERFACE_ENTRY(ISnapinAbout)
	END_COM_MAP()


	CSAFMQManagerRootAbout() {
		hSmallImage = ::LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_SAFMQMANAGERROOT_16));
		hLargeImage = ::LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_SAFMQMANAGERROOT_32));
		hIcon = ::LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SAFMQMANAGERROOT));
	}

	virtual ~CSAFMQManagerRootAbout() {
		if (hSmallImage)
			::FreeResource(hSmallImage);
		if (hLargeImage)
			::FreeResource(hLargeImage);
		if (hIcon)
			::FreeResource(hIcon);
	}


	STDMETHOD(GetSnapinDescription)(LPOLESTR *lpDescription)
	{
		USES_CONVERSION;
		TCHAR szBuf[256];
		if (::LoadString(_Module.GetResourceInstance(), IDS_SAFMQMANAGERROOT_DESC, szBuf, 256) == 0)
			return E_FAIL;

		*lpDescription = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(OLECHAR));
		if (*lpDescription == NULL)
			return E_OUTOFMEMORY;

		ocscpy(*lpDescription, T2OLE(szBuf));

		return S_OK;
	}

	STDMETHOD(GetProvider)(LPOLESTR *lpName)
	{
		USES_CONVERSION;
		TCHAR szBuf[256];
		if (::LoadString(_Module.GetResourceInstance(), IDS_SAFMQMANAGERROOT_PROVIDER, szBuf, 256) == 0)
			return E_FAIL;

		*lpName = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(OLECHAR));
		if (*lpName == NULL)
			return E_OUTOFMEMORY;

		ocscpy(*lpName, T2OLE(szBuf));

		return S_OK;
	}

	STDMETHOD(GetSnapinVersion)(LPOLESTR *lpVersion)
	{
		USES_CONVERSION;
		TCHAR szBuf[256];
		if (::LoadString(_Module.GetResourceInstance(), IDS_SAFMQMANAGERROOT_VERSION, szBuf, 256) == 0)
			return E_FAIL;

		*lpVersion = (LPOLESTR)CoTaskMemAlloc((lstrlen(szBuf) + 1) * sizeof(OLECHAR));
		if (*lpVersion == NULL)
			return E_OUTOFMEMORY;

		ocscpy(*lpVersion, T2OLE(szBuf));

		return S_OK;
	}

	STDMETHOD(GetSnapinImage)(HICON *hAppIcon)
	{
		*hAppIcon = hIcon;
		return S_OK;
	}

	STDMETHOD(GetStaticFolderImage)(HBITMAP *hSmallImage,
		HBITMAP *hSmallImageOpen,
		HBITMAP *hLargeImage,
		COLORREF *cMask)
	{
		*hSmallImageOpen = *hSmallImage = this->hSmallImage;
		*hLargeImage = this->hLargeImage;
		*cMask = RGB(255,255,255); // white
		return S_OK;
	}
};

#endif

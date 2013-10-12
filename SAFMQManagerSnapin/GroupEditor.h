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
// GroupEditor.h : Declaration of the CGroupEditor

#ifndef __GROUPEDITOR_H_
#define __GROUPEDITOR_H_

#include "resource.h"       // main symbols
#include <atlctl.h>
#include "LocalSettingsCP.h"


/////////////////////////////////////////////////////////////////////////////
// CGroupEditor
class ATL_NO_VTABLE CGroupEditor : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IGroupEditor, &IID_IGroupEditor, &LIBID_SAFMQMANAGERSNAPINLib>,
	public CComCompositeControl<CGroupEditor>,
	public IPersistStreamInitImpl<CGroupEditor>,
	public IOleControlImpl<CGroupEditor>,
	public IOleObjectImpl<CGroupEditor>,
	public IOleInPlaceActiveObjectImpl<CGroupEditor>,
	public IViewObjectExImpl<CGroupEditor>,
	public IOleInPlaceObjectWindowlessImpl<CGroupEditor>,
	public IConnectionPointContainerImpl<CGroupEditor>,
	public IPersistStorageImpl<CGroupEditor>,
	public ISpecifyPropertyPagesImpl<CGroupEditor>,
	public IQuickActivateImpl<CGroupEditor>,
	public IDataObjectImpl<CGroupEditor>,
	public IProvideClassInfo2Impl<&CLSID_GroupEditor, &DIID__IGroupEditorEvents, &LIBID_SAFMQMANAGERSNAPINLib>,
	public IPropertyNotifySinkCP<CGroupEditor>,
	public CComCoClass<CGroupEditor, &CLSID_GroupEditor>,
	public CProxy_IGroupEditorEvents< CGroupEditor >
{
public:
	CGroupEditor();
	virtual ~CGroupEditor();


DECLARE_REGISTRY_RESOURCEID(IDR_GROUPEDITOR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CGroupEditor)
	COM_INTERFACE_ENTRY(IGroupEditor)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IViewObjectEx)
	COM_INTERFACE_ENTRY(IViewObject2)
	COM_INTERFACE_ENTRY(IViewObject)
	COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceObject)
	COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY(IOleControl)
	COM_INTERFACE_ENTRY(IOleObject)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IQuickActivate)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_PROP_MAP(CGroupEditor)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_CONNECTION_POINT_MAP(CGroupEditor)
	CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
	CONNECTION_POINT_ENTRY(DIID__IGroupEditorEvents)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CGroupEditor)
	CHAIN_MSG_MAP(CComCompositeControl<CGroupEditor>)

	COMMAND_ID_HANDLER(IDC_ADD_USER, OnAddUsers)
	COMMAND_ID_HANDLER(IDC_APPLY, OnApply)

	NOTIFY_HANDLER(IDC_USERS, LVN_KEYDOWN, OnUsersKey)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

BEGIN_SINK_MAP(CGroupEditor)
	//Make sure the Event Handlers have __stdcall calling convention
END_SINK_MAP()


	STDMETHOD(OnAmbientPropertyChange)(DISPID dispid)
	{
		if (dispid == DISPID_AMBIENT_BACKCOLOR)
		{
			SetBackgroundColorFromAmbient();
			FireViewChange();
		}
		return IOleControlImpl<CGroupEditor>::OnAmbientPropertyChange(dispid);
	}



	HWND Create(HWND hWndParent, RECT& r, LPARAM dwInitParam = NULL);
	LRESULT OnUsersKey(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	// Commands
	LRESULT OnAddUsers(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnApply(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);




// IViewObjectEx
	DECLARE_VIEW_STATUS(0)

// IGroupEditor
public:
	STDMETHOD(Initialize)(/*[in]*/ BSTR groupname, /*[in]*/ BOOL modifyUsers, /*[in]*/ BOOL modifyGroups, /*[in]*/ BOOL modifyQueues, /*[in]*/ VARIANT userList);

	enum { IDD = IDD_GROUPEDITOR };
private:
	CListControl	users;
	CCheckBox		modifyUsers;
	CCheckBox		modifyGroups;
	CCheckBox		modifyQueues;
	CWindow			name;

	HIMAGELIST		images;
};

#endif //__GROUPEDITOR_H_

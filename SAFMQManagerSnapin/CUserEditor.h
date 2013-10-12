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
// UserEditor.h : Declaration of the CUserEditor

#ifndef __CUSEREDITOR_H_
#define __CUSEREDITOR_H_

#include "resource.h"       // main symbols
#include <atlctl.h>
#include "LocalSettingsCP.h"


/////////////////////////////////////////////////////////////////////////////
// CUserEditor
class ATL_NO_VTABLE CUserEditor : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IConnectionPointContainerImpl<CUserEditor>,
	public IDispatchImpl<IUserEditor, &IID_IUserEditor, &LIBID_SAFMQMANAGERSNAPINLib>,
	public CComCompositeControl<CUserEditor>,
	public IPersistStreamInitImpl<CUserEditor>,
	public IOleControlImpl<CUserEditor>,
	public IOleObjectImpl<CUserEditor>,
	public IOleInPlaceActiveObjectImpl<CUserEditor>,
	public IViewObjectExImpl<CUserEditor>,
	public IOleInPlaceObjectWindowlessImpl<CUserEditor>,
	public IPersistStorageImpl<CUserEditor>,
	public ISpecifyPropertyPagesImpl<CUserEditor>,
	public IQuickActivateImpl<CUserEditor>,
	public IDataObjectImpl<CUserEditor>,
	public IProvideClassInfo2Impl<&CLSID_UserEditor, NULL, &LIBID_SAFMQMANAGERSNAPINLib>,
	public CComCoClass<CUserEditor, &CLSID_UserEditor>,
	public CProxy_IUserEditorEvents< CUserEditor >
{
public:
	CUserEditor();
	virtual ~CUserEditor();

DECLARE_REGISTRY_RESOURCEID(IDR_CUSEREDITOR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CUserEditor)
	COM_INTERFACE_ENTRY(IUserEditor)
//DEL 	COM_INTERFACE_ENTRY(IDispatch)
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
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IQuickActivate)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	COM_INTERFACE_ENTRY2(IDispatch, IUserEditor)
END_COM_MAP()

BEGIN_PROP_MAP(CUserEditor)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

BEGIN_MSG_MAP(CUserEditor)
	CHAIN_MSG_MAP(CComCompositeControl<CUserEditor>)
	MESSAGE_HANDLER(WM_DRAWITEM, OnOwnerDraw)

	COMMAND_ID_HANDLER(IDC_SETPASSWORD, OnSetPassword)
	COMMAND_ID_HANDLER(IDC_ADD_GROUP, OnAddGroup)
	COMMAND_ID_HANDLER(IDC_APPLY, OnApply)


	NOTIFY_HANDLER(IDC_GROUPS, LVN_KEYDOWN, OnGroupKey)

END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

BEGIN_SINK_MAP(CUserEditor)
	//Make sure the Event Handlers have __stdcall calling convention
END_SINK_MAP()

	STDMETHOD(OnAmbientPropertyChange)(DISPID dispid)
	{
		if (dispid == DISPID_AMBIENT_BACKCOLOR)
		{
			SetBackgroundColorFromAmbient();
			FireViewChange();
		}
		return IOleControlImpl<CUserEditor>::OnAmbientPropertyChange(dispid);
	}

	HWND Create(HWND hWndParent, RECT& r, LPARAM dwInitParam = NULL);

	LRESULT OnOwnerDraw(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnGroupKey(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	// Commands
	LRESULT OnSetPassword(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAddGroup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnApply(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);


// IViewObjectEx
	DECLARE_VIEW_STATUS(0)

// IUserEditor
public:
	STDMETHOD(Initialize)(/*[in]*/ BSTR username, /*[in]*/ BSTR description, /*[in]*/ BOOL modifyUsers, /*[in]*/ BOOL modifyGroups, /*[in]*/ BOOL modifyQueues, /*[in]*/ VARIANT groupList);

	enum { IDD = IDD_USEREDITOR };

private:

	CCheckBox		modifyUsers;
	CCheckBox		modifyGroups;
	CCheckBox		modifyQueues;
	CWindow			name;
	CWindow			description;
	CListControl	groups;

	_bstr_t			username;

	HIMAGELIST		images;

public :
BEGIN_CONNECTION_POINT_MAP(CUserEditor)
	CONNECTION_POINT_ENTRY(DIID__IUserEditorEvents)
END_CONNECTION_POINT_MAP()
};

#endif //__CUSEREDITOR_H_

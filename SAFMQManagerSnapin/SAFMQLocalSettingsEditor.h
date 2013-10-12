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
// SAFMQLocalSettingsEditor.h : Declaration of the CSAFMQLocalSettingsEditor

#ifndef __SAFMQLOCALSETTINGSEDITOR_H_
#define __SAFMQLOCALSETTINGSEDITOR_H_

#include "resource.h"       // main symbols
#include <atlctl.h>
#include "ConfigData.h"
#include <iostream>
#include <windowsx.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CSAFMQLocalSettingsEditor

class DataException : public exception
{
public:
	DataException(const string& msg) throw() : exception() {
		this->msg = msg;
	}
	DataException(const DataException& src) throw() {
		*this = src;
	}
	exception& operator=(const exception& src) throw() {
		msg = src.what();
	}
	virtual ~DataException() throw() {
	}
	virtual const char* what() const throw() {
		return msg.c_str();
	}

private:
	string msg;
};


/**
Implements the SAFMQ Local Settings Editor ActiveX control
*/
class ATL_NO_VTABLE CSAFMQLocalSettingsEditor : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<ISAFMQLocalSettingsEditor, &IID_ISAFMQLocalSettingsEditor, &LIBID_SAFMQMANAGERSNAPINLib>,
	public CComCompositeControl<CSAFMQLocalSettingsEditor>,
	public IPersistStreamInitImpl<CSAFMQLocalSettingsEditor>,
	public IOleControlImpl<CSAFMQLocalSettingsEditor>,
	public IOleObjectImpl<CSAFMQLocalSettingsEditor>,
	public IOleInPlaceActiveObjectImpl<CSAFMQLocalSettingsEditor>,
	public IViewObjectExImpl<CSAFMQLocalSettingsEditor>,
	public IOleInPlaceObjectWindowlessImpl<CSAFMQLocalSettingsEditor>,
	public IPersistStorageImpl<CSAFMQLocalSettingsEditor>,
	public ISpecifyPropertyPagesImpl<CSAFMQLocalSettingsEditor>,
	public IQuickActivateImpl<CSAFMQLocalSettingsEditor>,
	public IDataObjectImpl<CSAFMQLocalSettingsEditor>,
	public IProvideClassInfo2Impl<&CLSID_SAFMQLocalSettingsEditor, NULL, &LIBID_SAFMQMANAGERSNAPINLib>,
	public CComCoClass<CSAFMQLocalSettingsEditor, &CLSID_SAFMQLocalSettingsEditor>
{
public:
	typedef CComCompositeControl<CSAFMQLocalSettingsEditor>											compositeBase;
	typedef CComControl< CSAFMQLocalSettingsEditor, CAxDialogImpl< CSAFMQLocalSettingsEditor > >	base;
	
	DECLARE_REGISTRY_RESOURCEID(IDR_SAFMQLOCALSETTINGSEDITOR)

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CSAFMQLocalSettingsEditor)
		COM_INTERFACE_ENTRY(ISAFMQLocalSettingsEditor)
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
		COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
		COM_INTERFACE_ENTRY(IQuickActivate)
		COM_INTERFACE_ENTRY(IPersistStorage)
		COM_INTERFACE_ENTRY(IDataObject)
		COM_INTERFACE_ENTRY(IProvideClassInfo)
		COM_INTERFACE_ENTRY(IProvideClassInfo2)
	END_COM_MAP()

	BEGIN_PROP_MAP(CSAFMQLocalSettingsEditor)
		PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
		PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
		// Example entries
		// PROP_ENTRY("Property Description", dispid, clsid)
		// PROP_PAGE(CLSID_StockColorPage)
	END_PROP_MAP()

	BEGIN_MSG_MAP(CSAFMQLocalSettingsEditor)
		COMMAND_ID_HANDLER(IDC_REVERT, OnRevert)
		COMMAND_ID_HANDLER(IDC_APPLY, OnApply)
		COMMAND_ID_HANDLER(IDC_BROWSE_CONFIG, OnBrowseConfig)
		COMMAND_ID_HANDLER(IDC_BROWSE_QUEUE, OnBrowseQueue)

		COMMAND_ID_HANDLER(IDC_BROWSE_SSL_CERT, OnBrowseCert)
		COMMAND_ID_HANDLER(IDC_BROWSE_SSL_KEY, OnBrowseKey)
		COMMAND_ID_HANDLER(IDC_BROWSE_SSL_CA, OnBrowseCA)
		COMMAND_ID_HANDLER(IDC_BROWSE_SSL_CA_DIR, OnBrowseCADir)
		COMMAND_ID_HANDLER(IDC_BROWSE_SSL_CLIENT_CERT, OnBrowseClientCert)
		COMMAND_ID_HANDLER(IDC_BROWSE_SSL_CLIENT_KEY, OnBrowseClientKey)

		CHAIN_MSG_MAP(compositeBase)
		CHAIN_MSG_MAP(base)
	END_MSG_MAP()
	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	BEGIN_SINK_MAP(CSAFMQLocalSettingsEditor)
		//Make sure the Event Handlers have __stdcall calling convention
	END_SINK_MAP()


	CSAFMQLocalSettingsEditor();

	STDMETHOD(OnAmbientPropertyChange)(DISPID dispid)
	{
		if (dispid == DISPID_AMBIENT_BACKCOLOR)
		{
			SetBackgroundColorFromAmbient();
			FireViewChange();
		}
		return IOleControlImpl<CSAFMQLocalSettingsEditor>::OnAmbientPropertyChange(dispid);
	}

	HWND Create(HWND hWndParent, RECT& r, LPARAM dwInitParam = NULL);

	// Commands
	LRESULT OnRevert(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnApply(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseQueue(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseConfig(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnBrowseCert(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseKey(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseCA(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseCADir(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseClientCert(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBrowseClientKey(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	// IViewObjectEx
	DECLARE_VIEW_STATUS(0)

protected:

	// initialization and data access
	void onInit();
	void putData();
	void getData();


	void browseDir(CWindow& src, const string& message);
	void browseFile(CWindow& src, const string& message, const char* filetype);

	// utility methods
	static void setupLogCombo(CComboBox& combo);
	void restartSAFMQ();

// ISAFMQLocalSettingsEditor
public:
	enum { IDD = IDD_SAFMQLOCALSETTINGSEDITOR };

protected:
	CWindow		queueDir;
	CWindow		configDir;
	CWindow		port;
	CWindow		address;

	CCheckBox	forward;
	CCheckBox	acceptForwards;

	CComboBox	startupLog;
	CComboBox	shutdownLog;
	CComboBox	signonLog;
	CComboBox	userLog;
	CComboBox	groupLog;
	CComboBox	queueLog;
	CComboBox	messageLog;
	CComboBox	forwardLog;
	CComboBox	infoLog;

	CWindow		sslPort;
	CWindow		sslAddress;
	CWindow		sslCert;
	CWindow		sslKey;
	CWindow		sslCA;
	CWindow		sslCADir;
	CCheckBox	sslRequireCert;
	CWindow		sslClientCert;
	CWindow		sslClientKey;


	CConfigData	data;

};

#endif //__SAFMQLOCALSETTINGSEDITOR_H_

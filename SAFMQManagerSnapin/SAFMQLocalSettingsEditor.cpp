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
// SAFMQLocalSettingsEditor.cpp : Implementation of CSAFMQLocalSettingsEditor

#include "stdafx.h"
//#include <shldisp.h>
#include <exdisp.h>

#include "SAFMQLocalSettingsEditor.h"

const char* PEM_FILTER = "Pem Files (*.pem)\0*.PEM\0All Files (*.*)\0*.*\0\0";
const char* ALL_FILTER = "All Files (*.*)\0*.*\0\0";

/////////////////////////////////////////////////////////////////////////////
// CSAFMQLocalSettingsEditor

/**
Default Constructor.
*/
CSAFMQLocalSettingsEditor::CSAFMQLocalSettingsEditor()
{
	m_bWindowOnly = TRUE;
	CalcExtent(m_sizeExtent);
}

/**
Intercepts the window create to initialize the control's display.  After creating the
window, this method initializes internal references to the child controls and sets
data loaded from the configuration file into the display.

@param hWndParent The parent of the control
@param r The bounds of the control
@param dwInitParam Initialization Parameters
*/
HWND CSAFMQLocalSettingsEditor::Create(HWND hWndParent, RECT& r, LPARAM dwInitParam)
{
	HWND theWnd = CComCompositeControl<CSAFMQLocalSettingsEditor>::Create(hWndParent, r, dwInitParam);

	COLORREF	color = ::GetSysColor(COLOR_3DFACE);
	m_hbrBackground = ::CreateSolidBrush(color);

	onInit();
	putData();
	return theWnd;
}

/**
Handles clicks of the "Revert" button.  Restores the loaded or recently applied data.

@param wNotifyCode [ignored]
@param wID [ignored]
@param hWndCtrl [ignored]
@param bHandled [out] Set to true;
@return TRUE in all cases
*/
LRESULT CSAFMQLocalSettingsEditor::OnRevert(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	putData();

	return bHandled = TRUE;
}

/**
Handles clicks of the "Apply" button.  Stores the newly input data and queries
if SAFMQ should be restarted.

@param wNotifyCode [ignored]
@param wID [ignored]
@param hWndCtrl [ignored]
@param bHandled [out] Set to true;
@return TRUE in all cases
*/
LRESULT CSAFMQLocalSettingsEditor::OnApply(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	try {
		getData();
	} catch (const DataException& e) {
		CWindow::MessageBox(e.what(), "Settings Error", MB_OK | MB_ICONERROR);
		return bHandled = TRUE;
	}
	data.storeSettings();

	int result = CWindow::MessageBox("Would you like to restart SAFMQ now?", "Restart SAFMQ ServeR", MB_YESNO | MB_ICONQUESTION);
	if (result == IDYES) {
		restartSAFMQ();
	}


	return bHandled = TRUE;
}


LRESULT CSAFMQLocalSettingsEditor::OnBrowseQueue(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	browseDir(queueDir, "Please select the Queue Storage Directory");
	return bHandled = TRUE;
}

LRESULT CSAFMQLocalSettingsEditor::OnBrowseConfig(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	browseDir(configDir, "Please select the Configuration Settings Directory");
	return bHandled = TRUE;
}


LRESULT CSAFMQLocalSettingsEditor:: OnBrowseCert(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	browseFile(sslCert, "Please select the Server SSL Certificate PEM file", PEM_FILTER);
	return bHandled = TRUE;
}

LRESULT CSAFMQLocalSettingsEditor::OnBrowseKey(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	browseFile(sslKey, "Please select the Server SSL Private Key PEM file", PEM_FILTER);
	return bHandled = TRUE;
}

LRESULT CSAFMQLocalSettingsEditor::OnBrowseCA(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	browseFile(sslCA, "Please select the Certificate Authority List file", ALL_FILTER);
	return bHandled = TRUE;
}

LRESULT CSAFMQLocalSettingsEditor::OnBrowseCADir(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	browseDir(sslCADir, "Please select the Certificate Authority Directory");
	return bHandled = TRUE;
}

LRESULT CSAFMQLocalSettingsEditor::OnBrowseClientCert(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	browseFile(sslClientCert, "Please select the Client Use SSL Certificate PEM file", PEM_FILTER);
	return bHandled = TRUE;
}

LRESULT CSAFMQLocalSettingsEditor::OnBrowseClientKey(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	browseFile(sslClientKey, "Please select the Client Use SSL Private Key PEM file", PEM_FILTER);
	return bHandled = TRUE;
}



void CSAFMQLocalSettingsEditor::browseFile(CWindow& src, const string& message, const char* filetype)
{
	OPENFILENAME	params;
	char			filebase[1024];
	char			filetitle[1024];

	strcpy(filebase, getText(src).c_str());
	filetitle[0] = '\0';

	memset(&params, 0, sizeof(params));
	params.lStructSize = sizeof(params);
	params.hwndOwner = src;
	params.hInstance = _Module.GetResourceInstance();
	params.lpstrFilter = filetype;
	params.lpstrCustomFilter = NULL;
//	params.nMaxCustFilter; 
//	params.nFilterIndex; 
	params.lpstrFile = filebase;
	params.nMaxFile = sizeof(filebase);
  	params.lpstrFileTitle = filetitle;
  	params.nMaxFileTitle = sizeof(filetitle);
	params.lpstrInitialDir = NULL;
	params.lpstrTitle = message.c_str();
	params.Flags = OFN_FILEMUSTEXIST;
//	params.nFileOffset; 
//	params.nFileExtension; 
//	params.lpstrDefExt; 
//	params.lCustData; 
//	params.lpfnHook; 
//	params.lpTemplateName; 
	if (GetOpenFileName(&params)) {
		src.SetWindowText(params.lpstrFile);
	}
}

/**
Open a Shell dialog to choose a folter for the item being selected
@param src The source control which will be set to the new value
@param message The message to be displayed to the user
*/
void CSAFMQLocalSettingsEditor::browseDir(CWindow& src, const string& message)
{
	CComPtr<IShellDispatch>	pshell;
	pshell.CoCreateInstance(CLSID_Shell);

	if (pshell) {
		CComPtr<Folder>	folder;
		_bstr_t			title = message.c_str();
		long			options = 0;
		_variant_t		root = 0L;

		pshell->BrowseForFolder((long)(HWND)src, title, options, root, &folder);
		if (folder) {
			BSTR title;
			CComPtr<Folder>			parent;
			CComPtr<FolderItems>	items;
			CComPtr<FolderItem>		item;

			folder->get_Title(&title);
			folder->get_ParentFolder(&parent);
			if (parent) {
				parent->Items(&items);
				if (items) {
					BSTR	path;
					BSTR	name;
					long	idx = 0;
					while (true) {
						item = NULL;
						items->Item(_variant_t(idx++), &item);
						if (!item)
							break;

						item->get_Name(&name);

						if (wcsicmp(title, name) == 0) {
							item->get_Path(&path);
							src.SetWindowText(_bstr_t(path));
						}
					}

				}
			}
		}
	}
}

/**
Restarts the SAFMQ service.
*/
void CSAFMQLocalSettingsEditor::restartSAFMQ() {

	const char* serviceName = "safmq";

	SC_HANDLE manager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (manager) {

		SC_HANDLE service = OpenService(manager,serviceName,SERVICE_START | SERVICE_STOP | SERVICE_INTERROGATE | SERVICE_QUERY_STATUS);
		if (service) {

			const char* param[] = {"safmq.exe",""};

			SERVICE_STATUS	status;
			if (QueryServiceStatus(service, &status)) {
				if (status.dwCurrentState != SERVICE_STOPPED) {

					if (!ControlService(service, SERVICE_CONTROL_STOP, &status))
						return;

					while (status.dwCurrentState != SERVICE_STOPPED) {
						// TODO: Figure out some sort of timeout value
						QueryServiceStatus(service, &status);
						Sleep(100);
					}
				}
			}

			if (StartService(service,1,param)) {
				MessageBox("SAFMQ Successfully Restarted", "SAFMQ Restarted", MB_OK | MB_ICONINFORMATION);
			} else {
				DWORD error = GetLastError();
				switch (error) {
					case ERROR_ACCESS_DENIED: 
						MessageBox("The specified handle was not opened with SERVICE_START access.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
					case ERROR_INVALID_HANDLE: 
						MessageBox("The specified handle is invalid.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
					case ERROR_PATH_NOT_FOUND: 
						MessageBox("The service binary file could not be found.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
					case ERROR_SERVICE_ALREADY_RUNNING: 
						MessageBox("An instance of the service is already running.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
					case ERROR_SERVICE_DATABASE_LOCKED: 
						MessageBox("The database is locked.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
					case ERROR_SERVICE_DEPENDENCY_DELETED: 
						MessageBox("The service depends on a service that does not exist or has been marked for deletion.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
					case ERROR_SERVICE_DEPENDENCY_FAIL: 
						MessageBox("The service depends on another service that has failed to start.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
					case ERROR_SERVICE_DISABLED: 
						MessageBox("The service has been disabled.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
					case ERROR_SERVICE_LOGON_FAILED: 
						MessageBox("The service could not be logged on. This error occurs if the service was started from an account that does not have the \"Log on as a service\" right.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
					case ERROR_SERVICE_MARKED_FOR_DELETE: 
						MessageBox("The service has been marked for deletion.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
					case ERROR_SERVICE_NO_THREAD: 
						MessageBox("A thread could not be created for the service.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
					case ERROR_SERVICE_REQUEST_TIMEOUT: 
						MessageBox("The process for the service was started, but it did not call StartServiceCtrlDispatcher, or the thread that called StartServiceCtrlDispatcher may be blocked in a control handler function.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
					default:
						MessageBox("An error occured attempting to start the service.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
				}
			}	
			CloseServiceHandle(service);
		} else {
			switch (GetLastError()) {
				case ERROR_ACCESS_DENIED:
					MessageBox("Access denied accessing service to start.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
				case ERROR_SERVICE_DOES_NOT_EXIST:
					MessageBox("Service does not exist, please install first.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
				default:
					MessageBox("An error occurred accessing service.", "SAFMQ Restart", MB_OK | MB_ICONERROR);
			}
		}
		CloseServiceHandle(manager);
	} else {
		switch (GetLastError()) {
			case ERROR_ACCESS_DENIED:
				MessageBox("Access denied accessing service manager.", "SAFMQ Restart", MB_OK | MB_ICONERROR); break;
			default:
				MessageBox("occurred accessing service.", "SAFMQ Restart", MB_OK | MB_ICONERROR);
		}		
	}
}

/**
Initializes the references to the editor's child controls
*/
void CSAFMQLocalSettingsEditor::onInit() {
	queueDir = GetDlgItem(IDC_QDIR);
	configDir = GetDlgItem(IDC_CONFIGDIR);
	port = GetDlgItem(IDC_PORT);
	address = GetDlgItem(IDC_ADDRESS);
	
	forward = GetDlgItem(IDC_FORWARD);
	acceptForwards = GetDlgItem(IDC_ACCEPT_FORWARD);

	startupLog = GetDlgItem(IDC_LOG_STARTUP);
	shutdownLog = GetDlgItem(IDC_LOG_SHUTDOWN);
	signonLog = GetDlgItem(IDC_LOG_SIGNON);
	userLog = GetDlgItem(IDC_LOG_USER);
	groupLog = GetDlgItem(IDC_LOG_GROUP);
	queueLog = GetDlgItem(IDC_LOG_QUEUE);
	messageLog = GetDlgItem(IDC_LOG_MESSAGE);
	forwardLog = GetDlgItem(IDC_LOG_FORWARD);
	infoLog = GetDlgItem(IDC_LOG_INFO);


	sslPort = GetDlgItem(IDC_SSL_PORT);
	sslAddress = GetDlgItem(IDC_SSL_ADDRESS);
	sslCert = GetDlgItem(IDC_SSL_CERT);
	sslKey = GetDlgItem(IDC_SSL_KEY);
	sslCA = GetDlgItem(IDC_SSL_CA);
	sslCADir = GetDlgItem(IDC_SSL_CA_DIR);
	sslRequireCert = GetDlgItem(IDC_SSL_REQUIRE_CERT);
	sslClientCert = GetDlgItem(IDC_SSL_CLIENT_CERT);
	sslClientKey = GetDlgItem(IDC_SSL_CLIENT_KEY);
}


/**
Places data into the editor's child controls
*/
void CSAFMQLocalSettingsEditor::putData()
{
	// Config Directory and Port
	queueDir.SetWindowText(data.getSetting(CConfigData::QUEUE_DIR).c_str());
	configDir.SetWindowText(data.getSetting(CConfigData::CONFIG_DIR).c_str());
	port.SetWindowText(data.getSetting(CConfigData::PORT).c_str());
	address.SetWindowText(data.getSetting(CConfigData::ADDRESS).c_str());

	// Boolean Forward Settings
	forward.SetCheck(stricmp(data.getSetting(CConfigData::ENABLE_FORWARDING).c_str(), "yes") == 0);
	acceptForwards.SetCheck(stricmp(data.getSetting(CConfigData::ACCEPT_FORWARDS).c_str(), "yes") == 0);

	// Log Controls
	setupLogCombo(startupLog);
	startupLog.SetWindowText(data.getSetting(CConfigData::STARTUP_LOG).c_str());
	setupLogCombo(shutdownLog);
	shutdownLog.SetWindowText(data.getSetting(CConfigData::SHUTDOWN_LOG).c_str());
	setupLogCombo(signonLog);
	signonLog.SetWindowText(data.getSetting(CConfigData::SIGNON_LOG).c_str());
	setupLogCombo(userLog);
	userLog.SetWindowText(data.getSetting(CConfigData::USER_LOG).c_str());
	setupLogCombo(groupLog);
	groupLog.SetWindowText(data.getSetting(CConfigData::GROUP_LOG).c_str());
	setupLogCombo(queueLog);
	queueLog.SetWindowText(data.getSetting(CConfigData::QUEUE_LOG).c_str());
	setupLogCombo(messageLog);
	messageLog.SetWindowText(data.getSetting(CConfigData::MESSAGE_LOG).c_str());
	setupLogCombo(forwardLog);
	forwardLog.SetWindowText(data.getSetting(CConfigData::FORWARD_LOG).c_str());
	setupLogCombo(infoLog);
	infoLog.SetWindowText(data.getSetting(CConfigData::INFO_LOG).c_str());

	sslPort.SetWindowText(data.getSetting(CConfigData::SSL_PORT).c_str());
	sslAddress.SetWindowText(data.getSetting(CConfigData::SSL_ADDRESS).c_str());
	sslCert.SetWindowText(data.getSetting(CConfigData::SSL_CERT).c_str());
	sslKey.SetWindowText(data.getSetting(CConfigData::SSL_KEY).c_str());
	sslCA.SetWindowText(data.getSetting(CConfigData::SSL_CA).c_str());
	sslCADir.SetWindowText(data.getSetting(CConfigData::SSL_CA_DIR).c_str());
	sslRequireCert.SetCheck(stricmp(data.getSetting(CConfigData::SSL_REQUIRE_CERT).c_str(), "yes") == 0);
	sslClientCert.SetWindowText(data.getSetting(CConfigData::SSL_CLIENT_CERT).c_str());
	sslClientKey.SetWindowText(data.getSetting(CConfigData::SSL_CLIENT_KEY).c_str());
}

void CSAFMQLocalSettingsEditor::getData() throw (DataException)
{
	string	tmp = getText(port);
	if (tmp.length()) {
		for(string::size_type x=0; x<tmp.length(); x++)
			if (tmp[x] < '0' || tmp[x] > '9')
				throw DataException("\"Port\" must contain only numeric digits");
	} else {
		throw DataException("\"Port\" is a required field please enter a number");
	}

	tmp = getText(sslPort);
	if (tmp.length()) {
		for(string::size_type x=0; x<tmp.length(); x++)
			if (tmp[x] < '0' || tmp[x] > '9')
				throw DataException("\"SSL Port\" must contain only numeric digits");
	}


	data.applySetting(CConfigData::QUEUE_DIR, getText(queueDir));
	data.applySetting(CConfigData::CONFIG_DIR, getText(configDir));
	data.applySetting(CConfigData::PORT, getText(port));
	data.applySetting(CConfigData::ADDRESS, getText(address));

	data.applySetting(CConfigData::ENABLE_FORWARDING, forward.GetCheck() ? "yes" : "no");
	data.applySetting(CConfigData::ACCEPT_FORWARDS, acceptForwards.GetCheck() ? "yes" : "no");

	data.applySetting(CConfigData::STARTUP_LOG, getText(startupLog));
	data.applySetting(CConfigData::SHUTDOWN_LOG, getText(shutdownLog));
	data.applySetting(CConfigData::SIGNON_LOG, getText(signonLog));
	data.applySetting(CConfigData::USER_LOG, getText(userLog));
	data.applySetting(CConfigData::GROUP_LOG, getText(groupLog));
	data.applySetting(CConfigData::QUEUE_LOG, getText(queueLog));
	data.applySetting(CConfigData::MESSAGE_LOG, getText(messageLog));
	data.applySetting(CConfigData::FORWARD_LOG, getText(forwardLog));
	data.applySetting(CConfigData::INFO_LOG, getText(infoLog));


	data.applySetting(CConfigData::SSL_PORT, getText(sslPort));
	data.applySetting(CConfigData::SSL_ADDRESS, getText(sslAddress));
	data.applySetting(CConfigData::SSL_CERT, getText(sslCert));
	data.applySetting(CConfigData::SSL_KEY, getText(sslKey));
	data.applySetting(CConfigData::SSL_CA, getText(sslCA));
	data.applySetting(CConfigData::SSL_CA_DIR, getText(sslCADir));
	data.applySetting(CConfigData::SSL_REQUIRE_CERT, sslRequireCert.GetCheck() ? "yes" : "no");
	data.applySetting(CConfigData::SSL_CLIENT_CERT, getText(sslClientCert));
	data.applySetting(CConfigData::SSL_CLIENT_KEY, getText(sslClientKey));
}


void CSAFMQLocalSettingsEditor::setupLogCombo(CComboBox& combo) {
	combo.ResetContent();
	combo.AddString("SYSTEM_LOG");
	combo.AddString("STDERR");
	combo.AddString("STDOUT");
	combo.AddString("NONE");
	combo.AddString("c:\\logs\\daily%Y%m%d.log");
	combo.AddString("c:\\logs\\hourly%Y%m%d%H.log");
}




/*
CString CBrowseForFolderApp::BrowseForFolder(
	LPCTSTR lpszTitle,
	UINT nFlags,
	LPCTSTR lpszRoot)
{
	CoIShellDispatch disp;

	IDispatch* lpDispatch;
	HRESULT hResult; // Initialize COM and create an instance of the  
	// Shell COM Class. Note: The CLSID CLSID_Shell  
	// is already defined in EXDISP.H.
	hResult = ::CoInitialize(NULL);
	hResult = ::CoCreateInstance(CLSID_Shell, NULL,  CLSCTX_SERVER,  IID_IDispatch, (void**)&disp);
	
	COleVariant var;
	var.Clear();
	var.SetString(lpszRoot, VT_BSTR);

	lpDispatch = disp.BrowseForFolder(NULL, lpszTitle,  nFlags, var);  // Initialize the Folder object and get its Title. CoFolder
	
	folder(lpDispatch);
	
	CString str = folder.GetTitle();
	folder.ReleaseDispatch();
	disp.ReleaseDispatch();// Return the selected folder's title 
	return str;
}
*/
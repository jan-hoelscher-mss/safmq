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
// NewServerDlg.h: interface for the CNewServerDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWSERVERDLG_H__A0A48C8C_CD5F_4E07_9C44_FAB8E3290697__INCLUDED_)
#define AFX_NEWSERVERDLG_H__A0A48C8C_CD5F_4E07_9C44_FAB8E3290697__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

class CNewServerDlg : public CDialogImpl<CNewServerDlg>  
{
public:
	enum { IDD = IDD_NEW_SERVER };

private:
	std::string	password;
	std::string username;
	std::string url;
	bool		savePassword;

public:
	CNewServerDlg();
	virtual ~CNewServerDlg();


	virtual LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	virtual LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	const std::string& getPassword() { return password; }
	const std::string& getUsername() { return username; }
	const std::string& getURL() { return url; }
	bool getSavePassword() { return savePassword; }

	BEGIN_MSG_MAP(CNewServerDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()
};

#endif // !defined(AFX_NEWSERVERDLG_H__A0A48C8C_CD5F_4E07_9C44_FAB8E3290697__INCLUDED_)

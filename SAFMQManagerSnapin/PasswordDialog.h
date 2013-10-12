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
// PasswordDialog.h: interface for the CPasswordDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PASSWORDDIALOG_H__B7F0D9D8_86A0_4DC1_B6B7_D89FB74F8899__INCLUDED_)
#define AFX_PASSWORDDIALOG_H__B7F0D9D8_86A0_4DC1_B6B7_D89FB74F8899__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

class CPasswordDialog : public CDialogImpl<CPasswordDialog>  
{
public:
	enum { IDD = IDD_PASSWORD };

private:
	_bstr_t		username;
	_bstr_t		bszPassword;

	CWindow		name;
	CWindow		password;
	CWindow		password2;

public:
	CPasswordDialog(const _bstr_t& username);
	virtual ~CPasswordDialog();

	HWND Create( HWND hWndParent, LPARAM dwInitParam = NULL );
	HWND Create( HWND hWndParent, RECT&, LPARAM dwInitParam = NULL );

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	const _bstr_t& getPassword() { return bszPassword; }

	BEGIN_MSG_MAP(CPasswordDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

private:
	void init();
};

#endif // !defined(AFX_PASSWORDDIALOG_H__B7F0D9D8_86A0_4DC1_B6B7_D89FB74F8899__INCLUDED_)

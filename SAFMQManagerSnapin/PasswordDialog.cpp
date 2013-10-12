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
// PasswordDialog.cpp: implementation of the CPasswordDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PasswordDialog.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPasswordDialog::CPasswordDialog(const _bstr_t& username)
{
	this->username = username;
}

CPasswordDialog::~CPasswordDialog()
{

}


HWND CPasswordDialog::Create( HWND hWndParent, LPARAM dwInitParam)
{
	HWND hwnd = CDialogImpl<CPasswordDialog>::Create(hWndParent, dwInitParam);
	if (hwnd)
		init();
	return hwnd;
}

HWND CPasswordDialog::Create( HWND hWndParent, RECT& r, LPARAM dwInitParam)
{
	HWND hwnd = CDialogImpl<CPasswordDialog>::Create(hWndParent, r, dwInitParam);
	if (hwnd)
		init();
	return hwnd;
}

LRESULT CPasswordDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	init();

	return bHandled = FALSE;
}

void CPasswordDialog::init()
{
	name = GetDlgItem(IDC_NAME);
	password = GetDlgItem(IDC_PASSWORD);
	password2 = GetDlgItem(IDC_PASSWORD2);

	name.SetWindowText(username);
}

LRESULT CPasswordDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::string passwd = getText(password);
	std::string passwd2 = getText(password2);

	if (passwd == passwd2) {
		bszPassword = passwd.c_str();
		EndDialog(IDOK);
	} else {
		this->MessageBox("Sorry, the passwords entered do not match", "Password Error", MB_ICONERROR | MB_OK);
	}

	return bHandled = TRUE;
}

LRESULT CPasswordDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	this->EndDialog(IDCANCEL);
	return bHandled = TRUE;
}



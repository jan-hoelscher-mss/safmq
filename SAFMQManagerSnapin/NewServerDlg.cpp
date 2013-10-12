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
// NewServerDlg.cpp: implementation of the CNewServerDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewServerDlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewServerDlg::CNewServerDlg()
{
	savePassword = false;
}

CNewServerDlg::~CNewServerDlg()
{

}


LRESULT CNewServerDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return bHandled = TRUE;
}

LRESULT CNewServerDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	char	buffer[1024];

	GetDlgItemText(IDC_URL, buffer, sizeof(buffer));
	url = buffer;
	GetDlgItemText(IDC_USERNAME, buffer, sizeof(buffer));
	username = buffer;
	GetDlgItemText(IDC_PASSWORD, buffer, sizeof(buffer));
	password = buffer;

	CCheckBox	passwd;
	passwd = GetDlgItem(IDC_REM_PASSWORD);
	savePassword = passwd.GetCheck() != 0;

	EndDialog(IDOK);
	return bHandled = TRUE;
}

LRESULT CNewServerDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return bHandled = TRUE;
}

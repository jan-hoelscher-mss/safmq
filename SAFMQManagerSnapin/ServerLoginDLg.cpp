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
// ServerLoginDLg.cpp: implementation of the CServerLoginDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerLoginDlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServerLoginDlg::CServerLoginDlg()
{

}

CServerLoginDlg::~CServerLoginDlg()
{

}


LRESULT CServerLoginDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetDlgItemText(IDC_QUESTION,	"Please enter the login information\n"
									"for the SAFMQ server");
	SetWindowText("Server Login");

	SetDlgItemText(IDC_URL, svrName.c_str());
	SetDlgItemText(IDC_USERNAME, usrName.c_str());


	return bHandled = TRUE;
}


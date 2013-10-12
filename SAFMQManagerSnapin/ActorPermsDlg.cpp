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
// ActorPermsDlg.cpp: implementation of the CActorPermsDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ActorPermsDlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CActorPermsDlg::CActorPermsDlg(const string& name, bool isGroup, bool read, bool write, bool security, bool destroy)
{
	this->name = name;
	this->isGroup = isGroup;
	this->read = read;
	this->write = write;
	this->security = security;
	this->destroy = destroy;
}

CActorPermsDlg::~CActorPermsDlg()
{

}


LRESULT CActorPermsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	readBox = GetDlgItem(IDC_READ);
	writeBox = GetDlgItem(IDC_WRITE);
	securityBox = GetDlgItem(IDC_SECURITY);
	destroyBox = GetDlgItem(IDC_DESTROY);
	description = GetDlgItem(IDC_DESCRIPTION);

	readBox.SetCheck(read);
	writeBox.SetCheck(write);
	securityBox.SetCheck(security);
	destroyBox.SetCheck(destroy);

	string msg;
	if (isGroup)
		msg = "the group: ";
	else
		msg = "the user: ";		
	msg += name;

	description.SetWindowText(msg.c_str());

	return bHandled = TRUE;
}

LRESULT CActorPermsDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	read = readBox.GetCheck() != 0;
	write = writeBox.GetCheck() != 0;
	security = securityBox.GetCheck() != 0;
	destroy = destroyBox.GetCheck() != 0;

	EndDialog(IDOK);
	return bHandled = TRUE;
}

LRESULT CActorPermsDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return bHandled = TRUE;
}

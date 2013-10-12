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
// PasswordDialog.cpp: implementation of the CUserAddDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UserAddDialog.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserAddDialog::CUserAddDialog(std::vector<std::string>& users)
{
	this->users.assign(users.begin(), users.end());
}

CUserAddDialog::~CUserAddDialog()
{

}


HWND CUserAddDialog::Create( HWND hWndParent, LPARAM dwInitParam)
{
	HWND hwnd = CDialogImpl<CUserAddDialog>::Create(hWndParent, dwInitParam);
	if (hwnd)
		init();
	return hwnd;
}

HWND CUserAddDialog::Create( HWND hWndParent, RECT& r, LPARAM dwInitParam)
{
	HWND hwnd = CDialogImpl<CUserAddDialog>::Create(hWndParent, r, dwInitParam);
	if (hwnd)
		init();
	return hwnd;
}

LRESULT CUserAddDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	init();

	return bHandled = FALSE;
}

void CUserAddDialog::init()
{
	src = GetDlgItem(IDC_SRC);
	dest = GetDlgItem(IDC_DEST);

	for(std::vector<std::string>::iterator i = users.begin(); i != users.end(); i++)
		src.AddString(i->c_str());
}

LRESULT CUserAddDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int	items = dest.GetCount();

	users.clear();
	for(int x=0; x < items; x++) {
		users.push_back(dest.GetText(x));
	}

	EndDialog(IDOK);

	return bHandled = TRUE;
}

LRESULT CUserAddDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	this->EndDialog(IDCANCEL);
	return bHandled = TRUE;
}

void CUserAddDialog::BuildAndRemoveSelected(CListBox& lb, std::vector<std::string>& str)
{
	str.clear();

	int	selcnt = lb.GetSelCount();
	if (selcnt) {
		int	*sels = new int[selcnt];
		lb.GetSelItems(selcnt, sels);
		for(int x=selcnt -1; x >= 0; --x) {
			str.push_back(lb.GetText(sels[x]));
			lb.DeleteString(sels[x]);
		}
	}
}

LRESULT CUserAddDialog::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::vector<std::string> moves;
	BuildAndRemoveSelected(src, moves);
	for(std::vector<std::string>::iterator i = moves.begin(); i != moves.end(); i++)
		dest.AddString(i->c_str());


	return bHandled = TRUE;
}

LRESULT CUserAddDialog::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::vector<std::string> moves;
	BuildAndRemoveSelected(dest, moves);
	for(std::vector<std::string>::iterator i = moves.begin(); i != moves.end(); i++)
		src.AddString(i->c_str());

	return bHandled = TRUE;
}

void CUserAddDialog::getUsers(std::vector<std::string>& users)
{
	users.clear();
	users.assign(this->users.begin(), this->users.end());
}

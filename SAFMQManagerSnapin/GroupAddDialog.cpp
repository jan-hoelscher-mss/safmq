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
// PasswordDialog.cpp: implementation of the CGroupAddDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GroupAddDialog.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGroupAddDialog::CGroupAddDialog(std::vector<std::string>& groups)
{
	this->groups.assign(groups.begin(), groups.end());
}

CGroupAddDialog::~CGroupAddDialog()
{

}


HWND CGroupAddDialog::Create( HWND hWndParent, LPARAM dwInitParam)
{
	HWND hwnd = CDialogImpl<CGroupAddDialog>::Create(hWndParent, dwInitParam);
	if (hwnd)
		init();
	return hwnd;
}

HWND CGroupAddDialog::Create( HWND hWndParent, RECT& r, LPARAM dwInitParam)
{
	HWND hwnd = CDialogImpl<CGroupAddDialog>::Create(hWndParent, r, dwInitParam);
	if (hwnd)
		init();
	return hwnd;
}

LRESULT CGroupAddDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	init();

	return bHandled = FALSE;
}

void CGroupAddDialog::init()
{
	src = GetDlgItem(IDC_SRC);
	dest = GetDlgItem(IDC_DEST);

	for(std::vector<std::string>::iterator i = groups.begin(); i != groups.end(); i++)
		src.AddString(i->c_str());
}

LRESULT CGroupAddDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int	items = dest.GetCount();

	groups.clear();
	for(int x=0; x < items; x++) {
		groups.push_back(dest.GetText(x));
	}

	EndDialog(IDOK);

	return bHandled = TRUE;
}

LRESULT CGroupAddDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	this->EndDialog(IDCANCEL);
	return bHandled = TRUE;
}

void CGroupAddDialog::BuildAndRemoveSelected(CListBox& lb, std::vector<std::string>& str)
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

LRESULT CGroupAddDialog::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::vector<std::string> moves;
	BuildAndRemoveSelected(src, moves);

	for(std::vector<std::string>::iterator i = moves.begin(); i != moves.end(); i++)
		dest.AddString(i->c_str());


	return bHandled = TRUE;
}

LRESULT CGroupAddDialog::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	std::vector<std::string> moves;
	BuildAndRemoveSelected(dest, moves);
	for(std::vector<std::string>::iterator i = moves.begin(); i != moves.end(); i++)
		src.AddString(i->c_str());

	return bHandled = TRUE;
}

void CGroupAddDialog::getGroups(std::vector<std::string>& groups)
{
	groups.clear();
	groups.assign(this->groups.begin(), this->groups.end());
}

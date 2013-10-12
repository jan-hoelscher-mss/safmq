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
// NewServerDlg.cpp: implementation of the CAddActorDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AddActorDlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAddActorDlg::CAddActorDlg()
{
}

CAddActorDlg::~CAddActorDlg()
{

}


LRESULT CAddActorDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	userSrc = GetDlgItem(IDC_USER_SRC);
	userDest = GetDlgItem(IDC_USER_DEST);
	groupSrc = GetDlgItem(IDC_GROUP_SRC);
	groupDest = GetDlgItem(IDC_GROUP_DEST);

	vector<string>::iterator i;

	for(i = users.begin(); i != users.end(); ++i)
		userSrc.AddString(i->c_str());

	for(i = groups.begin(); i != groups.end(); ++i)
		groupSrc.AddString(i->c_str());

	return bHandled = TRUE;
}

LRESULT CAddActorDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int		x;
	int		count;

	users.clear();
	groups.clear();

	count = userDest.GetCount();
	if (count > 0) {
		for(x=0; x<count; x++) {
			users.push_back(userDest.GetText(x));
		}
	}

	count = groupDest.GetCount();
	if (count > 0) {
		for(x=0; x<count; x++) {
			groups.push_back(groupDest.GetText(x));
		}
	}


	EndDialog(IDOK);
	return bHandled = TRUE;
}

LRESULT CAddActorDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return bHandled = TRUE;
}

static void move(CListBox& src, CListBox& dest)
{
	int		x;
	int		count;
	int		*items;

	count = src.GetSelCount();
	if (count > 0) {
		items = new int[count];
		src.GetSelItems(count, items);
		for(x=0;x<count;x++)
			dest.AddString(src.GetText(items[x]).c_str());
		for(x=count-1;x>=0;x--)
			src.DeleteString(items[x]);
		delete items;
	}
}


LRESULT CAddActorDlg::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	move(userSrc, userDest);
	move(groupSrc, groupDest);
	return bHandled = TRUE;
}

LRESULT CAddActorDlg::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	move(userDest, userSrc);
	move(groupDest, groupSrc);
	return bHandled = TRUE;
}

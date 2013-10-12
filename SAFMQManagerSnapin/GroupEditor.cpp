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
// GroupEditor.cpp : Implementation of CGroupEditor

#include "stdafx.h"
#include "GroupEditor.h"
#include "UserAddDialog.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CGroupEditor

CGroupEditor::CGroupEditor()
{
	m_bWindowOnly = TRUE;
	images = ImageList_LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_STRIP_16), 16, 0, RGB(0, 128, 128), IMAGE_BITMAP, LR_CREATEDIBSECTION);

	CalcExtent(m_sizeExtent);
}

CGroupEditor::~CGroupEditor()
{
	FreeResource(images);
}

HWND CGroupEditor::Create(HWND hWndParent, RECT& r, LPARAM dwInitParam)
{
	HWND theWnd = CComCompositeControl<CGroupEditor>::Create(hWndParent, r, dwInitParam);

	COLORREF	color = ::GetSysColor(COLOR_3DFACE);
	m_hbrBackground = ::CreateSolidBrush(color);

	modifyUsers	= GetDlgItem(IDC_MOD_USERS);
	modifyGroups = GetDlgItem(IDC_MOD_GROUPS);
	modifyQueues = GetDlgItem(IDC_MOD_QUEUES);
	name = GetDlgItem(IDC_NAME);
	users = GetDlgItem(IDC_USERS);

	users.SetImageList(images, LVSIL_SMALL);

	return theWnd;
}

LRESULT CGroupEditor::OnUsersKey(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NMLVKEYDOWN*	info = ( NMLVKEYDOWN*)pnmh;

	if (info->wVKey == VK_DELETE) {
		int iItem = -1;
		vector<int>	deletes;
		while ((iItem = users.GetNextItem(iItem, LVNI_SELECTED)) >= 0) {
			deletes.push_back(iItem);
		}

		sort(deletes.begin(),deletes.end());
		vector<int>::reverse_iterator	r;
		for(r = deletes.rbegin(); r != deletes.rend(); r++) {
			users.DeleteItem(*r);
		}
	}
	return bHandled = FALSE;
}

// Commands
LRESULT CGroupEditor::OnAddUsers(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	VARIANT	vusers;

	if (Fire_GetAvailableUsers(&vusers) == S_OK) {
		vector<string>					u;
		vector<string>::iterator		p;

		VariantVector<BSTR, VT_BSTR>	vusrs(vusers, true);
		int								count = vusrs.getBounds();

		vusrs.lock();
		int x;
		for(x = 0; x < count; x++) {
			string	name = _bstr_t(vusrs[x]);
			u.push_back(name);
		}

		LVITEM							item;
		char							buffer[1024] = "";

		memset(&item, 0, sizeof(item));
		item.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
		item.pszText = buffer;
		item.cchTextMax = sizeof(buffer);

		count = users.GetItemCount();
		for (x=0; x<count; x++) {
			item.iItem = x;
			users.GetItem(&item);
			p = std::find(u.begin(), u.end(), string(item.pszText));
			if (p != u.end())
				u.erase(p);
		}

		CUserAddDialog	dlg(u);
		if (dlg.DoModal() == IDOK) {
			dlg.getUsers(u);
			for (x=0; x<(int)u.size(); x++) {
				users.InsertItem(LVIF_TEXT | LVIF_IMAGE, count + x, u[x].c_str(), 0, 0, IMG_USER, 0);
			}
		}
	}

	return bHandled = FALSE;
}

LRESULT CGroupEditor::OnApply(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	LVITEM							item;
	char							buffer[1024] = "";

	memset(&item, 0, sizeof(item));
	item.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
	item.pszText = buffer;
	item.cchTextMax = sizeof(buffer);

	int count = users.GetItemCount();
	VariantVector<BSTR, VT_BSTR>	vusrs(count);
	
	vusrs.lock();
	for (int x=0; x<count; x++) {
		item.iItem = x;
		users.GetItem(&item);
		vusrs[x] = _bstr_t(item.pszText).copy();
	}
	vusrs.unlock();

	Fire_ApplyChanges(modifyUsers.GetCheck(), modifyGroups.GetCheck(), modifyQueues.GetCheck(), vusrs);

	return bHandled = FALSE;
}


STDMETHODIMP CGroupEditor::Initialize(BSTR groupname, BOOL modifyUsers, BOOL modifyGroups, BOOL modifyQueues, VARIANT userList)
{
	this->name.SetWindowText(_bstr_t(groupname));
	this->modifyUsers.SetCheck(modifyUsers);
	this->modifyGroups.SetCheck(modifyGroups);
	this->modifyQueues.SetCheck(modifyQueues);

	VariantVector<BSTR, VT_BSTR>	vusrs(userList);
	int								count = vusrs.getBounds();

	vusrs.lock();
	for(int x = 0; x < count; x++) {
		users.InsertItem(LVIF_TEXT | LVIF_IMAGE, count + x, _bstr_t(vusrs[x]), 0, 0, IMG_USER, 0);
	}
	vusrs.unlock();

	return S_OK;
}

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
// CUserEditor.cpp : Implementation of CCUserEditor

#include "stdafx.h"
#include "CUserEditor.h"
#include "PasswordDialog.h"
#include "GroupAddDialog.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CUserEditor
CUserEditor::CUserEditor()
{
	m_bWindowOnly = TRUE;
	images = ImageList_LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_STRIP_16), 16, 0, RGB(0, 128, 128), IMAGE_BITMAP, LR_CREATEDIBSECTION);

	CalcExtent(m_sizeExtent);
}

CUserEditor::~CUserEditor()
{
	::FreeResource(images);	
}

HWND CUserEditor::Create(HWND hWndParent, RECT& r, LPARAM dwInitParam)
{
	HWND theWnd = CComCompositeControl<CUserEditor>::Create(hWndParent, r, dwInitParam);

	COLORREF	color = ::GetSysColor(COLOR_3DFACE);
	m_hbrBackground = ::CreateSolidBrush(color);


	modifyUsers	= GetDlgItem(IDC_MOD_USERS);
	modifyGroups = GetDlgItem(IDC_MOD_GROUPS);
	modifyQueues = GetDlgItem(IDC_MOD_QUEUES);
	name = GetDlgItem(IDC_NAME);
	description = GetDlgItem(IDC_DESCRIPTION);
	groups = GetDlgItem(IDC_GROUPS);

	groups.SetImageList(images, LVSIL_SMALL);

	return theWnd;
}

LRESULT CUserEditor::OnOwnerDraw(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DRAWITEMSTRUCT	*dis = (DRAWITEMSTRUCT*)lParam;
	return bHandled=FALSE;
}

STDMETHODIMP CUserEditor::Initialize(BSTR username, BSTR description, BOOL modifyUsers, BOOL modifyGroups, BOOL modifyQueues, VARIANT groupList)
{
	this->username = username;

	this->name.SetWindowText(this->username);
	this->description.SetWindowText(_bstr_t(description));
	this->modifyUsers.SetCheck(modifyUsers);
	this->modifyGroups.SetCheck(modifyGroups);
	this->modifyQueues.SetCheck(modifyQueues);

	VariantVector<BSTR, VT_BSTR>	vgrps(groupList);
	int								count = vgrps.getBounds();

	vgrps.lock();
	for(int x = 0; x < count; x++) {
		groups.InsertItem(LVIF_TEXT | LVIF_IMAGE, count + x, _bstr_t(vgrps[x]), 0, 0, IMG_GROUP, 0);
	}
	vgrps.unlock();

	return S_OK;
}


LRESULT CUserEditor::OnSetPassword(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CPasswordDialog dlg(this->username);

	if (dlg.DoModal() == IDOK) {
		Fire_ChangePassword(dlg.getPassword());
	}
	return bHandled = TRUE;
}

LRESULT CUserEditor::OnAddGroup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	VARIANT	varGroups;

	memset(&varGroups, 0, sizeof(varGroups));

	if (Fire_GetAvailableGroups(&varGroups) == S_OK) {
		VariantVector<BSTR, VT_BSTR>	vgrps(varGroups, true); // take ownership and release when done
		vector<string>					gps;
		vector<string>::iterator		p;

		vgrps.lock();
		long	count = vgrps.getBounds();
		long	x;

		for(x=0; x<count; x++)
			gps.push_back((const char*)_bstr_t(vgrps[x]));
		vgrps.unlock();
		
		LVITEM							item;
		char							buffer[1024] = "";

		memset(&item, 0, sizeof(item));
		item.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
		item.pszText = buffer;
		item.cchTextMax = sizeof(buffer);

		count = groups.GetItemCount();
		for (x=0; x<count; x++) {
			item.iItem = x;
			groups.GetItem(&item);
			p = std::find(gps.begin(), gps.end(), string(item.pszText));
			if (p != gps.end())
				gps.erase(p);
		}

		CGroupAddDialog dlg(gps);
		if (dlg.DoModal() == IDOK) {
			dlg.getGroups(gps);
			for(x=0; x<(int)gps.size(); x++) {
				groups.InsertItem(LVIF_TEXT | LVIF_IMAGE, count + x, gps[x].c_str(), 0, 0, IMG_GROUP, 0);
			}
		}
	}

	return bHandled = TRUE;
}

LRESULT CUserEditor::OnApply(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int								count = groups.GetItemCount();
	LVITEM							item;
	char							buffer[1024] = "";
	VariantVector<BSTR, VT_BSTR>	gl(count);

	memset(&item, 0, sizeof(item));
	item.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
	item.pszText = buffer;
	item.cchTextMax = sizeof(buffer);

	gl.lock();
	for(int x=0; x<count; x++) {
		item.iItem = x;
		BOOL res = groups.GetItem(&item);
		gl[x] = _bstr_t(item.pszText).copy();
	}
	gl.unlock();

	this->Fire_ApplyChanges(modifyUsers.GetCheck(), modifyGroups.GetCheck(), modifyQueues.GetCheck(), gl);

	return bHandled = TRUE;
}

LRESULT CUserEditor::OnGroupKey(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	NMLVKEYDOWN*	info = ( NMLVKEYDOWN*)pnmh;

	if (info->wVKey == VK_DELETE) {
		int iItem = -1;
		vector<int>	deletes;
		while ((iItem = groups.GetNextItem(iItem, LVNI_SELECTED)) >= 0) {
			deletes.push_back(iItem);
		}

		sort(deletes.begin(),deletes.end());
		vector<int>::reverse_iterator	r;
		for(r = deletes.rbegin(); r != deletes.rend(); r++) {
			groups.DeleteItem(*r);
		}
	}
	return bHandled = FALSE;
}

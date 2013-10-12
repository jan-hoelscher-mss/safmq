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
#ifndef _COMMONCONTROLS_H_
#define _COMMONCONTROLS_H_

#include <commctrl.h>


/**
Provides methods to access a Window as if it were a Check Box Control
*/
class CCheckBox : public CWindow
{
public:
	void SetCheck(BOOL state) {
		SendMessage(BM_SETCHECK, state, 0);
	}

	BOOL GetCheck() {
		return SendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED;
	}

	CWindow& operator=(HWND hWnd) {
		return CWindow::operator=(hWnd);
	}
};

/**
Provides methods to access a Window as if it were a Combo Box Control
*/
class CComboBox : public CWindow
{
public:
	int AddString(const char* s) {
		return SendMessage(CB_ADDSTRING, 0, (LPARAM)s);
	}

	void ResetContent() {
		SendMessage(CB_RESETCONTENT, 0, 0);
	}

	int DeleteString(int idx) {
		return SendMessage(CB_DELETESTRING, idx, 0);
	}


	CWindow& operator=(HWND hWnd) {
		return CWindow::operator=(hWnd);
	}
};


class CListBox : public CWindow
{
public:
	int AddString(const char* item) {
		return (int)SendMessage(LB_ADDSTRING, 0, (LPARAM)item);
	}

	int DeleteString(UINT idx) {
		return (int)SendMessage(LB_DELETESTRING, idx, 0);
	}

	int GetSel(int nIndex) {
		return (int)SendMessage(LB_GETSEL, nIndex, 0);
	}
	
	int SetSel(int nIndex, BOOL bSelect) {
		return (int)SendMessage(LB_SETSEL, bSelect, nIndex);
	}
	
	int GetSelCount() {
		return (int)SendMessage(LB_GETSELCOUNT, 0, 0);
	}

	int GetSelItems(int nMaxItems, LPINT rgIndex) {
		return (int)SendMessage(LB_GETSELITEMS, nMaxItems, (LPARAM)rgIndex);
	}

	int GetCount() {
		return (int)SendMessage(LB_GETCOUNT, 0, 0);
	}
	
	int GetCurSel() {
		return (int)SendMessage(LB_GETCURSEL, 0, 0);
	}

	int SetCurSel(int nSelect) {
		return (int)SendMessage(LB_SETCURSEL, nSelect, 0);
	}

	std::string GetText(int nIndex) { 
		char	buf[1024];

		if ((int)::SendMessage(m_hWnd, LB_GETTEXT, nIndex, (LPARAM)buf) != LB_ERR) {
			return buf;
		}
		return "";
	}

	
	CWindow& operator=(HWND hWnd) {
		return CWindow::operator=(hWnd);
	}
};


class CListControl : public CWindow
{
public:
	BEGIN_MSG_MAP(CListControl)
		MESSAGE_HANDLER(WM_CHAR, OnChar);
	END_MSG_MAP()
	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		ATLTRACE("CListControl::OnChar 0x%04X\n", wParam);
		return bHandled = FALSE;
	}

	/**
	@return The number of items in the list
	*/
	int GetItemCount() {
		return (int) SendMessage(LVM_GETITEMCOUNT, 0, 0L);
	}

	/**
	Gets data about an item.  Note LVITEM::iItem, LVITEM::iSubItem identify the item or subitem
	to be retrieved.
	
	@param pItem [in,out] Set the iItem/iSubItem field to select the item.
	
	@return TRUE if the items was found
	*/
	BOOL GetItem(LVITEM* pItem) { 
		return (BOOL) SendMessage(LVM_GETITEM, 0, (LPARAM)pItem);
	}

	/**
	Changes the attributes of an item in the list.
	
	@param nItem
	@param nSubItem
	@param nMask
	@param lpszItem 
	@param nImage
	@param nState
	@param nStateMask
	@param lParam

	@return TRUE if the item was changed
	*/
	BOOL SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem, int nImage, UINT nState, UINT nStateMask, LPARAM lParam) {
		LVITEM lvi;
		lvi.mask = nMask;
		lvi.iItem = nItem;
		lvi.iSubItem = nSubItem;
		lvi.stateMask = nStateMask;
		lvi.state = nState;
		lvi.pszText = (LPTSTR) lpszItem;
		lvi.iImage = nImage;
		lvi.lParam = lParam;
		return (BOOL) SendMessage(LVM_SETITEM, 0, (LPARAM)&lvi);
	}

	/**
	Adds the item to the list.
	
	@param nMask
	@param nItem
	@param lpszItem 
	@param nState
	@param nStateMask
	@param nImage
	@param lParam

	@return TRUE if the item was changed
	*/
	int InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState, UINT nStateMask, int nImage, LPARAM lParam) {
		LVITEM item;
		item.mask = nMask;
		item.iItem = nItem;
		item.iSubItem = 0;
		item.pszText = (LPTSTR)lpszItem;
		item.state = nState;
		item.stateMask = nStateMask;
		item.iImage = nImage;
		item.lParam = lParam;
		return (int)SendMessage(LVM_INSERTITEM, 0, (LPARAM)&item);
	}

	/**
	Adds an item to the list
	*/
	int InsertItem(int nItem, LPCTSTR lpszItem) {
		return InsertItem(LVIF_TEXT, nItem, lpszItem, 0, 0, 0, 0);
	}


	/**
	Sets the image list for the control
	@param hImageList the image list
	@param nImageList one of LVSIL_NORMAL, LVSIL_SMALL, LVSIL_STATE
	*/
	HIMAGELIST SetImageList(HIMAGELIST hImageList, int nImageList) {
		return (HIMAGELIST)SendMessage(LVM_SETIMAGELIST, nImageList, (LPARAM) hImageList);
	}

	/**
	Gets the next item in the list with flaged with the passed flag
	@param nItem Pass -1 to find the first item
	@param nFlags a combination of:			LVNI_ABOVE
											LVNI_ALL
											LVNI_BELOW
											LVNI_TOLEFT
											LVNI_TORIGHT
											LVNI_DROPHILTED
											LVNI_FOCUSED
											LVNI_SELECTED

	@return the index of the next item matching the passed flag
	*/
	int GetNextItem(int nItem, int nFlags) {
		return (int)SendMessage(LVM_GETNEXTITEM, nItem, MAKELPARAM(nFlags, 0));
	}



	int InsertColumn(int nCol, const LVCOLUMN* pColumn) {
		return (int)SendMessage(LVM_INSERTCOLUMN, nCol, (LPARAM)pColumn);
	}

	int InsertColumn(int nCol, LPCTSTR lpszColumnHeading, int nWidth = -1, int nFormat = LVCFMT_LEFT, int nSubItem = -1)
	{
		LVCOLUMN column;
		column.mask = LVCF_TEXT|LVCF_FMT;
		column.pszText = (LPTSTR)lpszColumnHeading;
		column.fmt = nFormat;
		if (nWidth != -1)
		{
			column.mask |= LVCF_WIDTH;
			column.cx = nWidth;
		}
		if (nSubItem != -1)
		{
			column.mask |= LVCF_SUBITEM;
			column.iSubItem = nSubItem;
		}
		return InsertColumn(nCol, &column);
	}

	BOOL DeleteItem(int nItem) { 
		return (BOOL) SendMessage(LVM_DELETEITEM, nItem, 0L);
	}


	CWindow& operator=(HWND hWnd) {
		return CWindow::operator=(hWnd);
	}
};

inline std::string getText(CWindow& w) {
	char		value[1024];
	w.GetWindowText(value, sizeof(value));
	return value;
}

#endif

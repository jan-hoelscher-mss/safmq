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
// PasswordDialog.h: interface for the CGroupAddDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CGROUPADDDIALOG_H_)
#define _CGROUPADDDIALOG_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

class CGroupAddDialog : public CDialogImpl<CGroupAddDialog>  
{
public:
	enum { IDD = IDD_GROUPADD };

private:
	CListBox	src;
	CListBox	dest;

	std::vector<std::string>	groups;

public:
	CGroupAddDialog(std::vector<std::string>& groups);
	virtual ~CGroupAddDialog();

	HWND Create( HWND hWndParent, LPARAM dwInitParam = NULL );
	HWND Create( HWND hWndParent, RECT&, LPARAM dwInitParam = NULL );

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	void getGroups(std::vector<std::string>& groups);


	void BuildAndRemoveSelected(CListBox& lb, std::vector<std::string>& str);


	BEGIN_MSG_MAP(CGroupAddDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)

		COMMAND_ID_HANDLER(IDC_ADD, OnAdd)
		COMMAND_ID_HANDLER(IDC_REMOVE, OnRemove)
	END_MSG_MAP()

private:
	void init();
};

#endif // !defined(_CGROUPADDDIALOG_H_)

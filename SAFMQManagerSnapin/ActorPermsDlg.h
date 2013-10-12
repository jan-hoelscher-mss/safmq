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
// ActorPermsrDlg.h: interface for the CActorPermsDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_ACTOR_PERMS_DLG)
#define _ACTOR_PERMS_DLG

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

class CActorPermsDlg : public CDialogImpl<CActorPermsDlg>  
{
private:
	bool	read;
	bool	write;
	bool	security;
	bool	destroy;
	bool	isGroup;
	string	name;

	CCheckBox	readBox;
	CCheckBox	writeBox;
	CCheckBox	securityBox;
	CCheckBox	destroyBox;
	CWindow		description;

public:
	enum { IDD = IDD_ACTOR_PERMS };

private:
	std::string queueName;

public:
	CActorPermsDlg(const string& name, bool isGroup, bool read, bool write, bool security, bool destroy);
	virtual ~CActorPermsDlg();


	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	
	bool getRead() { return read; }
	bool getWrite() { return write; }
	bool getSecurity() { return security; }
	bool getDestroy() { return destroy; }


	BEGIN_MSG_MAP(CActorPermsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()
};

#endif // !defined(_NEW_USER_DLG)

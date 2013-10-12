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
// ServerLoginDLg.h: interface for the CServerLoginDLg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERLOGINDLG_H__2FBFF588_DAE7_492B_86DD_3C13A2F0F90C__INCLUDED_)
#define AFX_SERVERLOGINDLG_H__2FBFF588_DAE7_492B_86DD_3C13A2F0F90C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewServerDlg.h"

class CServerLoginDlg : public CNewServerDlg  
{
private:
	string	svrName;
	string	usrName;

public:
	CServerLoginDlg();
	virtual ~CServerLoginDlg();

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void setServerName(const string& serverName) { svrName = serverName; }
	void setUserName(const string& userName) { usrName = userName; }
};

#endif // !defined(AFX_SERVERLOGINDLG_H__2FBFF588_DAE7_492B_86DD_3C13A2F0F90C__INCLUDED_)

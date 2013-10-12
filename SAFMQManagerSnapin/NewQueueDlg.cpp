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
// NewQueueDlg.cpp: implementation of the CNewQueueDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewQueueDlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewQueueDlg::CNewQueueDlg()
{

}

CNewQueueDlg::~CNewQueueDlg()
{

}


LRESULT CNewQueueDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return bHandled = TRUE;
}

LRESULT CNewQueueDlg::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	char	buffer[1024];

	GetDlgItemText(IDC_QUEUENAME, buffer, sizeof(buffer));
	queueName = buffer;

	EndDialog(IDOK);
	return bHandled = TRUE;
}

LRESULT CNewQueueDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(IDCANCEL);
	return bHandled = TRUE;
}

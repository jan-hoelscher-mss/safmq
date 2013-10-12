/*
 Copyright 2004-2008 Matthew J. Battey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software distributed
	under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied. See the License for the
	specific language governing permissions and limitations under the License.
*/
#include "StdAfx.h"
#include "SafmqServiceInstaller.h"
#include "SafmqSecurityLogin.h"
#include "ServiceLoginUtility.h"

namespace SafmqServiceInstaller {

	String^ makeString(LPWSTR src)
	{
		StringBuilder^ res = gcnew StringBuilder();
		for(WCHAR* p = src; *p; ++p) {
			res->Append(*p, 1);
		}
		return res->ToString();
	}

	void SafmqServiceInstaller::Install(IDictionary^ savedState) {
		Installer::Install(savedState);

		String^	path = this->Context->Parameters["dir"] + exeLocation;
		SafmqSecurityLogin^ dlg = gcnew SafmqSecurityLogin();
		if (dlg->ShowDialog() == DialogResult::OK) {
			String^	login = nullptr;
			String^ password = nullptr;

			if (!dlg->UseLocalSystem()) {
				login = dlg->GetUserName();
				password = dlg->GetPassword();

				if (login->IndexOf("\\") == -1) {
					WCHAR			computerName[1024];
					DWORD siz = sizeof(computerName)/sizeof(WCHAR);
					w32::GetComputerName(computerName, &siz);
					computerName[siz] = 0;

					login = makeString(computerName) + "\\" + login;
				}

				if (!ServiceLoginUtility::GrantServiceLogon(login)) {
					MessageBox::Show("Unable to grant \"Logon as a Service\" to: " + login, "Policy Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
				} else {
					MessageBox::Show("Granted \"Logon as a Service\" to: " + login, "Policy Setting", MessageBoxButtons::OK, MessageBoxIcon::Information);
				}
			}

			SC_HANDLE manager = w32::OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
			if (manager) {
				SC_HANDLE schService = w32::CreateService( 
						manager,
						serviceName,
						serviceDesc,
						SERVICE_ALL_ACCESS,
						SERVICE_WIN32_OWN_PROCESS,
						SERVICE_AUTO_START,
						SERVICE_ERROR_NORMAL,
						path,					// service's binary 
						"",						// no load ordering group 
						NULL,					// no tag identifier 
						nullptr,				// no dependencies 
						login,					// LocalSystem account 
						password);				// no password 
				if (schService) {
					w32::CloseServiceHandle(schService);
				} else {
					DWORD err = w32::GetLastError();

					throw gcnew Exception("Service Creation Failed (" + err + ")");
				}
				w32::CloseServiceHandle(manager);
			}
		} else {
			throw gcnew Exception("User Canceled");
		}
	}

	void SafmqServiceInstaller::Uninstall(IDictionary^ savedState) {
		Installer::Uninstall(savedState);

		SC_HANDLE manager = w32::OpenSCManager(nullptr,nullptr,SC_MANAGER_CREATE_SERVICE);

		if (manager) {
			SC_HANDLE schService = w32::OpenService(manager,serviceName,SERVICE_ALL_ACCESS);
			if (schService) {
				SERVICE_STATUS stat;

				w32::ControlService(schService, SERVICE_CONTROL_STOP, &stat);
				
				if (!w32::DeleteService(schService)) {
					MessageBox::Show("Unable to remove SAFMQ service", "SAFMQ Uninstall");
				}
			}
			w32::CloseServiceHandle(manager);
		}
	}
}
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

// Copied from WinNT.h and WinSvc.h
// To enable non-managed system calls.
#pragma once

// BEGIN CLI declarations for ADVAPI32 entrypoints


using namespace System;
using namespace System::Text;
using namespace System::Runtime::InteropServices;


namespace w32 {
	[DllImport("advapi32", CharSet=CharSet::Auto, EntryPoint="CreateServiceW")]
	extern "C" SC_HANDLE CreateService (
		SC_HANDLE	hSCManager,
		String^		lpServiceName,
		String^		lpDisplayName,
		DWORD		dwDesiredAccess,
		DWORD		dwServiceType,
		DWORD		dwStartType,
		DWORD		dwErrorControl,
		String^		lpBinaryPathName,
		String^		lpLoadOrderGroup,
		LPDWORD		lpdwTagId,
		String^		lpDependencies,
		String^		lpServiceStartName,
		String^		lpPassword
	);

	[DllImport("advapi32", CharSet=CharSet::Auto, EntryPoint="OpenSCManagerW")]
	SC_HANDLE OpenSCManager (
		String^	lpMachineName,
		String^	lpDatabaseName,
		DWORD	dwDesiredAccess
	);

	[DllImport("advapi32", CharSet=CharSet::Auto, EntryPoint="CloseServiceHandle")]
	BOOL CloseServiceHandle (SC_HANDLE schService);

	[DllImport("advapi32", CharSet=CharSet::Auto, EntryPoint="OpenServiceW")]
	SC_HANDLE OpenService(
		SC_HANDLE	hSCManager,  // handle to SCM database
		String^		lpServiceName, // service name
		DWORD		dwDesiredAccess  // access
	);


	[DllImport("advapi32", CharSet=CharSet::Auto, EntryPoint="DeleteService")]
	BOOL DeleteService(SC_HANDLE schService);


	[DllImport("advapi32", CharSet=CharSet::Auto, EntryPoint="ControlService")]
	BOOL ControlService(
		SC_HANDLE			hService,               // handle to service
		DWORD				dwControl,                  // control code
		LPSERVICE_STATUS	lpServiceStatus  // status information
	);


	[DllImport("Kernel32", CharSet=CharSet::Auto, EntryPoint="GetLastError")]
	DWORD GetLastError();




	[DllImport("advapi32", CharSet=CharSet::Auto, EntryPoint="LookupAccountNameW")]
	BOOL LookupAccountName(String^ lpSystemName, String^ lpAccountName, PSID Sid, LPDWORD cbSid, LPTSTR ReferencedDomainName, LPDWORD cchReferencedDomainName, PSID_NAME_USE peUse);

	[DllImport("advapi32", CharSet=CharSet::Auto, EntryPoint="LsaOpenPolicy")]
	NTSTATUS LsaOpenPolicy(PLSA_UNICODE_STRING SystemName, PLSA_OBJECT_ATTRIBUTES ObjectAttributes, ACCESS_MASK DesiredAccess, PLSA_HANDLE PolicyHandle);

	[DllImport("advapi32", CharSet=CharSet::Auto, EntryPoint="LsaAddAccountRights")]
	NTSTATUS LsaAddAccountRights(LSA_HANDLE PolicyHandle, PSID AccountSid, PLSA_UNICODE_STRING UserRights, ULONG CountOfRights);

	[DllImport("advapi32", CharSet=CharSet::Auto, EntryPoint="LsaRemoveAccountRights")]
	NTSTATUS LsaRemoveAccountRights(LSA_HANDLE PolicyHandle, PSID AccountSid, BOOLEAN AllRights, PLSA_UNICODE_STRING UserRights, ULONG CountOfRights);

	[DllImport("advapi32", CharSet=CharSet::Auto, EntryPoint="LsaClose")]
	NTSTATUS LsaClose(LSA_HANDLE ObjectHandle);

	[DllImport("Kernel32", CharSet=CharSet::Auto, EntryPoint="GetComputerNameW")]
	BOOL GetComputerName(LPTSTR lpBuffer, LPDWORD lpnSize);


}

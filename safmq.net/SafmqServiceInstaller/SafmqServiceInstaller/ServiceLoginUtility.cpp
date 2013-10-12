#include "StdAfx.h"
#include "ServiceLoginUtility.h"

using namespace System;

#define INITIAL_SID_SIZE				256
#define INITIAL_REFERENCED_DOMAIN_SIZE	256

class LSAString
{
protected:
	LSA_UNICODE_STRING	str;
public:
	LSAString(LPWSTR src) {
		if (src) {
			str.Length = wcslen(src) * sizeof(WCHAR);
			str.MaximumLength = str.Length + sizeof(WCHAR);
			str.Buffer = src;
		} else {
			memset(&str, 0, sizeof(str));
		}
	}

	operator PLSA_UNICODE_STRING() {
		return str.Buffer == NULL ? NULL : &str;
	}
};


ServiceLoginUtility::ServiceLoginUtility(void)
{

}


bool ServiceLoginUtility::GrantServiceLogon(String^ login)
{
	LSA_OBJECT_ATTRIBUTES	oa;
	LSA_HANDLE				hPolicy;
	NTSTATUS				res;
	PSID					pSID = NULL;
	bool					granted = false;

	memset(&oa, 0, sizeof(oa));
	res = w32::LsaOpenPolicy(NULL, &oa, POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES, &hPolicy);
	if (res == 0) {
		SID_NAME_USE	nameuse;
		DWORD			sidSize = INITIAL_SID_SIZE;
		DWORD			refdomSize = INITIAL_REFERENCED_DOMAIN_SIZE;
		LPWSTR			refDom;
		DWORD			error;

		pSID = (PSID)malloc(sidSize);
		refDom = (LPWSTR)malloc(refdomSize);



		while (!w32::LookupAccountName(nullptr, login, pSID, &sidSize, refDom, &refdomSize, &nameuse)) {
			error = w32::GetLastError();
			if (error == ERROR_INSUFFICIENT_BUFFER) {
				pSID = (PSID)realloc(pSID, sidSize);
				refDom = (LPWSTR)realloc(refDom, refdomSize);
			} else {
				free(pSID);
				pSID = NULL;
				break;
			}
		}
		free(refDom);
		
		if (pSID) {
			res = w32::LsaAddAccountRights(hPolicy, pSID, LSAString(L"SeServiceLogonRight"), 1);
			if (res == 0) {
				granted = true;
			}
			free(pSID);
			w32::LsaClose(hPolicy);
		}
	}

	return granted;
}

/*
NTSTATUS OpenPolicy(LPWSTR ServerName, DWORD DesiredAccess, LSA_HANDLE* pPolicyHandle)
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    memset(&ObjectAttributes, 0, sizeof(ObjectAttributes));
	return w32::LsaOpenPolicy((PLSA_UNICODE_STRING)LSAString(ServerName), &ObjectAttributes, DesiredAccess, pPolicyHandle);
}


BOOL GetAccountSid(LPTSTR SystemName, LPTSTR AccountName, PSID *Sid)
{
    LPTSTR ReferencedDomain=NULL;
    DWORD cbSid=128;    // initial allocation attempt
    DWORD cchReferencedDomain=16; // initial allocation size
    SID_NAME_USE peUse;
    BOOL bSuccess=FALSE; // assume this function will fail

	*Sid = (PSID)malloc(cbSid);
	memset(*Sid, 0, cbSid);
	ReferencedDomain = (LPTSTR)malloc(cchReferencedDomain * sizeof(TCHAR));
    while(!w32::LookupAccountName(
                    SystemName,         // machine to lookup account on
                    AccountName,        // account to lookup
                    *Sid,               // SID of interest
                    &cbSid,             // size of SID
                    ReferencedDomain,   // domain account was found on
                    &cchReferencedDomain,
                    &peUse
                    )) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            *Sid = (PSID)realloc(*Sid,cbSid);
            ReferencedDomain = (LPTSTR)realloc(ReferencedDomain, cchReferencedDomain * sizeof(TCHAR));
		} else {
			free(*Sid);
			*Sid = NULL;
			break;
		}
    }
	free(ReferencedDomain);
    return *Sid != NULL;
}


NTSTATUS SetPrivilegeOnAccount(
    LSA_HANDLE PolicyHandle,    // open policy handle
    PSID AccountSid,            // SID to grant privilege to
    LPWSTR PrivilegeName,       // privilege to grant (Unicode)
    BOOL bEnable                // enable or disable
)
{
    if(bEnable) {
        return w32::LsaAddAccountRights(
                PolicyHandle,       // open policy handle
                AccountSid,         // target SID
                LSAString(PrivilegeName),   // privileges
                1							// privilege count
                );
    }
    else {
        return w32::LsaRemoveAccountRights(
                PolicyHandle,       // open policy handle
                AccountSid,         // target SID
                FALSE,              // do not disable all rights
                LSAString(PrivilegeName),   // privileges
                1                   // privilege count
                );
    }
}
*/
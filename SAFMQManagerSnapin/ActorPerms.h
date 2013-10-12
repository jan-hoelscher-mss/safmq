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
// ActorPerms.h : Declaration of the CActorPerms

#ifndef __ACTORPERMS_H_
#define __ACTORPERMS_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CActorPerms
class ATL_NO_VTABLE CActorPerms : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CActorPerms, &CLSID_ActorPerms>,
	public IDispatchImpl<IActorPerms, &IID_IActorPerms, &LIBID_SAFMQMANAGERSNAPINLib>
{
protected:
	_bstr_t	name;
	long	type;
	BOOL	modifyQueues;
	BOOL	modifyUsers;
	BOOL	modifyGroups;

public:
	CActorPerms()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_ACTORPERMS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CActorPerms)
	COM_INTERFACE_ENTRY(IActorPerms)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IActorPerms
public:
	STDMETHOD(get_ModifyQueues)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_ModifyQueues)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_ModifyUsers)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_ModifyUsers)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_ModifyGroups)(/*[out, retval]*/ BOOL *pVal);
	STDMETHOD(put_ModifyGroups)(/*[in]*/ BOOL newVal);
	STDMETHOD(get_Type)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Type)(/*[in]*/ long newVal);
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Name)(/*[in]*/ BSTR newVal);
};

#endif //__ACTORPERMS_H_

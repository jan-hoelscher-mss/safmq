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
// ActorPerms.cpp : Implementation of CActorPerms
#include "stdafx.h"
#include "SAFMQManagerSnapin.h"
#include "ActorPerms.h"

/////////////////////////////////////////////////////////////////////////////
// CActorPerms


STDMETHODIMP CActorPerms::get_Name(BSTR *pVal)
{
	*pVal = name.copy();
	
	return S_OK;
}

STDMETHODIMP CActorPerms::put_Name(BSTR newVal)
{
	name = newVal;

	return S_OK;
}

STDMETHODIMP CActorPerms::get_Type(long *pVal)
{
	*pVal = type;

	return S_OK;
}

STDMETHODIMP CActorPerms::put_Type(long newVal)
{
	type = newVal;

	return S_OK;
}

STDMETHODIMP CActorPerms::get_ModifyGroups(BOOL *pVal)
{
	*pVal = modifyGroups;

	return S_OK;
}

STDMETHODIMP CActorPerms::put_ModifyGroups(BOOL newVal)
{
	modifyGroups = newVal;

	return S_OK;
}

STDMETHODIMP CActorPerms::get_ModifyUsers(BOOL *pVal)
{
	*pVal = modifyUsers;

	return S_OK;
}

STDMETHODIMP CActorPerms::put_ModifyUsers(BOOL newVal)
{
	modifyUsers = newVal;

	return S_OK;
}

STDMETHODIMP CActorPerms::get_ModifyQueues(BOOL *pVal)
{
	*pVal = modifyQueues;

	return S_OK;
}

STDMETHODIMP CActorPerms::put_ModifyQueues(BOOL newVal)
{
	modifyQueues = newVal;

	return S_OK;
}

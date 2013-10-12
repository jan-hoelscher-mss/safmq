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
// LocalSettings.cpp: implementation of the CLocalSettings class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LocalSettings.h"
#include "SAFMQManagerRoot.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// {3A801BF6-EDEE-45bd-99B7-80AFE43163BE}
static const GUID CLocalSettings_NODETYPE = 
{ 0x3a801bf6, 0xedee, 0x45bd, { 0x99, 0xb7, 0x80, 0xaf, 0xe4, 0x31, 0x63, 0xbe } };


const GUID*		CLocalSettings::m_NODETYPE = &CLocalSettings_NODETYPE;
const OLECHAR*	CLocalSettings::m_SZNODETYPE = OLESTR("3A801BF6-EDEE-45bd-99B7-80AFE43163BE");
const OLECHAR*	CLocalSettings::m_SZDISPLAY_NAME = OLESTR("Local Settings");
const CLSID*	CLocalSettings::m_SNAPIN_CLASSID = &CLSID_SAFMQManagerRoot;


CLocalSettings::CLocalSettings(CConsoleProvider* provider) : CNode<CLocalSettings, CSAFMQManagerRoot, CSAFMQManagerRootComponent>(provider)
{
	memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
	m_scopeDataItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
	m_scopeDataItem.displayname = MMC_CALLBACK;
	m_scopeDataItem.nImage = IMG_SAFMQ; 		// May need modification
	m_scopeDataItem.nOpenImage = IMG_SAFMQ; 	// May need modification
	m_scopeDataItem.lParam = (LPARAM) this;
	m_scopeDataItem.cChildren = 0;

	memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));

	this->m_bstrDisplayName = m_SZDISPLAY_NAME;
}

CLocalSettings::~CLocalSettings()
{

}

HRESULT CLocalSettings::GetResultViewType(LPOLESTR *ppViewType,long *pViewOptions)
{
	LPOLESTR lpOleStr = OLESTR("{6507769E-8DF7-4956-9A3D-85EDFE9951A7}");
	*ppViewType = static_cast<LPOLESTR>(CoTaskMemAlloc((wcslen(lpOleStr) + 1) * sizeof(WCHAR)));
	wcscpy(*ppViewType, lpOleStr);

	*pViewOptions = MMC_VIEW_OPTIONS_CREATENEW;
	return S_FALSE;
}
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
// LocalSettings.h: interface for the CLocalSettings class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOCALSETTINGS_H__8EAC01D9_D328_41F2_9F76_28E370840335__INCLUDED_)
#define AFX_LOCALSETTINGS_H__8EAC01D9_D328_41F2_9F76_28E370840335__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Node.h"

class CSAFMQManagerRoot;
class CSAFMQManagerRootComponent;

class CLocalSettings : public  CNode<CLocalSettings, CSAFMQManagerRoot, CSAFMQManagerRootComponent>
{
public:
	static const GUID*		m_NODETYPE;
	static const OLECHAR*	m_SZNODETYPE;
	static const OLECHAR*	m_SZDISPLAY_NAME;
	static const CLSID*		m_SNAPIN_CLASSID;

public:
	CLocalSettings(CConsoleProvider* provider);
	virtual ~CLocalSettings();


	STDMETHOD(GetResultViewType)(LPOLESTR *ppViewType,long *pViewOptions);

};

#endif // !defined(AFX_LOCALSETTINGS_H__8EAC01D9_D328_41F2_9F76_28E370840335__INCLUDED_)

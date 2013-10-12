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
// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__04692E65_9990_446C_B6F6_559F90E720E7__INCLUDED_)
#define AFX_STDAFX_H__04692E65_9990_446C_B6F6_559F90E720E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#pragma warning(disable:4290) // ignore warnings about c++ exception signatures not implemented.

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;

#include <atlcom.h>
#include <atlwin.h>
#include <atlhost.h>
#include <atlctl.h>

#include <comdef.h>
#include <ShlObj.h>

#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>

#include "vvec.h"

#include "SAFMQManagerSnapin.h"

#include "CommonControls.h"

using namespace std;

#define IMG_SAFMQ 0
#define IMG_SERVER_U 1
#define IMG_SERVER_C 2
#define IMG_SERVER_DC 3
#define IMG_USER 4
#define IMG_GROUP 5
#define IMG_QUEUE 6

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__04692E65_9990_446C_B6F6_559F90E720E7__INCLUDED)

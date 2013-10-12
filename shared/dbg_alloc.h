#ifndef _DBG_ALLOC_H_
#define _DBG_ALLOC_H_

#ifdef	__USE_DBG_ALLOC

//#warning "Using Debug Allocation Routines"

#include <bits/c++config.h>
#include <cstring>
#include <climits>
#include <cstdlib>
#include <cstddef>
#include <new>
#include <memory>
#include <string>
#include <set>

#include <stdlib.h>

#define DEBUG_NEW new(__FILE__, __LINE__)
void* operator new(size_t nSize, const char* lpszFileName, int nLine);
void* operator new[](size_t nSize, const char* lpszFileName, int nLine);

void operator delete(void* p);
void operator delete[](void* p);

#endif //	__USE_DBG_ALLOC

void dbg_dump();
#endif


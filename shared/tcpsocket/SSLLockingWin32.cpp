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
#include <windows.h>
#include "SSLLocking.h"
#include <openssl/crypto.h>

 
struct SSLLockingData {
	CRITICAL_SECTION	*sections;

	SSLLockingData() {
		int count;
		sections = new CRITICAL_SECTION[count = CRYPTO_num_locks()];
		for(int x=0; x<count; x++) {
			::InitializeCriticalSection(sections + x);
		}
	}

	~SSLLockingData() {
		delete [] sections;
	}
};

struct CRYPTO_dynlock_value {
	CRITICAL_SECTION	section;
};

SSLLockingData*	SSLLocking::pData;

SSLLocking::SSLLocking()
{
	pData = new SSLLockingData;

	CRYPTO_set_locking_callback(SSLLockingFunction);
	CRYPTO_set_id_callback(SSLIDFunction);
	CRYPTO_set_dynlock_create_callback(SSLDynamicLockCreate);
	CRYPTO_set_dynlock_destroy_callback(SSLDynamicLockDestroy);
	CRYPTO_set_dynlock_lock_callback(SSLDynamicLock);
}

SSLLocking::~SSLLocking()
{
	delete pData;
}

void SSLLocking::SSLLockingFunction(int mode, int lockNum, const char* file, int line)
{
	if (mode & CRYPTO_LOCK) {
		::EnterCriticalSection(pData->sections + lockNum);
	} else {
		::LeaveCriticalSection(pData->sections + lockNum);
	}
}

unsigned long SSLLocking::SSLIDFunction()
{
	return (unsigned long)::GetCurrentThread();	
}

struct CRYPTO_dynlock_value* SSLLocking::SSLDynamicLockCreate(const char* file, int line)
{
	CRYPTO_dynlock_value* v = new CRYPTO_dynlock_value;
	::InitializeCriticalSection(&v->section);
	return v;
}

void SSLLocking::SSLDynamicLockDestroy(struct CRYPTO_dynlock_value* lock, const char* file, int line)
{
	delete lock;
}

void SSLLocking::SSLDynamicLock(int mode, struct CRYPTO_dynlock_value* lock, const char* file, int line)
{
	if (mode & CRYPTO_LOCK) {
		::EnterCriticalSection(&lock->section);
	} else {
		::LeaveCriticalSection(&lock->section);
	}
}


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




This software implements a platform independent Store and Forward Message Queue.
*/
#include "Mutex.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
struct MutexData {
	MutexData() {
		::InitializeCriticalSection(&csmtx);
	}
	~MutexData() {
		::DeleteCriticalSection(&csmtx);
	}
	void lock() {
		::EnterCriticalSection(&csmtx);
	}
	void unlock() {
		::LeaveCriticalSection(&csmtx);
	}
	CRITICAL_SECTION	csmtx;
};
#else
#include <pthread.h>
struct MutexData {
	MutexData() {
		::pthread_mutex_init(&mutex,NULL);
	}
	~MutexData() {
		::pthread_mutex_destroy(&mutex);
	}
	void lock() {
		::pthread_mutex_lock(&mutex);
	}
	void unlock() {
		::pthread_mutex_unlock(&mutex);
	}
	pthread_mutex_t mutex;
};
#endif


Mutex::Mutex()
{
	data = new MutexData;
}

Mutex::~Mutex()
{
	delete data;	
}

void Mutex::Lock()
{
	data->lock();
}

void Mutex::Unlock()
{
	data->unlock();
}


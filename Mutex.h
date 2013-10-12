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




This software implements a platform independent mutual exclusion class
*/
#if !defined(_MUTEX_H_)
#define _MUTEX_H_

struct MutexData;

class Mutex  
{
public:
	Mutex();
	virtual ~Mutex();

	void Lock();
	void Unlock();
protected:
	MutexData	*data;
};

class MutexLock
{
public:
	MutexLock(Mutex* mtx) {
		this->mtx = mtx;
		this->mtx->Lock();
	}

	virtual ~MutexLock() {
		this->mtx->Unlock();
	}
protected:
	Mutex	*mtx;
};

#endif // !defined(_MUTEX_H_)

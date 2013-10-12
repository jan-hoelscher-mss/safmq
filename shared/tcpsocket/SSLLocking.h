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
#ifndef _SAFMQ_SSL_LOCKING_H_
#define _SAFMQ_SSL_LOCKING_H_


struct CRYPTO_dynlock_value;
struct SSLLockingData;

class SSLLocking
{
public:
	SSLLocking();
	~SSLLocking();

	static void SSLLockingFunction(int mode, int lockNum, const char* file, int line);
	static unsigned long SSLIDFunction();

	static struct CRYPTO_dynlock_value* SSLDynamicLockCreate(const char* file, int line);
	static void SSLDynamicLockDestroy(struct CRYPTO_dynlock_value* lock, const char* file, int line);
	static void SSLDynamicLock(int mode, struct CRYPTO_dynlock_value* lock, const char* file, int line);

private:
	static SSLLockingData*	pData;
};


#endif

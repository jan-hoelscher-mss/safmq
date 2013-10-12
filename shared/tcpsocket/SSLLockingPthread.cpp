#include "SSLLocking.h"
#include <openssl/crypto.h>
#include <pthread.h>

 
struct SSLLockingData {
	pthread_mutex_t	*sections;

	SSLLockingData() {
		int count;
		sections = new pthread_mutex_t[count = CRYPTO_num_locks()];
		for(int x=0; x<count; x++) {
			::pthread_mutex_init(sections + x, NULL);
		}
	}

	~SSLLockingData() {
		delete [] sections;
	}
};

struct CRYPTO_dynlock_value {
	pthread_mutex_t	section;
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
		::pthread_mutex_lock(pData->sections + lockNum);
	} else {
		::pthread_mutex_unlock(pData->sections + lockNum);
	}
}

unsigned long SSLLocking::SSLIDFunction()
{
	return (unsigned long)::pthread_self();	
}

struct CRYPTO_dynlock_value* SSLLocking::SSLDynamicLockCreate(const char* file, int line)
{
	CRYPTO_dynlock_value* v = new CRYPTO_dynlock_value;
	::pthread_mutex_init(&v->section, NULL);
	return v;
}

void SSLLocking::SSLDynamicLockDestroy(struct CRYPTO_dynlock_value* lock, const char* file, int line)
{
	delete lock;
}

void SSLLocking::SSLDynamicLock(int mode, struct CRYPTO_dynlock_value* lock, const char* file, int line)
{
	if (mode & CRYPTO_LOCK) {
		::pthread_mutex_lock(&lock->section);
	} else {
		::pthread_mutex_unlock(&lock->section);
	}
}


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
#include "safmq.h" // include the SAFMQ_INT32 definition
#include "safmq-Signal.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>
struct signalData {
	signalData(bool broadcast) {
		event = ::CreateEvent(NULL,broadcast?TRUE:FALSE,FALSE,NULL);
	}
	~signalData() {
		::CloseHandle(event);
	}
	
	Signal::Result wait(SAFMQ_INT32 timeoutmilliseconds) {
		DWORD result;
		if (timeoutmilliseconds < 0)
			result = ::WaitForSingleObject(event,INFINITE);
		else
			result = ::WaitForSingleObject(event,timeoutmilliseconds);
		if (result == WAIT_OBJECT_0)
			return Signal::SIGNAL_SET;
		else if (result == WAIT_TIMEOUT)
			return Signal::SIGNAL_TIMEOUT;
		return Signal::SIGNAL_ERROR;
	}
	
	void set() {
		::PulseEvent(event);
	}

	HANDLE	event;
};
#else
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
struct signalData {
	signalData(bool broadcast) {
		::pthread_cond_init(&cond,NULL);
		::pthread_mutex_init(&mtx, NULL);
		this->broadcast = broadcast;
	}
	~signalData() {
		::pthread_cond_destroy(&cond);
		::pthread_mutex_destroy(&mtx);
	}
	
	Signal::Result wait(SAFMQ_INT32 timeoutmilliseconds) {
		int retcode;

		if (timeoutmilliseconds < 0) {
			pthread_mutex_lock(&mtx);
			retcode = ::pthread_cond_wait(&cond, &mtx);
			pthread_mutex_unlock(&mtx);
		} else {
			struct timeval	now;
			struct timespec timeout;

			gettimeofday(&now,NULL);
			timeout.tv_sec = now.tv_sec + (timeoutmilliseconds / 1000);
			timeout.tv_nsec = now.tv_usec * 1000 + (timeoutmilliseconds % 1000) * 1000 * 1000;
			if (timeout.tv_nsec / (1000*1000*1000)) {
				timeout.tv_sec += timeout.tv_nsec / (1000*1000*1000);
				timeout.tv_nsec %= (1000*1000*1000);
			}
			pthread_mutex_lock(&mtx);
			retcode = ::pthread_cond_timedwait(&cond, &mtx, &timeout);
			pthread_mutex_unlock(&mtx);
		}

		if (retcode == 0)
			return Signal::SIGNAL_SET;
		else if (retcode == ETIMEDOUT)
			return Signal::SIGNAL_TIMEOUT;
		return Signal::SIGNAL_ERROR;
	}
	
	void set() {
		if (broadcast) {
			pthread_cond_broadcast(&cond);
			pthread_cond_signal(&cond);
		} else {
			::pthread_cond_signal(&cond);
		}
	}

	pthread_cond_t	cond;
	pthread_mutex_t	mtx;
	bool			broadcast;
};
#endif

///////////////////////////////////////////////////////////////////////////////
//
//
Signal::Signal(bool broadcast)
{
	data = new signalData(broadcast);
}

Signal::~Signal()
{
	delete data;
}

Signal::Result Signal::WaitFor(SAFMQ_INT32 timeoutmilliseconds)
{
	return data->wait(timeoutmilliseconds);
}

void Signal::Set()
{
	data->set();
}

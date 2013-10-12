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




This software implements a platform independent thread library
*/
#include "dbg_alloc.h"
#ifdef	__USE_DBG_ALLOC
#define new DEBUG_NEW
#endif

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "thdlib.h"

using namespace std;
//#ident "@(#) $RCSfile: thdlib.cpp,v $ $Revision: 1.10 $ $Date: 2005/12/01 22:54:04 $"


extern "C" {
	typedef void* (*thd_launchp)(void *);
}



#ifdef _WIN32
//////////////////////////////////////////////////////
// windows specific routines
//////////////////////////////////////////////////////
	struct thd_cntl {
		thd_cntl() {
			the_thread = NULL;
		}
		~thd_cntl() {
			if (the_thread)
				::CloseHandle(the_thread);
		}
		HANDLE		the_thread;
	};

	unsigned long STDCALL thd_start(void* param) {
		return (long)((Thread*)param)->run();
	}

	Thread::Thread(attachment a)
	{
		m_bStop = false;
		m_pcntl = new thd_cntl;
	}
	
	Thread::~Thread() {
		delete m_pcntl;
	}

	int Thread::start()
	{
		return (int)(m_pcntl->the_thread = ::CreateThread(NULL,0,thd_start,this,0,0));
	}

	void* Thread::join()
	{
		unsigned long	exitcode;
		::WaitForSingleObject(m_pcntl->the_thread, INFINITE);
		::GetExitCodeThread(m_pcntl->the_thread, &exitcode);
		return (void*)exitcode;
	}

#else
//////////////////////////////////////////////////////
// pthread specific routines
//////////////////////////////////////////////////////
	/**
	Private Contrl structure for the Thread object.  Contains
	a thread attribute value, and the pthread_t pointer to the thread.
	It is declared here so that the implementation may be changed at
	any time and not affect client programs.
	*/
	struct thd_cntl {
		/// Thread Attribute structure, use to set thread model
		pthread_attr_t	attr;
		/// Reference to the thread.
		pthread_t		thread;
	};

	void* thd_launch(void* param) {
		return ((Thread*)param)->run();
	}
	/**
		Constructs the Thread.  Sets the stop flag to false and
		allocates a control stucture
	*/
	Thread::Thread(attachment a)
	{
		m_bStop = false;
		m_pcntl = new thd_cntl;

		pthread_attr_init(&m_pcntl->attr);
		pthread_attr_setscope(&m_pcntl->attr, PTHREAD_SCOPE_SYSTEM);

		if(a == Thread::joinable)
			pthread_attr_setdetachstate(&(m_pcntl->attr),PTHREAD_CREATE_JOINABLE);
		else if(a == Thread::detached)
			pthread_attr_setdetachstate(&(m_pcntl->attr),PTHREAD_CREATE_DETACHED);
	}

	/**
		Deallocates the control structure.
	*/
	Thread::~Thread()
	{
		pthread_attr_destroy(&m_pcntl->attr);
		delete m_pcntl;
	}

	/**
		Initiates the thread.
	*/
	int Thread::start()
	{
		return pthread_create(&m_pcntl->thread, &m_pcntl->attr, 
				(thd_launchp)thd_launch, this);
	}

	/**
		Waits until the thread has completed, and returns the
		threads return value
	*/
	void* Thread::join()
	{
		void*	ret = NULL;
		//cout << "waiting for " << m_pcntl->thread << endl;
		pthread_join(m_pcntl->thread, &ret);
		return ret;
	}

#endif

//////////////////////////////////////////////////////
// OS Inspecific routines
//////////////////////////////////////////////////////

/**
	Sets the stop flag for the thread
*/
void Thread::stop()
{
	m_bStop = true;
}


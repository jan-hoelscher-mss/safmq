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




This software implements a platform independent C++ thread library.
*/
#if !defined _THDLIB_H_
#define _THDLIB_H_
//#ident "@(#) $RCSfile: thdlib.h,v $ $Revision: 1.3 $ $Date: 2005/12/01 22:54:04 $"

struct thd_cntl;

#ifdef _WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

/**
Base class for thread objects.  Alows derived classes to implement the
<code>run()</code> method to implement a thread.  Thread parameters
can be passed as members to the thread.  Communication with a thread
could be implemented by members accesable to the outside world and
to the thread it self.  <code>Thread::run()</code> is a non static pure
virtual, allowing implementers of the <code>Thread</code> abstract base class
to have their own main processing method.  Basic operation is as follows.

<code><pre>
	class MyThread : public Thread {
	protected:
		virtual void* run(void* param);
	};

	void* MyThread::run() {
		// do my operations, parameters are object members....

		return NULL;	// Return value cast as void*
	}

	int main() {
		MyThread	*thd = new MyThread;
		thd->start();
		thd->join();
		return 0;
	}</pre></code>
*/
class Thread {
public:
	enum attachment { detached, joinable };

	Thread(attachment a = Thread::joinable);
	virtual ~Thread();

	virtual int start();
	virtual void stop();
	virtual void* join();
protected:
	/**
		Worker method of the thread.
		@return The return value of the program cast to void*
	*/
	virtual void* run() = 0;

	/**
		Stop flag for the thread.  If a thread has a main loop
		it should check this flag to signal completion
	*/
	bool	m_bStop;

protected:
	/// Private control structure.
	thd_cntl*	m_pcntl;

	friend void* thd_launch(void* );
	friend unsigned long STDCALL thd_start(void* );
};

#endif

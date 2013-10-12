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




This software implements a platform independent event-signal class.
*/
#if !defined(_SAFMQ_SIGNAL_H_)
#define _SAFMQ_SIGNAL_H_

struct signalData;

class Signal  
{
public:
	enum Result { SIGNAL_TIMEOUT, SIGNAL_SET, SIGNAL_ERROR };

	Signal(bool broadcast = false);
	virtual ~Signal();

	virtual Signal::Result WaitFor(SAFMQ_INT32 timeoutmilliseconds);
	virtual void Set();
private:
	signalData	*data;
};

#endif // !defined(_SAFMQ_SIGNAL_H_)

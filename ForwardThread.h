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
#if !defined(_FORWARDTHREAD_H_)
#define _FORWARDTHREAD_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "thdlib.h"
#include "safmq-Signal.h"
#include "safmq.h"


class ForwardThread : public Thread  
{
public:
	ForwardThread();
	virtual ~ForwardThread();

	virtual void stop();

protected:
	virtual void* run();
	void SendError(safmq::QueueMessage& src, safmq::ErrorCode err);

	Signal	quitsignal;
};

#endif // !defined(AFX_FORWARDTHREAD_H__C6E89BA2_EBB4_4EDE_A80F_AC22B6263071__INCLUDED_)

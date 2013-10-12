/*
 Copyright 2005-2007 Matthew J. Battey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software distributed
	under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied. See the License for the
	specific language governing permissions and limitations under the License.


This software implements a Java interface to SAFMQ (see http://safmq.sourceforge.net).

*/
package com.safmq.jms;

import java.net.URI;

public class Queue extends Destination implements javax.jms.Queue {
	Queue(URI uri) {
		super(uri);
	}

	/**
	Provides the name of the queue associated with this queue.
	@return The name of the queue specified by this object
	*/
	public String getQueueName() {
		return uri.getPath().substring(1);
	}

	public String toString() {
		return uri.toString();
	}
}

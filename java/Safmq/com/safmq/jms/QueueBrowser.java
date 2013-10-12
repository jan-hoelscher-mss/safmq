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

import java.util.ArrayList;
import java.util.Enumeration;

import javax.jms.JMSException;
import javax.jms.Queue;

import com.safmq.CursorHandle;
import com.safmq.MessageQueue;
import com.safmq.QueueMessage;
import com.safmq.Safmq;

public class QueueBrowser implements javax.jms.QueueBrowser {
	Queue			queue;
	String			selector;
	MessageQueue	theQueue;
	ArrayList		enums = new ArrayList();
	
	class SafmqJMSQueueEnumeration implements Enumeration {
		CursorHandle	cursor = new CursorHandle();

		SafmqJMSQueueEnumeration() throws JMSException {
			int ec = theQueue.OpenCursor(cursor);
			if (ec != Safmq.EC_NOERROR)
				throw new JMSException(getClass().getName() + ": OpenCursor Error: " + Safmq.ecDecode(ec));
		}
		
		void close() {
			theQueue.CloseCursor(cursor);
		}
		
		public boolean hasMoreElements() {
			return theQueue != null && cursor != null && theQueue.TestCursor(cursor) == Safmq.EC_NOERROR;
		}
		
		public Object nextElement() {
			QueueMessage	msg = new QueueMessage();
			if (theQueue.PeekCursor(true, cursor, msg) == Safmq.EC_NOERROR) {
				theQueue.AdvanceCursor(cursor);
				
				switch (msg.getBodyType()) {
					case Safmq.BT_TEXT:
					case Safmq.BT_WTEXT: {
						try {
							return new TextMessage(msg);
						} catch (JMSException e) {
							e.printStackTrace();
							return null;
						}						}
					default: {
						try {
							return new BytesMessage(msg);
						} catch (JMSException e) {
							e.printStackTrace();
							return null;
						}
					}
				}
			}
			return null;
		}
	}
	
	QueueBrowser(MessageQueue theQueue, String selector) throws JMSException {
		this.theQueue = theQueue;
		this.selector = selector;
	}
	
	public void close() throws JMSException {
		for(int x = 0; x < enums.size(); x++) {
			((SafmqJMSQueueEnumeration)enums.get(x)).close();
		}
		theQueue.Close();
		theQueue = null;
	}

	public Enumeration getEnumeration() throws JMSException {
		SafmqJMSQueueEnumeration e = new SafmqJMSQueueEnumeration();
		enums.add(e);
		return e;
	}

	public String getMessageSelector() throws JMSException {
		return selector;
	}

	public Queue getQueue() throws JMSException {
		return queue;
	}
}

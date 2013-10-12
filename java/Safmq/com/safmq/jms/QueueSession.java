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

import javax.jms.JMSException;
import javax.jms.Queue;
import javax.jms.QueueReceiver;
import javax.jms.QueueSender;

import com.safmq.MQConnection;
import com.safmq.MessageQueue;

public class QueueSession extends Session implements javax.jms.QueueSession {
	QueueSession(MQConnection con, String serverURI, boolean transacted) {
		super(con, serverURI, transacted);
	}

	public QueueReceiver createReceiver(Queue queue) throws JMSException {
		return createReceiver(queue, null);
	}

	public QueueReceiver createReceiver(Queue queue, String messageSelector) throws JMSException {
		try {
			return new com.safmq.jms.QueueReceiver(new MessageQueue(queue.getQueueName(), con), (com.safmq.jms.Queue) queue,
			        messageSelector);
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public QueueSender createSender(Queue queue) throws JMSException {
		try {
			return new com.safmq.jms.QueueSender(new MessageQueue(queue.getQueueName(), con), (com.safmq.jms.Queue)queue);
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

}

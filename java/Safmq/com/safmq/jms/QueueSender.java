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
import javax.jms.Message;
import javax.jms.Queue;

import com.safmq.MessageQueue;

public class QueueSender extends MessageProducer implements javax.jms.QueueSender {
	QueueSender(MessageQueue queue, com.safmq.jms.Queue dest) throws JMSException {
		super(queue, dest);
	}
	public Queue getQueue() throws JMSException {
		return (Queue)dest;
	}

	public void send(Queue queue, Message message) throws JMSException {
		send((Destination)queue, message, getDeliveryMode(), getPriority(), getTimeToLive());
	}

	public void send(Queue queue, Message message, int deliveryMode, int priority, long timeToLive) throws JMSException {
		send((Destination)queue, message, deliveryMode, priority, timeToLive);
	}
}

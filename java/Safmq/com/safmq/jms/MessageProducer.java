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

import javax.jms.DeliveryMode;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.JMSSecurityException;
import javax.jms.Message;
import javax.jms.ResourceAllocationException;

import com.safmq.MessageQueue;
import com.safmq.QueueHandle;
import com.safmq.QueueMessage;
import com.safmq.Safmq;

public class MessageProducer implements javax.jms.MessageProducer {
	MessageQueue	queue;
	int				defaultPriority = Safmq.MP_STANDARD;
	long			defaultTTL = -1;
	Destination		dest;
	
	MessageProducer(MessageQueue queue, Destination dest) throws JMSException {
		this.queue = queue;
		this.dest = dest;
	}
	
	public void close() throws JMSException {
		try {
			queue.Close();
			queue = null;
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public int getDeliveryMode() throws JMSException {
		return DeliveryMode.PERSISTENT;
	}

	public Destination getDestination() throws JMSException {
		return dest;
	}

	public boolean getDisableMessageID() throws JMSException {
		return false;
	}

	public boolean getDisableMessageTimestamp() throws JMSException {
		return false;
	}

	public int getPriority() throws JMSException {
		return defaultPriority;
	}

	public long getTimeToLive() throws JMSException {
		return defaultTTL < 0 ? 0 :  defaultTTL;
	}

	public void send(Message message) throws JMSException {
		send(message, getDeliveryMode(), getPriority(), getTimeToLive());
	}

	public void send(Destination destination, Message message) throws JMSException {
		send(destination, message, getDeliveryMode(), getPriority(), getTimeToLive());
	}

	public void send(Message message, int deliveryMode, int priority, long timeToLive) throws JMSException {
		try {
			QueueMessage msg = ((com.safmq.jms.Message)message).getMessage();
			if (timeToLive > 0)
				msg.setTimeToLiveSeconds((int)(timeToLive / 1000));
			else
				msg.setTimeToLiveSeconds(-1);
			msg.setMessagePriority((byte)priority);
			int ec = queue.Enqueue(msg);
			switch (ec) {
				case Safmq.EC_NOERROR: break;
				case Safmq.EC_NOTAUTHORIZED:
					throw new JMSSecurityException(getClass().getName() + ": Enqueue Error: " + Safmq.ecDecode(ec));
				case Safmq.EC_FILESYSTEMERROR:
					throw new ResourceAllocationException(getClass().getName() + ": Enqueue Error: " + Safmq.ecDecode(ec));
				default:
					throw new JMSException(getClass().getName() + ": Enqueue Error: " + Safmq.ecDecode(ec));
			}
			if (timeToLive > 0)
				message.setJMSExpiration(msg.getTimeStamp() + (timeToLive/1000)*1000);
			message.setJMSDestination(getDestination());
			message.setJMSDeliveryMode(getDeliveryMode());
			message.setJMSPriority(getPriority());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public void send(Destination destination, Message message, int deliveryMode, int priority, long timeToLive) throws JMSException {
		try {
			QueueHandle	qh = new QueueHandle();
			String		qn = ((com.safmq.jms.Queue)destination).getQueueName();
			int			ec = queue.getConnection().OpenQueue(qn, qh);
			if (ec == Safmq.EC_NOERROR) {
				QueueMessage	msg = ((com.safmq.jms.Message)message).getMessage();
				if (timeToLive > 0)
					msg.setTimeToLiveSeconds((int)(timeToLive / 1000));
				else
					msg.setTimeToLiveSeconds(-1);
				msg.setMessagePriority((byte)priority);
				ec = queue.getConnection().Enqueue(qh, msg);
				queue.getConnection().CloseQueue(qh);
				if (ec != Safmq.EC_NOERROR)
					throw new JMSException(getClass().getName() + ": Enqueue Error:" + Safmq.ecDecode(ec));
				if (timeToLive > 0)
					message.setJMSExpiration(msg.getTimeStamp() + (timeToLive/1000)*1000);
				message.setJMSDestination(destination);
				message.setJMSDeliveryMode(getDeliveryMode()); // NOTE: Must be changed if SAFMQ ads delivery modes
				message.setJMSPriority(priority);

			} else {
				throw new JMSException(getClass().getName() + ": OpenQueue Error:" + Safmq.ecDecode(ec));
			}
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public void setDeliveryMode(int deliveryMode) throws JMSException {
		// NOTE: SAFMQ only has a single delivery mode
	}

	public void setDisableMessageID(boolean value) throws JMSException {
		// NOTE: SAFMQ 0.6.0 does not support message disableing
	}

	public void setDisableMessageTimestamp(boolean value) throws JMSException {
		// NOTE: SAFMQ 0.6.0 does not support message disableing
	}

	public void setPriority(int defaultPriority) throws JMSException {
		this.defaultPriority = defaultPriority;
	}

	public void setTimeToLive(long timeToLive) throws JMSException {
		this.defaultTTL = timeToLive;
	}

}

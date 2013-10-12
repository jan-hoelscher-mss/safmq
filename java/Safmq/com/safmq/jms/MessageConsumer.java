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
import javax.jms.MessageListener;

import com.safmq.MessageQueue;
import com.safmq.QueueMessage;
import com.safmq.Safmq;

public class MessageConsumer implements javax.jms.MessageConsumer {
	MessageQueue	que;
	String			messageSelector;
	Destination		dest;
	
	MessageConsumer(MessageQueue que, Destination dest, String messageSelector) throws JMSException {
		this.que = que;
		this.dest = dest;
		this.messageSelector = messageSelector;
	}
	
	public void close() throws JMSException {
		try {
			que.Close();
			que = null;
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public MessageListener getMessageListener() throws JMSException {
		// TODO getMessageListener()
		return null;
	}

	public String getMessageSelector() throws JMSException {
		return messageSelector;
	}

	public Message receive() throws JMSException {
		return retrieve(-1); // negative == infinite timeout
	}

	public Message receive(long timeout) throws JMSException {
		if (timeout == 0)
			timeout = -1; // infinite wait;
		else 
			timeout /= 1000;
		return retrieve((int)timeout);
	}

	public Message receiveNoWait() throws JMSException {
		return retrieve(0);
	}

	private Message retrieve(int seconds) throws JMSException {
		QueueMessage	msg = new QueueMessage();
		int ec = que.Retrieve(true, seconds, msg);
		if (ec == Safmq.EC_NOERROR) {
			if (msg.getBodyType() == Safmq.BT_TEXT || msg.getBodyType() == Safmq.BT_WTEXT)
				return new com.safmq.jms.TextMessage(msg);
			return new com.safmq.jms.BytesMessage(msg);
		} else if (ec == Safmq.EC_NOMOREMESSAGES) {
			return null;
		} else if (ec == Safmq.EC_TIMEDOUT) {
			return null;
		}
		throw new JMSException(getClass().getName() + ": Retrieve Error:" + Safmq.ecDecode(ec));		
	}
	
	public void setMessageListener(MessageListener listener) throws JMSException {
		// TODO setMessageListener()
	}
}

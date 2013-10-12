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

import java.io.Serializable;
import java.net.URI;

import javax.jms.BytesMessage;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.MapMessage;
import javax.jms.Message;
import javax.jms.MessageConsumer;
import javax.jms.MessageListener;
import javax.jms.MessageProducer;
import javax.jms.ObjectMessage;
import javax.jms.Queue;
import javax.jms.QueueBrowser;
import javax.jms.StreamMessage;
import javax.jms.TemporaryQueue;
import javax.jms.TemporaryTopic;
import javax.jms.TextMessage;
import javax.jms.Topic;
import javax.jms.TopicSubscriber;

import com.safmq.MQConnection;
import com.safmq.MessageQueue;
import com.safmq.QueueHandle;
import com.safmq.QueueMessage;
import com.safmq.Safmq;

public class Session implements javax.jms.Session {
	MQConnection 	con;
	boolean			transacted;
	String			serverURI;
	
	Session(MQConnection con, String serverURI, boolean transacted) {
		this.con = con;
		this.transacted = transacted;
		this.serverURI = serverURI;
	}
	
	public void close() throws JMSException {
		con.Close();
	}

	public void commit() throws JMSException {
		int ec = con.CommitTransaction();
		if (ec != Safmq.EC_NOERROR) {
			throw new JMSException("Commit Error:" + Safmq.ecDecode(ec));
		}
	}

	public QueueBrowser createBrowser(Queue queue) throws JMSException {
		try {
			return new com.safmq.jms.QueueBrowser(new MessageQueue(queue.getQueueName(), con), null);
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public QueueBrowser createBrowser(Queue queue, String messageSelector) throws JMSException {
		try {
			return new com.safmq.jms.QueueBrowser(new MessageQueue(queue.getQueueName(), con), messageSelector);
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public BytesMessage createBytesMessage() throws JMSException {
		QueueMessage	msg = new QueueMessage();
		msg.setBodyType(Safmq.BT_BINARY);
		return new com.safmq.jms.BytesMessage(msg);
	}

	public MessageConsumer createConsumer(Destination destination) throws JMSException {
		if (!(destination instanceof com.safmq.jms.Queue))
			throw new JMSException("Invalid destination data type, SAFMQ 0.6.0 support Queue destinations");

		try {
			return new com.safmq.jms.MessageConsumer(new MessageQueue(((Queue) destination).getQueueName(), con),
			        (com.safmq.jms.Queue) destination, null);
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public MessageConsumer createConsumer(Destination destination, String messageSelector) throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does not support message slectors");
	}

	public MessageConsumer createConsumer(Destination destination, String messageSelector, boolean noLocal) throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does not support message slectors");
	}

	public TopicSubscriber createDurableSubscriber(Topic topic, String name) throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does not support durrable subscriptions");
	}

	public TopicSubscriber createDurableSubscriber(Topic topic, String name, String messageSelector, boolean noLocal) throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does not support durrable subscriptions");
	}

	public MapMessage createMapMessage() throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does not support MapMessage objects");
	}

	public Message createMessage() throws JMSException {
		QueueMessage msg = new QueueMessage();
		msg.setBodyType(Safmq.BT_BINARY);
		
		return new com.safmq.jms.BytesMessage(msg);
	}

	public ObjectMessage createObjectMessage() throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does not support ObjectMessage objects");
	}

	public ObjectMessage createObjectMessage(Serializable object) throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does not support ObjectMessage objects");
	}

	public MessageProducer createProducer(Destination destination) throws JMSException {
		if (!(destination instanceof com.safmq.jms.Queue))
			throw new JMSException("Invalid destination data type, SAFMQ 0.6.0 support Queue destinations");
		try {
			return new com.safmq.jms.MessageProducer(new MessageQueue(((Queue) destination).getQueueName(), con),
			        (com.safmq.jms.Queue) destination);
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public Queue createQueue(String queue) throws JMSException {
		String url = serverURI + "/" + queue;
		try {
			return new com.safmq.jms.Queue(new URI(url));
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public StreamMessage createStreamMessage() throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does nto support StreamMessage objects");
	}

	public TemporaryQueue createTemporaryQueue() throws JMSException {
		String[] tmpQueueName = new String[1];
		QueueHandle tmpQueue = new QueueHandle();
		int ec = con.CreateTempQueue(tmpQueueName, tmpQueue);
		
		
		if (ec == Safmq.EC_NOERROR){
			String url = serverURI + "/" + tmpQueueName[0];
			try {
				return new com.safmq.jms.TemporaryQueue(new URI(url), new MessageQueue(con, tmpQueue));
			} catch (Exception e) {
				JMSException je = new JMSException(e.toString());
				je.setLinkedException(e);
				throw je;
			}
		} else {
			throw new JMSException("Unabel to allocate Temporary Queue: " + Safmq.ecDecode(ec));
		}
	}

	public TemporaryTopic createTemporaryTopic() throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does not support temporary topics");
	}

	public TextMessage createTextMessage() throws JMSException {
		QueueMessage	msg = new QueueMessage();
		msg.setBodyType(Safmq.BT_TEXT);
		TextMessage	tm = new com.safmq.jms.TextMessage(msg);
		return tm;
	}

	public TextMessage createTextMessage(String text) throws JMSException {
		TextMessage	tm = createTextMessage();
		tm.setText(text);
		return tm;
	}

	public Topic createTopic(String arg0) throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does not support topics");
	}

	public int getAcknowledgeMode() throws JMSException {
		return (this.transacted) ? SESSION_TRANSACTED : AUTO_ACKNOWLEDGE;
	}

	public MessageListener getMessageListener() throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does not support session message listeners");
	}

	public boolean getTransacted() throws JMSException {
		return transacted;
	}

	public void recover() throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does not support the recover interface");
	}

	public void rollback() throws JMSException {
		int ec = con.RollbackTransaction();
		if (ec != Safmq.EC_NOERROR) throw new JMSException("Rollback Error:" + Safmq.ecDecode(ec));
	}

	public void run() {
		// NOTE: Nothing to do in a run opteration
	}

	public void setMessageListener(MessageListener messageListener) throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does not support session level message listeners");
	}

	public void unsubscribe(String name) throws JMSException {
		throw new JMSException("SAFMQ 0.6.0 does not support durrable subscribers");
	}
}

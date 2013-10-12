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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.net.URI;
import java.util.Enumeration;

import javax.jms.Destination;
import javax.jms.JMSException;

import com.safmq.QueueMessage;
import com.safmq.UUID;

public abstract class Message implements javax.jms.Message {
	QueueMessage 	msg;
	Destination		dest = null;
	long			expiration;
	
	Message(QueueMessage msg) {
		this.msg = msg;
	}
	
	abstract QueueMessage getMessage() throws Exception;
	
	public void acknowledge() throws JMSException {
		// NOTE: All SAFMQ messages are auto acknowledge in version 0.6.0
	}

	public void clearProperties() throws JMSException {
		// NOTE: Label is the only clearable property
		msg.setLabel("");
	}

	public boolean getBooleanProperty(String name) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public byte getByteProperty(String name) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public double getDoubleProperty(String name) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public float getFloatProperty(String name) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public int getIntProperty(String name) throws JMSException {
		if (name.equals("body-size"))
			return msg.getBodySize();
		return 0;
	}

	public String getJMSCorrelationID() throws JMSException {
		UUID uuid = msg.getReciptID();
		return "ID: " + uuid.toString();
	}

	public byte[] getJMSCorrelationIDAsBytes() throws JMSException {
		UUID uuid = msg.getReciptID();
		try {
			ByteArrayOutputStream	buf = new ByteArrayOutputStream();
			DataOutputStream		o = new DataOutputStream(buf);
			uuid.write(o);
			o.close();
			return buf.toByteArray();
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public int getJMSDeliveryMode() throws JMSException {
		return Message.DEFAULT_DELIVERY_MODE;
	}

	public Destination getJMSDestination() throws JMSException {
		return dest;
	}

	public long getJMSExpiration() throws JMSException {
		if (msg.getTimeToLiveSeconds() <= 0) {
			return 0;
		}
		return msg.getTimeStamp() + msg.getTimeToLiveSeconds() * 1000;
	}

	public String getJMSMessageID() throws JMSException {
		UUID id = msg.getMessageID();
		return "ID: " + id;
	}

	public int getJMSPriority() throws JMSException {
		return msg.getMessagePriority();
	}

	public boolean getJMSRedelivered() throws JMSException {
		// NOTE: Safmq does not support redelivered flags.
		return false;
	}

	public Destination getJMSReplyTo() throws JMSException {
		String rq = msg.getResponseQueueName();
		if (rq == null || rq.length() == 0)
			return null;
		
		try {
			URI u = new URI(rq);
			return new com.safmq.jms.Destination(u);
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public long getJMSTimestamp() throws JMSException {
		return msg.getTimeStamp();
	}

	public String getJMSType() throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public long getLongProperty(String name) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public Object getObjectProperty(String name) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	private static final String list[] = {"label", "body-size"};
	
	public Enumeration getPropertyNames() throws JMSException {
		return new Enumeration () {
			int x = 0;
			
			public boolean hasMoreElements() {
				return x < list.length;
			}
			
			public Object nextElement() {
				return list[x++];
			}
		};
	}

	public short getShortProperty(String name) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public String getStringProperty(String name) throws JMSException {
		if (name.equals("label"))
			return msg.getLabel();
		return null;
	}

	public boolean propertyExists(String name) throws JMSException {
		return name.equals("label") ||
				name.equals("body-size");
	}

	public void setBooleanProperty(String name, boolean value) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public void setByteProperty(String name, byte value) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public void setDoubleProperty(String name, double value)
			throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public void setFloatProperty(String name, float value) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public void setIntProperty(String name, int value) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public void setJMSCorrelationID(String correlationID) throws JMSException {
		if (correlationID.length() >= 39) {
			UUID id = new UUID(correlationID.substring(3));
			msg.setReciptID(id);
		}
	}

	public void setJMSCorrelationIDAsBytes(byte[] correlationID) throws JMSException {
		try {
			DataInputStream in = new DataInputStream(new ByteArrayInputStream(correlationID));
			UUID id = new UUID();
			id.read(in);
			msg.setReciptID(id);
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public void setJMSDeliveryMode(int deliveryMode) throws JMSException {
		// NOTE: SAFMQ only supports default delivery mode
	}

	public void setJMSDestination(Destination destination) throws JMSException {
		this.dest = destination;
	}

	public void setJMSExpiration(long expiration) throws JMSException {
		this.expiration = expiration;
	}

	public void setJMSMessageID(String id) throws JMSException {
		throw new JMSException("SAFMQ does not support setting message id's by clients");
	}

	public void setJMSPriority(int priority) throws JMSException {
		msg.setMessagePriority((byte)priority);
	}

	public void setJMSRedelivered(boolean redelivered) throws JMSException {
		// NOTE: SAFMQ does not support message redelivery in version 0.6.0
	}

	public void setJMSReplyTo(Destination replyTo) throws JMSException {
		msg.setResponseQueueName(((com.safmq.jms.Destination)replyTo).getUri().toString());
	}

	public void setJMSTimestamp(long timestamp) throws JMSException {
		throw new JMSException("SAFMQ does not support setting a message's timestamp");
	}

	public void setJMSType(String type) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public void setLongProperty(String name, long value) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public void setObjectProperty(String name, Object value)
			throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public void setShortProperty(String name, short value) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public void setStringProperty(String name, String value) throws JMSException {
		if (name.equals("label"))
			msg.setLabel(value);
	}

}

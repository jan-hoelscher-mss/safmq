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

import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

import javax.jms.JMSException;

import com.safmq.QueueMessage;

/**
 * 
 * @author Matthew Battey
 * @deprecated
 */
public class StreamMessage extends com.safmq.jms.BytesMessage implements javax.jms.StreamMessage {
	ObjectInputStream		in = null;
	ObjectOutputStream	out = null;
	
	StreamMessage(QueueMessage msg) throws JMSException {
		super(msg);
		try {
			in = new ObjectInputStream(msg.getInputStream());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}
	

	public String readString() throws JMSException {
		return readUTF();
	}

	public void writeString(String value) throws JMSException {
		writeUTF(value);
	}
	

}

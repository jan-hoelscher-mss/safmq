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

import java.io.OutputStream;

import javax.jms.JMSException;

import com.safmq.QueueMessage;

public class TextMessage extends Message implements javax.jms.TextMessage {
	String			text = null;
	boolean			textSet = false;
	
	TextMessage(QueueMessage msg) throws JMSException {
		super(msg);
		
		try {
			if (msg.getBodySize() > 0) {
				byte buffer[] = new byte[msg.getBodySize()];
				msg.getInputStream().read(buffer);
				text = new String(buffer);
			}
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}
	
	public String getText() throws JMSException {
		return text;
	}

	public void setText(String string) throws JMSException {
		text = string;
		textSet = true;
	}

	public void clearBody() throws JMSException {
		text = null;
	}
	
	QueueMessage getMessage() {
		try {
			OutputStream out = msg.getOutputStream();
			out.write(text.getBytes());
			out.close();
			return msg;
		} catch (Exception e) {
			return null;
		}
	}
}

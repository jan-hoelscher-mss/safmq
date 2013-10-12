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

import javax.jms.ConnectionConsumer;
import javax.jms.JMSException;
import javax.jms.Queue;
import javax.jms.QueueSession;
import javax.jms.ServerSessionPool;
import javax.net.ssl.SSLContext;

import com.safmq.MQBuilder;

public class QueueConnection extends Connection implements javax.jms.QueueConnection {
	
	QueueConnection(URI serverURI, String userName, String password, SSLContext ctx) throws JMSException {
		super(serverURI, userName, password, ctx);
	}
	
	public ConnectionConsumer createConnectionConsumer(Queue queue, String messageSelector, ServerSessionPool sessionPool,	int maxMessages) throws JMSException {
		return super.createConnectionConsumer(queue, messageSelector, sessionPool, maxMessages);
	}

	public QueueSession createQueueSession(boolean transacted, int acknowledgeMode) throws JMSException {
		try {
			QueueSession s = new com.safmq.jms.QueueSession(MQBuilder.buildConnection(serverURI,username,password,ctx), serverURI.toString(), transacted);
			sessions.add(s);
			return s;
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}	
	}

}

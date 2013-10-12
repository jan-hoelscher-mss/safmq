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

import javax.jms.JMSException;
import javax.jms.QueueConnection;
import javax.net.ssl.SSLContext;

public class QueueConnectionFactory extends ConnectionFactory implements javax.jms.QueueConnectionFactory {
	public QueueConnectionFactory() {
	}
	
	public QueueConnectionFactory(URI serverURI, String userName, String password, SSLContext ctx) {
		super(serverURI, userName, password, ctx);
	}
	
	public QueueConnection createQueueConnection() throws JMSException {
		return createQueueConnection(getUserName(), getPassword());
	}

	public QueueConnection createQueueConnection(String userName, String password) throws JMSException {
		return new com.safmq.jms.QueueConnection(getServerURI(), userName, password, ctx);
	}
}

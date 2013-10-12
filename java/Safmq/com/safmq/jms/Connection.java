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
import java.util.Vector;

import javax.jms.ConnectionConsumer;
import javax.jms.ConnectionMetaData;
import javax.jms.Destination;
import javax.jms.ExceptionListener;
import javax.jms.JMSException;
import javax.jms.ServerSessionPool;
import javax.jms.Session;
import javax.jms.Topic;
import javax.net.ssl.SSLContext;

import com.safmq.MQBuilder;

/**
 * Represents a connection to a SAFMQ server.  SAFMQ does not support "sessions"
 * so the SAFMQ "session" object is the real connection, and the connection
 * is a place holder for connection information.
 *
 * @author Matthew J. Battey
 * @date Jun 29, 2007
 */
public class Connection implements javax.jms.Connection {
	String 				clientID = ""; 
	String 				username = null;
	String 				password = null;
	SSLContext			ctx = null;
	URI					serverURI = null;
	ExceptionListener	exceptionListener = null;
	Vector				sessions = new Vector();
		
	Connection() {
	}

	Connection(URI serverURI, String username, String password, SSLContext ctx) {
		setServerURI(serverURI);
		setUsername(username);
		setPassword(password);
		setContext(ctx);
	}	
	
	void setServerURI(URI serverURI) {
		this.serverURI = serverURI;
	}
	
	void setUsername(String username) {
		this.username = username;
	}
	
	void setPassword(String password) {
		this.password = password;
	}
	
	void setContext(SSLContext ctx) {
		this.ctx = ctx;
	}

	/////////////////////////////////////////////////////////////////
	// Begin JMS Interface
	public String getClientID() throws JMSException {
		return clientID;
	}

	public void setClientID(String clientID) throws JMSException {
		this.clientID = clientID;
	}

	public ConnectionMetaData getMetaData() throws JMSException {
		return new com.safmq.jms.ConnectionMetaData();
	}

	public ExceptionListener getExceptionListener() throws JMSException {
		return exceptionListener;
	}

	public void setExceptionListener(ExceptionListener exceptionListener) throws JMSException {
		this.exceptionListener = exceptionListener;
	}

	public void start() throws JMSException {
		// NOTE: Do nothing, SAFMQ doesn't have an anagulus start routine
	}

	public void stop() throws JMSException {
		// NOTE: Do nothing, SAFMQ doesn't have an anagulus stop routine
	}

	
	public void close() throws JMSException {
		for (int x = 0; x < sessions.size(); ++x)
			try { ((Session)sessions.get(x)).close(); } catch (Throwable e) {}
	}
	
	public ConnectionConsumer createConnectionConsumer(Destination destination, String messageSelector, ServerSessionPool sessionPool, int maxMessages)  throws JMSException {
		if (!(destination instanceof com.safmq.jms.Destination) )
			throw new JMSException("Destinations must be " + com.safmq.jms.Destination.class.getName() + " derived");
		if (messageSelector != null && !messageSelector.equals(""))
			throw new JMSException("Message Selectors not implemented");
		if (sessionPool != null)
			throw new JMSException("Server Session Pool not implemented");
		throw new JMSException("Not Implemented");
	}
	 
	public ConnectionConsumer createDurableConnectionConsumer(Topic topic, String subscriptionName, String messageSelector, ServerSessionPool sessionPool, int maxMessages) throws JMSException {
		throw new JMSException("Not Implemented");
	}

	public Session createSession(boolean transacted, int acknowledgeMode) throws JMSException {
		try {
			Session s = new com.safmq.jms.Session(MQBuilder.buildConnection(serverURI,username,password,ctx), serverURI.toString(), transacted);
			sessions.add(s);
			return s;
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

}

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

import javax.jms.Connection;
import javax.jms.JMSException;
import javax.net.ssl.SSLContext;

public class ConnectionFactory implements javax.jms.ConnectionFactory {
	String 	userName = null;
	String 	password = null;
	URI		serverURI = null;
	SSLContext ctx = null;
	
	public ConnectionFactory() {
	}
	
	public ConnectionFactory(URI serverURI, String userName, String password, SSLContext ctx) {
		this.serverURI = serverURI;
		this.userName = userName;
		this.password = password;
		this.ctx = ctx;
	}
	
	public Connection createConnection() throws JMSException {
		return createConnection(getUserName(), getPassword());
	}

	public Connection createConnection(String paramUserName, String paramPassword) throws JMSException {
		return new com.safmq.jms.Connection(getServerURI(), paramUserName, paramPassword, ctx);
	}

	String getPassword() {
		return password;
	}

	void setPassword(String password) {
		this.password = password;
	}

	URI getServerURI() {
		return serverURI;
	}

	void setServerURI(URI serverURI) {
		this.serverURI = serverURI;
	}

	public String getUserName() {
		return userName;
	}

	public void setUserName(String userName) {
		this.userName = userName;
	}

	public SSLContext getSSLContext() {
		return ctx;
	}

	public void setSSLContext(SSLContext ctx) {
		this.ctx = ctx;
	}

}

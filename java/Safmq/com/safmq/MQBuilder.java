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

Created on Mar 21, 2005
*/
package com.safmq;

import java.io.IOException;
import java.net.Socket;
import java.net.URI;

import javax.net.SocketFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;

/**
 * A static builder class which builds connections to SAFMQ servers and message
 * queues.
 * 
 * <p>In the case that a connection to a queue server is sought, use the method
 * <code>MQBuilder.buildConnection(String url, String user, String password)</code>.
 * This method will evaluate the URL and create a new connection to the queue
 * server.</p>
 * 
 * <p>In the case that a connection to a message queue is sought, the use the
 * method <code>MQBuilder.buildMessageQueue(String url, String user, String password)</code>.
 * This method will evaluate the URL and create a new connection tot he queue
 * server and wrap that connection with a <code>MessageQueue</code> instance.
 * 
 * @author Matt
 */
public class MQBuilder {
	/**
	 * The number of the default port for SAFMQ servers.
	 */
	public static final int DEF_PORT = 9000;
	public static final int DEF_SSL_PORT = 9001;
	public static final String SAFMQ_PROTO = "safmq";
	public static final String SAFMQ_SSL_PROTO = "safmqs";
	
	private static SSLContext	defaultContext = null;
	
	/**
	 * Constructs a <code>MQConnection<code> connection object connected to a 
	 * SAFMQ server.   
	 * 
	 * @param uri		The URI of the message queue object in the format:
	 * 					<code>protocol://user:password@server-name:port/</code> 
	 * 					where the ":port" , "user",	and "password" portion of the url 
	 * 					is optional.  Valid selections for "protocol" are "safmq" for
	 * 					a clear text connection and "safmqs" for a SSL encrypted connection.
	 * @param user		[optional] The name of the user which is connecting to the server, used if
	 * 					not specified in the uri
	 * @param password	[optional] The password of the user which is connecting to the server, used if
	 * 					not specified in the uri
	 * 
	 * @return A valid <code>MQConnection<code> connection object connected to 
	 * 		a SAFMQ server.
	 * 
	 * @throws MQException In the case that the server refuses the supplied 
	 * 			login credintials, or in the case that the URL could not be correctly
	 * 			parsed. See MQConnection.MQConnection(String,int,String,String) for
	 * 			details on connection errors.
	 * 
	 * @throws IOException In the case that there is a network error attempting
	 * 			to connect to the server via the TCP/IP network.
	 * @see MQConnection#MQConnection(Socket,String,String)
	 */
	static public MQConnection buildConnection(URI uri, String user, String password) throws MQException, IOException {
		return MQBuilder.buildConnection(uri, user, password, defaultContext);
	}
	
	/**
	 * Constructs a <code>MQConnection<code> connection object connected to a 
	 * SAFMQ server.   
	 * 
	 * @param uri		The URI of the message queue object in the format:
	 * 					<code>protocol://user:password@server-name:port/</code> 
	 * 					where the ":port" , "user",	and "password" portion of the url 
	 * 					is optional.  Valid selections for "protocol" are "safmq" for
	 * 					a clear text connection and "safmqs" for a SSL encrypted connection.
	 * @param user		[optional] The name of the user which is connecting to the server, used if
	 * 					not specified in the uri
	 * @param password	[optional] The password of the user which is connecting to the server, used if
	 * 					not specified in the uri
	 * @param ctx		[optional] A SSLContext to use when connecting via SSL.  This will override
	 * 					the contex set by calling MQCOnnection.setSSLContext(SSLContext).
	 * 
	 * @return A valid <code>MQConnection<code> connection object connected to 
	 * 		a SAFMQ server.
	 * 
	 * @throws MQException In the case that the server refuses the supplied 
	 * 			login credintials, or in the case that the URL could not be correctly
	 * 			parsed. See MQConnection.MQConnection(String,int,String,String) for
	 * 			details on connection errors.
	 * 
	 * @throws IOException In the case that there is a network error attempting
	 * 			to connect to the server via the TCP/IP network.
	 * @see MQConnection#MQConnection(Socket,String,String)
	 */
	static public MQConnection buildConnection(URI uri, String user, String password, SSLContext ctx) throws MQException, IOException {
		String userinfo = uri.getUserInfo();
		StringBuffer	u, p;
		
		if (parseUserInfo(userinfo,u=new StringBuffer(),p=new StringBuffer())) {
			user = u.toString();
			password = p.toString();
		}
				
		return new MQConnection(allocSocket(uri.getScheme(),uri.getHost(),uri.getPort(), ctx),user,password);
	}
	
	/**
	 * Constructs a new <code>MQConnection</code> connection object and then a
	 * <code>MessageQueue</code> to wrap it.
	 * 
	 * 
	 * @param uri		The URI of the message queue object in the format:
	 * 					<code>protocol://user:password@server-name:port/queue</code> 
	 * 					where the ":port" , "user",	and "password" portion of the url 
	 * 					is optional.  Valid selections for "protocol" are "safmq" for
	 * 					a clear text connection and "safmqs" for a SSL encrypted connection.
	 * @param user		[optional] The name of the user which is connecting to the server, used if
	 * 					not specified in the uri
	 * @param password	[optional] The password of the user which is connecting to the server, used if
	 * 					not specified in the uri
	 * @return A valid <code>MessageQueue</code> connected to a message queue
	 * 			on a SAFMQ server.
	 * 
	 * @throws MQException In the case that the server refuses the supplied 
	 * 			login credintials, or in the case that the URL could not be correctly
	 * 			parsed.  See errors from MQConnection(String,String,String) and 
	 * 			MQConnection.OpenQueue(String,QueueHandle).
	 * 
	 * @throws IOException In the case that there is a network error attempting
	 * 			to connect to the server via the TCP/IP network.
	 * 
	 * @see MQConnection#MQConnection(Socket,String,String)
	 * @see MQConnection#OpenQueue(String,QueueHandle)
	 */
	static public MessageQueue buildMessageQueue(URI uri, String user, String password) throws MQException, IOException {
		return MQBuilder.buildMessageQueue(uri, user, password, defaultContext);
	}
	
	/**
	 * Constructs a new <code>MQConnection</code> connection object and then a
	 * <code>MessageQueue</code> to wrap it.
	 * 
	 * 
	 * @param uri		The URI of the message queue object in the format:
	 * 					<code>protocol://user:password@server-name:port/queue</code> 
	 * 					where the ":port" , "user",	and "password" portion of the url 
	 * 					is optional.  Valid selections for "protocol" are "safmq" for
	 * 					a clear text connection and "safmqs" for a SSL encrypted connection.
	 * @param user		[optional] The name of the user which is connecting to the server, used if
	 * 					not specified in the uri
	 * @param password	[optional] The password of the user which is connecting to the server, used if
	 * 					not specified in the uri
	 * @param ctx		[optional] A SSLContext to use when connecting via SSL.  This will override
	 * 					the contex set by calling MQCOnnection.setSSLContext(SSLContext).
	 * @return A valid <code>MessageQueue</code> connected to a message queue
	 * 			on a SAFMQ server.
	 * 
	 * @throws MQException In the case that the server refuses the supplied 
	 * 			login credintials, or in the case that the URL could not be correctly
	 * 			parsed.  See errors from MQConnection(String,String,String) and 
	 * 			MQConnection.OpenQueue(String,QueueHandle).
	 * 
	 * @throws IOException In the case that there is a network error attempting
	 * 			to connect to the server via the TCP/IP network.
	 * 
	 * @see MQConnection#MQConnection(Socket,String,String)
	 * @see MQConnection#OpenQueue(String,QueueHandle)
	 */
	static public MessageQueue buildMessageQueue(URI uri, String user, String password, SSLContext ctx) throws MQException, IOException {
		String userinfo = uri.getUserInfo();
		StringBuffer	u, p;
		
		if (parseUserInfo(userinfo,u=new StringBuffer(),p=new StringBuffer())) {
			user = u.toString();
			password = p.toString();
		}

		String resource = uri.getPath();
		if (resource != null && resource.length() > 0 && resource.charAt(0) == '/')
			resource = resource.substring(1);

		return new MessageQueue(resource, new MQConnection(allocSocket(uri.getScheme(),uri.getHost(),uri.getPort(), ctx),user,password));
	}
	
	/**
	 * Sets the SSLContext object for use in SSL communications.  Using an SSL context allows
	 * the client program to determine certificate verification routines, and provide
	 * a client certificate to the server.
	 * 
	 * @param ctx The SSLContext to use.  Specifying null will cause the use of the default
	 * 				SSLContext.
	 */
	static public void setSSLContext(SSLContext ctx) {
		defaultContext = ctx;	
	}
	
	static protected boolean parseUserInfo(String userinfo, StringBuffer user, StringBuffer password)
	{
		if (userinfo != null && userinfo.length() > 0) {
			int sep = userinfo.indexOf(':');
			if (user.length() > 0) user.delete(0,-1);
			if (password.length() > 0) password.delete(0,-1);
			if (sep != -1) {
				user.append(userinfo.substring(0,sep));
				if (sep+1 < userinfo.length())
					password.append(userinfo.substring(sep+1));
			} else {
				user.append(userinfo);	
			}
			return true;
		}
		return false;		
	}
	
	static protected Socket allocSocket(String protocol, String host, int port, SSLContext ctx) throws IOException, MQException {
		Socket	s;
		if (protocol == null)
			protocol = SAFMQ_PROTO;
		if (protocol.equals(SAFMQ_PROTO)) {
			if (port == -1)
				port = DEF_PORT;
			s = new Socket(host,port);
		} else if (protocol.equals(SAFMQ_SSL_PROTO)) {
			if (port == -1)
				port = DEF_SSL_PORT;
			SocketFactory f;
			if (ctx != null)
				f = ctx.getSocketFactory();
			else
				f = SSLSocketFactory.getDefault();
			synchronized (f) {
				s = f.createSocket(host,port);
			}
		} else {
			throw new MQException("Unknown Protocol", Safmq.EC_ERROR);	
		} 
		return s;
	}
	
}

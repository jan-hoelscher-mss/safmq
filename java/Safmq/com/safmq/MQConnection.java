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

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.URI;
import java.util.Vector;

/**
 * This class provides an interface to communicate with a SAFMQ message queue
 * server.  Typically instances of this class are constructed by the 
 * <code>MQBuilder</code> class, however constructing the class directly
 * is also allowed.  
 * 
 * <p>Once constructed, this class may be used to construct
 * instances of the class <code>MessageQueue</code> to abstract the connection
 * interface to the message queue level.</p>
 * 
 * <p><b>Note:</b> This class is not thread safe, and applications should
 * take precautions to not initiate communications with a message queue via
 * more than one (1) thread simultaneously.</p>
 * 
 * @author Matt
 * @see MQBuilder#buildConnection(URI,String,String)
 * @see MessageQueue
 */
public class MQConnection {
	final static String FORWARD_QUEUE_NAME = "_FORWARD_";

	Socket				s;
	DataInputStream		in;
	DataOutputStream	out;
	int					server_major_protocol_version;
	int					server_minor_protocol_version;
	
	/**
	 * Result data from a call to <code>MQConnection.EnumerateQueues(Vector)</code>.
	 * Instances of this class are placed into the vector passed to 
	 * <code>EnumerateQueues()</code> to detail the specifics of the queue.
	 * 
	 * @author matt
	 * @see MQConnection#EnumerateQueues(Vector) MQConnection.EnumerateQueues(Vector)
	 */
	public class QueueData {
		String name;
		String owner;
		
		/**
		 * Constructs the QueueData object from the queue's name and owner.
		 * @param name  The name of the queue.
		 * @param owner	The name of the queue's owner.
		 */
		QueueData(String name, String owner) {
			this.name = name;
			this.owner = owner;
		}
		/**
		 * Provides the queue's name.
		 * @return The queue's name.
		 */
		public String getName() {
			return name;
		}
		/**
		 * Provides the queue's owner's name.
		 * @return	The queue's owner's name.
		 */
		public String getOwner() {
			return owner;
		}
	}
	
	/**
	 * Contains an entity's permission when accessing a queue.
	 * 
	 * @author Matt
	 */
	public class QueuePermissions {
		String	entityName;
		boolean isgroup;
		boolean	read;
		boolean	write;
		boolean	destroy;
		boolean changeSecurity;
	
		/**
		 * Constructs the QueuePermissions from the name of the entity and 
		 * flags regarding its relationship with the queried queue.  This object
		 * is used as a result to 
		 * <code>MQConnection.queueEnumeratePermissions(String,Vector)</code>.
		 * 
		 * @param entityName	The name of the entity these permissions pertain to
		 * @param isgroup		A flag indicating this entity is a group (otherwise 
		 * 						a user)
		 * @param read			A flag indicating whether this entity may read from 
		 * 						the queue
		 * @param write			A flag indicating whether this entity may write to 
		 * 						the queue
		 * @param destroy		A flag indicating whether this entity may remove the 
		 * 						queue from the server
		 * @param changeSecurity	A flag indicating whether this entity may change 
		 * 							the security permissions of the queue
		 */
		QueuePermissions(String entityName, boolean isgroup, boolean read, boolean write, boolean destroy, boolean changeSecurity) {
			this.entityName = entityName;
			this.isgroup = isgroup;
			this.read = read;
			this.write = write;
			this.destroy = destroy;
			this.changeSecurity = changeSecurity;
		}
	
	
		/**
		 * Provides the name of the entity these permissions pertain to.
		 * @return Returns the name of the entity these permissions pertain to.
		 */
		public String getEntityName() {
			return entityName;
		}

		/**
		 * Provides the flag whether this entity may remove the queue from the server.
		 * 
		 * @return Returns the flag whether this entity may remove the queue from the server.
		 */
		public boolean getDestroy() {
			return destroy;
		}
		/**
		 * Provides a flag whether this entity is a group.
		 * @return Returns a flag whether this entity is a group.
		 */
		public boolean getIsGroup() {
			return isgroup;
		}
		/**
		 * Provides a flag whether this entity is allowed to read from the queue.
		 * @return Returns a flag whether this entity is allowed to read from the queue.
		 */
		public boolean getRead() {
			return read;
		}
		/**
		 * Provides a flag whether this entity is allowed to write to the queue.
		 * @return Returns a flag whether this entity is allowed to write to the queue.
		 */
		public boolean getWrite() {
			return write;
		}
		/**
		 * Provides a flag whether the entity is allowed to change the security for the queue.
		 * @return The a flag whether the entity is allowed to change the security for the queue.
		 */
		public boolean getChangeSecurity() {
			return changeSecurity;
		}
	}
	
	/**
	 * Result data from a call to <code>MQConnection.EnumerateUsers(Vector)</code>.
	 * Instances of this class are placed into the vector passed to 
	 * <code>EnumerateUsers()</code> to detail the specifics of the user.
	 * 
	 * @author matt
	 */
	public class UserDescription {
		String name;
		String description;
		
		/**
		 * Constructs the object from the user's name and description.
		 * 
		 * @param name			The name of the user
		 * @param description	The description of the user
		 */
		UserDescription(String name, String description) {
			this.name = name;
			this.description = description;
		}
		/**
		 * Provides the name of the user
		 * @return The name of the user
		 */
		public String getName() {
			return name;
		}
		/**
		 * Provides the description of the user
		 * @return The description of the user
		 */
		public String getDescription() {
			return description;
		}
		
		public boolean equals(Object o) {
			return o != null && (this==o || ((o instanceof UserDescription) && ((UserDescription)o).getName().equals(getName())) || o.equals(getName()));
		}
		
		public String toString() {
			return getClass().getName()+":["+getName()+"]";	
		}
	}
	
	/**
	 * For external use of UserDescriptions.
	 * 
	 * @param name Name of the account
	 * @param desc Description of the account
	 * @return A newly allocated <code>UserDescription</code> object.
	 */
	public UserDescription genDesc(String name, String desc) {
		return new UserDescription(name,desc);	
	}
	
	/**
	 * Constructs the object from the name of the server, the tcp/ip port to be used
	 * the user's name and password.  
	 *
	 * @param s				A socket connected to the server.
	 * @param user			The name of the user wishing to connect to the server
	 * @param password		The password of the user wishing to connect to the server
	 * @throws MQException	In the case that the supplied credentials are not accepted by
	 * 						the SAFMQ server. The value of <code>MQException.getErrorCode()</code>
	 * 						could be but is not limited to:
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_LOGIN</code></td>
	 *				<td>The login credentials supplied were not accepted.
	 *				</td></tr>
	 *</table>
	 * 
	 * @throws IOException	In the case the SAFMQ server was not able to be contacted
	 * 						via TCP/IP.
	 */
	public MQConnection(Socket s, String user, String password) throws MQException, IOException {
		this.s = s;
		out = new DataOutputStream(new BufferedOutputStream(s.getOutputStream(),s.getSendBufferSize()*4));
		in = new DataInputStream(new BufferedInputStream(s.getInputStream()));
			
		LOGIN_PARAMS	params = new LOGIN_PARAMS();
		paramutil.arrayCopy(params.username, user);
		paramutil.arrayCopy(params.password, password);
	
		out.write(Safmq.CMD_LOGIN);
		params.write(out);
		out.flush();
	
		int err = getResponseCode();
		if (err != Safmq.EC_NOERROR && err != Safmq.EC_NOTIMPLEMENTED) {
			throw new MQException("Login attempt failed error code (" + err+")", err);
		} else if (err == Safmq.EC_NOERROR) {
			LOGIN_RESPONSE resp = new LOGIN_RESPONSE();
			resp.read(in);
			server_major_protocol_version = resp.major_version;
			server_minor_protocol_version = resp.minor_version;
			if (server_major_protocol_version < Safmq.SAFMQ_PROTOCOL_MAJOR_VERSION)
				throw new MQException ("Login attempt failed error code ("+Safmq.EC_UNSUPPORTED_PROTOCOL+")", Safmq.EC_UNSUPPORTED_PROTOCOL);
		}
	}

	/**
	 * Closes the connection to the server.
	 *
	 */
	public void Close() {
		try {
			in.close();
			out.close();
			s.close();
		} catch (Exception e) {
		}
	}
	
	public int GetServerMajorProtocolVersion() {
		return server_major_protocol_version;
	}
		
	public int GetServerMinorProtocolVersion() {
		return server_major_protocol_version;
	}

	
	/**
	 * Opens a queue for reading and writing. The queue is then after
	 * referenced by the parameter <code>handle</code> for calls such as 
	 * <code>Enqueue(QueueHandle,QueueMessage)</code> and 
	 * <code>Retrieve(QueueHandle,boolean,int,QueueMessage)</code>.
	 * 
	 * <p><b>Note</b>: Queues which have been opened by a call to <code>OpenQueue()</code>
	 * must be closed by a call to <code>CloseQueue(QueueHandle)</code> if the queue
	 * is not closed, resources allocated by a call to <code>OpenQueue()</code>
	 * will not be released.</p>
	 * 
	 * @param queuename The name of the queue
	 * @param handle 	Receives a reference to the queue
	 * 
	 * @return <code>Safmq.EC_NOERROR</code> on success otherwise errors such as these, but
	 * not limited to could be produced:
	 * 
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The queue does not exist on the server specified.
	 *				</td></tr>
	 *</table>	 
	 *
	 *  @see #CloseQueue(QueueHandle) CloseQueue(QueueHandle handle)
	 */
	public int OpenQueue(String queuename, QueueHandle handle) {
		int ret;
		QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	params = new QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS();

		paramutil.arrayCopy(params.queuename, queuename);
		try {
			out.write(Safmq.CMD_QUEUE_OPEN);
			params.write(out);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				handle.handle = in.readInt();
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Closes a queue which was previously opened.
	 * 
	 * @param handle Reference to the previously opened queue
	 * @return <code>Safmq.EC_NOERROR</code> on success otherwise errors such as these, but
	 * not limited to could be produced:
	 * 
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTOPEN</code></td>
	 *				<td>The queue was not currently in an open status in the
	 *					context of this connection.
	 *				</td></tr>
	 *</table>
	 */
	public int CloseQueue(QueueHandle handle) {
		int ret = Safmq.EC_NOERROR;
		QUEUE_CLOSE_OPEN_CURSOR_PARAMS	params = new QUEUE_CLOSE_OPEN_CURSOR_PARAMS();
		params.queueID = handle.handle;
		
		try {
			out.write(Safmq.CMD_QUEUE_CLOSE);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Gets statistics about an open queue.  Places statistic information into the parameter <code>stats</code>.
	 * 
	 * @param qhandle Handle to an open queue
	 * @param includeStorageBytes Causes server to calculate the number of bytes on disk
	 * @param includeMessageBytes Causes the server to calculate the number of bytes in queue, may be less than bytes on disk
	 * @param stats Receives the statistics information.

	@return Safmq.EC_NOERROR on success <br>
		Safmq.EC_NETWORKERROR<br>
		Safmq.EC_NOTOPEN <br>
	 */
	public int GetQueueStatistics(QueueHandle qhandle, boolean includeStorageBytes, boolean includeMessageBytes, QueueStatistics stats)
	{
		QUEUE_STATS_PARAMS params = new QUEUE_STATS_PARAMS(qhandle, includeStorageBytes, includeMessageBytes);
		QUEUE_STATS_RESPONSE response = new QUEUE_STATS_RESPONSE();
		response.stats = stats;
		
		try {
			out.write(Safmq.CMD_QUEUE_STATS);
			params.write(out);
			out.flush();
			response.read(in);
		} catch (IOException e) {
			response.errorcode = Safmq.EC_NETWORKERROR;
		}
		
		return response.errorcode;
	}

	/**
	 * Gets statistics for an entire server.
	 * 
	 * @param stats Receives the statistics information.
	 * 	@return Safmq.EC_NOERROR on success <br>
	 * 	Safmq.EC_NETWORKERROR<br>
	 * 	Safmq.EC_NOTOPEN <br>
	 */
	public int GetServerStatistics(QueueStatistics stats)
	{
		QUEUE_STATS_RESPONSE response = new QUEUE_STATS_RESPONSE();
		response.stats = stats;
		
		try {
			out.write(Safmq.CMD_SERVER_STATS);
			out.flush();
			response.read(in);
		} catch (IOException e) {
			response.errorcode = Safmq.EC_NETWORKERROR;
		}
		
		return response.errorcode;
	}
	
	/**
	 * Retrieves information about connect clients.
	 * @param clients Receives a list of ClientInfo objects
	 * @return ErrorCode.EC_NOERROR, ErrorCode.EC_NETWORKERROR
	 */
	int GetClientInfo(Vector clients)
	{
		CLIENT_INFO_RESPONSE resp = new CLIENT_INFO_RESPONSE();
		resp.clientInfo = clients;
		clients.clear();

		try {
			out.write(Safmq.CMD_CLIENT_INFO);
			out.flush();
			resp.read(in);
		} catch (IOException e) {
			resp.errorcode = Safmq.EC_NETWORKERROR;
		}
		return resp.errorcode;
	}

	
	/**
	Enqueues a message for message relay forwarding to another queue/queue server.  After
	completion, the return code will be EC_NOERROR on success and the <code>msg</code>'s 
	time stamp and message will have been set.  It is important to note if the response
	queue name is not set, any errors from the final destination queue/queue server will
	be lost.  It is suggested to use round tripping unless error determination is not
	required, otherwise errors generated by the final destination server such as:<br>
		<table border=0>
		<tr><td>EC_DOESNOTEXIST
		<tr><td>EC_NOTAUTHORIZED
		<tr><td>EC_WRONGMESSAGETYPE
		<tr><td>EC_NOTOPEN
		<tr><td>EC_FORWARDNOTALLOWED
		<tr><td>EC_DUPLICATEMSGID
		</table>
	will not returned to the client.
 

	@param uri A safmq URI in the format safmq://user:password@server:port/queuename -or- for ssl safmqs://user:password@server:port/queuename.<br>
				Note: the port specification is optional.
	@param msg The message to be sent.

	@return Safmq.EC_NOERROR on success <br>
		Safmq.EC_INVALIDNAME in case of an invalid URL <br>
		Safmq.EC_NETWORKERROR<br>
		Safmq.EC_NOTAUTHORIZED <br>
		Safmq.EC_NOTOPEN <br>
		Safmq.EC_WRONGMESSAGETYPE <br>
		Safmq.EC_FORWARDNOTALLOWED
	*/
	public int EnqueueWithRelay(URI uri, QueueMessage msg) {
		int ret;
		try {
			QueueHandle	forward = new QueueHandle();
			String		dest = "";
			String 		userinfo = uri.getUserInfo();
			String		user, password;
			
			user = password = "";

			ret = OpenQueue(FORWARD_QUEUE_NAME,forward);
			if (ret == Safmq.EC_NOERROR) {
				StringBuffer	u, p;
		
				if (MQBuilder.parseUserInfo(userinfo,u=new StringBuffer(),p=new StringBuffer())) {
					user = u.toString();
					password = p.toString();
				}
				
				if (uri.getScheme().length() > 0)
					dest = uri.getScheme() + ":";
				dest += "//";
				
				
				if (user.length() > 0) {
					dest += user;
					if (password.length() > 0)
						dest += ":" + password;
					dest += "@";
				}
				dest += uri.getHost();
				if (uri.getPort() > 0)
					dest += ":" + uri.getPort();
				dest += uri.getPath();

				msg.setLabel(dest+"?label="+msg.getLabel());
				ret = Enqueue(forward, msg);

				int ec = CloseQueue(forward);
				if (ret == Safmq.EC_NOERROR && ec != Safmq.EC_NOERROR)
					ret = ec;
			} else if (ret == Safmq.EC_DOESNOTEXIST) {
				ret = Safmq.EC_FORWARDNOTALLOWED;
			}
		} catch (Exception e) {
			ret = Safmq.EC_INVALIDNAME;
		}
		return ret;
	}
	
	/**
	 * Places a message on the queue.  The object <code>msg</code> must have
	 * been previously prepared before calling this method.  Upon successful sending
	 * of the message, the message id and message time stamp will be set by the server
	 * and may be retrieved by a call to <code>QueueMessage.getMessageID()</code>
	 * and <code>QueueMessage.getTimeStamp()</code> respectively.
	 * 
	 * <p>Note: Message responders will typically place the message id of the original
	 * message in the receipt id of the message being returned.  This round-trip
	 * message identification is provided by SAFMQ as a method for coordinated two-way
	 * communications.</p>
	 * 
	 * @param	handle	A reference to a queue opened by a call to <code>OpenQueue()</code> 
	 * @param	msg		The message to be placed on the queue
	 * @return <code>Safmq.EC_NOERROR</code> on success, Otherwise results may be such listed
	 * below but are not limited to:
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_ALREADYCLOSED</code></td>
	 *				<td>The queue reference is not valid as it has 
	 *					been closed.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The user is not authorized to write messages to this
	 *					queue.
	 *				</td></tr>
	 *</table>
	 *
	 * @see #OpenQueue(String, QueueHandle)
	 */
	public int Enqueue(QueueHandle handle, QueueMessage msg) {
		int ret = Safmq.EC_NETWORKERROR;
		ENQUEUE_PARAMS	params = new ENQUEUE_PARAMS();

		params.queueID = handle.handle;
		params.msg = msg;

		try {
			out.write(Safmq.CMD_ENQUEUE);
			params.write(out);
			out.flush();

			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				ENQUEUE_RESPONSE_DATA	data = new ENQUEUE_RESPONSE_DATA();
				data.msgID = msg.messageID; // set the response data msgID to the msg.messageID so that it can be read back
				data.read(in);
				msg.timeStamp = data.timestamp;
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	

	/**
	 * Retrieves the highest priority FIFO message present on the queue.  The results
	 * are placed in the object <code>msg</code>.
	 * 
	 * <p>Note: Retrieved messages are removed from the queue</p>
	 * 
	 * @param qhandle		A handle to a queue previously opened by a call to OpenQueue(String)
	 * @param retrievebody	A flag indicating whether the body of the message should be retrieved
	 * @param timeoutseconds	The number of seconds to wait before returning, a value of zero (0) will
	 * 							cause the method to return immediately if no messages are present on the queue,
	 * 							a value of (-1) will cause the method to wait until a message is placed on the queue.
	 * @param msg			Receives the contents of the message.
	 * 
	 * @return Upon success a value of <code>Safmq.EC_NOERROR</code> is returned and
	 * 			the resulting message is placed in <code>msg</code>, otherwise 
	 * 			errors such as but not limited to could occur: <br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOMOREMESSAGES</code></td>
	 *				<td>The queue is empty and no more messages are
	 *					available.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_ALREADYCLOSED</code></td>
	 *				<td>The queue reference is not valid as it has 
	 *					been closed.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The user is not authorized to read messages from this
	 *					queue.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTOPEN</code></td>
	 *				<td>The queue specified has not been opened by this connection.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_TIMEDOUT</code></td>
	 *				<td>The operation timed out before a message became available.
	 *				</td></tr>
	 *</table>
	 * @see 	#OpenQueue(String, QueueHandle)
	 */
	public int Retrieve(QueueHandle qhandle, boolean retrievebody, int timeoutseconds, QueueMessage msg) {
		int	ret = Safmq.EC_NOERROR;

		RETRIEVE_PEEK_FRONT_PARAMS	params = new RETRIEVE_PEEK_FRONT_PARAMS();
		params.queueID = qhandle.handle;
		params.retrievebody = (byte)(retrievebody ? 1 : 0);
		params.timeoutseconds = timeoutseconds;

		try {
			out.write(Safmq.CMD_RETRIEVE);
			params.write(out);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				msg.read(in, retrievebody);
				ret = ackRetrieve(qhandle,msg);
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}

	/**
	 * Retrieves the message identified by <code>id</code> in the message's 
	 * receipt id, set prior to the message having been enqueued (See: 
	 * <code>Enqueue(QueueMessage)</code>. The results  are placed in the object
	 * <code>msg</code>.  
	 * 
	 * <p>Note: Message responders will typically place the message id of the original
	 * message in the receipt id of the message being returned.  This round-trip
	 * message identification is provided by SAFMQ as a method for coordinated two-way
	 * communications.</p>
	 * 
	 * <p>Note: Retrieved messages are removed from the queue.</p>
	 * 
	 * @param qhandle		A handle to a queue previously opened by a call to OpenQueue(String)
	 * @param retrievebody	A flag indicating whether the body of the message should be retrieved
	 * @param id			The UUID of the message to be retrieved.
	 * @param timeoutseconds		The number of seconds to wait before returning, a value of zero (0) will
	 * 						cause the method to return immediately if no messages are present on the queue,
	 * 						a value of (-1) will cause the method to wait until a message is placed on the queue.
	 * @param msg			Receives the contents of the message.
	 * 
	 * @return	Upon success a value of <code>Safmq.EC_NOERROR</code>, otherwise 
	 * 			errors such as but not limited to could occur: <br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOMOREMESSAGES</code></td>
	 *				<td>The queue is empty and no more messages are
	 *					available.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_ALREADYCLOSED</code></td>
	 *				<td>The queue reference is not valid as it has 
	 *					been closed.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The user is not authorized to read messages from this
	 *					queue.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTOPEN</code></td>
	 *				<td>The queue specified has not been opened by this connection.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_TIMEDOUT</code></td>
	 *				<td>The operation timed out before a message became available.
	 *				</td></tr>
	 *</table>
	 * @see #Enqueue(QueueHandle, QueueMessage) Enqueue(QueueMessage)
	 * @see #OpenQueue(String, QueueHandle)
	 */
	public int RetrieveID(QueueHandle qhandle, boolean retrievebody, UUID id, int timeoutseconds, QueueMessage msg) {
		int	ret = Safmq.EC_NOERROR;

		RETRIEVE_ID_PEEK_ID_PARAMS	params = new RETRIEVE_ID_PEEK_ID_PARAMS();
		params.queueID = qhandle.handle;
		params.retrievebody = (byte)(retrievebody?1:0);
		params.reciptID = id;
		params.timeoutseconds = timeoutseconds;
		
		try {
			out.write(Safmq.CMD_RETRIEVE_ID);
			params.write(out);out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				msg.read(in,retrievebody);
				ret = ackRetrieveCursor(qhandle,msg);
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}

	/**
	 * Retrieves the message pointed to by <code>cursorID</code>. The results  
	 * are placed in the object <code>msg</code>.
	 * 
	 * <p>Note: Retrieved messages are removed from the queue.</p>
	 * 
	 * @param qhandle		A handle to a queue previously opened by a call to OpenQueue(String)
	 * @param retrievebody	A flag indicating whether the body of the message should be retrieved
	 * @param cursorID		The cursor indicating the current position in the queue to be read from
	 * @param msg			Receives the contents of the message.
	 * 
	 * @return	Upon success a value of <code>Safmq.EC_NOERROR</code>, otherwise 
	 * 			errors such as but not limited to could occur: <br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOMOREMESSAGES</code></td>
	 *				<td>The queue is empty and no more messages are
	 *					available.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_ALREADYCLOSED</code></td>
	 *				<td>The queue reference is not valid as it has 
	 *					been closed.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The user is not authorized to read messages from this
	 *					queue.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_CURSORINVALIDATED</code></td>
	 *				<td>The cursor no longer points to a valid location in the
	 *					queue.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTOPEN</code></td>
	 *				<td>The queue specified has not been opened by this connection.
	 *				</td></tr>
	 *</table>
	 * @see #OpenQueue(String, QueueHandle)
	 */
	public int RetrieveCursor(QueueHandle qhandle, boolean retrievebody, CursorHandle cursorID, QueueMessage msg) {
		int	ret = Safmq.EC_NOERROR;

		PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS	params = new PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS();
		params.queueID = qhandle.handle;
		params.retrievebody = (byte)(retrievebody?1:0);
		params.cursorID = cursorID.handle;

		try {
			out.write(Safmq.CMD_RETRIEVE_CURSOR);
			params.write(out);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				msg.read(in,retrievebody);
				ret = ackRetrieveCursor(qhandle,msg);
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}

	/**
	 * Gathers the highest priority FIFO message present on the queue.  The results
	 * are placed in the object <code>msg</code>.  Any errors from the operation are
	 * returned on the stack.    The message retrieved is <i>not</i> removed from the 
	 * queue and is available for reading by other queue readers.
	 * 
	 * @param qhandle		A handle to a queue previously opened by a call to OpenQueue(String)
	 * @param retrievebody	A flag indicating whether the body of the message should be retrieved
	 * @param timeoutseconds	The number of seconds to wait before returning, a value of zero (0) will
	 * 						cause the method to return immediately if no messages are present on the queue,
	 * 						a value of (-1) will cause the method to wait until a message is placed on the queue.
	 * @param msg			Receives the contents of the message.
	 * 
	 * @return	Upon success a value of <code>Safmq.EC_NOERROR</code>, otherwise 
	 * 			errors such as but not limited to could occur: <br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOMOREMESSAGES</code></td>
	 *				<td>The queue is empty and no more messages are
	 *					available.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_ALREADYCLOSED</code></td>
	 *				<td>The queue reference is not valid as it has 
	 *					been closed.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The user is not authorized to read messages from this
	 *					queue.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTOPEN</code></td>
	 *				<td>The queue specified has not been opened by this connection.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_TIMEDOUT</code></td>
	 *				<td>The operation timed out before a message became available.
	 *				</td></tr>
	 *</table>
	 * @see 	#OpenQueue(String, QueueHandle)
	 */
	public int PeekFront(QueueHandle qhandle, boolean retrievebody, int timeoutseconds, QueueMessage msg) {
		int	ret = Safmq.EC_NOERROR;

		RETRIEVE_PEEK_FRONT_PARAMS	params = new RETRIEVE_PEEK_FRONT_PARAMS();
		params.queueID = qhandle.handle;
		params.retrievebody = (byte)(retrievebody?1:0);
		params.timeoutseconds = timeoutseconds;

		try {
			out.write(Safmq.CMD_PEEK_FRONT);
			params.write(out);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR)
				msg.read(in,retrievebody);
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}

	/**
	 * Gathers the message identified by <code>id</code> in the message's 
	 * receipt id, set prior to the message having been enqueued (See: 
	 * <code>Enqueue(QueueMessage)</code>. The results  are placed in the object
	 * <code>msg</code>.  The message retrieved is <i>not</i> removed from the 
	 * queue and is available for reading by other queue readers.
	 * 
	 * <p>Note: Message responders will typically place the message id of the original
	 * message in the receipt id of the message being returned.  This round-trip
	 * message identification is provided by SAFMQ as a method for coordinated two-way
	 * communications.</p>
	 * 
	 * @param qhandle		A handle to a queue previously opened by a call to OpenQueue(String)
	 * @param retrievebody	A flag indicating whether the body of the message should be retrieved
	 * @param id			The UUID of the message to be retrieved.
	 * @param timeoutseconds	The number of seconds to wait before returning, a value of zero (0) will
	 * 						cause the method to return immediately if no messages are present on the queue,
	 * 						a value of (-1) will cause the method to wait until a message is placed on the queue.
	 * @param msg			Receives the contents of the message.
	 * 
	 * @return	Upon success a value of <code>Safmq.EC_NOERROR</code>, otherwise 
	 * 			errors such as but not limited to could occur: <br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOMOREMESSAGES</code></td>
	 *				<td>The queue is empty and no more messages are
	 *					available.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_ALREADYCLOSED</code></td>
	 *				<td>The queue reference is not valid as it has 
	 *					been closed.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The user is not authorized to read messages from this
	 *					queue.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTOPEN</code></td>
	 *				<td>The queue specified has not been opened by this connection.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_TIMEDOUT</code></td>
	 *				<td>The operation timed out before a message became available.
	 *				</td></tr>
	 *</table>
	 * @see #Enqueue(QueueHandle, QueueMessage) Enqueue(QueueMessage)
	 * @see #OpenQueue(String, QueueHandle)
	 */
	public int PeekID(QueueHandle qhandle, boolean retrievebody, UUID id, int timeoutseconds, QueueMessage msg) {
		int	ret = Safmq.EC_NOERROR;

		RETRIEVE_ID_PEEK_ID_PARAMS	params = new RETRIEVE_ID_PEEK_ID_PARAMS();
		params.queueID = qhandle.handle;
		params.retrievebody = (byte)(retrievebody?1:0);
		params.reciptID = id;
		params.timeoutseconds = timeoutseconds;
		
		try {
			out.write(Safmq.CMD_PEEK_ID);
			params.write(out);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR)
				msg.read(in,retrievebody);
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}

	/**
	 * Retrieves the message pointed to by <code>cursorID</code>. The results  
	 * are placed in the object <code>msg</code>.  The message retrieved is 
	 * <i>not</i> removed from the queue and is available for reading by other 
	 * queue readers.
	 * 
	 * <p>Note: Retrieved messages are removed from the queue.</p>
	 * 
	 * @param qhandle		A handle to a queue previously opened by a call to OpenQueue(String)
	 * @param retrievebody	A flag indicating whether the body of the message should be retrieved
	 * @param cursorID		The cursor indicating the current position in the queue to be read from
	 * @param msg			Receives the contents of the message.
	 * 
	 * @return	Upon success a value of <code>Safmq.EC_NOERROR</code>, otherwise 
	 * 			errors such as but not limited to could occur: <br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOMOREMESSAGES</code></td>
	 *				<td>The queue is empty and no more messages are
	 *					available.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_ALREADYCLOSED</code></td>
	 *				<td>The queue reference is not valid as it has 
	 *					been closed.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The user is not authorized to read messages from this
	 *					queue.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTOPEN</code></td>
	 *				<td>The queue specified has not been opened by this connection.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_CURSORINVALIDATED</code></td>
	 *				<td>The cursor no longer points to a valid location in the
	 *					queue.
	 *				</td></tr>
	 *</table>
	 * @see #OpenQueue(String, QueueHandle)
	 */
	public int PeekCursor(QueueHandle qhandle, boolean retrievebody, CursorHandle cursorID, QueueMessage msg) {
		int	ret = Safmq.EC_NOERROR;

		PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS	params = new PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS();
		params.queueID = qhandle.handle;
		params.retrievebody = (byte)(retrievebody?1:0);
		params.cursorID = cursorID.handle;

		try {
			out.write(Safmq.CMD_PEEK_CURSOR);
			params.write(out);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				msg.read(in,retrievebody);
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Opens a cursor for sequential reading of the queue.  The newly opened
	 * cursor points to the oldest, highest priority message in the queue.
	 * 
	 * <p>Cursors may be used to violate the priority FIFO nature of the queue
	 * by scanning the queue and only retreiving messages which meet the 
	 * criteria of the reader.  Cursor access is sequential when used in conjunction
	 * with <code>AdvanceCursor(CursorHandle)</code>, however specific messages may
	 * be sought out by a call to <code>SeekID(UUID,int,CursorHandle)</code>.  </p>
	 * 
	 * @param qhandle		A handle to a queue previously opened by a call to OpenQueue(String)
	 * @param cursorID 	A previously allocated reference to a <code>CursorHandle</code>
	 * 					which will receive the reference to the cursor.
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The user is not authorized to read messages from this
	 *					queue.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTOPEN</code></td>
	 *				<td>The queue specified has not been opened by this connection.
	 *				</td></tr>
	 *</table>
	 *
	 *  @see #OpenQueue(String, QueueHandle)
	 */
	public int OpenCursor(QueueHandle qhandle, CursorHandle cursorID) {
		int ret = Safmq.EC_NOERROR;
		QUEUE_CLOSE_OPEN_CURSOR_PARAMS	params = new QUEUE_CLOSE_OPEN_CURSOR_PARAMS();
		params.queueID = qhandle.handle;
		
		try {
			out.write(Safmq.CMD_OPEN_CURSOR);
			params.write(out);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				cursorID.handle = in.readInt();
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}

		return ret;
	}
	
	/**
	 * Closes a cursor when it is no longer needed to access the queue.  The
	 * cursor is invalidated after a call to <code>CloseCursor()</code> and
	 * should not be used again.
	 * 
	 * @param qhandle		A handle to a queue previously opened by a call to OpenQueue(String)
	 * @param cursorID The cursor to be closed.
	 * 
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The user is not authorized to read messages from this
	 *					queue.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTOPEN</code></td>
	 *				<td>The queue specified has not been opened by this connection.
	 *				</td></tr>
	 *</table>
	 *
	 *  @see #OpenQueue(String, QueueHandle)
	 */
	public int CloseCursor(QueueHandle qhandle, CursorHandle cursorID) {
		int ret = Safmq.EC_NOERROR;
		CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS	params = new CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS();
		params.queueID = qhandle.handle;
		params.cursorID = cursorID.handle;

		try {
			out.write(Safmq.CMD_CLOSE_CURSOR);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Advances the cursor to the next message in the queue.
	 * 
	 * @param qhandle		A handle to a queue previously opened by a call to OpenQueue(String)
	 * @param cursorID The cursor to be advanced.
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The user is not authorized to read messages from this
	 *					queue.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTOPEN</code></td>
	 *				<td>The queue specified has not been opened by this connection.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_CURSORINVALIDATED</code></td>
	 *				<td>The cursor is no longer valid.
	 *				</td></tr>
	 *</table>
	 *
	 *  @see #OpenQueue(String, QueueHandle)
	 */
	public int AdvanceCursor(QueueHandle qhandle, CursorHandle cursorID) {
		int ret = Safmq.EC_NOERROR;
		CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS	params = new CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS();
		params.queueID = qhandle.handle;
		params.cursorID = cursorID.handle ;

		try {
			out.write(Safmq.CMD_ADVANCE_CURSOR);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}

		return ret;
	}

	/**
	 * Attempts to locate the messaged indicated by <code>id</code>.  If the message
	 * is succesfully located, the cursor will point to the message idenfified
	 * and the message may be retrieved there after.
	 * 
	 * @param qhandle	A handle to a queue previously opened by a call to OpenQueue(String)
	 * @param id		The receipt UUID of the message to be located 
	 * @param timeoutseconds	The maximum number of seconds the call should be allowed
	 * 					before resulting in a "timeout".
	 * @param cursorID	The cursor which should be be assigned the position
	 * 					of the message indicated by <code>id</code>
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The user is not authorized to read messages from this
	 *					queue.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTOPEN</code></td>
	 *				<td>The queue specified has not been opened by this connection.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_CURSORINVALIDATED</code></td>
	 *				<td>The cursor is no longer valid.
	 *				</td></tr>
	 *</table>
	 *
	 *  @see #OpenQueue(String, QueueHandle)
	 */
	public int SeekID(QueueHandle qhandle, UUID id, int timeoutseconds, CursorHandle cursorID) {
		int ret = Safmq.EC_NOERROR;
		SEEK_ID_PARAMS	params = new SEEK_ID_PARAMS();
		params.queueID = qhandle.handle;
		params.cursorID = cursorID.handle;
		params.reciptID = id;
		params.timeoutseconds = timeoutseconds;

		try {
			out.write(Safmq.CMD_SEEK_ID);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}

	/**
	 * Determines whether the cursor indicated by <code>cursorID</code> is still
	 * a valid cursor, and has not been invalidated by other queue readers.
	 * 
	 * @param cursorID	The cursor which should be be evaluated
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTOPEN</code></td>
	 *				<td>The queue specified has not been opened by this connection.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_CURSORINVALIDATED</code></td>
	 *				<td>The cursor is no longer valid.
	 *				</td></tr>
	 *</table>
	 *
	 *  @see #OpenQueue(String, QueueHandle)
	 */
	public int TestCursor(QueueHandle qhandle, CursorHandle cursorID) {
		int ret = Safmq.EC_NOERROR;
		CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS	params = new CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS();
		params.queueID = qhandle.handle;
		params.cursorID = cursorID.handle;

		try {
			out.write(Safmq.CMD_TEST_CURSOR);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}

		return ret;
	}
	
	/**
	 * Begins a transaction which causes all Retrieve & Enqueue operations to be committed
	 * atomically.
	 * 
	 * @return <code>Safmq.EC_NOERROR</code>, <code>Safmq.EC_NETWORKERROR</code>, 
	 * 			<code>Safmq.EC_ALREADYOPEN</code> if a transaction is already open  
	 */
	public int BeginTransaction() {
		try {
			out.write(Safmq.CMD_BEGIN_TRANSACTION);
			out.writeInt(0);
			out.flush();
			return getResponseCode();
		} catch (IOException e) {
			return Safmq.EC_NETWORKERROR;
		}
	}
	
	/**
	 * Commits a group of Retrieve & Enqueue operations atomically.
	 * 
	 * @return <code>Safmq.EC_NOERROR</code>, <code>Safmq.EC_NETWORKERROR</code> 
	 */
	public int CommitTransaction() {
		try {
			out.write(Safmq.CMD_COMMIT_TRANSACTION);
			out.writeInt(0);
			out.flush();
			return getResponseCode();
		} catch (IOException e) {
			return Safmq.EC_NETWORKERROR;
		}
	}
	
	/**
	 * Rolls back or "undoes" a group of Retrieve & Enqueue operations atomically.
	 * 
	 * @return <code>Safmq.EC_NOERROR</code>, <code>Safmq.EC_NETWORKERROR</code> 
	 */
	public int RollbackTransaction() {
		try {
			out.write(Safmq.CMD_ROLLBACK_TRANSACTION);
			out.writeInt(0);
			out.flush();
			return getResponseCode();
		} catch (IOException e) {
			return Safmq.EC_NETWORKERROR;
		}
	}
	
	/**
	 * Commits and closes a group of Retrieve & Enqueue operations.
	 * 
	 * @return <code>Safmq.EC_NOERROR</code>, <code>Safmq.EC_NETWORKERROR</code>,
	 * 	<code>Safmq.EC_NOTOPEN</code> in the case that a transaction was not open.
	 */
	public int EndTransaction() {
		try {
			out.write(Safmq.CMD_END_TRANSACTION);
			out.writeInt(0);
			out.flush();
			return getResponseCode();
		} catch (IOException e) {
			return Safmq.EC_NETWORKERROR;
		}
	}
	
	
	
	////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Retrieves a list of all queues served by the connection and the queue's owner.
	 * The passed <code>Vector qnames</code> receives <Code>MQConnection.QueueData</code>
	 * objects to enumerate the queue.
	 * 
	 * @param qnames The vector which will be emptied then populated with <code>MQconnection.QueueData</code> objects.
	 * 
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *</table>
	 *
	 *  @see MQConnection.QueueData
	 */
	public int EnumerateQueues(Vector qnames) {
		int	ret = Safmq.EC_NETWORKERROR;
		qnames.clear();
		try {
			out.write(Safmq.CMD_ENUM_QUEUES);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				int	nqueues = in.readInt();
				QUEUE_CONFIG_ENTRY	qent = new QUEUE_CONFIG_ENTRY();
				for(int x=0; x < nqueues; x++) {
					qent.read(in);
					qnames.add(new QueueData(new String(qent.queuename,0,zlength(qent.queuename)),new String(qent.owner,0,zlength(qent.owner))));
				}
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}

	/**
	 * Retrieves a list of users which may access the SAFMQ server.  The result
	 * user information is in the form of <code>MQConnection.UserDescription</code>
	 * objects placed in the passed <code>Vector, users</code>.
	 * 
	 * @param users The vector which will be emptied then populated with <code>MQConnection.UserDescription</code> objects
	 * 
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *</table>
	 *
	 *  @see MQConnection.UserDescription
	 */
	public int EnumerateUsers(Vector users) {
		int	ret = Safmq.EC_NETWORKERROR;
		
		users.clear();
		try {
			out.write(Safmq.CMD_ENUM_USERS);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				int		nUsers = in.readInt();
				byte	username[] = new byte[Safmq.USER_NAME_LENGTH];
				byte	description[] = new byte[Safmq.DESCRIPTION_LENGTH];

				for(int x=0;x < nUsers; x++) {
					in.readFully(username);
					in.readFully(description);
					users.add(new UserDescription(new String(username,0,zlength(username)),new String(description,0,zlength(description))));
				}
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		
		return ret;
	}

	/**
	 * Retrieves a list of all groups available on the SAFMQ server.  The result
	 * group information is in the form of <code>String</code> objects representing
	 * the name of the group.
	 * 
	 * @param groups The vector which will be emptied then populated with
	 * 					<code>String</code> objects containing the names of the
	 * 					groups.
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *</table>
	 */
	public int EnumerateGroups(Vector groups) {
		int	ret = Safmq.EC_NETWORKERROR;
		
		groups.clear();
		try {
			out.write(Safmq.CMD_ENUM_GROUPS);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				int	nGroups = in.readInt();
				byte	groupname[] = new byte[Safmq.GROUP_NAME_LENGTH];
				for(int x=0; x < nGroups; x++) {
					in.readFully(groupname);
					groups.add(new String(groupname,0,zlength(groupname)));
				} 
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}

		return ret;
	}

	/**
	 * Create a new user by providing a user name, password and description.
	 * 
	 * @param username		The name of the new user
	 * @param password		The password for the new user, cannot be null
	 * @param description	The description for the new user.
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_ALREADYDEFINED</code></td>
	 *				<td>A user with this name already exists
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to create users.
	 *				</td></tr>
	 *</table>
	 * 
	 */
	public int CreateUser(String username, String password, String description) {
		USER_CREATE_PARAMS	params = new USER_CREATE_PARAMS();
		int					ret = Safmq.EC_NETWORKERROR;

		paramutil.arrayCopy(params.username,username);
		paramutil.arrayCopy(params.password,password);
		paramutil.arrayCopy(params.description,description);

		try {
			out.write(Safmq.CMD_USER_CREATE);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}

		return ret;
	}
	
	/**
	 * Delete a user from the SAFMQ server.
	 * @param username The name of the suer to delete.
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to create users.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The user specified does not exist.
	 *				</td></tr>
	 *</table>
	 */
	public int DeleteUser(String username) {
		USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS	params = new USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS();
		int													ret = Safmq.EC_NETWORKERROR;

		paramutil.arrayCopy(params.username,username);
		try {
			out.write(Safmq.CMD_USER_DELETE);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Change the user's permissions to those specified.
	 * 
	 * @param username		Name of the user to change
	 * @param modifyqueues	Allow the user to create queues
	 * @param modifyusers	Allow the user to create and delete users and set permmsions
	 * @param modifygroups	Allow the user to create, delete, and change group membership and permissions
	 * 
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to modify user permissions.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The specified user does not exist on the SAFMQ server.
	 *				</td></tr>
	 *</table>
	 */
	public int UserSetPermissions(String username, boolean modifyqueues, boolean modifyusers, boolean modifygroups) {
		USER_SET_PERMS_PARAMS	params = new USER_SET_PERMS_PARAMS();
		int						ret = Safmq.EC_NETWORKERROR;
		
		paramutil.arrayCopy(params.username,username);
		params.modify_queues = (byte)(modifyqueues?1:0);
		params.modify_users = (byte)(modifyusers?1:0);
		params.modify_groups = (byte)(modifygroups?1:0);
		try {
			out.write(Safmq.CMD_USER_SET_PERMS);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Retrieve the permissions for a specific user.
	 * 
	 * @param username	The name of the user
	 * @param actorPerms	Receives the permissions for the user, must not be null.
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The specified user does not exist on the SAFMQ server, or the
	 *					user has no permissions set.
	 *				</td></tr>
	 *</table>
	 */
	public int UserGetPermissions(String username, ActorPermissions actorPerms) {
		USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS	params =  new USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS();
		int													ret = Safmq.EC_NETWORKERROR;

		paramutil.arrayCopy(params.username,username);
		try {
			out.write(Safmq.CMD_USER_GET_PERMS);
			params.write(out);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data	perms = new USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data();
				perms.read(in);
				actorPerms.setModifyqueues(perms.modify_queues != 0);
				actorPerms.setModifyusers(perms.modify_users != 0);
				actorPerms.setModifygroups(perms.modify_groups != 0);
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Changes the password for the specified user.
	 * 
	 * @param username	Name of the user
	 * @param password	New password for the user.
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to create users.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The specified user does not exist on the SAFMQ server.
	 *				</td></tr>
	 *</table>
	 */
	public int SetPassword(String username, String password) {
		int							ret = Safmq.EC_NETWORKERROR;
		USER_SET_PASSWORD_PARAMS	params = new USER_SET_PASSWORD_PARAMS();

		paramutil.arrayCopy(params.username, username);
		paramutil.arrayCopy(params.password, password);
		try {
			out.write(Safmq.CMD_USER_SET_PASSWORD);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}

	/**
	Adds an X509 digitital certificate identity mapping.  This method should be used
	to provide for a passwordless authentication using SSL and X509 digital certificates.

	<p><b>Example:</b> (the sample X509 certificate shipped with SAFMQ source)</p>
	<pre>
	Subject: C=US, ST=Nebraska, L=Omaha, O=SAFMQ, CN=Matthew Battey/emailAddress=mattbattey@users.sourceforge.net
	Issuer:  C=US, ST=Nebraska, L=Omaha, O=SAFMQ, CN=Matthew Battey/emailAddress=mattbattey@users.sourceforge.net
	</pre>

	Thawte's Issuer DN:
	<pre>
	Issuer: C=ZA, ST=Western Cape, L=Cape Town, O=Thawte Consulting cc, OU=Certification Services Division, CN=Thawte Premium Server CA/emailAddress=premium-server@thawte.com
	</pre>

	<p> In this case the issuer and subject are the same value, because the certificate
	shipped with SAFMQ is a "self-signed" certificate.  Typically the Subject DN will be set
	by you and the Issuer DN will be determined by the organization certifying the authenticity
	of the certificate's origin.</p>

	<p><b>Note</b>: The Java implentation produces distinguished name strings in reverse
	order in comparison to the default output from tools like OpenSSL.  SAFMQ uses OpenSSL
	so these must be in the order that OpenSSL would report the value pair order.</p>

	@param subjectDN [in] the Subject Distinguished Name of the certificate
	@param issuerDN [in] the Issuer Distinguished Name of the certificate
	@param username [in] the target SAFMQ login id (username)
	@return EC_NOERROR upon success, EC_DOESNOTEXIST if the user ID does not exist, EC_NOTAUTHORIZED, EC_NETWORKERROR
	*/
	public int AddUserIdentity(String subjectDN, String issuerDN, String username)
	{
		int	ret = Safmq.EC_NETWORKERROR;
		ADD_USER_IDENTITY_PARAMS	params = new ADD_USER_IDENTITY_PARAMS(subjectDN, issuerDN, username);
		
		try {
			out.write(Safmq.CMD_USER_ADD_IDENT);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}

		return ret;
	}

	/**
	Removes an X509 digitital certificate identity mapping.  This method should be used
	to provide for a passwordless authentication using SSL and X509 digital certificates.

	@param subjectDN [in] the Subject Distinguished Name of the certificate
	@param issuerDN [in] the Issuer Distinguished Name of the certificate
	@return EC_NOERROR upon success, EC_DOESNOTEXIST if the user ID does not exist, EC_NOTAUTHORIZED, EC_NETWORKERROR
	*/
	public int RemoveUerIdentity(String subjectDN, String issuerDN)
	{
		int	ret = Safmq.EC_NETWORKERROR;
		REMOVE_USER_IDENTITY	params = new REMOVE_USER_IDENTITY(subjectDN, issuerDN);
		
		try {
			out.write(Safmq.CMD_USER_DEL_IDENT);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}

		return ret;
	}

	/**
	Lists a set of X509 subject name and issuer names associated with the passed SAFMQ user login id.

	@param username [in] The user name used to search for X509 identities.
	@param ids [out] This vector of safmq::X509Identity objects is populated upon successful return
	@return EC_NOERROR on success, EC_NETWORKERROR on network issue
	*/
	public int EnumerateUserIdentities(String username, Vector ids)
	{
		int	ret = Safmq.EC_NETWORKERROR;
		ENUM_USER_IDENTITY	params = new ENUM_USER_IDENTITY(username);

		ids.clear();
		
		try {
			out.write(Safmq.CMD_USER_ENUM_IDENT);
			params.write(out);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				int	count = in.readInt();
				byte subjectDN[];
				byte issuerDN[];
				for( ; count > 0; --count) {
					subjectDN = new byte[in.readInt()];
					in.readFully(subjectDN);
					issuerDN = new byte[in.readInt()];
					in.readFully(issuerDN);
					ids.add(new X509Identity(new String(subjectDN), new String(issuerDN)));
				}
			}
		}  catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}

		return ret;
	}

	
	
	
	/**
	 * Creates a new security group on the SAFMQ server.
	 * 
	 * @param groupname Name of the group to create.
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_ALREADYDEFINED</code></td>
	 *				<td>A group with this name already exists
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to create groups.
	 *				</td></tr>
	 *</table>
	 */
	public int CreateGroup(String groupname) {
		GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	params = new GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS();
		int																	ret = Safmq.EC_NETWORKERROR;

		paramutil.arrayCopy(params.groupname,groupname);
		try {
			out.write(Safmq.CMD_GROUP_CREATE);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Removes the security group from the SAFMQ server.
	 * @param groupname The name of the group
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to create groups.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The group specified does not exist.
	 *				</td></tr>
	 *</table>
	 */
	public int DeleteGroup(String groupname) {
		GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	params = new GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS();
		int																	ret = Safmq.EC_NETWORKERROR;

		paramutil.arrayCopy(params.groupname,groupname);
		try {
			out.write(Safmq.CMD_GROUP_DELETE);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Changes the permissions for the specified group.
	 * 
	 * @param groupname		The name of the group
	 * @param modifyqueues	Whether the group may create queues
	 * @param modifyusers	Whether the group may create and delete users and set permissions
	 * @param modifygroups	Whether the gorup may create, delete and change group membership and permissions
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to modify group permissions.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The specified group does not exist on the SAFMQ server.
	 *				</td></tr>
	 *</table>
	 */
	public int GroupSetPermissions(String groupname, boolean modifyqueues, boolean modifyusers, boolean modifygroups) {
		GROUP_SET_PERMS_PARAMS	params = new GROUP_SET_PERMS_PARAMS();
		int						ret = Safmq.EC_NETWORKERROR;

		paramutil.arrayCopy(params.groupname,groupname);
		params.modify_queues = (byte)(modifyqueues?1:0);
		params.modify_users = (byte)(modifyusers?1:0);
		params.modify_groups = (byte)(modifygroups?1:0);
		try {
			out.write(Safmq.CMD_GROUP_SET_PERMS);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Retrieves the permissions of a specific group.
	 * 
	 * @param groupname The name of the group
	 * @param actorPerms	Receives the permissions, must not be null
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The specified group does not exist on the SAFMQ server, or no
	 *					permissions have been set for that group.
	 *				</td></tr>
	 *</table>
	 */
	public int GroupGetPermissions(String groupname, ActorPermissions actorPerms) {
		GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	params = new GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS();
		int																	ret = Safmq.EC_NETWORKERROR;

		paramutil.arrayCopy(params.groupname,groupname);
		try {
			out.write(Safmq.CMD_GROUP_GET_PERMS);
			params.write(out);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data	perms = new USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data();
				perms.read(in);
				actorPerms.setModifyqueues(perms.modify_queues != 0);
				actorPerms.setModifyusers(perms.modify_users != 0);
				actorPerms.setModifygroups(perms.modify_groups != 0);
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Adds a user to a group.
	 * 
	 * @param groupname	The name of the group
	 * @param username	The name of the user
	 * 
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to modify group permissions.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The specified group does not exist on the SAFMQ server.
	 *				</td></tr>
	 *</table>
	 */
	public int GroupAddUser(String groupname, String username) {
		GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS	params = new GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS();
		int										ret = Safmq.EC_NETWORKERROR;

		paramutil.arrayCopy(params.groupname,groupname);
		paramutil.arrayCopy(params.username,username);
		
		try {
			out.write(Safmq.CMD_GROUP_ADD_USER);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Deletes a user from a group.
	 * 
	 * @param groupname	The name of the group
	 * @param username	The name of the user
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to modify group permissions.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The specified group does not exist on the SAFMQ server.
	 *				</td></tr>
	 *</table>
	 */
	public int GroupDeleteUser(String groupname, String username) {
		GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS	params = new GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS();
		int										ret = Safmq.EC_NETWORKERROR;

		paramutil.arrayCopy(params.groupname,groupname);
		paramutil.arrayCopy(params.username,username);

		try {
			out.write(Safmq.CMD_GROUP_DELETE_USER);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Retrieves a the list of users in a group.  The names of the users are
	 * deposited into the <code>Vector users</code> in the form of <code>String</code>
	 * objects.
	 * 
	 * @param groupname	The name of the users.
	 * @param users		A vector receiving <code>String</code> objects containing
	 * 					the user's name.
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The specified group does not exist on the SAFMQ server.
	 *				</td></tr>
	 *</table>
	 */
	public int GroupGetUsers(String groupname, Vector users) {
		GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	params = new GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS();
		int																	ret = Safmq.EC_NETWORKERROR;

		paramutil.arrayCopy(params.groupname,groupname);
		users.clear();

		try {
			out.write(Safmq.CMD_GROUP_GET_USERS);
			params.write(out);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				int	nUsers;
				int	x;
				byte	username[] = new byte[Safmq.USER_NAME_LENGTH];

				nUsers = in.readInt();
				for(x=0;x<nUsers;x++) {
					in.readFully(username);
					users.add(new String(username,0,zlength(username)));
				}
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}

	/**
	 * Retrieves a the list of groups the in which the user resides.  The names of the groups are
	 * deposited into the <code>Vector users</code> in the form of <code>String</code>
	 * objects.
	 * 
	 * @param username	The name of the users.
	 * @param groups	A vector receiving <code>String</code> objects containing
	 * 					the group's name.
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The specified user does not exist on the SAFMQ server.
	 *				</td></tr>
	 *</table>
	 */
	public int UserGetGroups(String username, Vector groups) {
		USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS	params = new USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS();
		int													ret = Safmq.EC_NETWORKERROR;

		paramutil.arrayCopy(params.username, username);
		groups.clear();

		try {
			out.write(Safmq.CMD_USER_GET_GROUPS);
			params.write(out);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				int	nGroups;
				int	x;
				byte	groupname[] = new byte[Safmq.GROUP_NAME_LENGTH];

				nGroups = in.readInt();
				for(x=0;x<nGroups; x++) {
					in.readFully(groupname);
					groups.add(new String(groupname,0,zlength(groupname)));
				}
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}

	/**
	 * Creates a new queue on the SAFMQ server.  The queue's owner is automatically
	 * set to the current logged in user.
	 * 
	 * @param queuename The name of the new queue to be created.
	 * 
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_ALREADYDEFINED</code></td>
	 *				<td>A queue with this name already exists
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to create queues.
	 *				</td></tr>
	 *</table>
	 */
	public int CreateQueue(String queuename) {
		int ret = Safmq.EC_NETWORKERROR;
		QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	params = new QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS();

		paramutil.arrayCopy(params.queuename,queuename);

		try {
			out.write(Safmq.CMD_QUEUE_CREATE);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Creates a new temporary message queue.  The queue is removed from the server
	 * once all clients close their handles to the queue, explicitly or implicity
	 * by disconnecting from the server.<br>
	 * 
	 * After calling <code>CreateTempQueue</code> the SAFMQ server has created a queue,
	 * flagged as temporary, returns the server generated name of the queue, and a handle
	 * to that queue.  Closing the queue removes it from the system, there is no need to 
	 * explicitly delete it.  Using the Closing technique, it is possible that the queue
	 * may remain open, because other clients have access to the queue.
	 * 
	 * @param tmpQueueName [out] Receives the name of the temporary queue, relative to the server,
	 * 							when supplying this queue name to other clients, the server URL
	 * 							must be prepended to the name.  i.e. If the url
	 * 							safmq://localhost was used to open the connection use
	 * 							<code>"safmq://localhost/" + tmpQueue</code> when supplying
	 * 							the queue name to other clients. Must be an array of atleast
	 * 							one element, first element will receive the name.
	 * 
	 * @param tmpQueue [out] Receives a handle to the open queue
	 * 
	 * @return EC_NOERROR, EC_NETWORKERROR, EC_ALREADYDEFINED, EC_NOTAUTHORIZED
	*/
	public int CreateTempQueue(String[] tmpQueueName, QueueHandle tmpQueue) {
		QUEUE_CREATE_TEMP_RESPONSE resp = new QUEUE_CREATE_TEMP_RESPONSE();

		resp.errorcode = Safmq.EC_NETWORKERROR; // flag as a network error because the first failure will be writing to the server
		try {
			out.write(Safmq.CMD_QUEUE_CREATE_TEMP);
			out.flush();
			resp.read(in);
			if (resp.errorcode == Safmq.EC_NOERROR) {
				tmpQueueName[0] = resp.queueName;
				tmpQueue.handle = resp.queueID;
			}
		} catch (IOException e) {
			resp.errorcode = Safmq.EC_NETWORKERROR;
		}
		return resp.errorcode;
	}
	
	/**
	 * Attempts to remove a queue fromt he SAFMQ server.  Queues may only be removed
	 * if the user has the rights to do so and the queue is not currently in use.
	 * 
	 * @param queuename The name of the queue to be removed.
	 * 
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_QUEUEOPEN</code></td>
	 *				<td>The queue is currently in use by some connection.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to delete this queue.
	 *				</td></tr>
	 *</table>
	 */
	public int DeleteQueue(String queuename) {
		int 															ret = Safmq.EC_NETWORKERROR;
		QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	params = new QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS();

		paramutil.arrayCopy(params.queuename,queuename);

		try {
			out.write(Safmq.CMD_QUEUE_DELETE);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Attempts to set the security profile for a specified user.  When a queue
	 * is first created only the queue's creator (owner) may alter the queue's
	 * security.
	 * 
	 * @param queuename		Name of the queue
	 * @param username		Name of the user
	 * @param rd			User may read from the queue
	 * @param wr			User may write to the queue
	 * @param destroy		User may delete the queue
	 * @param changesecurity	User may alter security of the queue
	 * 
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to change the permissions of the user.
	 *				</td></tr>
	 *</table>
	 */
	public int QueueSetUserPermission(String queuename, String username, boolean rd, boolean wr, boolean destroy, boolean changesecurity) {
		int							ret = Safmq.EC_NETWORKERROR;
		QUEUE_SET_USER_PERMS_PARAMS	params = new QUEUE_SET_USER_PERMS_PARAMS();

		paramutil.arrayCopy(params.queuename, queuename);
		paramutil.arrayCopy(params.username, username);
		params.read = (byte)(rd?1:0);
		params.write = (byte)(wr?1:0);
		params.destroy = (byte)(destroy?1:0);
		params.change_security = (byte)(changesecurity?1:0);
		try {
			out.write(Safmq.CMD_QUEUE_SET_USER_PERMS);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Removes permissions for a specified user.
	 * 
	 * @param queuename The name of the queuee
	 * @param username The name of the user 
	 * 
 	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to change the permissions of the user.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The security record for that user does not exist.
	 *				</td></tr>
	 *</table>
	 */
	public int QueueDeleteUserPermission(String queuename, String username) {
		int							ret = Safmq.EC_NETWORKERROR;
		QUEUE_DEL_USER_PERMS_PARAM	params = new QUEUE_DEL_USER_PERMS_PARAM();

		paramutil.arrayCopy(params.queuename, queuename);
		paramutil.arrayCopy(params.username, username);
		try {
			out.write(Safmq.CMD_QUEUE_DEL_USER_PERMS);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}

	/**
	 * Attempts to set the security profile for a specified group.  When a queue
	 * is first created only the queue's creator (owner) may alter the queue's
	 * security.
	 * 
	 * @param queuename		Name of the queue
	 * @param groupname		Name of the group
	 * @param rd			Group may read from the queue
	 * @param wr			Group may write to the queue
	 * @param destroy		Group may delete the queue
	 * @param changesecurity	Group may alter security of the queue
	 * 
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to change the permissions of the user.
	 *				</td></tr>
	 *</table>
	 */
	public int QueueSetGroupPermission(String queuename, String groupname, boolean rd, boolean wr, boolean destroy, boolean changesecurity) {
		int								ret = Safmq.EC_NETWORKERROR;
		QUEUE_SET_GROUP_PERMS_PARAMS	params = new QUEUE_SET_GROUP_PERMS_PARAMS();

		paramutil.arrayCopy(params.queuename, queuename);
		paramutil.arrayCopy(params.groupname, groupname);
		params.read = (byte)(rd?1:0);
		params.write = (byte)(wr?1:0);
		params.destroy = (byte)(destroy?1:0);
		params.change_security = (byte)(changesecurity?1:0);
		try {
			out.write(Safmq.CMD_QUEUE_SET_GROUP_PERMS);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}

	/**
	 * Removes permissions for a specified group.
	 * 
	 * @param queuename The name of the queue
	 * @param groupname The name of the group 
	 * 
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_NOTAUTHORIZED</code></td>
	 *				<td>The current logged in user is not authorized to change the permissions of the group.
	 *				</td></tr>
	 *<tr><td><code>Safmq.EC_DOESNOTEXIST</code></td>
	 *				<td>The security record for that group does not exist.
	 *				</td></tr>
	 *</table>
	 */
	public int QueueDeleteGroupPermission(String queuename, String groupname) {
		int							ret = Safmq.EC_NETWORKERROR;
		QUEUE_DEL_GROUP_PERMS_PARAM	params = new QUEUE_DEL_GROUP_PERMS_PARAM();

		paramutil.arrayCopy(params.queuename, queuename);
		paramutil.arrayCopy(params.groupname, groupname);
		try {
			out.write(Safmq.CMD_QUEUE_DEL_GROUP_PERMS);
			params.write(out);
			out.flush();
			ret = getResponseCode();
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}

	/**
	 * Retrieves the permission profile for the specified queue.  Resultant permissions
	 * are placed in the <code>Vector perms</code> as <code>QueuePermissions</code>
	 * objects.
	 * 
	 * @param queuename The name of the queue.
	 * @param perms		The vector which will be emptied and populated with <code>MQConnection.QueuePermissions</code> objects.
	 * 
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise errors returned
	 * could be but are not limited to:<br>
	 *<table border=0 cellpadding=3 cellspacing=0>
	 *<tr><td><code>Safmq.EC_NETWORKERROR</code></td>
	 *				<td>A networking error has occurred and the connection
	 *					is no longer valid.
	 *				</td></tr>
	 *</table>
	 *
	 * @see QueuePermissions
	 */
	public int QueueEnumeratePermissions(String queuename, Vector perms) {
		int																ret = Safmq.EC_NETWORKERROR;
		QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	params = new QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS();

		paramutil.arrayCopy(params.queuename, queuename);
		perms.clear();

		try {
			out.write(Safmq.CMD_QUEUE_ENUM_PERMS);
			params.write(out);
			out.flush();
			ret = getResponseCode();
			if (ret == Safmq.EC_NOERROR) {
				int nPerms = in.readInt();
				int x;
				QUEUE_PERM_DATA	data = new QUEUE_PERM_DATA();
				perms.removeAllElements();
				for(x=0;x<nPerms; x++) {
					data.read(in);
					perms.addElement( new QueuePermissions( new String(data.entityname,0,zlength(data.entityname)),
															data.isgroup!=0,
															data.read!=0,
															data.write!=0,
															data.destroy!=0,
															data.change_security!=0));
				}
			}
		} catch (IOException e) {
			ret = Safmq.EC_NETWORKERROR;
		}
		return ret;
	}
	
	/**
	 * Internal method which acknowledges the reception of data from a "Retrieve" 
	 * operation.  
	 *  
	 * @param qhandle	The queue being utilized.
	 * @param msg		The message that was retrieved.
	 *  
	 * @return	The error response code form the server.
	 * 
	 * @throws IOException In the case of a network error.
	 */
	int ackRetrieve(QueueHandle qhandle, QueueMessage msg) throws IOException {
		RETRIEVE_ACK_PARAMS	params = new RETRIEVE_ACK_PARAMS();
		params.queueID = qhandle.handle;
		params.msgID = msg.getMessageID();
		
		out.write(Safmq.CMD_RETRIEVE_ACK);
		params.write(out);
		out.flush();
		return getResponseCode();
	}

	/**
	 * Internal method which acknowledges the reception of data from a "Retrieve"
	 * operation.
	 * 
	 * @param qhandle	The queue being utilized
	 * @param msg		The message that was retrieved
	 * @return	The error response code from the server.
	 * 
	 * @throws IOException In the case of a network error.
	 */
	int ackRetrieveCursor(QueueHandle qhandle, QueueMessage msg) throws IOException {
		RETRIEVE_ACK_PARAMS	params = new RETRIEVE_ACK_PARAMS();
		params.queueID = qhandle.handle;
		params.msgID = msg.getMessageID();

		out.write(Safmq.CMD_RETRIEVE_CURSOR_ACK);
		params.write(out);
		out.flush();
		return getResponseCode();
	}

	
	/**
	 * Retrieves the response code from the stream.
	 * 
	 * @return the response code from the stream.
	 */
	int getResponseCode() {
		try {
			return in.readInt();
		} catch (IOException e) {
			return Safmq.EC_NETWORKERROR;
		}
	}
	
	/**
	 * Calculates the length of a zero terminated string (zstring) within the
	 * array of bytes passed.
	 * 
	 * @param src The source array to be evaluated
	 * 
	 * @return The number of bytes in the zero terminated string.
	 */
	int zlength(byte src[]) {
		int len;
		for(len=0; len < src.length && src[len] != 0; len++) { }
		return len;
	}
}

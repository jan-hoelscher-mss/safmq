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

/**
 * This class represents a SAFMQ message queue by providing a facade in front of
 * the <code>MQConnection</code>.  This facade provides an interface to 
 * queue operations such as enqueueing messages, retrieving messages, and scanning
 * the queue for messages with particular attributes.<br>
 * 
 * <b>Note</b>:  When a queue is no longer in use, a call to <code>Close()</code>
 * should be made so that resources on the server are released.  Additionally,
 * when cursors are no longer needed, a call to <code>CloseCursor()</code> should
 * be made.
 * 
 * @see MessageQueue#Close()
 * @see MessageQueue#CloseCursor(CursorHandle)
 * @see MQConnection
 * 
 * @author Matt
 */
public class MessageQueue {
	/**
	 * Contains the queue's name.
	 */
	String 			queuename;
	/**
	 * A reference to the MQConnection object connected to the server
	 */
	MQConnection 	con;
	/**
	 * Maintains the queue's identity handle with the server.
	 */
	QueueHandle		que;
	
	/**
	 * Constructs the <code>MessageQueue</code> object, from the queue's name
	 * and a connection to the queue server.  This method opens a reference to
	 * the queue.  In case of an error, an <code>MQException</code> is thrown
	 * reporting the status of the connection.
	 * 
	 * @param	queuename	The name of the queue to be opened
	 * @param	con			A valid connection to a SAFMQ server 
	 * @throws	MQException	In the case that OpenQueue reports an error
	 */
	public MessageQueue(String queuename, MQConnection con) throws MQException {
		this.queuename	= queuename;
		this.con		= con;
		que = new QueueHandle();
		int err = con.OpenQueue(queuename,que);
		if (err != Safmq.EC_NOERROR) {
			throw new MQException("Login attempt failed, SAFMQ Error Code(" + err+")", err);
		}
	}
	
	/**
	 * Constructs the <code>MessageQueue</code> object, from an open
	 * handle to a message queue.  This interface is useful when working
	 * with a Temporary Queue, or already open queue.
	 * 
	 * @param	con			A valid connection to a SAFMQ server 
	 * @throws	MQException	In the case that OpenQueue reports an error
	 */
	public MessageQueue(MQConnection con, QueueHandle que) {
		this.queuename = null;
		this.con = con;
		this.que = que;
	}
	
	/**
	 * Closes the connection to the queue.  After a call to close the MessageQueue
	 * is no longer valid and should not be used.
	 * 
	 * @return The result code from a call to <code>MQConnection.CloseQueue()</code>
	 * @see MQConnection#CloseQueue(QueueHandle) MQConnection.CloseQueue(QueueHandle) for details on errors
	 */
	public int Close() {
		return con.CloseQueue(que);
	}
	
	/**
	 * Gets statistics about an open queue.  Places statistic information into the parameter <code>stats</code>.
	 * 
	 * @param includeStorageBytes Causes server to calculate the number of bytes on disk
	 * @param includeMessageBytes Causes the server to calculate the number of bytes in queue, may be less than bytes on disk
	 * @param stats Receives the statistics information.
	 * @return Safmq.EC_NOERROR on success <br>
	 * 	Safmq.EC_NETWORKERROR<br>
	 * 	Safmq.EC_NOTOPEN <br>
	 */
	public int GetQueueStatistics(boolean includeStorageBytes, boolean includeMessageBytes, QueueStatistics stats){
		return con.GetQueueStatistics(que, includeStorageBytes, includeMessageBytes, stats);
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
	 * @param	msg	The message to be placed on the queue
	 * @return	The results of a call to <code>MQConnection.Enqueue()</code>
	 * @see MQConnection#Enqueue(QueueHandle,QueueMessage) MQConnection.Enqueue(QueueHandle,QueueMessage) for details on errors
	 */
	public int Enqueue(QueueMessage msg) {
		return con.Enqueue(que,msg);
	}

	/**
	 * Retrieves the highest priority FIFO message present on the queue.  The results
	 * are placed in the object <code>msg</code>.
	 * 
	 * <p>Note: Retrieved messages are removed from the queue</p>
	 * 
	 * @param retrievebody	A flag indicating whether the body of the message should be retrieved
	 * @param timeout		The number of seconds to wait before returning, a value of zero (0) will
	 * 						cause the method to return immediately if no messages are present on the queue,
	 * 						a value of (-1) will cause the method to wait until a message is placed on the queue.
	 * @param msg			Receives the contents of the message.
	 * 
	 * @return	Upon success a value of <code>Safmq.EC_NOERROR</code>, otherwise
	 * 			see MQConnection.Retrieve() for result codes.
	 * @see MQConnection#Retrieve(QueueHandle,boolean,int,QueueMessage) MQConnection.Retrieve(QueueHandle,boolean,int,QueueMessage)
	 */
	public int Retrieve(boolean retrievebody, int timeout, QueueMessage msg) {
		return con.Retrieve(que,retrievebody,timeout,msg);
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
	 * @param retrievebody	A flag indicating whether the body of the message should be retrieved
	 * @param id			The UUID of the message to be retrieved.
	 * @param timeout		The number of seconds to wait before returning, a value of zero (0) will
	 * 						cause the method to return immediately if no messages are present on the queue,
	 * 						a value of (-1) will cause the method to wait until a message is placed on the queue.
	 * @param msg			Receives the contents of the message.
	 * 
	 * @return	Upon success a value of <code>Safmq.EC_NOERROR</code>, otherwise
	 * 			see MQConnection.RetrieveID() for result codes.
	 * 
	 * @see MQConnection#RetrieveID(QueueHandle,boolean,UUID,int,QueueMessage) MQConnection.Retrieve(QueueHandle,boolean,int,QueueMessage)
	 * @see #Enqueue(QueueMessage) Enqueue(QueueMessage)
	 */
	public int RetrieveID(boolean retrievebody, UUID id, int timeout, QueueMessage msg) {
		return con.RetrieveID(que,retrievebody,id,timeout,msg);
	}
	
	/**
	 * Retrieves the message pointed to by <code>cursorID</code>. The results  
	 * are placed in the object <code>msg</code>.
	 * 
	 * <p>Note: Retrieved messages are removed from the queue.</p>
	 * 
	 * @param retrievebody	A flag indicating whether the body of the message should be retrieved
	 * @param cursorID		The cursor indicating the current position in the queue to be read from
	 * @param msg			Receives the contents of the message.
	 * 
	 * @return	Upon success a value of <code>Safmq.EC_NOERROR</code>, otherwise
	 * 			see MQConnection.Retrieve() for result codes.
	 * @see MQConnection#RetrieveCursor(QueueHandle,boolean,CursorHandle,QueueMessage) MQConnection.Retrieve(QueueHandle,boolean,int,QueueMessage)
	 */
	public int RetrieveCursor(boolean retrievebody, CursorHandle cursorID, QueueMessage msg) {
		return con.RetrieveCursor(que,retrievebody,cursorID,msg);
	}

	/**
	 * Gathers the highest priority FIFO message present on the queue.  The results
	 * are placed in the object <code>msg</code>.  Any errors from the operation are
	 * returned on the stack.    The message retrieved is <i>not</i> removed from the 
	 * queue and is available for reading by other queue readers.
	 * 
	 * @param retrievebody	A flag indicating whether the body of the message should be retrieved
	 * @param timeout		The number of seconds to wait before returning, a value of zero (0) will
	 * 						cause the method to return immediately if no messages are present on the queue,
	 * 						a value of (-1) will cause the method to wait until a message is placed on the queue.
	 * @param msg			Receives the contents of the message.
	 * 
	 * @return	Upon success a value of <code>Safmq.EC_NOERROR</code>, otherwise
	 * 			see MQConnection.Retrieve() for result codes.
	 * @see MQConnection#PeekFront(QueueHandle,boolean,int,QueueMessage) MQConnection.Retrieve(QueueHandle,boolean,int,QueueMessage)
	 */
	public int PeekFront(boolean retrievebody, int timeout, QueueMessage msg) {
		return con.PeekFront(que,retrievebody, timeout, msg);
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
	 * @param retrievebody	A flag indicating whether the body of the message should be retrieved
	 * @param id			The UUID of the message to be retrieved.
	 * @param timeout		The number of seconds to wait before returning, a value of zero (0) will
	 * 						cause the method to return immediately if no messages are present on the queue,
	 * 						a value of (-1) will cause the method to wait until a message is placed on the queue.
	 * @param msg			Receives the contents of the message.
	 * 
	 * @return	Upon success a value of <code>Safmq.EC_NOERROR</code>, otherwise
	 * 			see MQConnection.PeekID() for result codes.
	 * 
	 * @see MQConnection#PeekID(QueueHandle,boolean,UUID,int,QueueMessage) MQConnection.Retrieve(QueueHandle,boolean,int,QueueMessage)
	 * @see #Enqueue(QueueMessage) Enqueue(QueueMessage)
	 */
	public int PeekID(boolean retrievebody, UUID id, int timeout, QueueMessage msg) {
		return con.PeekID(que,retrievebody,id,timeout,msg);
	}

	/**
	 * Retrieves the message pointed to by <code>cursorID</code>. The results  
	 * are placed in the object <code>msg</code>.  The message retrieved is 
	 * <i>not</i> removed from the queue and is available for reading by other 
	 * queue readers.
	 * 
	 * <p>Note: Retrieved messages are removed from the queue.</p>
	 * 
	 * @param retrievebody	A flag indicating whether the body of the message should be retrieved
	 * @param cursorID		The cursor indicating the current position in the queue to be read from
	 * @param msg			Receives the contents of the message.
	 * 
	 * @return	Upon success a value of <code>Safmq.EC_NOERROR</code>, otherwise
	 * 			see <code>MQConnection.PeekCursor()</code> for result codes.
	 * @see MQConnection#PeekCursor(QueueHandle,boolean,CursorHandle,QueueMessage) MQConnection.Retrieve(QueueHandle,boolean,int,QueueMessage)
	 */
	public int PeekCursor(boolean retrievebody, CursorHandle cursorID, QueueMessage msg) {
		return con.PeekCursor(que,retrievebody,cursorID,msg);
	}

	/**
	 * Opens a cursor for sequential reading of the queue.  The newly opened
	 * cursor points to the oldest, highest priority message in the queue.
	 * 
	 * <p>Cursors may be used to violate the priority FIFO nature of the queue
	 * by scanning the queue and only retrieving messages which meet the 
	 * criteria of the reader.  Cursor access is sequential when used in conjunction
	 * with <code>AdvanceCursor(CursorHandle)</code>, however specific messages may
	 * be sought out by a call to <code>SeekID(UUID,int,CursorHandle)</code>.  </p>
	 * 
	 * @param cursorID 	A previously allocated reference to a <code>CursorHandle</code>
	 * 					which will receive the reference to the cursor.
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise see 
	 * 		<code>MQConnection.OpenCursor(QueueHandle,CursorHandle)</code>
	 * 		for error codes.
	 * 
	 * @see MQConnection#OpenCursor(QueueHandle,CursorHandle) MQConnection.OpenCursor(QueueHandle,CursorHandle)
	 */
	public int OpenCursor(CursorHandle cursorID) {
		return con.OpenCursor(que,cursorID);
	}
	
	/**
	 * Closes a cursor when it is no longer needed to access the queue.  The
	 * cursor is invalidated after a call to <code>CloseCursor()</code> and
	 * should not be used again.
	 * 
	 * @param cursorID The cursor to be closed.
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise see 
	 * 		<code>MQConnection.CloseCursor(QueueHandle,CursorHandle)</code>
	 * 		for error codes.
	 * @see MQConnection#CloseCursor(QueueHandle,CursorHandle) MQConnection.CloseCursor(QueueHandle,CursorHandle)
	 */
	public int CloseCursor(CursorHandle cursorID) {
		return con.CloseCursor(que,cursorID);
	}

	/**
	 * Advances the cursor to the next message in the queue.
	 * 
	 * @param cursorID The cursor to be advanced.
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise see 
	 * 		<code>MQConnection.AdvanceCursor(QueueHandle,CursorHandle)</code>
	 * 		for error codes.
	 * @see MQConnection#AdvanceCursor(QueueHandle,CursorHandle) MQConnection.AdvanceCursor(QueueHandle,CursorHandle)
	 */
	public int AdvanceCursor(CursorHandle cursorID) {
		return con.AdvanceCursor(que,cursorID);
	}
	
	/**
	 * Attempts to locate the messaged indicated by <code>id</code>.  If the message
	 * is successfully located, the cursor will point to the message identified
	 * and the message may be retrieved there after.
	 * 
	 * @param id		The receipt UUID of the message to be located 
	 * @param timeout	The maximum number of seconds the call should be allowed
	 * 					before resulting in a "timeout".
	 * @param cursorID	The cursor which should be be assigned the position
	 * 					of the message indicated by <code>id</code>
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise see 
	 * 		<code>MQConnection.SeekID(QueueHandle,UUID,int,CursorHandle)</code>
	 * 		for error codes.
	 * @see MQConnection#SeekID(QueueHandle,UUID,int,CursorHandle) MQConnection.SeekID(QueueHandle,UUID,int,CursorHandle)
	 */
	public int SeekID(UUID id, int timeout, CursorHandle cursorID) {
		return con.SeekID(que,id,timeout,cursorID);
	}

	/**
	 * Determines whether the cursor indicated by <code>cursorID</code> is still
	 * a valid cursor, and has not been invalidated by other queue readers.
	 * 
	 * @param cursorID	The cursor which should be be evaluated
	 * @return <code>Safmq.EC_NOERROR</code> upon success, otherwise see 
	 * 		<code>MQConnection.TestCursor(QueueHandle,CursorHandle)</code>
	 * 		for error codes.
	 * @see MQConnection#TestCursor(QueueHandle,CursorHandle) MQConnection.TestCursor(QueueHandle,CursorHandle)
	 */
	public int TestCursor(CursorHandle cursorID) {
		return con.TestCursor(que,cursorID);
	}
	
	public MQConnection getConnection() {
		return con;
	}
}

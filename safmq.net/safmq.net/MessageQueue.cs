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


Created on Aug 13, 2007
*/
using System;
using System.Collections.Generic;
using System.Text;

namespace safmq.net {
    /**
     * <summary>
     * This class represents a SAFMQ message queue by providing a facade infront of
     * the <c>MQConnection</c>.  This facade provides an interface to 
     * queue operations such as enqueueing messages, retreiving messages, and scanning
     * the queue for messages with particular attributes.</summary>
     * <remarks>
     * <b>Note</b>:  When a queue is nolonger in use, a call to <c>Close()</c>
     * should be made so that resources on the server are released.  Additionally,
     * when cursors are nolonger needed, a call to <c>CloseCursor()</c> should
     * be made.
     * 
     * @author Matt
     * </remarks>
     * 
     * <seealso cref="MessageQueue.Close()"/>
     * <seealso cref="MessageQueue.CloseCursor(CursorHandle)"/>
     * <seealso cref="MQConnection"/>
     * 
     */
    public class MessageQueue : IDisposable {
	    /// <summary>
        /// Contains the queue's name.
	    /// </summary>
	    string 			queuename;
	    /// <summary>
        /// A reference to the MQConnection object connected to the server
	    /// </summary>
	    MQConnection 	con;
	    /// <summary>
        /// Maintains the queue's identity handle with the server.
	    /// </summary>
	    QueueHandle		que = new QueueHandle();

        /**
         * <summary>
         * Constructs the <c>MessageQueue</c> object, from the queue's name
         * and a connection to the queue server.  This method opens a reference to
         * the queue.  In case of an error, an <c>MQException</c> is thrown
         * reporting the status of the connection.
         * </summary>
         * 
         * <param name="queuename">The name of the queue to be opened</param>
         * <param name="con">A valid connection to a SAFMQ server</param>
         * 
         * <exception cref="MQException">In the case that OpenQueue reports an error</exception>
         */
        public MessageQueue(string queuename, MQConnection con) {
		    this.queuename	= queuename;
		    this.con		= con;
		    ErrorCode err = con.OpenQueue(queuename,que);
		    if (err != ErrorCode.EC_NOERROR) {
			    throw new MQException("Login attempt failed, SAFMQ Error Code(" + err+")", err);
		    }
	    }

		/**
		 * <summary>Provided to support the IDisposable interface.</summary>
		 * <remarks>Allows use of "using" blocks to automatically release resources.</remarks>
		 */
		public void Dispose () {
			Close();
		}

        /**
         * <summary>
         * Closes the connection to the queue.  After a call to close the MessageQueue
         * is no longer valid and should not be used.
         * </summary>
         * 
         * <returns>The result code from a call to <c>MQConnection.CloseQueue()</c></returns>
         * 
         * <seealso cref="MQConnection.CloseQueue(QueueHandle)">MQConnection.CloseQueue(QueueHandle) for details on errors</seealso>
         */
        public ErrorCode Close() {
		    return con.CloseQueue(que);
	    }

        /**
         * <summary>
         * Places a message on the queue.  The object <c>msg</c> must have
         * been previously prepared before calling this method.  Upon successfull sending
         * of the message, the message id and message timestamp will be set by the server
         * and may be retrieved by a call to <c>QueueMessage.getMessageID()</c>
         * and <c>QueueMessage.getTimeStamp()</c> respectively.
         * </summary>
         * 
         * <remarks>
         * <para>Note: Message responsders will typically place the message id of the original
         * message in the recipt id of the message being returned.  This round-trip
         * message identification is provided by SAFMQ as a method for coordinated two-way
         * communications.</para>
         * </remarks>
         * 
         * <param name="msg">The message to be placed on the queue</param>
         * <returns>The results of a call to <c>MQConnection.Enqueue()</c></returns>
         * <seealso cref="MQConnection.Enqueue(QueueHandle,QueueMessage)">MQConnection.Enqueue(QueueHandle,QueueMessage) for details on errors</seealso>
         */
        public ErrorCode Enqueue(QueueMessage msg) {
		    return con.Enqueue(que,msg);
	    }

        /**
         * <summary>
         * Retrieves the highest priority FIFO message present on the queue.  The results
         * are placed in the object <c>msg</c>.
         * 
         * <para>Note: Retrieved messages are removed from the queue</para>
         * </summary>
         * 
         * <param name="retrievebody">
         *                      A flag indicating whether the body of the message should be retrieved</param>
         * <param name="timeout">
         *                      The number of seconds to wait before returning, a value of zero (0) will
         * 						cause the method to return immediately if no messages are present on the queue,
         * 						a value of (-1) will cause the method to wait until a message is placed on the queue.
         *                      </param>
         * <param name="msg">   Receives the contents of the message.</param>
         * 
         * <returns>Upon success a value of <c>Safmq.EC_NOERROR</c>, otherwise
         * 			see MQConnection.Retrieve() for result codes.</returns>
         * <seealso cref="MQConnection.Retrieve(QueueHandle,bool,int,QueueMessage)">MQConnection.Retrieve(QueueHandle,bool,int,QueueMessage)</seealso>
         */
        public ErrorCode Retrieve(bool retrievebody, int timeout, QueueMessage msg) {
		    return con.Retrieve(que,retrievebody,timeout,msg);
	    }

        /**
         * <summary>
         * Retrieves the message identified by <c>id</c> in the message's 
         * recipt id, set prior to the message having been enqueued (See: 
         * <c>Enqueue(QueueMessage)</c>. The results  are placed in the object
         * <c>msg</c>.  
         * </summary>
         * 
         * <remarks>
         * <para>Note: Message responsders will typically place the message id of the original
         * message in the recipt id of the message being returned.  This round-trip
         * message identification is provided by SAFMQ as a method for coordinated two-way
         * communications.</para>
         * 
         * <para>Note: Retrieved messages are removed from the queue.</para>
         * </remarks>
         * 
         * <param name="retrievebody">
         *                      A flag indicating whether the body of the message should be retrieved</param>
         * <param name="id">    The UUID of the message to be retrieved.</param>
         * <param name="timeout">
         *                      The number of seconds to wait before returning, a value of zero (0) will
         * 						cause the method to return immediately if no messages are present on the queue,
         * 						a value of (-1) will cause the method to wait until a message is placed on the queue.
         *                      </param>
         * <param name="msg">   Receives the contents of the message.</param>
         * 
         * <returns>Upon success a value of <c>Safmq.EC_NOERROR</c>, otherwise
         * 			see MQConnection.RetrieveID() for result codes.</returns>
         * 
         * <seealso cref="MQConnection.RetrieveID(QueueHandle,bool,UUID,int,QueueMessage)">MQConnection.Retrieve(QueueHandle,bool,int,QueueMessage)</seealso>
         * <seealso cref="Enqueue(QueueMessage)">Enqueue(QueueMessage)</seealso>
         */
        public ErrorCode RetrieveID(bool retrievebody, UUID id, int timeout, QueueMessage msg) {
		    return con.RetrieveID(que,retrievebody,id,timeout,msg);
	    }

        /**
         * <summary>
         * Retrieves the message pointed to by <c>cursorID</c>. The results  
         * are placed in the object <c>msg</c>.
         * 
         * <para>Note: Retrieved messages are removed from the queue.</para>
         * </summary>
         * 
         * <param name="retrievebody">
         *                          A flag indicating whether the body of the message should be retrieved</param>
         * <param name="cursorID">  The cursor indicating the current position in the queue to be read from</param>
         * <param name="msg">       Receives the contents of the message.</param>
         * 
         * <returns>Upon success a value of <c>Safmq.EC_NOERROR</c>, otherwise
         * 			see MQConnection.Retrieve() for result codes.</returns>
         * <seealso cref="MQConnection.RetrieveCursor(QueueHandle,bool,CursorHandle,QueueMessage)">MQConnection.Retrieve(QueueHandle,bool,int,QueueMessage)</seealso>
         */
        public ErrorCode RetrieveCursor(bool retrievebody, CursorHandle cursorID, QueueMessage msg) {
		    return con.RetrieveCursor(que,retrievebody,cursorID,msg);
	    }

        /**
         * <summary>
         * Gathers the highest priority FIFO message present on the queue.  The results
         * are placed in the object <c>msg</c>.  Any errors from the operation are
         * returned on the stack.    The message retrieved is <i>not</i> remvoed from the 
         * queue and is available for reading by other queue readers.
         * </summary>
         * 
         * <param name="retrievebody">
         *                      A flag indicating whether the body of the message should be retrieved</param>
         * <param name="timeout">
         *                      The number of seconds to wait before returning, a value of zero (0) will
         * 						cause the method to return immediately if no messages are present on the queue,
         * 						a value of (-1) will cause the method to wait until a message is placed on the queue.
         *                      </param>
         * <param name="msg">   Receives the contents of the message.</param>
         * 
         * <returns>Upon success a value of <c>Safmq.EC_NOERROR</c>, otherwise
         * 			see MQConnection.Retrieve() for result codes.</returns>
         * <seealso cref="MQConnection.PeekFront(QueueHandle,bool,int,QueueMessage)">MQConnection.Retrieve(QueueHandle,bool,int,QueueMessage)</seealso>
         */
        public ErrorCode PeekFront(bool retrievebody, int timeout, QueueMessage msg) {
		    return con.PeekFront(que,retrievebody, timeout, msg);
	    }

        /**
         * <summary>
         * Gathers the message identified by <c>id</c> in the message's 
         * recipt id, set prior to the message having been enqueued (See: 
         * <c>Enqueue(QueueMessage)</c>. The results  are placed in the object
         * <c>msg</c>.  The message retrieved is <i>not</i> remvoed from the 
         * queue and is available for reading by other queue readers.
         * </summary>
         * 
         * <remarks>
         * <para>Note: Message responsders will typically place the message id of the original
         * message in the recipt id of the message being returned.  This round-trip
         * message identification is provided by SAFMQ as a method for coordinated two-way
         * communications.</para>
         * </remarks>
         * 
         * <param name="retrievebody">
         *                      A flag indicating whether the body of the message should be retrieved</param>
         * <param name="id">    The UUID of the message to be retrieved.</param>
         * <param name="timeout">
         *                      The number of seconds to wait before returning, a value of zero (0) will
         * 						cause the method to return immediately if no messages are present on the queue,
         * 						a value of (-1) will cause the method to wait until a message is placed on the queue.
         *                      </param>
         * <param name="msg">   Receives the contents of the message.</param>
         * 
         * <returns>Upon success a value of <c>Safmq.EC_NOERROR</c>, otherwise
         * 			see MQConnection.PeekID() for result codes.</returns>
         * 
         * <seealso cref="MQConnection.PeekID(QueueHandle,bool,UUID,int,QueueMessage)">MQConnection.Retrieve(QueueHandle,bool,int,QueueMessage)</seealso>
         * <seealso cref="Enqueue(QueueMessage)">Enqueue(QueueMessage)</seealso>
         */
        public ErrorCode PeekID(bool retrievebody, UUID id, int timeout, QueueMessage msg) {
		    return con.PeekID(que,retrievebody,id,timeout,msg);
	    }

        /**
         * <summary>
         * Retrieves the message pointed to by <c>cursorID</c>. The results  
         * are placed in the object <c>msg</c>.  The message retrieved is 
         * <i>not</i> remvoed from the queue and is available for reading by other 
         * queue readers.
         * 
         * <para>Note: Retrieved messages are removed from the queue.</para>
         * </summary>
         * 
         * <param name="retrievebody">  A flag indicating whether the body of the message should be retrieved</param>
         * <param name="cursorID">      The cursor indicating the current position in the queue to be read from</param>
         * <param name="msg">           Receives the contents of the message.</param>
         * 
         * <returns>Upon success a value of <c>Safmq.EC_NOERROR</c>, otherwise
         * 			see <c>MQConnection.PeekCursor()</c> for result codes.</returns>
         * <seealso cref="MQConnection.PeekCursor(QueueHandle,bool,CursorHandle,QueueMessage)">MQConnection.Retrieve(QueueHandle,bool,int,QueueMessage)</seealso>
         */
        public ErrorCode PeekCursor(bool retrievebody, CursorHandle cursorID, QueueMessage msg) {
		    return con.PeekCursor(que,retrievebody,cursorID,msg);
	    }

        /**
         * <summary>
         * Opens a cursor for sequential reading of the queue.  The newly opened
         * cursor points to the oldest, highest priority message in the queue.
         * </summary>
         * 
         * <remarks>
         * <para>Cursors may be used to violate the priority FIFO nature of the queue
         * by scanning the queue and only retreiving messages which meet the 
         * criteria of the reader.  Cursor access is sequential when used in conjunction
         * with <c>AdvanceCursor(CursorHandle)</c>, however specific messages may
         * be sought out by a call to <c>SeekID(UUID,int,CursorHandle)</c>.  </para>
         * </remarks>
         * 
         * <param name="cursorID">
         *                  A previously allocated reference to a <c>CursorHandle</c>
         * 					which will receive the reference to the cursor.</param>
         * 
         * <returns><c>Safmq.EC_NOERROR</c> upon success, otherwise see 
         * 		<c>MQConnection.OpenCursor(QueueHandle,CursorHandle)</c>
         * 		for error codes.
         * </returns>
         * <seealso cref="MQConnection.OpenCursor(QueueHandle,CursorHandle)">MQConnection.OpenCursor(QueueHandle,CursorHandle)</seealso>
         */
        public ErrorCode OpenCursor(CursorHandle cursorID) {
		    return con.OpenCursor(que,cursorID);
	    }

        /**
         * <summary>
         * Closes a cursor when it is nolonger needed to access the queue.  The
         * cursor is invalidated after a call to <c>CloseCursor()</c> and
         * should not be used again.
         * </summary>
         * 
         * <param name="cursorID">  The cursor to be closed.</param>
         * <returns><c>Safmq.EC_NOERROR</c> upon success, otherwise see 
         * 		<c>MQConnection.CloseCursor(QueueHandle,CursorHandle)</c>
         * 		for error codes.
         * </returns>
         * <seealso cref="MQConnection.CloseCursor(QueueHandle,CursorHandle)">MQConnection.CloseCursor(QueueHandle,CursorHandle)</seealso>
         */
        public ErrorCode CloseCursor(CursorHandle cursorID) {
		    return con.CloseCursor(que,cursorID);
	    }

        /**
         * <summary>
         * Advances the cursor to the next message in the queue.
         * </summary>
         * 
         * <param name="cursorID">  The cursor to be advanced.</param>
         * 
         * <returns><c>Safmq.EC_NOERROR</c> upon success, otherwise see 
         * 		<c>MQConnection.AdvanceCursor(QueueHandle,CursorHandle)</c>
         * 		for error codes.
         * </returns>
         * <seealso cref="MQConnection.AdvanceCursor(QueueHandle,CursorHandle)">MQConnection.AdvanceCursor(QueueHandle,CursorHandle)</seealso>
         */
        public ErrorCode AdvanceCursor(CursorHandle cursorID) {
		    return con.AdvanceCursor(que,cursorID);
	    }

        /**
         * <summary>
         * Attempts to locate the messaged indicated by <c>id</c>.  If the message
         * is succesfully located, the cursor will point to the message idenfified
         * and the message may be retrieved there after.
         * </summary>
         * 
         * <param name="id">The recipt UUID of the message to be located </param>
         * <param name="timeout">
         *                  The maximum number of seconds the call should be allowed
         * 					before resulting in a "timeout".</param>
         * <param name="cursorID">
         *                  The cursor which should be be assigned the position
         * 					of the message indicated by <c>id</c></param>
         * 
         * <returns><c>Safmq.EC_NOERROR</c> upon success, otherwise see 
         * 		<c>MQConnection.SeekID(QueueHandle,UUID,int,CursorHandle)</c>
         * 		for error codes.
         * </returns>
         * 
         * <seealso cref="MQConnection.SeekID(QueueHandle,UUID,int,CursorHandle)">MQConnection.SeekID(QueueHandle,UUID,int,CursorHandle)</seealso>
         */
        public ErrorCode SeekID(UUID id, int timeout, CursorHandle cursorID) {
		    return con.SeekID(que,id,timeout,cursorID);
	    }

        /**
         * <summary>
         * Determines whether the cursor indicated by <c>cursorID</c> is still
         * a valid cursor, and has not been invalidated by other queue readers.
         * </summary>
         * 
         * <param name="cursorID">  The cursor which should be be evaluated</param>
         * 
         * <returns><c>Safmq.EC_NOERROR</c> upon success, otherwise see 
         * 		<c>MQConnection.TestCursor(QueueHandle,CursorHandle)</c>
         * 		for error codes.
         * </returns>
         * 
         * <seealso cref="MQConnection.TestCursor(QueueHandle,CursorHandle)">MQConnection.TestCursor(QueueHandle,CursorHandle)</seealso>
         */
        public ErrorCode TestCursor(CursorHandle cursorID) {
		    return con.TestCursor(que,cursorID);
	    }
    }

}

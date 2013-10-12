/*
 Copyright 2005-2007 Matthew J. Battey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except input compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to input writing, software distributed
    under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.


Created on Aug 9, 2007

*/
using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Net;

namespace safmq.net
{
    /**
     * <summary>
     * This class provides an interface to communicate with a SAFMQ message queue
     * server.  Typically instances of this class are constructed by the 
     * <c>MQBuilder</c> class, however constructing the class directly
     * is also allowed.  
     * </summary>
     * <remarks>
     * <para>Once constructed, this class may be used to construct
     * instances of the class <c>MessageQueue</c> to abstract the connection
     * interface to the message queue level.</para>
     * 
     * <para><b>Note:</b> This class is not thread safe, and applications should
     * take precautions to not initiate communications with a message queue via
     * more than one (1) thread simultaneously.</para>
     * 
     * @author Matt
     * </remarks>
     * <seealso cref="MQBuilder.BuildConnection(Uri,string,string)"/>
     * <seealso cref="MessageQueue"/>
     */
    public class MQConnection : IDisposable {
    	private const string FORWARD_QUEUE_NAME = "_FORWARD_";
        private int server_major_protocol_version;
        private int server_minor_protocol_version;

        BufferedStream server;
        BinaryWriter output;
        BinaryReader input;

        /** 
         * <summary>
         * Constructs an <c>MQConnection</c> which represents a connection to the SAFMQ
         * server.
         * </summary>
         * <param name="connectedStream">A stream connected to a SAFMQ server.</param>
         * <param name="username">User ID to establish a security context</param>
         * <param name="password">secret word to identify the authenticity of the User ID</param>
         * <seealso cref="MQBuilder.BuildConnection(Uri,string,string)"/>
         */
        public MQConnection(Stream connectedStream, string username, string password) {
            server = new BufferedStream(connectedStream);
            output = new NetByteOrderWriter(server);
            input = new NetByteOrderReader(server);

            output.Write(Safmq.CMD_LOGIN);
            (new LOGIN_PARAMS(username == null ? "" : username, password == null ? "" : password)).Write(output);
            output.Flush();

            ErrorCode err = getResponseCode();
            if (err != ErrorCode.EC_NOERROR && err != ErrorCode.EC_NOTIMPLEMENTED) {
                throw new MQException("Login attempt failed error code (" + err + ")", err);
            } else if (err == ErrorCode.EC_NOERROR) {
                LOGIN_RESPONSE resp = new LOGIN_RESPONSE();
                resp.Read(input);
                server_major_protocol_version = resp.major_version;
                server_minor_protocol_version = resp.minor_version;
                if (server_major_protocol_version < Safmq.SAFMQ_PROTOCOL_MAJOR_VERSION)
                    throw new MQException("Login attempt failed error code (" + ErrorCode.EC_UNSUPPORTED_PROTOCOL + ")", ErrorCode.EC_UNSUPPORTED_PROTOCOL);
            }
        }

        /**
         * <summary>Provides the major protocol version of the server.</summary>
         * 
         * <returns>The major protocol version of the server.</returns>
         */
        public int ServerMajorProtocolVersion {
            get { return server_major_protocol_version; }
        }

        /**
         * <summary>Provides the minor protocol version of the server.</summary>
         * 
         * <returns>The Minor protocol version of the server.</returns>
         */
        public int ServerMinorProtocolVersion {
             get { return server_major_protocol_version; }
        }

		/**
		 * <summary>Provided to support the IDisposable interface.</summary>
		 * <remarks>Allows use of "using" blocks to automatically release resources.</remarks>
		 */
		public void Dispose ()
		{
			Close();
		}

        /**
         * <summary>
         * Opens a queue for reading and writing. The queue is then after
         * referenced by the parameter <c>handle</c> for calls such as 
         * <c>Enqueue(QueueHandle,QueueMessage)</c> and 
         * <c>Retrieve(QueueHandle,bool,int,QueueMessage)</c>.
         * </summary>
         * 
         * <remarks>
         * <para><b>Note</b>: Queues which have been opened by a call to <c>OpenQueue()</c>
         * must be closed by a cll to <c>CloseQueue(QueueHandle)</c> if the queue
         * is not closed, resources allocated by a call to <c>OpenQueue()</c>
         * will not be released.</para>
         * </remarks>
         * 
         * <param name="queuename">The name of the queue</param>
         * <param name="handle">Receives a reference to the queue</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> on success otherwise errors such as these, but
         * not limited to could be produced:
         * 
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The queue does not exist on the server specified.
         *				</td></tr>
         * </table>	 
         * </returns>
         * 
         * <see cref="CloseQueue(QueueHandle)">CloseQueue(QueueHandle handle)</see>
         */
        public ErrorCode OpenQueue(string queuename, QueueHandle handle) {
		    ErrorCode ret;
		    QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	qOpenParam = new QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS();
            qOpenParam.queuename = queuename;
		    try {
			    output.Write(Safmq.CMD_QUEUE_OPEN);
			    qOpenParam.Write(output);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    handle.handle = input.ReadInt32();
			    }
		    } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
        }

        /**
         * <summary>
         * Closes a queue which was previously opened.
         * </summary>
         * 
         * <param name="handle"> Reference to the previously opened queue</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> on success otherwise errors such as these, but
         * not limited to could be produced:
         * 
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTOPEN</c></td>
         *				<td>The queue was not currently input an open status input the
         *					context of this connection.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode CloseQueue(QueueHandle handle) {
            ErrorCode ret = ErrorCode.EC_NOERROR;
		    QUEUE_CLOSE_OPEN_CURSOR_PARAMS	parms = new QUEUE_CLOSE_OPEN_CURSOR_PARAMS();
		    parms.queueID = handle;
    		
		    try {
			    output.Write(Safmq.CMD_QUEUE_CLOSE);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
		    } catch (Exception ) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

		/**
		 * <summary>Gets statistics about an open queue.  Places statistic information into 
		 * the parameter <c>stats</c>.</summary>
		 * 
		 * <param name="qhandle"> Handle to an open queue</param>
		 * <param name="includeStorageBytes"> Causes server to calculate the number of bytes on disk</param>
		 * <param name="includeMessageBytes"> Causes the server to calculate the number of bytes in queue, 
		 *		may be less than bytes on disk</param>
		 * <param name="stats"> Receives the statistics information.</param>
		 * <returns> Safmq.EC_NOERROR on success <br/>
		 * 	Safmq.EC_NETWORKERROR<br/>
		 *  Safmq.EC_NOTOPEN </returns>
		 */
		public ErrorCode GetQueueStatistics (QueueHandle qhandle, bool includeStorageBytes, bool includeMessageBytes, out QueueStatistics stats)
		{
			QUEUE_STATS_PARAMS parms = new QUEUE_STATS_PARAMS(qhandle, includeStorageBytes, includeMessageBytes);
			QUEUE_STATS_RESPONSE response = new QUEUE_STATS_RESPONSE();
			response.stats = stats = new QueueStatistics();
			try {
				output.Write(Safmq.CMD_QUEUE_STATS);
				parms.Write(output);
				output.Flush();
				response.Read(input);
			} catch (IOException ) {
				response.errorcode = ErrorCode.EC_NETWORKERROR;
			}
			
			return response.errorcode;
		}

		/**
		 * <summary>Gets statistics for an entire server</summary>
		 * 
		 * <param name="stats"> Receives the statistics information.</param>
		 * <returns> Safmq.EC_NOERROR on success <br/>
		 * 	Safmq.EC_NETWORKERROR<br/>
		 *  Safmq.EC_NOTOPEN </returns>
		 */
			public ErrorCode GetServerStatistics (out QueueStatistics stats) {
			QUEUE_STATS_RESPONSE response = new QUEUE_STATS_RESPONSE();
			response.stats = stats = new QueueStatistics();
			try {
				output.Write(Safmq.CMD_SERVER_STATS);
				output.Flush();
				response.Read(input);
			} catch (IOException) {
				response.errorcode = ErrorCode.EC_NETWORKERROR;
			}

			return response.errorcode;
		}

		/// <summary>
		/// Retrieves information about connected clients.
		/// </summary>
		/// <param name="clients">Fills the list with client information. </param>
		/// <returns>ErrorCode.EC_NOERROR, ErrorCode.EC_NETWORKERROR</returns>
		ErrorCode GetClientInfo(List<ClientInfo> clients)
		{
			CLIENT_INFO_RESPONSE resp = new CLIENT_INFO_RESPONSE();
			resp.clientInfo = clients;
			clients.Clear();

			try {
				output.Write(Safmq.CMD_CLIENT_INFO);
				output.Flush();
				resp.Read(input);
			} catch (IOException) {
				resp.errorcode = ErrorCode.EC_NETWORKERROR;
			}
			return (ErrorCode)resp.errorcode;
		}

        /**
         * <summary>
         * Enqueues a message for message relay forwarding to another queue/queue server.
         * </summary>
         * 
         * <remarks>
         * After completion, the return code will be EC_NOERROR on success and the <c>msg</c>'s 
         *  timestamp and message will have been set.  It is important to note if the resposne
         * queue name is not set, any errors from the final destination queue/queue server will
         * be lost.  It is suggested to use roundtripping unless error determination is not
         * required, otherwise errors generated by the final destination server such as:<br/>
         * <table border="0">
         * <tr><td>EC_DOESNOTEXIST</td></tr>
         * <tr><td>EC_NOTAUTHORIZED</td></tr>
         * <tr><td>EC_WRONGMESSAGETYPE</td></tr>
         * <tr><td>EC_NOTOPEN</td></tr>
         * <tr><td>EC_FORWARDNOTALLOWED</td></tr>
         * <tr><td>EC_DUPLICATEMSGID</td></tr>
         * </table>
         * will not returned to the client.
         * </remarks>
         * 
         * <param name="uri">   A safmq Uri in the format safmq://user:password@server:port/queuename -or- for ssl safmqs://user:password@server:port/queuename.<br/>
         *                      Note: the port specification is optional.</param>
         * <param name="msg">   The message to be sent.</param>
         * 
         * <returns>ErrorCode.EC_NOERROR on success<br/>
         * <table border="0">
         * <tr><td>ErrorCode.EC_INVALIDNAME</td><td>incase of an invalid url</td></tr>
         * <tr><td>ErrorCode.EC_NETWORKERROR</td></tr>
         * <tr><td>ErrorCode.EC_NOTAUTHORIZED</td></tr>
         * <tr><td>ErrorCode.EC_NOTOPEN</td></tr>
         * <tr><td>ErrorCode.EC_WRONGMESSAGETYPE</td></tr>
         * <tr><td>ErrorCode.EC_FORWARDNOTALLOWED</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode EnqueueWithRelay(Uri uri, QueueMessage msg) {
            ErrorCode ret;
            try {
                QueueHandle forward = new QueueHandle();
                string dest = "";
                string userinfo = uri.UserInfo;
                string user, password;

                user = password = "";

                ret = OpenQueue(FORWARD_QUEUE_NAME, forward);
                if (ret == ErrorCode.EC_NOERROR) {
                    StringBuilder u, p;

                    if (MQBuilder.parseUserInfo(userinfo, u = new StringBuilder(), p = new StringBuilder())) {
                        user = u.ToString();
                        password = p.ToString();
                    }

                    if (uri.Scheme.Length > 0)
                        dest = uri.Scheme + ":";
                    dest += "//";


                    if (user.Length > 0) {
                        dest += user;
                        if (password.Length > 0)
                            dest += ":" + password;
                        dest += "@";
                    }
                    dest += uri.Host;
                    if (uri.Port > 0)
                        dest += ":" + uri.Port;
                    dest += uri.AbsolutePath;

                    msg.Label = (dest + "?label=" + msg.Label);
                    ret = Enqueue(forward, msg);

                    ErrorCode ec = CloseQueue(forward);
                    if (ret == ErrorCode.EC_NOERROR && ec != ErrorCode.EC_NOERROR)
                        ret = ec;
                } else if (ret == ErrorCode.EC_DOESNOTEXIST) {
                    ret = ErrorCode.EC_FORWARDNOTALLOWED;
                }
            } catch (Exception) {
                ret = ErrorCode.EC_INVALIDNAME;
            }
            return ret;
        }

        /**
         * <summary>
         * Places a message on the queue.  The object <c>msg</c> must have
         * been previously prepared before calling this method.  Upon successfull sending
         * of the message, the message id and message timestamp will be set by the server
         * and may be retrieved by a call to <c>QueueMessage.getMessageID()</c>
         * and <c>QueueMessage.getTimeStamp()</c> respectively.
         * </summary>
         * <remarks>
         * <para>Note: Message responsders will typically place the message id of the original
         * message input the recipt id of the message being returned.  This round-trip
         * message identification is provided by SAFMQ as a method for coordinated two-way
         * communications.</para>
         * </remarks>
         * 
         * <param name="handle">	A reference to a queue opened by a call to <c>OpenQueue()</c> </param>
         * <param name="msg">		The message to be placed on the queue</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> on success, Otherwise results may be such listed
         * below but are not limited to:
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_ALREADYCLOSED</c></td>
         *				<td>The queue reference is not valid as it has 
         *					been closed.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The user is not authorized to Write messages to this
         *					queue.
         *				</td></tr>
         * </table>
         * </returns>
         *
         * <seealso cref="OpenQueue(string, QueueHandle)"/>
         */
        public ErrorCode Enqueue(QueueHandle handle, QueueMessage msg) {
		    ErrorCode ret = ErrorCode.EC_NETWORKERROR;
		    ENQUEUE_PARAMS	parms = new ENQUEUE_PARAMS();

		    parms.queueID = handle;
		    parms.msg = msg;

		    try {
			    output.Write(Safmq.CMD_ENQUEUE);
			    parms.Write(output);
			    output.Flush();

			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    ENQUEUE_RESPONSE_DATA	data = new ENQUEUE_RESPONSE_DATA();
				    data.msgID = msg.MessageID; // set the response data msgID to the msg.messageID so that it can be Read back
				    data.Read(input);
				    msg.TimeStamp = data.timestamp;
			    }
		    } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Retrieves the highest priority FIFO message present on the queue.  The results
         * are placed input the object <c>msg</c>.
         * </summary>
         * 
         * <remarks>
         * <para>Note: Retrieved messages are removed from the queue</para>
         * </remarks>
         * 
         * <param name="qhandle">		A handle to a queue previously opened by a call to OpenQueue(string)</param>
         * <param name="retrievebody">	A flag indicating whether the body of the message should be retrieved</param>
         * <param name="timeoutseconds">The number of seconds to wait before returning, a value of zero (0) will
         * 							    cause the method to return immediately if no messages are present on the queue,
         * 							    a value of (-1) will cause the method to wait until a message is placed on the queue.
         *                              </param>
         * <param name="msg">			Receives the contents of the message.</param>
         * 
         * <returns>Upon success a value of <c>ErrorCode.EC_NOERROR</c> is returned and
         * 			the resulting message is placed input <c>msg</c>, otherwise 
         * 			errors such as but not limited to could occur: <br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOMOREMESSAGES</c></td>
         *				<td>The queue is empty and no more messages are
         *					available.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_ALREADYCLOSED</c></td>
         *				<td>The queue reference is not valid as it has 
         *					been closed.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The user is not authorized to Read messages from this
         *					queue.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTOPEN</c></td>
         *				<td>The queue specified has not been opened by this connection.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_TIMEDOUT</c></td>
         *				<td>The operation timed output before a message became available.
         *				</td></tr>
         * </table>
         * </returns>
         * 
         * <seealso cref="OpenQueue(string, QueueHandle)"/>
         */
        public ErrorCode Retrieve(QueueHandle qhandle, bool retrievebody, int timeoutseconds, QueueMessage msg) {
		    ErrorCode	ret = ErrorCode.EC_NOERROR;

		    RETRIEVE_PEEK_FRONT_PARAMS	parms = new RETRIEVE_PEEK_FRONT_PARAMS();
		    parms.queueID = qhandle;
		    parms.retrievebody = (byte)(retrievebody ? 1 : 0);
		    parms.timeoutseconds = timeoutseconds;

		    try {
			    output.Write(Safmq.CMD_RETRIEVE);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    msg.Read(input, retrievebody);
				    ret = ackRetrieve(qhandle,msg);
			    }
		    } catch (IOException) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
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
         * <param name="qhandle">		A handle to a queue previously opened by a call to OpenQueue(string)</param>
         * <param name="retrievebody">	A flag indicating whether the body of the message should be retrieved</param>
         * <param name="id">			The UUID of the message to be retrieved.</param>
         * <param name="timeoutseconds">
         *                              The number of seconds to wait before returning, a value of zero (0) will
         * 						        cause the method to return immediately if no messages are present on the queue,
         * 						        a value of (-1) will cause the method to wait until a message is placed on the queue.</param>
         * <param name="msg">			Receives the contents of the message.</param>
         * 
         * <returns>Upon success a value of <c>ErrorCode.EC_NOERROR</c>, otherwise 
         * 			errors such as but not limited to could occur: <br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOMOREMESSAGES</c></td>
         *				<td>The queue is empty and no more messages are
         *					available.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_ALREADYCLOSED</c></td>
         *				<td>The queue reference is not valid as it has 
         *					been closed.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The user is not authorized to Read messages from this
         *					queue.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTOPEN</c></td>
         *				<td>The queue specified has not been opened by this connection.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_TIMEDOUT</c></td>
         *				<td>The operation timed out before a message became available.
         *				</td></tr>
         * </table>
         * </returns>
         * 
         * <seealso cref="Enqueue(QueueHandle, QueueMessage)"/>
         * <seealso cref="OpenQueue(string, QueueHandle)"/>
         */
        public ErrorCode RetrieveID(QueueHandle qhandle, bool retrievebody, UUID id, int timeoutseconds, QueueMessage msg) {
		    ErrorCode	ret = ErrorCode.EC_NOERROR;

		    RETRIEVE_ID_PEEK_ID_PARAMS	parms = new RETRIEVE_ID_PEEK_ID_PARAMS();
		    parms.queueID = qhandle;
		    parms.retrievebody = (byte)(retrievebody?1:0);
		    parms.reciptID = id;
		    parms.timeoutseconds = timeoutseconds;
    		
		    try {
			    output.Write(Safmq.CMD_RETRIEVE_ID);
			    parms.Write(output);output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    msg.Read(input,retrievebody);
				    ret = ackRetrieveCursor(qhandle,msg);
			    }
		    } catch (IOException) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Retrieves the message pointed to by <c>cursorID</c>. The results  
         * are placed in the object <c>msg</c>.
         * </summary>
         * 
         * <remarks>
         * <para>Note: Retrieved messages are removed from the queue.</para>
         * </remarks>
         * 
         * <param name="qhandle">		A handle to a queue previously opened by a call to OpenQueue(string)</param>
         * <param name="retrievebody">	A flag indicating whether the body of the message should be retrieved</param>
         * <param name="cursorID">		The cursor indicating the current position in the queue to be Read from</param>
         * <param name="msg">			Receives the contents of the message.</param>
         * 
         * <returns>Upon success a value of <c>ErrorCode.EC_NOERROR</c>, otherwise 
         * 			errors such as but not limited to could occur: <br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOMOREMESSAGES</c></td>
         *				<td>The queue is empty and no more messages are
         *					available.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_ALREADYCLOSED</c></td>
         *				<td>The queue reference is not valid as it has 
         *					been closed.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The user is not authorized to Read messages from this
         *					queue.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_CURSORINVALIDATED</c></td>
         *				<td>The cursor nolonger points to a valid location in the
         *					queue.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTOPEN</c></td>
         *				<td>The queue specified has not been opened by this connection.
         *				</td></tr>
         * </table>
         * </returns>
         * 
         * <seealso cref="OpenQueue(string, QueueHandle)"/>
         */
        public ErrorCode RetrieveCursor(QueueHandle qhandle, bool retrievebody, CursorHandle cursorID, QueueMessage msg) {
            ErrorCode ret = ErrorCode.EC_NOERROR;

		    PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS	parms = new PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS();
		    parms.queueID = qhandle;
		    parms.retrievebody = (byte)(retrievebody?1:0);
		    parms.cursorID = cursorID;

		    try {
			    output.Write(Safmq.CMD_RETRIEVE_CURSOR);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    msg.Read(input,retrievebody);
				    ret = ackRetrieveCursor(qhandle,msg);
			    }
		    } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Gathers the highest priority FIFO message present on the queue.  The results
         * are placed in the object <c>msg</c>.  Any errors from the operation are
         * returned on the stack.    The message retrieved is <i>not</i> remvoed from the 
         * queue and is available for reading by other queue readers.
         * </summary>
         * 
         * <param name="qhandle">		A handle to a queue previously opened by a call to OpenQueue(string)</param>
         * <param name="retrievebody">	A flag indicating whether the body of the message should be retrieved</param>
         * <param name="timeoutseconds">The number of seconds to wait before returning, a value of zero (0) will
         * 	        					cause the method to return immediately if no messages are present on the queue,
         * 			        			a value of (-1) will cause the method to wait until a message is placed on the queue.
         *                              </param>
         * <param name="msg">			Receives the contents of the message.</param>
         * 
         * <returns>Upon success a value of <c>ErrorCode.EC_NOERROR</c>, otherwise 
         * 			errors such as but not limited to could occur: <br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOMOREMESSAGES</c></td>
         *				<td>The queue is empty and no more messages are
         *					available.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_ALREADYCLOSED</c></td>
         *				<td>The queue reference is not valid as it has 
         *					been closed.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The user is not authorized to Read messages from this
         *					queue.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTOPEN</c></td>
         *				<td>The queue specified has not been opened by this connection.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_TIMEDOUT</c></td>
         *				<td>The operation timed out before a message became available.
         *				</td></tr>
         * </table>
         * </returns>
         * 
         * <seealso cref="OpenQueue(string, QueueHandle)"/>
         */
        public ErrorCode PeekFront(QueueHandle qhandle, bool retrievebody, int timeoutseconds, QueueMessage msg) {
		    ErrorCode ret = ErrorCode.EC_NOERROR;

		    RETRIEVE_PEEK_FRONT_PARAMS	parms = new RETRIEVE_PEEK_FRONT_PARAMS();
		    parms.queueID = qhandle;
		    parms.retrievebody = (byte)(retrievebody?1:0);
		    parms.timeoutseconds = timeoutseconds;

		    try {
			    output.Write(Safmq.CMD_PEEK_FRONT);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR)
				    msg.Read(input,retrievebody);
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
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
         * <param name="qhandle">		A handle to a queue previously opened by a call to OpenQueue(string)</param>
         * <param name="retrievebody">	A flag indicating whether the body of the message should be retrieved</param>
         * <param name="id">			The UUID of the message to be retrieved.</param>
         * <param name="timeoutseconds">The number of seconds to wait before returning, a value of zero (0) will
         * 						        cause the method to return immediately if no messages are present on the queue,
         * 						        a value of (-1) will cause the method to wait until a message is placed on the queue.
         *                              </param>
         * <param name="msg">			Receives the contents of the message.</param>
         * 
         * <returns>Upon success a value of <c>ErrorCode.EC_NOERROR</c>, otherwise 
         * 			errors such as but not limited to could occur: <br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOMOREMESSAGES</c></td>
         *				<td>The queue is empty and no more messages are
         *					available.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_ALREADYCLOSED</c></td>
         *				<td>The queue reference is not valid as it has 
         *					been closed.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The user is not authorized to Read messages from this
         *					queue.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTOPEN</c></td>
         *				<td>The queue specified has not been opened by this connection.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_TIMEDOUT</c></td>
         *				<td>The operation timed out before a message became available.
         *				</td></tr>
         * </table>
         * </returns>
         * 
         * <seealso cref="Enqueue(QueueHandle, QueueMessage)"/>
         * <seealso cref="OpenQueue(string, QueueHandle)"/>
         */
        public ErrorCode PeekID(QueueHandle qhandle, bool retrievebody, UUID id, int timeoutseconds, QueueMessage msg) {
		    ErrorCode ret = ErrorCode.EC_NOERROR;

		    RETRIEVE_ID_PEEK_ID_PARAMS	parms = new RETRIEVE_ID_PEEK_ID_PARAMS();
		    parms.queueID = qhandle;
		    parms.retrievebody = (byte)(retrievebody?1:0);
		    parms.reciptID = id;
		    parms.timeoutseconds = timeoutseconds;
    		
		    try {
			    output.Write(Safmq.CMD_PEEK_ID);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR)
				    msg.Read(input, retrievebody);
		    } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Retrieves the message pointed to by <c>cursorID</c>. The results  
         * are placed in the object <c>msg</c>.  The message retrieved is 
         * <i>not</i> remvoed from the queue and is available for reading by other 
         * queue readers.
         * </summary>
         * 
         * <remarks>
         * <para>Note: Retrieved messages are removed from the queue.</para>
         * </remarks>
         * 
         * <param name="qhandle">		A handle to a queue previously opened by a call to OpenQueue(string)</param>
         * <param name="retrievebody">	A flag indicating whether the body of the message should be retrieved</param>
         * <param name="cursorID">		The cursor indicating the current position in the queue to be Read from</param>
         * <param name="msg">			Receives the contents of the message.</param>
         * 
         * <returns>Upon success a value of <c>ErrorCode.EC_NOERROR</c>, otherwise 
         * 			errors such as but not limited to could occur: <br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOMOREMESSAGES</c></td>
         *				<td>The queue is empty and no more messages are
         *					available.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_ALREADYCLOSED</c></td>
         *				<td>The queue reference is not valid as it has 
         *					been closed.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The user is not authorized to Read messages from this
         *					queue.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTOPEN</c></td>
         *				<td>The queue specified has not been opened by this connection.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_CURSORINVALIDATED</c></td>
         *				<td>The cursor nolonger points to a valid location in the
         *					queue.
         *				</td></tr>
         * </table>
         * </returns>
         * 
         * <seealso cref="OpenQueue(string, QueueHandle)"/>
         */
        public ErrorCode PeekCursor(QueueHandle qhandle, bool retrievebody, CursorHandle cursorID, QueueMessage msg) {
		    ErrorCode ret = ErrorCode.EC_NOERROR;

		    PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS	parms = new PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS();
		    parms.queueID = qhandle;
		    parms.retrievebody = (byte)(retrievebody?1:0);
		    parms.cursorID = cursorID;

		    try {
			    output.Write(Safmq.CMD_PEEK_CURSOR);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    msg.Read(input,retrievebody);
			    }
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
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
         * <param name="qhandle">	A handle to a queue previously opened by a call to OpenQueue(string)</param>
         * <param name="cursorID"> 	A previously allocated reference to a <c>CursorHandle</c>
         *      					which will receive the reference to the cursor.</param>
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The user is not authorized to Read messages from this
         *					queue.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTOPEN</c></td>
         *				<td>The queue specified has not been opened by this connection.
         *				</td></tr>
         * </table>
         * </returns>
         *
         * <seealso cref="OpenQueue(string, QueueHandle)"/>
         */
        public ErrorCode OpenCursor(QueueHandle qhandle, CursorHandle cursorID) {
		    ErrorCode ret = ErrorCode.EC_NOERROR;
		    QUEUE_CLOSE_OPEN_CURSOR_PARAMS	parms = new QUEUE_CLOSE_OPEN_CURSOR_PARAMS();
		    parms.queueID = qhandle;
    		
		    try {
			    output.Write(Safmq.CMD_OPEN_CURSOR);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    cursorID.handle = input.ReadInt32();
			    }
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }

		    return ret;
	    }

        /**
         * <summary>
         * Closes a cursor when it is nolonger needed to access the queue.  The
         * cursor is invalidated after a call to <c>CloseCursor()</c> and
         * should not be used again.
         * </summary>
         * 
         * <param name="qhandle">   A handle to a queue previously opened by a call to OpenQueue(string)</param>
         * <param name="cursorID">  The cursor to be closed.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The user is not authorized to Read messages from this
         *					queue.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTOPEN</c></td>
         *				<td>The queue specified has not been opened by this connection.
         *				</td></tr>
         * </table>
         * </returns>
         * 
         * <seealso cref="OpenQueue(string, QueueHandle)"/>
         */
        public ErrorCode CloseCursor(QueueHandle qhandle, CursorHandle cursorID) {
		    ErrorCode ret = ErrorCode.EC_NOERROR;
		    CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS	parms = new CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS();
		    parms.queueID = qhandle;
		    parms.cursorID = cursorID;

		    try {
			    output.Write(Safmq.CMD_CLOSE_CURSOR);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Advances the cursor to the next message in the queue.
         * </summary>
         * 
         * <param name="qhandle">   A handle to a queue previously opened by a call to OpenQueue(string)</param>
         * <param name="cursorID">  The cursor to be advanced.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The user is not authorized to Read messages from this
         *					queue.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTOPEN</c></td>
         *				<td>The queue specified has not been opened by this connection.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_CURSORINVALIDATED</c></td>
         *				<td>The cursor is no longer valid.
         *				</td></tr>
         * </table>
         * </returns>
         * 
         * <seealso cref="OpenQueue(string, QueueHandle)"/>
         */
        public ErrorCode AdvanceCursor(QueueHandle qhandle, CursorHandle cursorID) {
		    ErrorCode ret = ErrorCode.EC_NOERROR;
		    CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS	parms = new CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS();
		    parms.queueID = qhandle;
		    parms.cursorID = cursorID;

		    try {
			    output.Write(Safmq.CMD_ADVANCE_CURSOR);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }

		    return ret;
	    }

        /**
         * <summary>
         * Attempts to locate the messaged indicated by <c>id</c>.  If the message
         * is succesfully located, the cursor will point to the message idenfified
         * and the message may be retrieved there after.
         * </summary>
         * 
         * <param name="qhandle">		A handle to a queue previously opened by a call to OpenQueue(string)</param>
         * <param name="id">		    The recipt UUID of the message to be located </param>
         * <param name="timeoutseconds">The maximum number of seconds the call should be allowed
         *           					before resulting in a "timeout".</param>
         * <param name="cursorID">	    The cursor which should be be assigned the position
         *           					of the message indicated by <c>id</c></param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The user is not authorized to Read messages from this
         *					queue.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTOPEN</c></td>
         *				<td>The queue specified has not been opened by this connection.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_CURSORINVALIDATED</c></td>
         *				<td>The cursor is no longer valid.
         *				</td></tr>
         * </table>
         * </returns>
         * 
         * <seealso cref="OpenQueue(string, QueueHandle)"/>
         */
        public ErrorCode SeekID(QueueHandle qhandle, UUID id, int timeoutseconds, CursorHandle cursorID) {
		    ErrorCode ret = ErrorCode.EC_NOERROR;
		    SEEK_ID_PARAMS	parms = new SEEK_ID_PARAMS();
		    parms.queueID = qhandle;
		    parms.cursorID = cursorID;
		    parms.reciptID = id;
		    parms.timeoutseconds = timeoutseconds;

		    try {
			    output.Write(Safmq.CMD_SEEK_ID);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Determines whether the cursor indicated by <c>cursorID</c> is still
         * a valid cursor, and has not been invalidated by other queue readers.
         * </summary>
         * 
         * <param name="qhandle">   A QueueHandle associated with the cursor</param>
         * <param name="cursorID">	The cursor which should be be evaluated</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTOPEN</c></td>
         *				<td>The queue specified has not been opened by this connection.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_CURSORINVALIDATED</c></td>
         *				<td>The cursor is no longer valid.
         *				</td></tr>
         * </table>
         * </returns>
         * 
         * <seealso cref="OpenQueue(string, QueueHandle)"/>
         */
        public ErrorCode TestCursor(QueueHandle qhandle, CursorHandle cursorID) {
		    ErrorCode ret = ErrorCode.EC_NOERROR;
		    CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS	parms = new CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS();
		    parms.queueID = qhandle;
		    parms.cursorID = cursorID;

		    try {
			    output.Write(Safmq.CMD_TEST_CURSOR);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }

		    return ret;
	    }

        /**
         * <summary>
         * Begins a transaction which causes all Retrieve &amp; Enqueue operations to be committed
         * atomically.
         * </summary>
         * 
         * <returns>
         * The the following but not limited to:<br/>
         * <table border="0">
         * <tr><td><c>ErrorCode.EC_NOERROR</c></td></tr>
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td></tr>
         * <tr><td><c>ErrorCode.EC_ALREADYOPEN</c></td><td>if a transaction is already open</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode BeginTransaction() {
		    try {
			    output.Write(Safmq.CMD_BEGIN_TRANSACTION);
			    output.Write((int)0);
			    output.Flush();
			    return getResponseCode();
            } catch (Exception) {
			    return ErrorCode.EC_NETWORKERROR;
		    }
	    }

        /**
         * <summary>
         * Commits a group of Retrieve &amp; Enqueue operations atomically.
         * </summary>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c>, <c>ErrorCode.EC_NETWORKERROR</c></returns>
         */
        public ErrorCode CommitTransaction() {
		    try {
			    output.Write(Safmq.CMD_COMMIT_TRANSACTION);
			    output.Write((int)0);
			    output.Flush();
			    return getResponseCode();
            } catch (Exception) {
			    return ErrorCode.EC_NETWORKERROR;
		    }
	    }

        /**
         * <summary>
         * Rolls back or "undoes" a group of Retrieve &amp; Enqueue operations atomically.
         * </summary>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c>, <c>ErrorCode.EC_NETWORKERROR</c></returns>
         */
        public ErrorCode RollbackTransaction() {
		    try {
			    output.Write(Safmq.CMD_ROLLBACK_TRANSACTION);
			    output.Write((int)0);
			    output.Flush();
			    return getResponseCode();
            } catch (Exception) {
			    return ErrorCode.EC_NETWORKERROR;
		    }
	    }

        /**
         * <summary>
         * Commits and closes a group of Retrieve &amp; Enqueue operations.
         * </summary>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c>, <c>ErrorCode.EC_NETWORKERROR</c>,
         * <c>ErrorCode.EC_NOTOPEN</c> in the case that a transaction was not open.
         * </returns>
         */
        public ErrorCode EndTransaction() {
		    try {
			    output.Write(Safmq.CMD_END_TRANSACTION);
                output.Write((int)0);
			    output.Flush();
			    return getResponseCode();
            } catch (Exception) {
			    return ErrorCode.EC_NETWORKERROR;
		    }
	    }

        /**
         * <summary>
         * Retrieves a list of all queues served by the connection and the queue's owner.
         * The passed <c>List&lt;string&gt; qnames</c> receives <c>MQConnection.QueueData</c>
         * objects to enumerate the queue.
         * </summary>
         * 
         * <param name="qnames">The List&lt;&gt; which will be emptied then populated with 
         *                      <c>MQconnection.QueueData</c> objects.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * </table>
         * </returns>
         * 
         * <seealso cref="QueueData"/>
         */
        public ErrorCode EnumerateQueues(List<QueueData> qnames) {
		    ErrorCode ret = ErrorCode.EC_NETWORKERROR;
            qnames.Clear();
		    try {
			    output.Write(Safmq.CMD_ENUM_QUEUES);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    int	nqueues = input.ReadInt32();
				    QUEUE_CONFIG_ENTRY	qent = new QUEUE_CONFIG_ENTRY();
				    for(int x=0; x < nqueues; x++) {
					    qent.Read(input);
                        qnames.Add(new QueueData(   new string(Encoding.UTF8.GetChars(qent.queuename, 0, ioutil.length(qent.queuename))), 
                                                    new string(Encoding.UTF8.GetChars(qent.owner, 0, ioutil.length(qent.owner)))));
				    }
			    }
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Retrieves a list of users which may access the SAFMQ server.  The result
         * user information is in the form of <c>MQConnection.UserDescription</c>
         * objects placed in the passed <c>List&lt;UserDescription&gt;, users</c>.
         * </summary>
         * 
         * <param name="users"> [out] The List which will be emptied then populated 
         *                          with <c>MQConnection.UserDescription</c> objects</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * </table>
         * </returns>
         * 
         * <seealso cref="UserDescription"/>
         */
        public ErrorCode EnumerateUsers(List<UserDescription> users) {
		    ErrorCode ret = ErrorCode.EC_NETWORKERROR;
    		
		    users.Clear();
		    try {
			    output.Write(Safmq.CMD_ENUM_USERS);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    int     nUsers = input.ReadInt32();
				    byte[]  username = new byte[Safmq.USER_NAME_LENGTH];
                    byte[]  description = new byte[Safmq.DESCRIPTION_LENGTH];

				    for(int x=0;x < nUsers; x++) {
                        input.Read(username, 0, username.Length);
					    input.Read(description, 0, description.Length);
                        users.Add(new UserDescription(new string(Encoding.UTF8.GetChars(username, 0, ioutil.length(username))), 
                                                        new string(Encoding.UTF8.GetChars(description, 0, ioutil.length(description)))));
				    }
			    }
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
    		
		    return ret;
	    }

        /**
         * <summary>
         * Retrieves a list of all groups available on the SAFMQ server.  The result
         * group information is in the form of <c>string</c> objects representing
         * the name of the group.
         * </summary>
         * 
         * <param name="groups">The List&lt;String&gt; which will be emptied then populated with
         * 					    <c>string</c> objects containing the names of the
         * 					    groups.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode EnumerateGroups(List<string> groups) {
		    ErrorCode ret = ErrorCode.EC_NETWORKERROR;
    		
		    groups.Clear();
		    try {
			    output.Write(Safmq.CMD_ENUM_GROUPS);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    int	nGroups = input.ReadInt32();
                    byte[] groupname = new byte[Safmq.GROUP_NAME_LENGTH];
				    for(int x=0; x < nGroups; x++) {
                        input.Read(groupname, 0, groupname.Length);
					    groups.Add(new string(Encoding.UTF8.GetChars(groupname,0,ioutil.length(groupname))));
				    } 
			    }
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }

		    return ret;
	    }

        /**
         * <summary>
         * Create a new user by providing a user name, password and description.
         * </summary>
         * 
         * <param name="username">		The name of the new user</param>
         * <param name="password">		The password for the new user, cannot be null</param>
         * <param name="description">	The description for the new user.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_ALREADYDEFINED</c></td>
         *				<td>A user with this name already exists
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to create users.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode CreateUser(string username, string password, string description) {
		    USER_CREATE_PARAMS	parms = new USER_CREATE_PARAMS();
		    ErrorCode		ret = ErrorCode.EC_NETWORKERROR;

            parms.username = username;
            parms.password = password;
            parms.description = description;

		    try {
			    output.Write(Safmq.CMD_USER_CREATE);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }

		    return ret;
	    }

        /**
         * <summary>
         * Delete a user from the SAFMQ server.
         * </summary>
         * 
         * <param name="username"> The name of the suer to delete.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to create users.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The user specified does not exist.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode DeleteUser(string username) {
		    USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS	parms = new USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS();
		    ErrorCode										ret = ErrorCode.EC_NETWORKERROR;

            parms.username = username;
		    try {
			    output.Write(Safmq.CMD_USER_DELETE);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Change the user's permissions to those specified.
         * </summary>
         * 
         * <param name="username">		Name of the user to change</param>
         * <param name="modifyqueues">	Allow the user to create queues</param>
         * <param name="modifyusers">	Allow the user to create and delete users and set permmsions</param>
         * <param name="modifygroups">	Allow the user to create, delete, and change group membership and permissions</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         *      could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to modify user permissions.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The specified user does not exist on the SAFMQ server.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode UserSetPermissions(string username, bool modifyqueues, bool modifyusers, bool modifygroups) {
		    USER_SET_PERMS_PARAMS	parms = new USER_SET_PERMS_PARAMS();
		    ErrorCode			ret = ErrorCode.EC_NETWORKERROR;
    		
		    parms.username = username;
		    parms.modify_queues = (byte)(modifyqueues?1:0);
		    parms.modify_users = (byte)(modifyusers?1:0);
		    parms.modify_groups = (byte)(modifygroups?1:0);
		    try {
			    output.Write(Safmq.CMD_USER_SET_PERMS);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Retrieve the permissions for a specific user.
         * </summary>
         * 
         * <param name="username">      The name of the user</param>
         * <param name="actorPerms">	Receives the permissions for the user, must not be null.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The specified user does not exist on the SAFMQ server, or the
         *					user has no permissions set.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode UserGetPermissions(string username, ActorPermissions actorPerms) {
		    USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS	parms =  new USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS();
		    ErrorCode										ret = ErrorCode.EC_NETWORKERROR;

            parms.username = username;
		    try {
			    output.Write(Safmq.CMD_USER_GET_PERMS);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data	perms = new USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data();
				    perms.Read(input);
				    actorPerms.ModifyQueues = (perms.modify_queues != 0);
				    actorPerms.ModifyUsers = (perms.modify_users != 0);
				    actorPerms.ModifyGroups = (perms.modify_groups != 0);
			    }
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Changes the password for the specified user.
         * </summary>
         * 
         * <param name="username">	Name of the user</param>
         * <param name="password">	New password for the user.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to create users.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The specified user does not exist on the SAFMQ server.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode SetPassword(string username, string password) {
		    ErrorCode 			ret = ErrorCode.EC_NETWORKERROR;
		    USER_SET_PASSWORD_PARAMS	parms = new USER_SET_PASSWORD_PARAMS();

            parms.username = username;
            parms.password = password;
		    try {
			    output.Write(Safmq.CMD_USER_SET_PASSWORD);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

		/**
		 * <summary>
		 * Adds an X509 digitital certificate identity mapping.  This method should be used
		 * to provide for a passwordless authentication using SSL and X509 digital certificates.
		 * </summary>
		 * 
		 * <remarks>
		 * <p><b>Example:</b> (the sample X509 certificate shipped with SAFMQ source)</p>
		 * <pre>
		 * Subject: C=US, ST=Nebraska, L=Omaha, O=SAFMQ, CN=Matthew Battey/emailAddress=mattbattey@users.sourceforge.net
		 * Issuer:  C=US, ST=Nebraska, L=Omaha, O=SAFMQ, CN=Matthew Battey/emailAddress=mattbattey@users.sourceforge.net
		 * </pre>
		 * 
		 * Thawte's Issuer DN:
		 * <pre>
		 * Issuer: C=ZA, ST=Western Cape, L=Cape Town, O=Thawte Consulting cc, OU=Certification Services Division, CN=Thawte Premium Server CA/emailAddress=premium-server@thawte.com
		 * </pre>
		 * 
		 * <p>In this case the issuer and subject are the same value, because the certificate
		 * shipped with SAFMQ is a "self-signed" certificate.  Typically the Subject DN will be set
		 * by you and the Issuer DN will be determined by the organization certifying the authenticity
		 * of the certificate's origin.</p>
		 * 
		 * <p><b>Note</b>: The Java implentation produces distinguished name strings in reverse
		 * order in comparison to the default output from tools like OpenSSL.  SAFMQ uses OpenSSL
		 * so these must be in the order that OpenSSL would report the value pair order.</p>
		 * </remarks>
		 * 
		 * <param name="subjectDN">[in] the Subject Distinguished Name of the certificate</param>
		 * <param name="issuerDN"> [in] the Issuer Distinguished Name of the certificate</param>
		 * <param name="username"> [in] the target SAFMQ login id (username)</param>
		 * 
		 * <returns>
		 * Returns the following, but is not limited to:
		 * <table>
		 * <tr><td>EC_NOERROR</td><td>upon success</td></tr>
		 * <tr><td>EC_DOESNOTEXIST</td><td>if the user ID does not exist</td></tr>
		 * <tr><td>EC_NOTAUTHORIZED</td></tr>
		 * <tr><td>EC_NETWORKERROR</td></tr>
		 * </table>
		 * </returns>
		*/
		public ErrorCode AddUserIdentity(string subjectDN, string issuerDN, string username)
		{
			ErrorCode ret = ErrorCode.EC_NETWORKERROR;
			ADD_USER_IDENTITY_PARAMS	parms = new ADD_USER_IDENTITY_PARAMS(subjectDN, issuerDN, username);
			
			try {
				output.Write(Safmq.CMD_USER_ADD_IDENT);
				parms.Write(output);
				output.Flush();
				ret = getResponseCode();
			} catch (Exception ) {
				ret = ErrorCode.EC_NETWORKERROR;
			}

			return ret;
		}

		/**
		 * <summary>
		 * Removes an X509 digitital certificate identity mapping.  This method should be used
		 * to provide for a passwordless authentication using SSL and X509 digital certificates.
		 * </summary>
		 * 
		 * <param name="subjectDN">[in] the Subject Distinguished Name of the certificate</param>
		 * <param name="issuerDN">[in] the Issuer Distinguished Name of the certificate</param>
		 * 
		 * <returns>
		 * Returns the following, but is not limited to:
		 * <table>
		 * <tr><td>EC_NOERROR</td><td>upon success</td></tr>
		 * <tr><td>EC_DOESNOTEXIST</td><td>if the user ID does not exist</td></tr>
		 * <tr><td>EC_NOTAUTHORIZED</td></tr>
		 * <tr><td>EC_NETWORKERROR</td></tr>
		 * </table>
		 * </returns>
		*/
		public ErrorCode RemoveUerIdentity(string subjectDN, string issuerDN)
		{
			ErrorCode ret = ErrorCode.EC_NETWORKERROR;
			REMOVE_USER_IDENTITY	parms = new REMOVE_USER_IDENTITY(subjectDN, issuerDN);
			
			try {
				output.Write(Safmq.CMD_USER_DEL_IDENT);
				parms.Write(output);
				output.Flush();
				ret = getResponseCode();
			} catch (Exception ) {
				ret = ErrorCode.EC_NETWORKERROR;
			}

			return ret;
		}

		/**
		 * <summary>
		 * Lists a set of X509 subject name and issuer names associated with the passed SAFMQ user login id.
		 * </summary>
		 * <param name="username">[in] The user name used to search for X509 identities.</param>
		 * <param name="ids">[output] This vector of safmq::X509Identity objects is populated upon successful return</param>
		 * <returns>
		 * Returns the following but is not limited to:
		 * <table>
		 * <tr><td>EC_NOERROR</td><td>on success</td></tr>
		 * <tr><td>EC_NETWORKERROR</td><td>on network issue</td></tr>
		 * </table>
		 * </returns>
		*/
		public ErrorCode EnumerateUserIdentities(string username, List<X509Identity> ids)
		{
			ErrorCode	ret = ErrorCode.EC_NETWORKERROR;
			ENUM_USER_IDENTITY	parms = new ENUM_USER_IDENTITY(username);

			ids.Clear();
			
			try {
				output.Write(Safmq.CMD_USER_ENUM_IDENT);
				parms.Write(output);
				output.Flush();
				ret = getResponseCode();
				if (ret == ErrorCode.EC_NOERROR) {
					int count = input.ReadInt32();
					byte[] subjectDN;
					byte[] issuerDN;
					for( ; count > 0; --count) {
						subjectDN = input.ReadBytes(input.ReadInt32());
						issuerDN = input.ReadBytes(input.ReadInt32());
						ids.Add(new X509Identity(Encoding.UTF8.GetString(subjectDN), Encoding.UTF8.GetString(issuerDN)));
					}
				}
			}  catch (Exception ) {
				ret = ErrorCode.EC_NETWORKERROR;
			}

			return ret;
		}

        /**
         * <summary>
         * Creates a new security group on the SAFMQ server.
         * </summary>
         * 
         * <param name="groupname"> Name of the group to create.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_ALREADYDEFINED</c></td>
         *				<td>A group with this name already exists
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to create groups.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode CreateGroup(string groupname) {
		    GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	parms = new GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS();
		    ErrorCode														ret = ErrorCode.EC_NETWORKERROR;

            parms.groupname = groupname;
		    try {
			    output.Write(Safmq.CMD_GROUP_CREATE);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Removes the security group from the SAFMQ server.
         * </summary>
         * 
         * <param name="groupname"> The name of the group</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to create groups.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The group specified does not exist.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode DeleteGroup(string groupname) {
		    GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	parms = new GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS();
		    ErrorCode														ret = ErrorCode.EC_NETWORKERROR;

            parms.groupname = groupname;
		    try {
			    output.Write(Safmq.CMD_GROUP_DELETE);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Changes the permissions for the specified group.
         * </summary>
         * 
         * <param name="groupname">		The name of the group</param>
         * <param name="modifyqueues">	Whether the group may create queues</param>
         * <param name="modifyusers">	Whether the group may create and delete users and set permissions</param>
         * <param name="modifygroups">	Whether the gorup may create, delete and change group membership and permissions</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to modify group permissions.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The specified group does not exist on the SAFMQ server.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode GroupSetPermissions(string groupname, bool modifyqueues, bool modifyusers, bool modifygroups) {
		    GROUP_SET_PERMS_PARAMS	parms = new GROUP_SET_PERMS_PARAMS();
		    ErrorCode			ret = ErrorCode.EC_NETWORKERROR;

		    parms.groupname = groupname;
		    parms.modify_queues = (byte)(modifyqueues?1:0);
		    parms.modify_users = (byte)(modifyusers?1:0);
		    parms.modify_groups = (byte)(modifygroups?1:0);
		    try {
			    output.Write(Safmq.CMD_GROUP_SET_PERMS);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Retrieves the permissions of a specific group.
         * </summary>
         * 
         * <param name="groupname">     The name of the group</param>
         * <param name="actorPerms">	Receives the permissions, must not be null</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The specified group does not exist on the SAFMQ server, or no
         *					permissions have been set for that group.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode GroupGetPermissions(string groupname, ActorPermissions actorPerms) {
		    GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	parms = new GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS();
		    ErrorCode														ret = ErrorCode.EC_NETWORKERROR;

		    parms.groupname = groupname;
		    try {
			    output.Write(Safmq.CMD_GROUP_GET_PERMS);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data	perms = new USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data();
				    perms.Read(input);
				    actorPerms.ModifyQueues = (perms.modify_queues != 0);
				    actorPerms.ModifyUsers = (perms.modify_users != 0);
				    actorPerms.ModifyGroups = (perms.modify_groups != 0);
			    }
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Adds a user to a group.
         * </summary>
         * 
         * <param name="groupname">	The name of the group</param>
         * <param name="username">	The name of the user</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to modify group permissions.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The specified group does not exist on the SAFMQ server.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode GroupAddUser(string groupname, string username) {
		    GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS	parms = new GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS();
		    ErrorCode							ret = ErrorCode.EC_NETWORKERROR;

		    parms.groupname = groupname;
		    parms.username = username;
		    try {
			    output.Write(Safmq.CMD_GROUP_ADD_USER);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Deletes a user from a group.
         * </summary>
         * 
         * <param name="groupname">	The name of the group</param>
         * <param name="username">	The name of the user</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to modify group permissions.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The specified group does not exist on the SAFMQ server.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode GroupDeleteUser(string groupname, string username) {
		    GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS	parms = new GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS();
		    ErrorCode							ret = ErrorCode.EC_NETWORKERROR;

		    parms.groupname = groupname;
		    parms.username = username;

		    try {
			    output.Write(Safmq.CMD_GROUP_DELETE_USER);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Retrieves a the list of users in a group.  The names of the users are
         * deposited into the <c>List&lt;string&gt; users</c> in the form of <c>string</c>
         * objects.
         * </summary>
         * 
         * <param name="groupname">	The name of the users.</param>
         * <param name="users">		A List receiving <c>string</c> objects containing
         *      					the user's name.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The specified group does not exist on the SAFMQ server.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode GroupGetUsers(string groupname, List<string> users) {
		    GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	parms = new GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS();
		    ErrorCode														ret = ErrorCode.EC_NETWORKERROR;

		    parms.groupname = groupname;
		    users.Clear();

		    try {
			    output.Write(Safmq.CMD_GROUP_GET_USERS);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    int	nUsers;
				    int	x;
                    byte[] username = new byte[Safmq.USER_NAME_LENGTH];

				    nUsers = input.ReadInt32();
				    for(x=0;x<nUsers;x++) {
					    input.Read(username, 0,username.Length);
					    users.Add(new string(Encoding.UTF8.GetChars(username,0,ioutil.length(username))));
				    }
			    }
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Retrieves a the list of groups the in which the user resides.  The names of the groups are
         * deposited into the <c>List&lt;string&gt; users</c> in the form of <c>string</c>
         * objects.
         * </summary>
         * 
         * <param name="username">	The name of the users.</param>
         * <param name="groups">	A List&lt;string&gt; receiving <c>string</c> objects containing
         *      					the group's name.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The specified user does not exist on the SAFMQ server.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode UserGetGroups(string username, List<string> groups) {
		    USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS	parms = new USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS();
		    ErrorCode										ret = ErrorCode.EC_NETWORKERROR;

		    parms.username = username;
		    groups.Clear();

		    try {
			    output.Write(Safmq.CMD_USER_GET_GROUPS);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    int	nGroups;
				    int	x;
                    byte[] groupname = new byte[Safmq.GROUP_NAME_LENGTH];

				    nGroups = input.ReadInt32();
				    for(x=0;x<nGroups; x++) {
					    input.Read(groupname, 0, groupname.Length);
					    groups.Add(new string(Encoding.UTF8.GetChars(groupname,0,ioutil.length(groupname))));
				    }
			    }
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Creates a new queue on the SAFMQ server.  The queue's owner is automatically
         * set to the current logged in user.
         * </summary>
         * 
         * <param name="queuename"> The name of the new queue to be created.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_ALREADYDEFINED</c></td>
         *				<td>A queue with this name already exists
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to create queues.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode CreateQueue(string queuename) {
		    ErrorCode ret = ErrorCode.EC_NETWORKERROR;
		    QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	parms = new QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS();

		    parms.queuename = queuename;
		    try {
			    output.Write(Safmq.CMD_QUEUE_CREATE);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

		/**
		 * <summary>
		 * Creates a new temporary message queue.  The queue is removed from the server
		 * once all clients close their handles to the queue, explicitly or implicity
		 * by disconnecting from the server.</summary>
		 * 
		 * <remarks>
		 * After calling <c>CreateTempQueue</c> the SAFMQ server has created a queue,
		 * flagged as temporary, returns the server generated name of the queue, and a handle
		 * to that queue.  Closing the queue removes it from the system, there is no need to 
		 * explicitly delete it.  Using the Closing technique, it is possible that the queue
		 * may remain open, because other clients have access to the queue.
		 * </remarks>
		 * 
		 * <param name="tmpQueueName"> [out] Receives the name of the temporary queue, relative to the server,
		 * 							when supplying this queue name to other clients, the server URL
		 * 							must be prepended to the name.  i.e. If the url
		 * 							safmq://localhost was used to open the connection use
		 * 							<c>"safmq://localhost/" + tmpQueue</c> when supplying
		 * 							the queue name to other clients. Must be an array of atleast
		 * 							one element, first element will receive the name.</param>
		 * 
		 * <param name="tmpQueue"> [out] Receives a handle to the open queue</param>
		 * 
		 * <returns>EC_NOERROR, EC_NETWORKERROR, EC_ALREADYDEFINED, EC_NOTAUTHORIZED</returns>
		*/
		public ErrorCode CreateTempQueue (out String tmpQueueName, QueueHandle tmpQueue) {
			QUEUE_CREATE_TEMP_RESPONSE resp = new QUEUE_CREATE_TEMP_RESPONSE();

			resp.errorcode = ErrorCode.EC_NETWORKERROR; // flag as a network error because the first failure will be writing to the server
			tmpQueueName = "";

			try {
				output.Write(Safmq.CMD_QUEUE_CREATE_TEMP);
				output.Flush();
				resp.Read(input);
				if (resp.errorcode == ErrorCode.EC_NOERROR) {
					tmpQueueName = resp.queueName;
					tmpQueue.handle = resp.queueID.handle;
				}
			} catch (IOException ) {
				resp.errorcode = ErrorCode.EC_NETWORKERROR;
			}
			return resp.errorcode;
		}

        /**
         * <summary>
         * Attempts to remove a queue fromt he SAFMQ server.  Queues may only be removed
         * if the user has the rights to do so and the queue is not currently in use.
         * </summary>
         * 
         * <param name="queuename"> The name of the queue to be removed.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_QUEUEOPEN</c></td>
         *				<td>The queue is currently in use by some connection.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to delete this queue.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode DeleteQueue(string queuename) {
		    ErrorCode													ret = ErrorCode.EC_NETWORKERROR;
		    QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	parms = new QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS();

		    parms.queuename = queuename;

		    try {
			    output.Write(Safmq.CMD_QUEUE_DELETE);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Attempts to set the security profile for a specified user.  When a queue
         * is first created only the queue's creator (owner) may alter the queue's
         * security.
         * </summary>
         * 
         * <param name="queuename">		Name of the queue</param>
         * <param name="username">		Name of the user</param>
         * <param name="rd">			User may Read from the queue</param>
         * <param name="wr">			User may Write to the queue</param>
         * <param name="destroy">		User may delete the queue</param>
         * <param name="changesecurity">User may alter security of the queue</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to change the permissions of the user.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode QueueSetUserPermission(string queuename, string username, bool rd, bool wr, bool destroy, bool changesecurity) {
		    ErrorCode				ret = ErrorCode.EC_NETWORKERROR;
		    QUEUE_SET_USER_PERMS_PARAMS	parms = new QUEUE_SET_USER_PERMS_PARAMS();

		    parms.queuename = queuename;
		    parms.username = username;
		    parms.read = (byte)(rd?1:0);
		    parms.write = (byte)(wr?1:0);
		    parms.destroy = (byte)(destroy?1:0);
		    parms.change_security = (byte)(changesecurity?1:0);
		    try {
			    output.Write(Safmq.CMD_QUEUE_SET_USER_PERMS);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Removes permissions for a specified user.
         * </summary>
         * 
         * <param name="queuename"> The name of the queuee</param>
         * <param name="username"> The name of the user </param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to change the permissions of the user.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The security record for that user does not exist.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode QueueDeleteUserPermission(string queuename, string username) {
		    ErrorCode				ret = ErrorCode.EC_NETWORKERROR;
		    QUEUE_DEL_USER_PERMS_PARAM	parms = new QUEUE_DEL_USER_PERMS_PARAM();

		    parms.queuename = queuename;
		    parms.username = username;
		    try {
			    output.Write(Safmq.CMD_QUEUE_DEL_USER_PERMS);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Attempts to set the security profile for a specified group.  When a queue
         * is first created only the queue's creator (owner) may alter the queue's
         * security.
         * </summary>
         * 
         * <param name="queuename">		Name of the queue</param>
         * <param name="groupname">		Name of the group</param>
         * <param name="rd">			Group may Read from the queue</param>
         * <param name="wr">			Group may Write to the queue</param>
         * <param name="destroy">		Group may delete the queue</param>
         * <param name="changesecurity">Group may alter security of the queue</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to change the permissions of the user.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode QueueSetGroupPermission(string queuename, string groupname, bool rd, bool wr, bool destroy, bool changesecurity) {
		    ErrorCode					ret = ErrorCode.EC_NETWORKERROR;
		    QUEUE_SET_GROUP_PERMS_PARAMS	parms = new QUEUE_SET_GROUP_PERMS_PARAMS();

		    parms.queuename = queuename;
		    parms.groupname = groupname;
		    parms.read = (byte)(rd?1:0);
		    parms.write = (byte)(wr?1:0);
		    parms.destroy = (byte)(destroy?1:0);
		    parms.change_security = (byte)(changesecurity?1:0);
		    try {
			    output.Write(Safmq.CMD_QUEUE_SET_GROUP_PERMS);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Removes permissions for a specified group.
         * </summary>
         * 
         * <param name="queuename"> The name of the queue</param>
         * <param name="groupname"> The name of the group </param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_NOTAUTHORIZED</c></td>
         *				<td>The current logged in user is not authorized to change the permissions of the group.
         *				</td></tr>
         * <tr><td><c>ErrorCode.EC_DOESNOTEXIST</c></td>
         *				<td>The security record for that group does not exist.
         *				</td></tr>
         * </table>
         * </returns>
         */
        public ErrorCode QueueDeleteGroupPermission(string queuename, string groupname) {
		    ErrorCode				ret = ErrorCode.EC_NETWORKERROR;
		    QUEUE_DEL_GROUP_PERMS_PARAM	parms = new QUEUE_DEL_GROUP_PERMS_PARAM();

		    parms.queuename = queuename;
		    parms.groupname = groupname;
		    try {
			    output.Write(Safmq.CMD_QUEUE_DEL_GROUP_PERMS);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Retrieves the permission profile for the specified queue.  Resultant permissions
         * are placed in the <c>List&lt;QueuePermissions&gt; perms</c> as <c>QueuePermissions</c>
         * objects.
         * </summary>
         * 
         * <param name="queuename"> The name of the queue.</param>
         * <param name="perms">		The vector which will be emptied and populated with <c>MQConnection.QueuePermissions</c> objects.</param>
         * 
         * <returns><c>ErrorCode.EC_NOERROR</c> upon success, otherwise errors returned
         * could be but are not limited to:<br/>
         * <table border="0" cellpadding="3" cellspacing="0">
         * <tr><td><c>ErrorCode.EC_NETWORKERROR</c></td>
         *				<td>A networking error has occurred and the conneciton
         *					is nolonger valid.
         *				</td></tr>
         * </table>
         * </returns>
         * 
         * <seealso cref="QueuePermissions"/>
         */
        public ErrorCode QueueEnumeratePermissions(string queuename, List<QueuePermissions> perms) {
		    ErrorCode													ret = ErrorCode.EC_NETWORKERROR;
		    QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	parms = new QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS();

		    parms.queuename = queuename;
		    perms.Clear();

		    try {
			    output.Write(Safmq.CMD_QUEUE_ENUM_PERMS);
			    parms.Write(output);
			    output.Flush();
			    ret = getResponseCode();
			    if (ret == ErrorCode.EC_NOERROR) {
				    int nPerms = input.ReadInt32();
				    int x;
				    QUEUE_PERM_DATA	data = new QUEUE_PERM_DATA();
				    perms.Clear();
				    for(x=0;x<nPerms; x++) {
					    data.Read(input);
					    perms.Add( new QueuePermissions(    data.entityname,
															data.isgroup!=0,
															data.read!=0,
															data.write!=0,
															data.destroy!=0,
															data.change_security!=0));
				    }
			    }
            } catch (Exception) {
			    ret = ErrorCode.EC_NETWORKERROR;
		    }
		    return ret;
	    }

        /**
         * <summary>
         * Closes the connection to the server.
         * </summary>
         */
        public ErrorCode Close() {
            try {
                server.Close();
            } catch (Exception) {
                return ErrorCode.EC_NETWORKERROR;
            }
            return ErrorCode.EC_NOERROR;
        }


        /**
         * <summary>
         * Internal method which acknowledges the reception of data from a "Retrieve" 
         * operation.  
         * </summary>
         *  
         * <param name="qhandle">	The queue being utilized.</param>
         * <param name="msg">		The message that was retrieved.</param>
         *  
         * <returns>The error response code form the server.</returns>
         * 
         * <exception cref="Exception">In the case of a network error.</exception>
         */
        ErrorCode ackRetrieve(QueueHandle qhandle, QueueMessage msg) {
		    RETRIEVE_ACK_PARAMS	parms = new RETRIEVE_ACK_PARAMS();
		    parms.queueID = qhandle;
		    parms.msgID = msg.MessageID;
    		
		    output.Write(Safmq.CMD_RETRIEVE_ACK);
		    parms.Write(output);
            output.Flush();
		    return getResponseCode();
	    }

        /**
         * <summary>
         * Internal method which acknowledges the reception of data from a "Retrieve"
         * operation.
         * </summary>
         * 
         * <param name="qhandle">	The queue being utilized</param>
         * <param name="msg">		The message that was retrieved</param>
         * 
         * <returns>The error response code from the server.</returns>
         * 
         * <exception cref="Exception">In the case of a network error.</exception>
         */
        ErrorCode ackRetrieveCursor(QueueHandle qhandle, QueueMessage msg) {
		    RETRIEVE_ACK_PARAMS	parms = new RETRIEVE_ACK_PARAMS();
		    parms.queueID = qhandle;
		    parms.msgID = msg.MessageID;

		    output.Write(Safmq.CMD_RETRIEVE_CURSOR_ACK);
		    parms.Write(output);
            output.Flush();
		    return getResponseCode();
	    }


        /**
         * <summary>
         * Retrieves the response code from the stream.
         * </summary>
         * 
         * <returns>the response code from the stream.</returns>
         */
        ErrorCode getResponseCode() {
            try {
                return (ErrorCode)input.ReadInt32();
            } catch (Exception) {
                return ErrorCode.EC_NETWORKERROR;
            }
        }
    }


    /**
     * <summary>
     * Result data from a call to <c>MQConnection.EnumerateUsers(List&lt;UserDescription&gt;)</c>.
     * Instances of this class are placed into the vector passed to 
     * <c>EnumerateUsers()</c> to detail the specifics of the user.
     * </summary>
     * <remarks>
     * @author matt
     * </remarks>
     */
    public class UserDescription {
        private string name, description;

        /**
         * <summary>
         * Provides the name of the user
         * </summary>
         */
        public string Name {
            get { return name; }
        }
        /**
         * <summary>
         * Provides the description of the user
         * </summary>
         */
        public string Description {
            get { return description; }
        }

        /**
         * <summary>
         * Constructs the object from the user's name and description.
         * </summary>
         * 
         * <param name="name">			The name of the user</param>
         * <param name="description">	The description of the user</param>
         */
        internal UserDescription(string name, string description) {
            this.name = name;
            this.description = description;
        }

        /// <summary>
        /// Compares the <c>UserDescription</c> to the passed object reference.
        /// </summary>
        /// <param name="o">The object to compare to this instance.</param>
        /// <returns>true if the "o" is equivalent to this instance.</returns>
        public override bool Equals(Object o) {
			return o != null && (this==o || ((o is UserDescription) && ((UserDescription)o).Name.Equals(Name)) || o.Equals(Name));
		}

        /// <summary>
        /// Provides a string representation of the <c>UserDescription</c>
        /// </summary>
        /// <returns>Provides a string representation of the <c>UserDescription</c></returns>
        public override string ToString() {
            return Name;
        }

        /// <summary>
        /// Provides a has code for the name of the user.
        /// </summary>
        /// <returns>Hash code for the <c>Name</c> property</returns>
        public override int GetHashCode() {
            return Name.GetHashCode();
        }
    }

    /**
     * <summary>
     * Result data from a call to <c>MQConnection.EnumerateQueues(List&lt;QueueData&gt;)</c>.
     * Instances of this class are placed into the vector passed to 
     * <c>EnumerateQueues()</c> to detail the specifics of the queue.
     * </summary>
     * 
     * <remarks>
     * @author matt
     * </remarks>
     * 
     * <seealso cref="MQConnection.EnumerateQueues(List&lt;QueueData&gt;)"/>
     */
    public class QueueData {
        private string name = "", owner = "";

        /**
         * <summary>
         * Provides the name of the queue
         * </summary>
         */
        public string Name {
            get { return name; }
        }
        /**
         * <summary>
         * Provides the name of the owner of the queue
         * </summary>
         */
        public string Owner {
            get { return owner; }
        }

        /**
         * <summary>
         * Constructs the QueueData object from the queue's name and owner.
         * </summary>
         * 
         * <param name="name">  The name of the queue.</param>
         * <param name="owner">	The name of the queue's owner.</param>
         */
        internal QueueData(string name, string owner) {
            this.name = name;
            this.owner = owner;
        }
     }

     /**
      * <summary>
      * Contains an entity's permisison when accessing a queue.
      * 
      * </summary>
      * <remarks>
      * @author Matt
      * </remarks>
      */
     public class QueuePermissions {
        private string entityName;
        private bool isGroup, read, write, destroy, changeSecurity;

        /**
         * <summary>
         * Provides the name of the entity these permissions pertain to.
         * </summary>
         */
        public string EntityName { 
            get { return entityName; }
            //internal set { entityName = value; }
        }
        /**
         * <summary>
         * Provids a flag whether this entity is a group.
         * </summary>
         */
        public bool IsGroup { 
            get { return isGroup; }
        }

        /**
         * <summary>
         * Provides a flag whether this entity is allowed to Read from the queue.
         * </summary>
         */
        public bool Read {
            get { return read; }
        }
        /**
         * <summary>
         * Provides a flag whether this entity is allowed to Write to the queue.
         * </summary>
         */
        public bool Write {
            get { return write; }
        }
        /**
         * <summary>
         * Provides the flag whether this entity may remove the queue from the server.
         * </summary>
         */
        public bool Destroy {
            get { return destroy; }
        }
        /**
         * <summary>
         * Provides a flag whether the entity is allowed to change the security for the queue.
         * </summary>
         */
        public bool ChangeSecurity {
            get { return changeSecurity; }
        }

        /**
         * <summary>
         * Constructs the QueuePermissions from the name of the entity and 
         * flags regarding its relationsihp with the queried queue.  This object
         * is used as a result to 
         * <c>MQConnection.queueEnumeratePermissions(string,List&lt;QueuePermissions&gt;)</c>.
         * </summary>
         * 
         * <param name="entityName">	The name of the entity these permissions pertain to</param>
         * <param name="isgroup">		A flag indicating this entity is a group (otherwise 
         *      						a user)</param>
         * <param name="Read">			A flag indicating whether this entity may Read from 
         *      						the queue</param>
         * <param name="Write">			A flag indicating whether this entity may Write to 
         *      						the queue</param>
         * <param name="destroy">		A flag indicating whether this entity may remove the 
         * 					        	queue from the server</param>
         * <param name="changeSecurity">A flag indicating whether this entity may change 
         *  							the security permissions of the queue</param>
         */
        internal QueuePermissions(string entityName, bool isgroup, bool Read, bool Write, bool destroy, bool changeSecurity) {
            this.entityName = entityName;
            this.isGroup = isgroup;
            this.read = Read;
            this.write = Write;
            this.destroy = destroy;
            this.changeSecurity = changeSecurity;
        }
    }
}

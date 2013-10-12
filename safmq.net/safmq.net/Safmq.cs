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


Created on Aug 9, 2007

*/
using System;
using System.Collections.Generic;
using System.Text;

namespace safmq.net
{
    /**
     * Defnitions of constants used within SAFMQ
     */
    internal class Safmq {
   	    /**
	    The Current SAFMQ Protocol Major Version.  Note: Major Version will change when a 
	    previously established message buffer changes.
	    */
        internal const int SAFMQ_PROTOCOL_MAJOR_VERSION = 1;

	    /**
	    The Current SAFMQ Protocol Minor Version.  Note: Minor Version will change when
	    a new message buffer is added.
	    */
        internal const int SAFMQ_PROTOCOL_MINOR_VERSION = 1;

       	/**
	     * The size of an int basic type
	     */
        internal const int SIZE_INT = 4;
	    /**
	     * The size of a short basic type
	     */
	    internal const int SIZE_SHORT = 2;

	    internal const int	MSGLBL_LENGTH		= 256,
						    QNAME_LENGTH		= 1024,
						    GROUP_NAME_LENGTH	= 20,
						    USER_NAME_LENGTH	= 20,
						    PASSWORD_LENGTH		= 20,
						    DESCRIPTION_LENGTH	= 100;
        
        internal const byte
                        CMD_LOGIN 					= 0,
						CMD_ENUM_QUEUES				= 1,
						CMD_SERVER_STATS			= 2,
						CMD_CLIENT_INFO				= 3,
														
						CMD_USER_CREATE				= 10,
						CMD_USER_DELETE				= 11,
						CMD_ENUM_USERS				= 12,
						CMD_USER_SET_PERMS			= 13,
						CMD_USER_GET_PERMS			= 14,
						CMD_USER_GET_GROUPS			= 15,
						CMD_USER_SET_PASSWORD		= 16,
						CMD_USER_ADD_IDENT = 17,
						CMD_USER_DEL_IDENT = 18,
						CMD_USER_ENUM_IDENT = 19,

													
						CMD_GROUP_CREATE			= 20,
						CMD_GROUP_DELETE			= 21,
						CMD_ENUM_GROUPS				= 22,
												
						CMD_GROUP_ADD_USER			= 30,
						CMD_GROUP_DELETE_USER		= 31,
						CMD_GROUP_GET_USERS			= 32,
						CMD_GROUP_SET_PERMS			= 33,
						CMD_GROUP_GET_PERMS			= 34,
												
						CMD_QUEUE_CREATE			= 40,
						CMD_QUEUE_DELETE			= 41,
						CMD_QUEUE_SET_USER_PERMS	= 42,
						CMD_QUEUE_DEL_USER_PERMS	= 43,
						CMD_QUEUE_SET_GROUP_PERMS	= 44,
						CMD_QUEUE_DEL_GROUP_PERMS	= 45,
						CMD_QUEUE_ENUM_PERMS		= 46,
						CMD_QUEUE_CREATE_TEMP		= 47,
						CMD_QUEUE_STATS				= 48,
												
						CMD_QUEUE_OPEN				= 50,
						CMD_QUEUE_CLOSE				= 51,
					
						CMD_ENQUEUE					= 70,
						CMD_RETRIEVE				= 71,
						CMD_RETRIEVE_ACK			= 72,
						CMD_RETRIEVE_ID				= 73,
						CMD_PEEK_ID					= 74,
						CMD_PEEK_FRONT				= 75,
						CMD_RETRIEVE_CURSOR_ACK 	= 76,
					
						CMD_OPEN_CURSOR				= 80,
						CMD_CLOSE_CURSOR			= 81,
						CMD_ADVANCE_CURSOR			= 82,
						CMD_SEEK_ID					= 83,
						CMD_TEST_CURSOR				= 84,
						CMD_PEEK_CURSOR				= 85,
						CMD_RETRIEVE_CURSOR			= 86,
						
						CMD_BEGIN_TRANSACTION		= 100,
						CMD_COMMIT_TRANSACTION		= 101,
						CMD_ROLLBACK_TRANSACTION	= 102,
						CMD_END_TRANSACTION			= 103;
    }

    /// <summary>An enumeration of error codes</summary>
    public enum ErrorCode {
        /// <summary>No error has occured</summary>
        EC_NOERROR = 0,
        /// <summary>The operation timed out</summary>
        EC_TIMEDOUT,
        /// <summary>The user is not authorized to take the action</summary>
        EC_NOTAUTHORIZED,
        /// <summary>The resource is already closed</summary>
        EC_ALREADYCLOSED,
        /// <summary>The resource does not exist</summary>
        EC_DOESNOTEXIST,
        /// <summary>The resource has not been opened</summary>
        EC_NOTOPEN,
        /// <summary>A network error occured</summary>
        EC_NETWORKERROR,
        /// <summary>The Time to Live for a message has expired</summary>
        EC_TTLEXPIRED,
        /// <summary>The cursor has been invalidated</summary>
        EC_CURSORINVALIDATED,
        /// <summary>The message contains a duplicate message ID</summary>
        EC_DUPLICATEMSGID,
        /// <summary>The server is unavailable</summary>
        EC_SERVERUNAVAIL,
        /// <summary>No more messages remain</summary>
        EC_NOMOREMESSAGES,
        /// <summary>A file system error has occured</summary>
        EC_FILESYSTEMERROR,
        /// <summary>User cannot change the group</summary>
        EC_CANNOTCHANGEGROUP,
        /// <summary>User cannot change the user</summary>
        EC_CANNOTCHANGEUSER,
        /// <summary>The resource is already defined</summary>
        EC_ALREADYDEFINED,
        /// <summary>The operation is not implemented</summary>
        EC_NOTIMPLEMENTED,
        /// <summary>The queue is currently open by another connection</summary>
        EC_QUEUEOPEN,
        /// <summary>The user is not logged in</summary>
        EC_NOTLOGGEDIN,
        /// <summary>Invalid name for a resource</summary>
        EC_INVALIDNAME,
        /// <summary>System does not allow message forwarding (relaying)</summary>
        EC_FORWARDNOTALLOWED,
        /// <summary>Wronge type for message</summary>
        EC_WRONGMESSAGETYPE,
        /// <summary>Wrong Protocol Version</summary>
        EC_UNSUPPORTED_PROTOCOL,
        /// <summary>Transaction already open</summary>
        EC_ALREADYOPEN,

        /// <summary>Undefined error</summary>
        EC_ERROR = 10000,
    }

    /// <summary>An enumeration of message body types</summary>
    public enum BodyTypeEnum {
        /// <summary>Body Type containing a 4 byte long integer</summary>
        BT_LONG,
        /// <summary>Body Type containing a 2 byte short integer</summary>
        BT_SHORT,
        /// <summary>Body type containing a 1 byte integer</summary>
        BT_CHAR,
        /// <summary>Body type containging single byte text data</summary>
        BT_TEXT,
        /// <summary>Body type containg multi byte text data</summary>
        BT_WTEXT,
        /// <summary>Body type containging binary data</summary>
        BT_BINARY,
        /// <summary>Body type containg no data</summary>
        BT_NONE
    }

    /// <summary>An enumeration of message priorities</summary>
    public enum MessagePriorityEnum {
        /// <summary>Standard message priority</summary>
        MP_STANDARD = 0,
        /// <summary>Low message prority</summary>
        MP_LOW = 0,
        /// <summary>Medium-Low message prority</summary>
        MP_MEDIUMLOW = 1,
        /// <summary>Medium message prority</summary>
        MP_MEDIUM = 2,
        /// <summary>Medium-High message prority</summary>
        MP_MEDIUMHIGH = 3,
        /// <summary>High message prority</summary>
        MP_HIGH = 4,
        /// <summary>Highest message prority</summary>
        MP_HIGHEST = 5
    }

    ///<summary>An enumeration of message classes</summary>
    public enum MessageClassEnum {
        /// <summary>System error message class</summary>
        MC_SYSTEMERRMSG = -1,
        /// <summary>User message class</summary>
        MC_USERMSG = 0
    }

	/// <summary>
	/// Set of queue performance statistics.  When queried, all fields except
	/// <c>storageBytes</c> and <c>messageBytes</c> are returned.
	/// The byte count fields (<c>storageBytes</c> and <c>messageBytes</c>)
	/// require sequential access to the queue and/or file system and thus
	/// cause a performance penalty to acquire.
	/// </summary>
	public class QueueStatistics
	{
		/// <summary> Number of messages in a queue </summary>
		public long messageCount = 0;
		/// <summary> Number of bytes on disk, includes purged messages still on disk </summary>
		public long storageBytes = 0;
		/// <summary> Number of bytes in the queue, does not included purged messages </summary>
		public long messageBytes = 0;

		/// <summary> Number of messages enqueued in the last 10 seconds </summary>
		public int enqueued10second = 0;
		/// <summary> Number of messages enqueued in the last 60 seconds </summary>
		public int enqueued60second = 0;
		/// <summary> Number of messages enqueued in the last 300 seconds </summary>
		public int enqueued300second = 0;

		/// <summary> Number of messages retrieved in the last 10 seconds </summary>
		public int retrieved10second = 0;
		/// <summary> Number of messages retrieved in the last 60 seconds </summary>
		public int retrieved60second = 0;
		/// <summary> Number of messages retrieved in the last 300 seconds </summary>
		public int retrieved300second = 0;

		/// <summary> Number of messages "peeked" in the last 10 seconds </summary>
		public int peeked10second = 0;
		/// <summary> Number of messages "peeked" in the last 60 seconds </summary>
		public int peeked60second = 0;
		/// <summary> Number of messages "peeked" in the last 300 seconds </summary>
		public int peeked300second = 0;
	}

	/// <summary>
	/// Details about a client connection
	/// </summary>
	public class ClientInfo {
		/// <summary>
		/// User Name for the client connection
		/// </summary>
		public string	username;
		/// <summary>
		/// Client endpoint IP address
		/// </summary>
		public int		address;
		/// <summary>
		/// Client endpoint Port
		/// </summary>
		public short	port;
	}	
}

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
 * Defines constants for use in the <code>com.safmq</code> package.
 * 
 * @author Matt
 */
public class Safmq {
	/**
	The Current SAFMQ Protocol Major Version.  Note: Major Version will change when a 
	previously established message buffer changes.
	*/
	public final static int SAFMQ_PROTOCOL_MAJOR_VERSION = 1;

	/**
	The Current SAFMQ Protocol Minor Version.  Note: Minor Version will change when
	a new message buffer is added.
	*/
	public final static int SAFMQ_PROTOCOL_MINOR_VERSION = 2;
	
	/**
	 * The size of an int basic type
	 */
	public final static int SIZE_INT = 4;
	/**
	 * The size of a short basic type
	 */
	public final static int SIZE_SHORT = 2;
	
	/**
	 * System generated messages
	 */
	public static final int		MC_SYSTEMERRMSG = -1;
	/**
	 * User generated messges (all messages not system generated)
	 */
	public static final int		MC_USERMSG = 0;

	/**
	 * Body contains a single "int" integer
	 */
	public static final byte	BT_LONG		= 0;
	/**
	 * Body contains a single "short" integer
	 */
	public static final byte	BT_SHORT	= 1;
	/**
	 * Body contains a single char (byte)
	 */
	public static final byte	BT_CHAR		= 2;
	/**
	 * Body contains text (single byte characters)
	 */
	public static final byte	BT_TEXT		= 3;
	/**
	 * Body contains text (wide characters)
	 */
	public static final byte	BT_WTEXT	= 4;
	/**
	 * Body is binary
	 */
	public static final byte	BT_BINARY	= 5;
	/**
	 * No body typeing.
	 */
	public static final byte	BT_NONE		= 6;

	/**
	 * Messages with standard priority.
	 */
	public static final byte	MP_STANDARD = 0;
	/**
	 * Messages with low priority, same as standard priority.
	 */
	public static final byte	MP_LOW = 0;
	/**
	 * Messages with medium-low priority.
	 */
	public static final byte	MP_MEDIUMLOW=1;
	/**
	 * Messages with medium priority.
	 */
	public static final byte	MP_MEDIUM=2;
	/**
	 * Messages with medium-high priority.
	 */
	public static final byte	MP_MEDIUMHIGH=3;
	/**
	 * Messages with high priority.
	 */
	public static final byte	MP_HIGH=4;
	/**
	 * Messages with higest priority.
	 */
	public static final byte	MP_HIGHEST=5;
	
	
	/**
	 * No error occurd.
	 */
	public static final int		EC_NOERROR 				= 0;
	/**
	 * Operation timed out.
	 */
	public static final int		EC_TIMEDOUT				= 1;
	/**
	 * User not authorized to perform operation.
	 */
	public static final int		EC_NOTAUTHORIZED		= 2;
	/**
	 * Resource has already been closed
	 */
	public static final int		EC_ALREADYCLOSED		= 3;
	/**
	 * Resource does not exist.
	 */
	public static final int		EC_DOESNOTEXIST			= 4;
	/**
	 * Resource is not currently open.
	 */
	public static final int		EC_NOTOPEN				= 5;
	/**
	 * A network error has occured
	 */
	public static final int		EC_NETWORKERROR			= 6;
	/**
	 * The message's TTL has expired
	 */
	public static final int		EC_TTLEXPIRED			= 7;
	/**
	 * The cursor has become invalid
	 */
	public static final int		EC_CURSORINVALIDATED	= 8;
	/**
	 * The message id supplied is a duplicate of another message ID
	 */
	public static final int		EC_DUPLICATEMSGID		= 9;
	/**
	 * The server is unavilable
	 */
	public static final int		EC_SERVERUNAVAIL		= 10;
	/**
	 * No more messages are available.
	 */
	public static final int		EC_NOMOREMESSAGES		= 11;
	/**
	 * A file system error has occured
	 */
	public static final int		EC_FILESYSTEMERROR		= 12;
	/**
	 * User cannot change group permissions
	 */
	public static final int		EC_CANNOTCHANGEGROUP	= 13;
	/**
	 * User cannot change user permissions
	 */
	public static final int		EC_CANNOTCHANGEUSER		= 14;
	/**
	 * Named resource already exists with the same name
	 */
	public static final int		EC_ALREADYDEFINED		= 15;
	/**
	 * Request not implemented
	 */
	public static final int		EC_NOTIMPLEMENTED		= 16;
	/**
	 * Could not delete queue, it is currently in use
	 */
	public static final int		EC_QUEUEOPEN			= 17;
	/**
	 * The user is not logged into the SAFMQ server
	 */
	public static final int		EC_NOTLOGGEDIN			= 18;
	/**
	 * Name supplied is invalid
	 */
	public static final int		EC_INVALIDNAME			= 19;

	/**
	 *  System does not allow message forwarding (relaying)
	 */
	public static final int		EC_FORWARDNOTALLOWED	= 20;
	/**
	 *  Wronge type for message
	 */
	public static final int		EC_WRONGMESSAGETYPE		= 21;
	/**
	 * Wrong Protocol Version
	 */
	public static final int		EC_UNSUPPORTED_PROTOCOL	= 22;

	public static final int		EC_ALREADYOPEN			= 23;
	/**
	 * Unknow error has occured.
	 */
	public static final int		EC_ERROR 				= 10000;
	
	public static String ecDecode(int error) {
		switch(error) {
			case Safmq.EC_NOERROR : return "EC_NOERROR"; 
			case Safmq.EC_TIMEDOUT : return "EC_TIMEDOUT";
			case Safmq.EC_NOTAUTHORIZED : return "EC_NOTAUTHORIZED";
			case Safmq.EC_ALREADYCLOSED: return "EC_ALREADYCLOSED";
			case Safmq.EC_DOESNOTEXIST: return "EC_DOESNOTEXIST";
			case Safmq.EC_NOTOPEN: return "EC_NOTOPEN";
			case Safmq.EC_NETWORKERROR: return "EC_NETWORKERROR";
			case Safmq.EC_TTLEXPIRED: return "EC_TTLEXPIRED";
			case Safmq.EC_CURSORINVALIDATED: return "EC_CURSORINVALIDATED";
			case Safmq.EC_DUPLICATEMSGID: return "EC_DUPLICATEMSGID";
			case Safmq.EC_SERVERUNAVAIL: return "EC_SERVERUNAVAIL";
			case Safmq.EC_NOMOREMESSAGES: return "EC_NOMOREMESSAGES";
			case Safmq.EC_FILESYSTEMERROR: return "EC_FILESYSTEMERROR";
			case Safmq.EC_CANNOTCHANGEGROUP: return "EC_CANNOTCHANGEGROUP";
			case Safmq.EC_CANNOTCHANGEUSER: return "EC_CANNOTCHANGEUSER";
			case Safmq.EC_ALREADYDEFINED: return "EC_ALREADYDEFINED";
			case Safmq.EC_NOTIMPLEMENTED: return "EC_NOTIMPLEMENTED";
			case Safmq.EC_QUEUEOPEN: return "EC_QUEUEOPEN";
			case Safmq.EC_NOTLOGGEDIN: return "EC_NOTLOGGEDIN";
			case Safmq.EC_INVALIDNAME: return "EC_INVALIDNAME";
			case Safmq.EC_FORWARDNOTALLOWED: return "EC_FORWARDNOTALLOWED";
			case Safmq.EC_WRONGMESSAGETYPE: return "EC_WRONGMESSAGETYPE";
			case Safmq.EC_UNSUPPORTED_PROTOCOL: return "EC_UNSUPPORTED_PROTOCOL";
			case Safmq.EC_ALREADYOPEN: return "EC_ALREADYOPEN";
			case Safmq.EC_ERROR: return "EC_ERROR";
			default: return "UNKNOWN";
		}	
	}
	
	static final int	MSGLBL_LENGTH		= 256,
						QNAME_LENGTH		= 1024,
						GROUP_NAME_LENGTH	= 20,
						USER_NAME_LENGTH	= 20,
						PASSWORD_LENGTH		= 20,
						DESCRIPTION_LENGTH	= 100;
	
	
	
	static final byte 	CMD_LOGIN 					= 0,
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
						CMD_USER_ADD_IDENT			= 17,
						CMD_USER_DEL_IDENT			= 18,
						CMD_USER_ENUM_IDENT			= 19,
													
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
								
	/**
	 * This method provides a textual description of an error code generated
	 * by SAFMQ.
	 * 
	 * @param error The error code retrieved from a call to an MQConnection method
	 * 
	 * @return A textual description of the error code
	 */						
	public final static String errorDecode(int error) {
		switch (error) {
			case EC_NOERROR:
				return "No error Occured";
			case EC_TIMEDOUT:
				return "The operation timed out";
			case EC_NOTAUTHORIZED:
				return "The current user is not authorized to perform the action";
			case EC_ALREADYCLOSED:
				return "The resourse is already closed";
			case EC_DOESNOTEXIST:
				return "The resourse does not exist";
			case EC_NOTOPEN:
				return "The resource is not open";
			case EC_NETWORKERROR:
				return "A network communications error occured";
			case EC_TTLEXPIRED:
				return "The message time to live has expired";
			case EC_CURSORINVALIDATED:
				return "The supplied cursor has been invalidated because the message it pointed to was removed from the queue";
			case EC_DUPLICATEMSGID:
				return "The message sent has a message id already inuse";
			case EC_SERVERUNAVAIL:
				return "The server is unavailable";
			case EC_NOMOREMESSAGES:
				return "There are no more messages matching the query";
			case EC_FILESYSTEMERROR:
				return "An error occured attempting to read or write from the queue server's file system";
			case EC_CANNOTCHANGEGROUP:
				return "The user cannot change group permissions";
			case EC_CANNOTCHANGEUSER:
				return "The user cannot change user permissions";
			case EC_ALREADYDEFINED:
				return "The resourse is already defined";
			case EC_NOTIMPLEMENTED:
				return "The command is not implemented";
			case EC_QUEUEOPEN:
				return "The queue is currently open by another connection";
			case EC_NOTLOGGEDIN:
				return "The user is ot logged in";
			case EC_INVALIDNAME:
				return "The name supplied is not valid";
			case EC_ERROR:
				return "A general error has occured";
			default:
				return "Unknown error code";
		}
	}
}



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
 * Exception thrown in cases where the SAFMQ queue server has responded
 * with an error code from within a constructor.
 * 
 * @author Matt
 */
public class MQException extends Exception {
	int errorcode;
	
	/**
	 * Constructs the MQException from a description message and error code.
	 * 
	 * @param msg		The description message
	 * @param errorcode	The error code, typically defined in the class Safmq.
	 * 
	 * @see Safmq
	 */
	public MQException(String msg, int errorcode) {
		super(msg);
		this.errorcode = errorcode;
	}
	
	/**
	 * Provides the error code associated with the exception.
	 * @return The error code associated with the exception.
	 * 
	 * @see Safmq#EC_NOERROR
	 * @see Safmq#EC_TIMEDOUT
	 * @see Safmq#EC_NOTAUTHORIZED
	 * @see Safmq#EC_ALREADYCLOSED
	 * @see Safmq#EC_DOESNOTEXIST
	 * @see Safmq#EC_NOTOPEN
	 * @see Safmq#EC_NETWORKERROR
	 * @see Safmq#EC_TTLEXPIRED
	 * @see Safmq#EC_CURSORINVALIDATED
	 * @see Safmq#EC_DUPLICATEMSGID
	 * @see Safmq#EC_SERVERUNAVAIL
	 * @see Safmq#EC_NOMOREMESSAGES
	 * @see Safmq#EC_FILESYSTEMERROR
	 * @see Safmq#EC_CANNOTCHANGEGROUP
	 * @see Safmq#EC_CANNOTCHANGEUSER
	 * @see Safmq#EC_ALREADYDEFINED
	 * @see Safmq#EC_NOTIMPLEMENTED
	 * @see Safmq#EC_QUEUEOPEN
	 * @see Safmq#EC_NOTLOGGEDIN
	 * @see Safmq#EC_INVALIDNAME
	 * @see Safmq#EC_ERROR	 
	 */
	public int getErrorcode() {
		return errorcode;
	}
}




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
 * This mutable object represents a handle to a cursor.  Cursors allow the client to
 * break the priority FIFO and scan a queue in sequential order.
 * 
 * @author Matt
 */
public class CursorHandle {
	/**
	 * Contains the value of the handle.
	 */
	int handle;

	/**
	 * Public constructor for the cursor handle.  The data within the
	 * cursor object is not ment for external consumption.
	 */
	public CursorHandle() {}
}

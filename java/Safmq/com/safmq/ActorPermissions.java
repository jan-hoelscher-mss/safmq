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

Created on Mar 22, 2005
*/
package com.safmq;

/**
 * This mutable object represents an Actor's Permissions when interacting with SAFMQ.
 * Actors (users and groups) have individual permissions when accessing a SAFMQ
 * server.  These permissions include whether an actor can modify (create, change,
 * or delete) Users, Groups or Queues.  This object represents those permissions.
 * 
 * @see MQConnection#UserGetPermissions(String, ActorPermissions)
 * @see MQConnection#GroupGetPermissions(String, ActorPermissions)
 * 
 * @author Matt
 */
public class ActorPermissions {
	boolean	modifyusers = false;
	boolean modifygroups = false;
	boolean modifyqueues = false;
	
	/**
	 * Constructs the ActorPermissions object.  This method employs
	 * default construction.
	 */
	public ActorPermissions() {
	}
	
	/**
	 * @return Returns the modifygroups.
	 */
	public boolean getModifygroups() {
		return modifygroups;
	}
	/**
	 * @param modifygroups The modifygroups to set.
	 */
	public void setModifygroups(boolean modifygroups) {
		this.modifygroups = modifygroups;
	}
	/**
	 * @return Returns the modifyqueues.
	 */
	public boolean getModifyqueues() {
		return modifyqueues;
	}
	/**
	 * @param modifyqueues The modifyqueues to set.
	 */
	public void setModifyqueues(boolean modifyqueues) {
		this.modifyqueues = modifyqueues;
	}
	/**
	 * @return Returns the modifyusers.
	 */
	public boolean getModifyusers() {
		return modifyusers;
	}
	/**
	 * @param modifyusers The modifyusers to set.
	 */
	public void setModifyusers(boolean modifyusers) {
		this.modifyusers = modifyusers;
	}
}
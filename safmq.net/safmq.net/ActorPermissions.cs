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
     * <para>
     * This mutable object represents an Actor's Permissions when interacting with SAFMQ.
     * Actors (users and groups) have individual permissions when accessing a SAFMQ
     * server.  These permissions include whether an actor can modify (create, change,
     * or delete) Users, Groups or Queues.  This object represents those permissions.
     * </para>
     * @author Matt
     * </summary>
     * 
     * <seealso cref="MQConnection.UserGetPermissions(String, ActorPermissions)"/>
     * <seealso cref="MQConnection.GroupGetPermissions(String, ActorPermissions)"/>
     */
    public class ActorPermissions {
        private bool modUsers = false, modGroups = false, modQueues = false;

        /// <summary>
        /// A flag indicating whether the the Actor may modify user information
        /// </summary>
        public bool ModifyUsers {
            get { return modUsers; }
            internal set { modUsers = value; }
        }
        /// <summary>
        /// A flag indicating whether the Actor may modify group information
        /// </summary>
        public bool ModifyGroups { 
            get { return modGroups; }
            internal set { modGroups = value; }
        }
        /// <summary>
        /// A flag indicating whether the Actor may modify group information
        /// </summary>
        public bool ModifyQueues { 
            get { return modQueues; }
            internal set { modQueues = value; }
        }
    }
}

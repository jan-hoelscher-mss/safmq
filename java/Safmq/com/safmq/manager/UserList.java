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

This software implements a Java application to manage a SAFMQ server.
		
Created on May 13, 2005
*/
package com.safmq.manager;

import java.util.Vector;

import com.safmq.MQConnection;
import com.safmq.Safmq;

/**
 * @author matt
 *
 */
public	class UserList 
		extends AbstractManagerList 
{
	Server	server = null;
	
	public UserList(Server server) {
		this.server = server;		
	}
	
	public static final String NAME = "Users";
	public String toString() { 
		return UserList.NAME; 
	}

	public boolean reload() {
		MQConnection con = server.getCon();
		if (con != null) {
			MQConnection.UserDescription	desc;	
			Vector 	users = new Vector();
			
			Vector	list = getList();
			Vector 	dels = (Vector)list.clone();
			Vector 	adds = new Vector();
			User	user;
			int 	err = con.EnumerateUsers(users);
			int		x;
			
			if (err == Safmq.EC_NOERROR) {
				for(x = 0; x < users.size(); x++) {
					desc = (MQConnection.UserDescription)users.get(x);
					user = new User(desc.getName(),desc.getDescription(),server);
					
					Vector ids = new Vector();
					con.EnumerateUserIdentities(desc.getName(), ids);
					user.setIdentities(ids);
					
					if (!dels.remove(user))
						adds.add(user);
				}
				
				for(x=0;x<dels.size();x++)
					list.remove(dels.get(x));
				for(x=0;x<adds.size();x++)
					list.add(adds.get(x));
				return true;
			} else if (err == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError("An error occured retreiving the list of users from safmq://"+server.getName()
					+".\n\nTry reconnecting to the server and viewing the user list again to recover.",server);
			}
		} else
			getList().removeAllElements();
		return false;		
	}
}

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
public class GroupList extends AbstractManagerList {
	Server parent;
	
	public GroupList(Server parent) {
		this.parent = parent;
	}
	
	public static final String NAME = "Groups";
	public String toString() { 
		return GroupList.NAME; 
	}

	public boolean reload() {
		MQConnection con = parent.getCon();
		if (con != null) {
			Vector groups = new Vector();
			
			
			Vector	list = getList();
			Vector 	dels = (Vector)list.clone();
			Vector 	adds = new Vector();
			Group	group;
			int 	err = con.EnumerateGroups(groups);
			int		x;
			
			if (err == Safmq.EC_NOERROR) {
				for(x=0;x<groups.size();x++) {
					group = new Group((String)groups.get(x),parent);
					if (!dels.remove(group))
						adds.add(group);
				}
				
				for(x=0;x<dels.size();x++)
					list.remove(dels.get(x));
				for(x=0;x<adds.size();x++)
					list.add(adds.get(x));
				return true;
			} else if (err == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError(
					"An error occurred loading the list of groups from safmq://"+parent.getName()+
					".\n\nTry reconnecting to server and then viewing the list of groups again to recover.",parent);
			}
		} else
			getList().removeAllElements();
		return false;
	}
}

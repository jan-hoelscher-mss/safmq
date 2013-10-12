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
public	class QueueList 
		extends AbstractManagerList 
{
	Server server;
	
	public QueueList(Server server) {
		this.server = server;
	}
	
	public static final String NAME = "Queues";
	
	public String toString() { 
		return QueueList.NAME; 
	}

	public boolean reload() {
		MQConnection con = server.getCon();
		if (con != null) {
			MQConnection.QueueData	qdata;
			Vector	queues = new Vector();
			
			Vector	list = getList();
			Vector 	dels = (Vector)list.clone();
			Vector 	adds = new Vector();
			Queue	queue;
			int 	err = con.EnumerateQueues(queues);
			int		x;
			
			if (err == Safmq.EC_NOERROR) {
				for(x=0; x<queues.size(); x++) {
					qdata = (MQConnection.QueueData)queues.get(x);
					queue = new Queue(qdata.getName(),server);
					if (!dels.remove(queue))
						adds.add(queue);
				}

				for(x=0;x<dels.size();x++)
					list.remove(dels.get(x));
				for(x=0;x<adds.size();x++)
					list.add(adds.get(x));
				return true;
			} else if (err == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError("An error occured loading the list of queues from safmq://"+server.getName()
						+".\n\nTry reconnecting to the server and viewing the queues again to recover.",server);
			}
		} else
			list.removeAllElements();
		return false;
	}
}

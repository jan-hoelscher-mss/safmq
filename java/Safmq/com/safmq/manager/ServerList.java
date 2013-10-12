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

import java.beans.XMLDecoder;
import java.beans.XMLEncoder;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.Vector;

import javax.swing.tree.DefaultMutableTreeNode;



/**
 * @author matt
 *
 */
public class ServerList extends AbstractManagerList {
	final static String FILE_NAME = "manager_config.xml";
	

	
	public String toString() {
		return "SAFMQ Message Queue Servers";
	}
	
	public void load() {
//		addServer("192.168.1.141","admin","",true);
//		addServer("localhost","admin","",true);
		try {
			XMLDecoder d = new XMLDecoder(new BufferedInputStream(new FileInputStream(FILE_NAME)));
			Vector storeList = (Vector)d.readObject();
			for(int x=0;x<storeList.size();x++) {
				ServerSpec s = (ServerSpec)storeList.get(x);
				addServer(s.name,s.user,s.password,s.storePassword);	
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void store() {
		Vector 	list = getList();
		Vector	storeList = new Vector();
		Server s;
		for(int x=0; x<list.size(); x++) {
			s = (Server)list.get(x);
			
			storeList.add(new ServerSpec(s.getName(),s.getUsername(),s.getPassword(),s.getStorePassword()));
		}
		
		try {
			XMLEncoder e = new XMLEncoder(new BufferedOutputStream(new FileOutputStream(FILE_NAME)));
			e.writeObject(storeList);
			e.close();
		} catch (Exception e) {
			// TOOD: Report the error
			e.printStackTrace();	
		}	
	}
	
	/**
	 * Called to reload the list.  The list isn't ever reloaded from
	 * a server so this method simply returns true, that the data has
	 * been successfully loaded.
	 */
	public boolean reload() {
		return true;
	}
	
	public void onAddNode(DefaultMutableTreeNode n) {
		n.add(new DefaultMutableTreeNode("Not Connected"));
	}
	
	public void deleteServer(Server s) {
		getList().remove(s);
		store();	
	}
	
	public void addServer(String serverURL, String username, String password, boolean storePassword) {
		getList().add(new Server(serverURL,username,password,storePassword));
		store();
	}

	public void handleNetworkError(Server svr) {
		svr.handleNetworkError();
	}
}

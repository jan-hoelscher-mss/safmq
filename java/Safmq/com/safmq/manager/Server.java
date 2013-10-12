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

import java.net.URI;
import java.util.Vector;

import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JOptionPane;
import javax.swing.JTree;
import javax.swing.UIManager;
import javax.swing.tree.DefaultMutableTreeNode;

import com.safmq.MQBuilder;
import com.safmq.MQConnection;
import com.safmq.MQException;
import com.safmq.Safmq;


/**
 * @author matt
 *
 */
public	class Server 
		extends AbstractManagerList 
		implements IconItem, Deleteable
{
	String			name;
	String			username = null;
	String			password = null;
	boolean			storePassword = false;
	MQConnection	con = null;
	
	UserList		users = new UserList(this);
	GroupList		groups = new GroupList(this);
	QueueList		queues = new QueueList(this);
	
	boolean			con_attempted = false;
	
	static ImageIcon	unknown = null;
	static ImageIcon	connected = null;
	static ImageIcon	disconnected = null;
	static Icon	folder;
	
	static {
		try { unknown = new ImageIcon(Queue.class.getResource("images/serversmall_u.gif")); } catch (Exception e) { e.printStackTrace(); }
		try { connected = new ImageIcon(Queue.class.getResource("images/serversmall_c.gif")); } catch (Exception e) { e.printStackTrace(); }
		try { disconnected = new ImageIcon(Queue.class.getResource("images/serversmall_dc.gif")); } catch (Exception e) { e.printStackTrace(); }
		folder = UIManager.getIcon("Tree.closedIcon");
	}
		
	public Server(String name, String username, String password, boolean storePassword) {
		this.name = name;
		this.username = username;
		this.password = password;
		this.storePassword = storePassword;
	}
	
	public Server(String name) {
		this.name = name;
	}
	
	public boolean equals(Object o) {
		return this == o;
	}
	
	public String toString() {
		return name;
	}

	/**
	 * @return
	 */
	public MQConnection getCon() {
		return con;
	}

	/**
	 * @return
	 */
	public String getName() {
		return name;
	}

	protected boolean doConnect() throws Exception {
		con_attempted = true;
				
		if (username == null || username.length() == 0 || password == null ) {
			LoginDialog dlg = new LoginDialog(name,username);
			dlg.setVisible(true);
			String login = dlg.getLogin();
			String pwd = dlg.getPassword();
			if (!dlg.isGood() || login.length() == 0)
				throw new Exception("Not Connected");
					
			storePassword = dlg.getSavePassword();
			username = login;
			password = pwd;
			Manager.getInstance().storeList();
		}
		try {
			con = MQBuilder.buildConnection(new URI("//"+name),username,password);
		} catch (MQException e) {
			if (e.getErrorcode() == Safmq.EC_NOTAUTHORIZED) {
				int result = JOptionPane.showConfirmDialog(Manager.getInstance(),
						"The supplied user name and password were not recognized by safmq://"+name+"\n\n"+
						e.toString() +
						"\n\n"+
						"Would you like to try again?",
						"Server Connection", JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE);
				if (result == JOptionPane.YES_OPTION) {
					password = null;
					return doConnect();
				}
			}
			throw e;
		}
		return true;		
	}

	public boolean connect() throws Exception {
		if (con == null) {
			int result = JOptionPane.showConfirmDialog(Manager.getInstance(),
				"The SAFMQ Manager is not currently connected to: safmq://"+name+"\n" +
				"Would you like to connect?", 
				"Server Connection", JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE);
					
			if (result == JOptionPane.YES_OPTION) {
				return doConnect();
			}
			throw new Exception("Not Connected");
		}
		return false;
	}

	public void handleNetworkError() {
		con = null;	
	}
	
	/**
	 * @see com.safmq.manager.ServerTreeIconNode#getIcon()
	 */
	public Icon getIcon() {
		if (con != null)
			return connected;
		else if (con_attempted)
			return disconnected;
		else
			return unknown;
	}

	/**
	 * Causes the 
	 * @see com.safmq.manager.AbstractManagerList#onAddNode(javax.swing.tree.DefaultMutableTreeNode)
	 */
	public void onAddNode(DefaultMutableTreeNode n) {
		n.add(new DefaultMutableTreeNode());
	}

	/**
	 * @see com.safmq.manager.AbstractManagerList#reload()
	 */
	public boolean reload() {
		if (con == null) {
			Vector l = getList();
			try {
				if(connect()) {
					l.removeAllElements();
					l.add(queues);
					l.add(users);
					l.add(groups);
				}
			} catch (Exception e) {
				String s = e.toString();
				l.removeAllElements();
				l.add(s.substring(s.indexOf(":")+1));
				return false;
			}
		}
		return true;
	}
	
	/**
	 * @see com.safmq.manager.ServerTreeViewSelectionListener#doSelection(javax.swing.JTree, javax.swing.tree.DefaultMutableTreeNode)
	 */
	public void doSelection(JTree tree, DefaultMutableTreeNode node) {
		if (reload()) {
			setNodeChildren(tree, node);
			Manager.getInstance().setControlView(new IconListView(this,UIManager.getIcon("Tree.closedIcon")));	
		}	
	}


	public boolean addUser(String userName, String description, String password) {
		if (reload()) {
			int error = con.CreateUser(userName,password,description);
			if (error == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError("A network error occured creating the user \""+userName+"\".\n" +					"\nPlease try reconnecting and readding the user to recover.",this);
			} else if (error != Safmq.EC_NOERROR) {
				JOptionPane.showMessageDialog(Manager.getInstance(),"An error occured attempting to create the user \""+userName+"\".\n\n"+
						Safmq.errorDecode(error),"Add User", JOptionPane.ERROR_MESSAGE);
			} else {
				return true;
			}
		}
		return false;
	}
	
	public boolean addQueue(String queueName) {
		if (reload()) {
			int error = con.CreateQueue(queueName);
			if (error == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError("A network error occured creating the queue \""+queueName+"\".\n" +
					"\nPlease try reconnecting and readding the user to recover.",this);
			} else if (error != Safmq.EC_NOERROR) {
				JOptionPane.showMessageDialog(Manager.getInstance(),"An error occured attempting to create the queue \""+queueName+"\".\n\n"+
						Safmq.errorDecode(error),"Add Queue", JOptionPane.ERROR_MESSAGE);
			} else {
				return true;
			}
		}
		return false;
	}
	
	public boolean addGroup(String groupName) {
		if (reload()) {
			int error = con.CreateGroup(groupName);
			if (error == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError("A network error occured creating the group \""+groupName+"\".\n" +
					"\nPlease try reconnecting and readding the user to recover.",this);
			} else if (error != Safmq.EC_NOERROR) {
				JOptionPane.showMessageDialog(Manager.getInstance(),"An error occured attempting to create the group \""+groupName+"\".\n\n"+
						Safmq.errorDecode(error),"Add Group", JOptionPane.ERROR_MESSAGE);
			} else {
				return true;
			}
		}
		return false;
	}

	/**
	 * @return
	 */
	public String getPassword() {
		return password;
	}

	/**
	 * @return
	 */
	public boolean getStorePassword() {
		return storePassword;
	}

	/**
	 * @return
	 */
	public String getUsername() {
		return username;
	}

	/* (non-Javadoc)
	 * @see com.safmq.manager.Deleteable#delete()
	 */
	public boolean delete() {
		int res = JOptionPane.showConfirmDialog(Manager.getInstance(),
							"Are you sure you wish to remove the server \""+getName()+"\"\n",
							"User Deletion Confirmation",
							JOptionPane.YES_NO_OPTION);
		if (res == JOptionPane.YES_OPTION) {
			Manager.getInstance().deleteQueueServer(this);
		}
		return true;
	}

	/* (non-Javadoc)
	 * @see com.safmq.manager.Deleteable#getDescription()
	 */
	public String getDescription() {
		return getName();
	}

}

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

import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JOptionPane;
import javax.swing.JTree;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreePath;

import com.safmq.MQConnection;
import com.safmq.Safmq;

/**
 * @author matt
 *
 */
public class User implements ServerTreeViewSelectionListener, IconItem, TreePathable, IconListView.Activateable,Deleteable {
	String login;
	String description;
	Server server;
	
	Vector	identities;
	
	static ImageIcon icon = null;
	static {
		try {
			icon = new ImageIcon(User.class.getResource("images/usersmall.gif"));
		} catch (Exception e) {
		}
	}
	
	public User(String login, String description,Server server) {
		this.login = login;
		this.description = description;
		this.server = server;
	}
	
	public String toString() {
		if (description == null || description.length() == 0)
			return login;
		return description + "(" + login + ")";		
	}
	
	public boolean equals(Object o) {
		return o == this || ((o instanceof User) && login.equals(((User)o).login));	
	}
	
	/* (non-Javadoc)
	 * @see com.safmq.manager.ServerTreeViewSelectionListener#doSelection()
	 */
	public void doSelection(JTree tree, DefaultMutableTreeNode node) {
		Manager.getInstance().setControlView(new UserEditor(this));
	}

	/**
	 * @see com.safmq.manager.ServerTreeIconNode#getIcon()
	 */
	public Icon getIcon() {
		return icon;
	}
	
	/**
	 * @see com.safmq.manager.IconListView.Activateable#activate()
	 */
	public void activate() {
		if (getPath() != null) {
			Manager.getInstance().getTreeView().activatePath(getPath());	
		}
	}

	TreePath treePath = null;
	/**
	 * @see com.safmq.manager.TreePathable#getPath()
	 */
	public TreePath getPath() {
		return treePath;
	}

	/**
	 * @see com.safmq.manager.TreePathable#setPath(javax.swing.tree.TreePath)
	 */
	public void setPath(TreePath path) {
		treePath = path;
	}
	
	/**
	 * @see com.safmq.manager.Deleteable#delete()
	 */
	public boolean delete() {
		MQConnection con = server.getCon();
		if (con != null) {
			int res = JOptionPane.showConfirmDialog(Manager.getInstance(),
								"Are you sure you wish to delete the user \""+login+"\""
								+"+\n from the server safmq://"+server.toString()+"\n",
								"User Deletion Confirmation",
								JOptionPane.YES_NO_OPTION);
			if (res == JOptionPane.YES_OPTION) {
				int error = con.DeleteUser(login);
				if (error == Safmq.EC_NETWORKERROR) {
					Manager.getInstance().handleNetworkError("A network error occured deleting the user \""+login+"\"\n\n" +
						"Please try reconnecting to the server and deleting the group again to recover.", server);
				} else if (error != Safmq.EC_NOERROR) {
				} else {
					return true;	
				}
			}
		}
		return false;
	}
	
	/**
	 * @see com.safmq.manager.Deleteable#getDescription()
	 */
	public String getDescription() {
		return "User \""+login+"\"";
	}
	
	/**
	 * @return
	 */
	public String getLogin() {
		return login;
	}

	public String getTheDescription() {
		return description;	
	}

	/**
	 * @return
	 */
	public Server getServer() {
		return server;
	}

	public Vector getIdentities() {
		return identities;
	}

	public void setIdentities(Vector identities) {
		this.identities = identities;
	}

}

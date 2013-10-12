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
import com.safmq.MQConnection.QueuePermissions;

/**
 * @author matt
 *
 */
public class Queue implements ServerTreeViewSelectionListener, IconItem, TreePathable, IconListView.Activateable,Deleteable {
	String			name;
	Server			server;
	Vector			permissions = new Vector();
	Vector			deleted = new Vector();
	
	public class perms {
		String entity;
		boolean group;
		Boolean read;
		Boolean write;
		Boolean changeSecurity;
		Boolean destroy;
		boolean	modified;
		
		public perms(String entity, boolean group, boolean read, boolean write, boolean changeSecurity, boolean destroy) {
			this.entity = entity;
			this.group = group;
			this.read = new Boolean(read);
			this.write = new Boolean(write);
			this.changeSecurity = new Boolean(changeSecurity);
			this.destroy = new Boolean(destroy);
			modified = false;
		}
		public Boolean getDestroy() {
			return destroy;
		}
		public String getName() {
			return entity;
		}
		public boolean isGroup() {
			return group;
		}
		public Boolean getRead() {
			return read;
		}
		public Boolean getWrite() {
			return write;
		}
		public Boolean getChangeSecurity() {
			return changeSecurity;
		}
		public void setDestroy(Boolean destroy) {
			this.destroy = destroy;
		}
		public void setEntity(String entity) {
			this.entity = entity;
		}
		public void setRead(Boolean read) {
			this.read = read;
		}
		public void setWrite(Boolean write) {
			this.write = write;
		}
		public void setChangeSecurity(Boolean changeSecurity) {
			this.changeSecurity = changeSecurity;
		}
		public void setModified() {
			this.modified = true;
		}
		public String toString() {
			return getName();	
		}
	}
	
	static ImageIcon icon = null;
	
	static {
		try {
			icon = new ImageIcon(Queue.class.getResource("images/queuesmall.gif"));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public Queue(String name, Server server) {
		this.name = name;
		this.server = server;
	}
	
	public boolean equals(Object o) {
		return this == o || ((o instanceof Queue) && getName().equals(((Queue)o).getName()));	
	} 

	/**
	 * Retrieves the name of the message queue this Queue object represents
	 * @return The name of the message queue this Queue object represents
	 */
	public String getName() {
		return name;
	}

	/**
	 * Provides access to the security permissions available to the queue.  The
	 * returned vector contains objects of type com.safmq.QueuePermissions. 
	 * @return A vector of <code>com.safmq.QueuePermissions</code>
	 * @see com.safmq.QueuePermissions
	 */
	public Vector getPermissions() {
		return permissions;		
	}
	
	public String toString() {
		return name;
	}
	
	public boolean reload() {
		Vector tmp = new Vector();

		permissions.clear();
		deleted.clear();
		MQConnection con = server.getCon();
		if (con != null) {
			int error = con.QueueEnumeratePermissions(getName(), tmp);
			if (error == Safmq.EC_NOERROR) {
				for(int x=0;x<tmp.size();x++) {
					QueuePermissions p = (QueuePermissions)tmp.get(x);
					permissions.add(allocPerms(p.getEntityName(), p.getIsGroup(), p.getRead(), p.getWrite(), p.getChangeSecurity(), p.getDestroy()));
				}
				return true;
			} else if (error == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError(
					"A network error occured loading the queue permissions from safmq://"+server.getName()
					+".\n\nTry reconnecting to the server and editing the permissions again to recover.", server);		
			}
		}
		return false;
	}
	
	/* (non-Javadoc)
	 * @see com.safmq.manager.ServerTreeViewSelectionListener#doSelection()
	 */
	public void doSelection(JTree tree, DefaultMutableTreeNode node) {
		if (reload())
			Manager.getInstance().setControlView(new QueuePermissionEditor(this));
	}

	public perms allocPerms(String name, boolean isGroup, boolean read, boolean write, boolean changeSecurity, boolean destroy) {
		return new perms(name,isGroup,read,write,changeSecurity,destroy);		
	}

	public void deletePerm(perms deletedItem) {
		if (permissions.remove(deletedItem))
			deleted.add(deletedItem);
	}

	static final String NET_ERROR_MSG = "A network error occured while saving changes to permisisons.\n" +
										"Not all changes may have been saved.\n" +
										"Try Reconnecting to server and appling changes again.";

	public void saveChanges() {
		int				error;
		int				x;
		perms			p;
		MQConnection	con = server.getCon();
		if (con != null) {
			for(x=0;x<deleted.size();x++) {
				p = (perms)deleted.get(x);
				if (p.isGroup())
					error = con.QueueDeleteGroupPermission(getName(),p.getName());
				else
					error = con.QueueDeleteUserPermission(getName(),p.getName());
				if (error == Safmq.EC_NETWORKERROR) {
					Manager.getInstance().handleNetworkError(NET_ERROR_MSG,server);
					return;
				}
			}
			for(x=0;x<permissions.size();x++) {
				p = (perms)permissions.get(x);
				if (p.isGroup()) 
					error = con.QueueSetGroupPermission(getName(),p.getName(),
									p.getRead().booleanValue(),	p.getWrite().booleanValue(),
									p.getDestroy().booleanValue(), p.getChangeSecurity().booleanValue());
				else 	
					error = con.QueueSetUserPermission(getName(),p.getName(),
									p.getRead().booleanValue(),	p.getWrite().booleanValue(),
									p.getDestroy().booleanValue(), p.getChangeSecurity().booleanValue());
				if (error == Safmq.EC_NETWORKERROR) {
					Manager.getInstance().handleNetworkError(NET_ERROR_MSG,server);
					return;
				}
			}
		}
	}

	public Icon getIcon() {
		return icon;
	}

	/**
	 * @return
	 */
	public Server getServer() {
		return server;
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
								"Are you sure you wish to delete the queue \""+name+"\"\n"
								+"from the server safmq://"+server.toString()+"\n",
								"Queue Deletion Confirmation",
								JOptionPane.YES_NO_OPTION);
			if (res == JOptionPane.YES_OPTION) {
				int error = con.DeleteQueue(name);
				if (error == Safmq.EC_NETWORKERROR) {
					Manager.getInstance().handleNetworkError("A network error occured deleting the queue \""+name+"\"\n\n" +
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
		return "Queue \""+name+"\"";
	}
}

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
		
Created on May 24, 2005
*/
package com.safmq.manager;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.util.Vector;

import javax.swing.Action;
import javax.swing.DefaultListModel;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.KeyStroke;
import javax.swing.ListSelectionModel;

import com.safmq.MQConnection;
import com.safmq.Safmq;

/**
 * @author matt
 */
public class QueueAddActor extends JDialog {
	Queue queue;
	
	boolean good = false;
	
	JList	users = null;
	JList	groups = null;
	JList	newActors = null;
	
	DefaultListModel	usersList = null;
	DefaultListModel	groupsList = null;
	DefaultListModel	newActorsList = new DefaultListModel();
	
	static ImageIcon	addIcon = null;
	static ImageIcon	removeIcon = null;
	
	static {
		try {
			addIcon = new ImageIcon(Queue.class.getResource("images/add.gif"));
			removeIcon = new ImageIcon(Queue.class.getResource("images/remove.gif"));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	
	public QueueAddActor(Queue queue) throws Exception {
		super(Manager.getInstance(),"Add Users and Groups", true);
		this.queue = queue;
		JLabel				label;

		JPanel				panel = new JPanel();
		
		this.setContentPane(panel);


		loadUsers();
		loadGroups();		

		// Buidl the left panel ///////////////////////////////////////////////
		GridBagLayout		g = new GridBagLayout();
		GridBagConstraints	c = new GridBagConstraints();
		Insets origInsets = c.insets;
		JPanel	left = new JPanel(g);
		
		label = new JLabel("Available Users");
		c.anchor = GridBagConstraints.WEST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill 	= GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.weighty = 0;
		left.add(label,c);
		
		users = new JList(usersList);
		users.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
		users.setVisibleRowCount(5);
		JScrollPane userPane = new JScrollPane(users);
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 0.1;
		left.add(userPane,c);
		
		label = new JLabel("Available Groups");
		c.insets = new Insets(5,0,0,0);
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.weighty = 0;	
		left.add(label,c);
		
		groups = new JList(groupsList);
		groups.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
		groups.setVisibleRowCount(5);
		JScrollPane groupPane = new JScrollPane(groups);
		c.insets = origInsets;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 0.1;
		left.add(groupPane,c);
		///////////////////////////////////////////////////////////////////////
	
		// Build the center panel /////////////////////////////////////////////
		g = new GridBagLayout();
		c = new GridBagConstraints();
		JPanel center = new JPanel(g);

		JButton add;
		JButton remove;
		
		add = new JButton (new SimpleAction(addIcon==null?"Add":null, addIcon, true) {
			public void actionPerformed(ActionEvent e) {
				addSelection();
			}	
		});
		
		c.anchor = GridBagConstraints.CENTER;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.weighty = 0;	
		center.add(add,c);
		
		remove = new JButton (new SimpleAction(removeIcon == null?"Remove":null, removeIcon, true) {
			public void actionPerformed(ActionEvent e) {
				removeSelection();
			}	
		});
		
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.weighty = 0;	
		center.add(remove,c);
		center.validate();
		///////////////////////////////////////////////////////////////////////

		// Build the right panel //////////////////////////////////////////////
		g = new GridBagLayout();
		c = new GridBagConstraints();
		JPanel right = new JPanel(g);

		label = new JLabel("Users and Groups to Add");
		c.anchor = GridBagConstraints.WEST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.weighty = 0;
		right.add(label,c);
		
		newActors = new JList(newActorsList);
		newActors.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
		newActors.setVisibleRowCount(5);
		JScrollPane newActorsPane = new JScrollPane(newActors);
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		right.add(newActorsPane,c);
		///////////////////////////////////////////////////////////////////////


		// Build the bottom panel /////////////////////////////////////////////
		g = new GridBagLayout();
		c = new GridBagConstraints();
		JPanel bottom = new JPanel(g);
		
		Action okAction = new SimpleAction("OK", new Integer(KeyEvent.VK_ENTER), true) {
			public void actionPerformed(ActionEvent e) {
				good = true;
				setVisible(false);
			}
		};
		Action cancelAction = new SimpleAction("Cancel", new Integer(KeyEvent.VK_ESCAPE), true) {
			public void actionPerformed(ActionEvent e) {
				setVisible(false);
			}
		};
			
		// setup the enter key			
		panel.getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_ENTER,0), okAction);
		panel.getActionMap().put(okAction,okAction);
		// setup the escape key
		panel.getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE,0), cancelAction);
		panel.getActionMap().put(cancelAction,cancelAction);

				
		JButton ok = new JButton(okAction);
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.weighty = 1;
		c.gridwidth = GridBagConstraints.RELATIVE;
		c.anchor = GridBagConstraints.EAST;
		bottom.add(ok,c);

		JButton cancel = new JButton(cancelAction);
		c.insets = new Insets(0,10,0,0);
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 1;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.anchor = GridBagConstraints.EAST;
		bottom.add(cancel,c);
		///////////////////////////////////////////////////////////////////////


		// Build the display
		g = new GridBagLayout();
		c = new GridBagConstraints();
		getContentPane().setLayout(g);
		
		c = new GridBagConstraints();
		
		Insets i5555 = c.insets = new Insets(5,5,5,5);
		c.gridwidth = 1;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		getContentPane().add(left,c);

		c.gridwidth = GridBagConstraints.RELATIVE;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 1;
		getContentPane().add(center,c);

		c.insets = i5555;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		getContentPane().add(right,c);
		
		c.insets = i5555;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 0;
		getContentPane().add(bottom,c);

		pack();
	}
	
	void loadUsers() throws Exception {
		usersList = new DefaultListModel();
		
		MQConnection 	con = queue.getServer().getCon();
		Vector			actors = queue.getPermissions();
		if (con != null) {
			Vector	users = new Vector();
			int err = con.EnumerateUsers(users);
			if (err == Safmq.EC_NOERROR) {
				MQConnection.UserDescription 	desc;
				Queue.perms						perm;
				for(int x=0; x < users.size(); x++) {
					desc = (MQConnection.UserDescription)users.get(x);
					boolean found = false;
					for(int y=0; !found && y < actors.size(); y++) {
						perm = (Queue.perms)actors.get(y);
						found =  (!perm.isGroup() && perm.getName().equals(desc.getName()));
					}
					if (!found)
						usersList.addElement( queue.allocPerms(desc.getName(),false,false,false,false,false) );
				}
			} else if (err == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError("",queue.getServer());
				throw new Exception("Error loading users");
			}
		}	
	}
	
	void loadGroups() throws Exception {
		groupsList = new DefaultListModel();
		
		MQConnection 	con = queue.getServer().getCon();
		Vector			actors = queue.getPermissions();
		if (con != null) {
			Vector	groups = new Vector();
			int err = con.EnumerateGroups(groups);
			if (err == Safmq.EC_NOERROR) {
				String		 	desc;
				Queue.perms		perm;
				for(int x=0; x < groups.size(); x++) {
					desc = (String)groups.get(x);
					boolean found = false;
					for(int y=0; !found && y < actors.size(); y++) {
						perm = (Queue.perms)actors.get(y);
						found = (perm.isGroup() && perm.getName().equals(desc));
					}
					if (!found)
						groupsList.addElement( queue.allocPerms(desc,true,false,false,false,false) );
				}
			} else if (err == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError("",queue.getServer());
				throw new Exception("Error loading groups");
			}
		}	
	}
	
	void addSelection() {
		Object	selection[] = users.getSelectedValues();
		int x;
		for(x=0;x<selection.length;x++) {
			usersList.removeElement(selection[x]);
			newActorsList.addElement(selection[x]);
		}
		
		selection = groups.getSelectedValues();
		for(x=0;x<selection.length;x++) {
			groupsList.removeElement(selection[x]);
			newActorsList.addElement(selection[x]);	
		}		
	}
	
	void removeSelection() {
		Object selection[] = newActors.getSelectedValues();
		for(int x=0;x<selection.length;x++) {
			newActorsList.removeElement(selection[x]);
			if ( ((Queue.perms)selection[x]).isGroup() )
				groupsList.addElement(selection[x]);
			else
				usersList.addElement(selection[x]);
		}
	}
	
	/**
	 * @return
	 */
	public boolean isGood() {
		return good;
	}

	public Object[] getNewActors() {
		return newActorsList.toArray();
	}

}

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
		
Created on May 26, 2005
*/
package com.safmq.manager;

import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import com.safmq.*;

/**
 * @author matt
 */
public class GroupEditor extends JComponent implements ChangeListener {
	Group		group = null;
	JCheckBox	modUsers = new JCheckBox("Add/Modify Users");
	JCheckBox	modGroups = new JCheckBox("Add/Modify Groups");
	JCheckBox	modQueues = new JCheckBox("Add/Modify Queues");
	JList		usersList = new JList(new DefaultListModel());
	Action 		reloadAction = null;
	Action		applyAction = null;
	Action		addGroupsAction = null;
	Action		deleteGroupAction = null;
	
	static Icon	icon = null;
	
	static {
		try {
			icon = new ImageIcon(GroupEditor.class.getResource("images/group.gif"));
		} catch (Exception e) {
		}
	}
	
	GroupEditor(Group group) {
		JLabel l;
		
		this.group = group;
		
		loadData();
		
		reloadAction = new SimpleAction("Reload",new Integer(KeyEvent.VK_R),false) {
			public void actionPerformed(ActionEvent e) {
				doReload();
			}	
		};
		applyAction = new SimpleAction("Apply",new Integer(KeyEvent.VK_A),false) {
			public void actionPerformed(ActionEvent e) {
				doApply();
			}	
		};
		addGroupsAction = new SimpleAction("Users",new Integer(KeyEvent.VK_G), true) {
			public void actionPerformed(ActionEvent e) {
				doAddUsers();
			}
		};
		deleteGroupAction = new SimpleAction("Delete Group", (Integer)null, true) {
			public void actionPerformed(ActionEvent e) {
				doDeleteGroup();
			}
		};

		JButton reload = new JButton(reloadAction);
		JButton apply = new JButton(applyAction);
		JButton addGroups = new JButton(addGroupsAction);
		
		modUsers.addChangeListener(this);
		modGroups.addChangeListener(this);
		modQueues.addChangeListener(this);

		GridBagLayout 		g = new GridBagLayout();
		GridBagConstraints 	c = new GridBagConstraints();
		Insets				origInsets = c.insets;
		
		this.setLayout(g);
		
		
		// Setup the panels ///////////////////////////////////////////////////
		JPanel iconPanel = new JPanel();
		c.insets = new Insets(5,5,5,5);
		c.gridwidth = GridBagConstraints.RELATIVE;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 0;
		c.anchor = GridBagConstraints.NORTHWEST;
		this.add(iconPanel,c);
		
		JPanel dataPanel = new JPanel();
		c.insets = origInsets;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		JScrollPane jsp;
		this.add(jsp = new JScrollPane(dataPanel),c);
		jsp.setBorder(BorderFactory.createEmptyBorder());
		
		JPanel buttonPanel = new JPanel();
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 0;
		c.weighty = 0;
		this.add(buttonPanel,c);
		///////////////////////////////////////////////////////////////////////
		
		// Setup te icon panel ////////////////////////////////////////////////
		iconPanel.setLayout(g = new GridBagLayout());

		if (icon == null)
			icon = UIManager.getIcon("OptionPane.questionIcon");
		l = new JLabel(icon);
		c.anchor = GridBagConstraints.NORTHWEST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx=1;
		c.weighty=1;
		iconPanel.add(l,c);
		///////////////////////////////////////////////////////////////////////
		
		// Setup the button panel ////////////////////////////////////////////////
		buttonPanel.setLayout(g = new GridBagLayout());
		c.anchor = GridBagConstraints.EAST;
		c.insets = new Insets(5,0,5,20);
		c.gridwidth = 1;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.weighty = 0;
		buttonPanel.add(addGroups,c);
		
		
		c.insets = new Insets(5,0,5,10);
		c.gridwidth = GridBagConstraints.RELATIVE;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.gridwidth = 1;
		buttonPanel.add(reload,c);
		
		c.gridwidth = GridBagConstraints.REMAINDER;
		buttonPanel.add(apply,c);
		///////////////////////////////////////////////////////////////////////
		
		// Setup the data panel ///////////////////////////////////////////////
		dataPanel.setLayout(g = new GridBagLayout());
		c.insets = origInsets;
		c.anchor = GridBagConstraints.NORTHWEST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.weighty = 0;
		dataPanel.add(new JLabel("<html><font size=5><b>Group Name: </b>"+group.getName()),c);
		dataPanel.add(modUsers,c);
		dataPanel.add(modGroups,c);
		dataPanel.add(modQueues,c);
		
		c.insets = new Insets(5,0,0,0);
		dataPanel.add(new JLabel("Users:"),c);
		c.insets = new Insets(0,0,0,10);
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		usersList.setVisibleRowCount(-1);
		usersList.setLayoutOrientation(JList.VERTICAL_WRAP);
		usersList.setCellRenderer(new IconListRenderer());
		dataPanel.add(new JScrollPane(usersList),c);
		usersList.getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_DELETE,0),deleteGroupAction);
		usersList.getActionMap().put(deleteGroupAction,deleteGroupAction);
		///////////////////////////////////////////////////////////////////////
	}
	
	void loadData() {
		MQConnection con = group.getServer().getCon();
		if (con != null) {
			ActorPermissions 	perms = new ActorPermissions();
			Vector		users = new Vector();
			int error = con.GroupGetPermissions(group.getName(),perms);
			if (error == Safmq.EC_NOERROR || error == Safmq.EC_DOESNOTEXIST) {
				error = con.GroupGetUsers(group.getName(),users);
				if (error == Safmq.EC_NOERROR) {
					modUsers.setSelected(perms.getModifyusers());
					modGroups.setSelected(perms.getModifygroups());
					modQueues.setSelected(perms.getModifyqueues());
					DefaultListModel model = (DefaultListModel)usersList.getModel();
					model.removeAllElements();
					for(int x=0;x<users.size();x++)
						model.addElement(new User(users.get(x).toString(),"",null));
				} else if (error == Safmq.EC_NETWORKERROR) {
					Manager.getInstance().handleNetworkError(group.getServer());
				} else {
					JOptionPane.showMessageDialog(this,
								"An error occured attempting to list users for \""+group.getName()+"\"\n\n" +								Safmq.errorDecode(error),"Group Information",JOptionPane.ERROR_MESSAGE);
				}
			} else if (error == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError(group.getServer());
			} else {
				JOptionPane.showMessageDialog(this,
							"An error occured attempting to load permissions for \""+group.getName()+"\"\n\n" +
							Safmq.errorDecode(error),"Group Information",JOptionPane.ERROR_MESSAGE);
			}
		}		
	}
	
	void doReload() {
		loadData();
		reloadAction.setEnabled(false);
		applyAction.setEnabled(false);
	}
	
	void doApply() {
		int	error;
		MQConnection con = group.getServer().getCon();
		if (con != null) {
			error = con.GroupSetPermissions(group.getName(),modQueues.isSelected(),modUsers.isSelected(),modGroups.isSelected());
			if (error == Safmq.EC_NOERROR) {
				Vector users = new Vector();
				error = con.GroupGetUsers(group.getName(),users);
				if (error == Safmq.EC_NOERROR) {
					int x;
					DefaultListModel model = (DefaultListModel)usersList.getModel();
					
					for (x=0;x<users.size();x++) {
						error = con.GroupDeleteUser(group.getName(),(String)users.get(x));
						if (error == Safmq.EC_NETWORKERROR) {
							Manager.getInstance().handleNetworkError(group.getServer());
							return;
						} else if (error != Safmq.EC_NOERROR) {
							JOptionPane.showMessageDialog(this,
										"An error occured attempting to store membership for group \""+group.getName()+"\"\n\n" +
										Safmq.errorDecode(error),"Group Information",JOptionPane.ERROR_MESSAGE);
							doReload();
							return;
						}
					}
					
					for (x=0;x<model.size();x++) {
						error = con.GroupAddUser(group.getName(),((User)model.get(x)).getLogin());
						if (error == Safmq.EC_NETWORKERROR) {
							Manager.getInstance().handleNetworkError(group.getServer());
							return;
						} else if (error != Safmq.EC_NOERROR) {
							JOptionPane.showMessageDialog(this,
										"An error occured attempting to store membership for group \""+group.getName()+"\"\n\n" +
										Safmq.errorDecode(error),"Group Information",JOptionPane.ERROR_MESSAGE);
							doReload();
							return;
						}
					}
					
					applyAction.setEnabled(false);
					reloadAction.setEnabled(false);
				} else if(error == Safmq.EC_NETWORKERROR) {
					Manager.getInstance().handleNetworkError(group.getServer());
				} else {
					JOptionPane.showMessageDialog(this,
								"An error occured attempting to store membership for group \""+group.getName()+"\"\n\n" +
								Safmq.errorDecode(error),"Group Information",JOptionPane.ERROR_MESSAGE);
				}
			} else if (error == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError(group.getServer());
			} else {
				JOptionPane.showMessageDialog(this,
							"An error occured attempting to store permissions for group \""+group.getName()+"\"\n\n" +
							Safmq.errorDecode(error),"Group Information",JOptionPane.ERROR_MESSAGE);
				doReload();
			}
		}
		reloadAction.setEnabled(false);
		applyAction.setEnabled(false);
	}
	
	void doAddUsers() {
		MQConnection con = group.getServer().getCon();
		if (con != null) {
			Vector users = new Vector();
			int error = con.EnumerateUsers(users);
			if (error == Safmq.EC_NOERROR) {
				DefaultListModel 	model = (DefaultListModel)usersList.getModel();
				int					x;
				
				for(x=0; x<model.size(); x++) {
					users.remove(con.genDesc(((User)model.get(x)).getLogin(),""));
				}
					
				Vector g = new Vector();
				for(x=0;x<users.size();x++)
					g.add(new User(((MQConnection.UserDescription)users.get(x)).getName(),"",null));
				
				MultiChoiceDialog dlg = new MultiChoiceDialog("Please choose the users to add", "Add Users", g.toArray(), new IconListRenderer());
				dlg.setVisible(true);
				if (dlg.isGood()) {
					Object choices[] = dlg.getChoices();
					for(x=0;x<choices.length; x++)
						model.addElement(choices[x]);
					reloadAction.setEnabled(true);
					applyAction.setEnabled(true);
				}
				
			} else if (error == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError(group.getServer());
			} else {
				JOptionPane.showMessageDialog(this,
							"An error occured attempting to load the list of users from safmq://"+group.getServer().getName()+"\n\n" +
							Safmq.errorDecode(error),"Group Information",JOptionPane.ERROR_MESSAGE);
			}
		}
	}
	
	void doDeleteGroup() {
		int sel = usersList.getSelectedIndex();
		if (sel >= 0) {
			DefaultListModel model = (DefaultListModel)usersList.getModel();
			model.remove(sel);
			reloadAction.setEnabled(true);
			applyAction.setEnabled(true);
		}
	}
	/**
	 * @see javax.swing.event.ChangeListener#stateChanged(javax.swing.event.ChangeEvent)
	 */
	public void stateChanged(ChangeEvent e) {
		reloadAction.setEnabled(true);
		applyAction.setEnabled(true);
	}

}

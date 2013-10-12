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

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.util.Vector;

import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.DefaultListModel;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.UIManager;

import com.safmq.ActorPermissions;
import com.safmq.MQConnection;
import com.safmq.Safmq;
import com.safmq.X509Identity;
import com.safmq.manager.UserEditorPanels.BasicInfoPanel;
import com.safmq.manager.UserEditorPanels.IdentityPanel;

/**
 * @author matt
 */
public class UserEditor extends JComponent implements ItemListener {
	User		user = null;
	
	JTabbedPane	tab = new JTabbedPane();
	
	BasicInfoPanel	info;
	IdentityPanel	identity;
	
	Action 		reloadAction = null;
	Action		applyAction = null;
	Action 		setPasswordAction = null;
	Action		addGroupsAction = null;
	Action		deleteGroupAction = null;
	
	static Icon	icon = null;
	
	static {
		try {
			icon = new ImageIcon(UserEditor.class.getResource("images/user.gif"));
		} catch (Exception e) {
		}
	}
	
	UserEditor(User user) {
		this.user = user;
		
		
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
		setPasswordAction = new SimpleAction("Set Password",new Integer(KeyEvent.VK_S),true) {
			public void actionPerformed(ActionEvent e) {
				doSetPassword();
			}	
		};
		addGroupsAction = new SimpleAction("Groups",new Integer(KeyEvent.VK_G), true) {
			public void actionPerformed(ActionEvent e) {
				doAddGroups();
			}
		};
		deleteGroupAction = new SimpleAction("Delete Group", (Integer)null, true) {
			public void actionPerformed(ActionEvent e) {
				doDeleteGroup();
			}
		};

		info = new BasicInfoPanel(user, deleteGroupAction, this);
		identity = new IdentityPanel(user);
		identity.addItemListener(this);
		
		loadData();
		reloadAction.setEnabled(false); // disable, because loadData() may change the state
		applyAction.setEnabled(false);
		
		GridBagConstraints 	c = new GridBagConstraints();
		JPanel		stdPan = new JPanel();
		JPanel		identPan = new JPanel();
		
		stdPan.setName("Security Info");
		identPan.setName("X509 Identities");
		
		tab.addTab(stdPan.getName(), stdPan);
		tab.addTab(identPan.getName(), identPan);

		this.setLayout(new GridBagLayout());
		
		JButton reload = new JButton(reloadAction);
		JButton apply = new JButton(applyAction);
		JButton setPassword = new JButton(setPasswordAction);
		JButton addGroups = new JButton(addGroupsAction);
		


		// Setup  the main pannel containin the iconPanel, dataPanel and buttonPanel
		c = new GridBagConstraints();
		Insets origInsets = c.insets;
		stdPan.setLayout(new GridBagLayout());
		identPan.setLayout(new GridBagLayout());
		
		// Setup the panels ///////////////////////////////////////////////////
		// Icon Panel
		JPanel iconPanel = new JPanel();
		JPanel identIconPanel = new JPanel();
		c.insets = new Insets(2,2,0,0);
		c.gridwidth = GridBagConstraints.RELATIVE;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 0;
		c.weighty = 0;
		c.gridheight = 2;
		c.anchor = GridBagConstraints.NORTHWEST;
		stdPan.add(iconPanel, c);
		identPan.add(identIconPanel, c);

		// Name Panel
		JPanel namePanel = new JPanel();
		JPanel identNamePanel = new JPanel();
		c.insets = origInsets;
		c.anchor = GridBagConstraints.NORTHWEST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.weighty = 0;
		c.gridheight = 1;
		stdPan.add(namePanel, c);
		identPan.add(identNamePanel, c);
		
		// Info Panel
		c.insets = origInsets;
		c.anchor = GridBagConstraints.NORTHWEST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		c.gridheight = 1;
		JScrollPane jsp;
		stdPan.add(jsp = new JScrollPane(info),c);
		jsp.setBorder(BorderFactory.createEmptyBorder());
		
		c.fill = GridBagConstraints.BOTH;
		identPan.add(identity, c);
		
		///////////////////////////////////////////////////////////////////////

		// Build the base content /////////////////////////////////////////////
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.anchor = GridBagConstraints.NORTHWEST;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		this.add(tab, c);

		JPanel buttonPanel = new JPanel();
		c.insets = origInsets;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 0;
		c.weighty = 0;
		this.add(buttonPanel, c);
		///////////////////////////////////////////////////////////////////////
		
		
		// Setup te icon panel ////////////////////////////////////////////////
		iconPanel.setLayout(new GridBagLayout());
		identIconPanel.setLayout(new GridBagLayout());
		if (icon == null)
			icon = UIManager.getIcon("OptionPane.questionIcon");
		c.anchor = GridBagConstraints.NORTHWEST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx=1;
		c.weighty=1;
		iconPanel.add(new JLabel(icon),c);
		identIconPanel.add(new JLabel(icon),c);
		///////////////////////////////////////////////////////////////////////

		// Setup the name panel /////////////////////////////////////////////////
		namePanel.setLayout(new GridBagLayout());
		identNamePanel.setLayout(new GridBagLayout());
		c.insets = origInsets;
		c.anchor = GridBagConstraints.NORTHWEST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.weighty = 0;
		namePanel.add(new JLabel("<html><font size=5><b>Login: </b>"+user.getLogin()),c);
		namePanel.add(new JLabel("<html><font size=5><b>Description: </b>"+user.getTheDescription()),c);

		identNamePanel.add(new JLabel("<html><font size=5><b>Login: </b>"+user.getLogin()),c);
		identNamePanel.add(new JLabel("<html><font size=5><b>Description: </b>"+user.getTheDescription()),c);
		///////////////////////////////////////////////////////////////////////

		
		// Setup the button panel ////////////////////////////////////////////////
		buttonPanel.setLayout(new GridBagLayout());
		c.anchor = GridBagConstraints.EAST;
		c.insets = new Insets(5,0,5,20);
		c.gridwidth = 1;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.weighty = 1;
		buttonPanel.add(setPassword,c);
		
		c.insets = new Insets(5,0,5,10);
		c.weightx = 0;
		c.gridwidth = 1;
		buttonPanel.add(addGroups,c);
		
		c.gridwidth = GridBagConstraints.RELATIVE;
		buttonPanel.add(reload,c);
		
		c.gridwidth = GridBagConstraints.REMAINDER;
		buttonPanel.add(apply,c);
		///////////////////////////////////////////////////////////////////////
		

		///////////////////////////////////////////////////////////////////////
	}
	
	void loadData() {
		MQConnection con = user.getServer().getCon();
		if (con != null) {
			ActorPermissions 	perms = new ActorPermissions();
			Vector		groups = new Vector();
			int error = con.UserGetPermissions(user.getLogin(),perms);
			if (error == Safmq.EC_NOERROR || error == Safmq.EC_DOESNOTEXIST) {
				error = con.UserGetGroups(user.getLogin(),groups);
				if (error == Safmq.EC_NOERROR) {
					info.setModUsers(perms.getModifyusers());
					info.setModGroups(perms.getModifygroups());
					info.setModQueues(perms.getModifyqueues());
					DefaultListModel model = info.getGroupListModel();
					model.removeAllElements();
					for(int x=0;x<groups.size();x++)
						model.addElement(new Group(groups.get(x).toString(),null));
				} else if (error == Safmq.EC_NETWORKERROR) {
					Manager.getInstance().handleNetworkError(user.getServer());
				} else {
					JOptionPane.showMessageDialog(Manager.getInstance(),"An error occured attempting to load group membership for \""+user.getLogin()+"\".\n\n"+
							Safmq.errorDecode(error),"Edit Users", JOptionPane.ERROR_MESSAGE);
				}
			} else if (error == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError(user.getServer());
			} else {
				JOptionPane.showMessageDialog(Manager.getInstance(),"An error occured attempting to load permissions for \""+user.getLogin()+"\".\n\n"+
						Safmq.errorDecode(error),"Edit Users", JOptionPane.ERROR_MESSAGE);
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
		MQConnection con = user.getServer().getCon();
		if (con != null) {
			error = con.UserSetPermissions(user.getLogin(),info.getModQueues(),info.getModUsers(),info.getModGroups());
			if (error == Safmq.EC_NOERROR) {
				Vector groups = new Vector();
				error = con.UserGetGroups(user.getLogin(),groups);
				if (error == Safmq.EC_NOERROR) {
					int x;
					DefaultListModel model = info.getGroupListModel();
					
					for (x=0;x<groups.size();x++) {
						error = con.GroupDeleteUser((String)groups.get(x),user.getLogin());
						if (error == Safmq.EC_NETWORKERROR) {
							Manager.getInstance().handleNetworkError(user.getServer());
							return;
						} else if (error != Safmq.EC_NOERROR) {
							JOptionPane.showMessageDialog(Manager.getInstance(),"An error occured attempting to store group membership for \""+user.getLogin()+"\".\n\n"+
									Safmq.errorDecode(error),"Edit Users", JOptionPane.ERROR_MESSAGE);
							doReload();
							return;
						}
					}
					
					for (x=0;x<model.size();x++) {
						error = con.GroupAddUser(((Group)model.get(x)).getName(),user.getLogin());
						if (error == Safmq.EC_NETWORKERROR) {
							Manager.getInstance().handleNetworkError(user.getServer());
							return;
						} else if (error != Safmq.EC_NOERROR) {
							JOptionPane.showMessageDialog(Manager.getInstance(),"An error occured attempting to store group membership for \""+user.getLogin()+"\".\n\n"+
									Safmq.errorDecode(error),"Edit Users", JOptionPane.ERROR_MESSAGE);
							doReload();
							return;
						}
					}
					
					for(x = 0; x < identity.getRemoves().size(); x++) {
						X509Identity id = (X509Identity)identity.getRemoves().get(x);
						error = con.RemoveUerIdentity(id.getSubjectDN(), id.getIssuerDN());
						if (error == Safmq.EC_NETWORKERROR) {
							Manager.getInstance().handleNetworkError(user.getServer());
							return;
						} else if (error != Safmq.EC_NOERROR) {
							JOptionPane.showMessageDialog(Manager.getInstance(),"An error occured attempting to store X509 Identity Tokens for \""+user.getLogin()+"\".\n\n"+
									Safmq.errorDecode(error),"Edit Users", JOptionPane.ERROR_MESSAGE);
							doReload();
							return;
						}
					}
					
					for(x = 0; x < identity.getAdds().size(); x++) {
						X509Identity id = (X509Identity)identity.getAdds().get(x);
						error = con.AddUserIdentity(id.getSubjectDN(), id.getIssuerDN(), user.getLogin());
						if (error == Safmq.EC_NETWORKERROR) {
							Manager.getInstance().handleNetworkError(user.getServer());
							return;
						} else if (error != Safmq.EC_NOERROR) {
							JOptionPane.showMessageDialog(Manager.getInstance(),"An error occured attempting to store  X509 Identity Tokens for \""+user.getLogin()+"\".\n\n"+
									Safmq.errorDecode(error),"Edit Users", JOptionPane.ERROR_MESSAGE);
							doReload();
							return;
						}
					}
					
					applyAction.setEnabled(false);
					reloadAction.setEnabled(false);
				} else if(error == Safmq.EC_NETWORKERROR) {
					Manager.getInstance().handleNetworkError(user.getServer());
				} else {
					JOptionPane.showMessageDialog(Manager.getInstance(),"An error occured attempting to store permissions for \""+user.getLogin()+"\".\n\n"+
							Safmq.errorDecode(error),"Edit Users", JOptionPane.ERROR_MESSAGE);
				}
			} else if (error == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError(user.getServer());
			} else {
				JOptionPane.showMessageDialog(Manager.getInstance(),"An error occured attempting to store permissions for \""+user.getLogin()+"\".\n\n"+
						Safmq.errorDecode(error),"Edit Users", JOptionPane.ERROR_MESSAGE);
				doReload();
			}
		}
		reloadAction.setEnabled(false);
		applyAction.setEnabled(false);
	}
	
	void doSetPassword() {
		PasswordChangeDialog dlg = new PasswordChangeDialog(user.getLogin());
		dlg.setVisible(true);
		if (dlg.isGood()) {
			MQConnection con = user.getServer().getCon();
			if (con != null) {
				int error = con.SetPassword(user.getLogin(),dlg.getPassword());
				if (error == Safmq.EC_NETWORKERROR) {
					Manager.getInstance().handleNetworkError(user.getServer());
				} else if (error != Safmq.EC_NOERROR) {
					JOptionPane.showMessageDialog(Manager.getInstance(),"An error occured attempting to change the password for \""+user.getLogin()+"\".\n\n"+
							Safmq.errorDecode(error),"Edit Users", JOptionPane.ERROR_MESSAGE);
				}
			}
		}
	}
	
	void doAddGroups() {
		MQConnection con = user.getServer().getCon();
		if (con != null) {
			Vector groups = new Vector();
			int error = con.EnumerateGroups(groups);
			if (error == Safmq.EC_NOERROR) {
				DefaultListModel 	model = info.getGroupListModel();
				int					x;
				for(x=0; x<model.size(); x++)
					groups.remove(((Group)model.get(x)).getName());
				Vector g = new Vector();
				for(x=0;x<groups.size();x++)
					g.add(new Group((String)groups.get(x),null));
				
				MultiChoiceDialog dlg = new MultiChoiceDialog("Please choose the groups to add", "Add Groups", g.toArray(), new IconListRenderer());
				dlg.setVisible(true);
				if (dlg.isGood()) {
					Object choices[] = dlg.getChoices();
					for(x=0;x<choices.length; x++)
						model.addElement(choices[x]);
					reloadAction.setEnabled(true);
					applyAction.setEnabled(true);
				}
				
			} else if (error == Safmq.EC_NETWORKERROR) {
				Manager.getInstance().handleNetworkError(user.getServer());
			} else {
				JOptionPane.showMessageDialog(Manager.getInstance(),"An error occured attempting to list groups for sfamq://"+user.getServer().getName()+".\n\n"+
						Safmq.errorDecode(error),"Edit Users", JOptionPane.ERROR_MESSAGE);
			}
		}
	}
	
	void doDeleteGroup() {
		if (info.deleteSelectedGroup()) {
			reloadAction.setEnabled(true);
			applyAction.setEnabled(true);
		}
	}
	
	public void itemStateChanged(ItemEvent e) {
		reloadAction.setEnabled(true);
		applyAction.setEnabled(true);
	}
}

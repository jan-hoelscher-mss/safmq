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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

/**
 * Displays a dialog to log be used to log into a SAFMQ server.
 * @author matt
 */
public class LoginDialog extends JDialog {
	JTextField			login = new JTextField();
	JPasswordField		password = new JPasswordField();
	JCheckBox			savePassword = new JCheckBox("Store Password", false);
	
	boolean				good = false;
	static Icon			icon = null;
	
	static {
		try {
			icon = new ImageIcon(Queue.class.getResource("images/server.gif"));
		} catch (Exception e) {
			e.printStackTrace();
		}	
	}
	
	/**
	 * Constructs the login dialog, provided the name of the server.
	 * @param server Name of the server being connected to
	 * @param loginName Optional name of the user logging in.
	 */
	public LoginDialog(String server, String loginName) {
		super(Manager.getInstance(),"Login Into SAFMQ Server", true);
		
		Action okAction = new SimpleAction("OK",(Integer)null,true) {
			public void actionPerformed(ActionEvent e) {
				onOK();
			}
		};
		Action cancelAction = new SimpleAction("Cancel",(Integer)null,true) {
			public void actionPerformed(ActionEvent e) {
				setVisible(false);
			}
		};
		
		//GridBagLayout		g;
		GridBagConstraints	c = new GridBagConstraints();
		Insets				origInsets = c.insets;
		JButton				ok = new JButton(okAction);
		JButton 			cancel = new JButton(cancelAction);
		JLabel				l;
		JPanel				content = new JPanel();
		
		this.setContentPane(content);
		//content.setLayout(g = new GridBagLayout());
		content.setLayout(new GridBagLayout());
		
		if (icon == null)
			icon = UIManager.getIcon("OptionPane.questionIcon");
		l = new JLabel(icon);
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.weighty = 0;
		c.anchor = GridBagConstraints.WEST;
		c.insets = new Insets(5,5,5,50);
		content.add(l,c);
		
		l = new JLabel("<html>Please enter the user name and password <br>" +			"to log into safmq://"+server+".");
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 0;
		c.anchor = GridBagConstraints.NORTHWEST;
		c.insets = new Insets(5,0,5,5);
		content.add(l,c);
		

		Dimension d = login.getMinimumSize();
		d = new Dimension(150,(int)d.getHeight());

		l = new JLabel("Login:");
		c.gridwidth = GridBagConstraints.RELATIVE;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 0;
		c.anchor = GridBagConstraints.EAST;
		c.insets = origInsets;
		content.add(l,c);
		
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 0;
		c.anchor = GridBagConstraints.WEST;
		content.add(login,c);
		login.setMinimumSize(d);
		login.setPreferredSize(d);
		if (loginName != null)
			login.setText(loginName);

		l = new JLabel("Password:");
		c.gridwidth = GridBagConstraints.RELATIVE;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 0;
		c.anchor = GridBagConstraints.EAST;
		c.insets = origInsets;
		content.add(l,c);
		
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 0;
		c.anchor = GridBagConstraints.WEST;
		content.add(password,c);
		password.setMinimumSize(d);
		password.setPreferredSize(d);
		
		

		JPanel p = new JPanel();
		c.gridwidth = GridBagConstraints.RELATIVE;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 0;
		c.weighty = 0;
		content.add(p,c);
		
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 0;
		c.anchor = GridBagConstraints.WEST;
		content.add(savePassword,c);

		// Add the OK and Cancel buttons in a new panel.		
		p = new JPanel();
		c.anchor = GridBagConstraints.WEST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		content.add(p,c);

		//p.setLayout(g = new GridBagLayout());
		p.setLayout(new GridBagLayout());

		c.insets = new Insets(10,5,5,5);
		c.anchor = GridBagConstraints.EAST;
		c.gridwidth = GridBagConstraints.RELATIVE;
		c.fill = GridBagConstraints.NONE;
		c.weightx=1;
		p.add(ok,c);
		
		c.anchor = GridBagConstraints.EAST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx=0;
		p.add(cancel,c);

		content.getInputMap(JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT).put(KeyStroke.getKeyStroke(KeyEvent.VK_ENTER,0),okAction);
		content.getActionMap().put(okAction,okAction);
		
		content.getInputMap(JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT).put(KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE,0),cancelAction);
		content.getActionMap().put(cancelAction,cancelAction);
		
		pack();
		this.setResizable(false);
	}
	
	void onOK() {
		good = true;
		setVisible(false);	
	}

	/**
	 * Provides the flag indicating the user clicked the OK button.
	 * @return The flag indicating the user clicked the OK button.
	 */
	public boolean isGood() {
		return good;
	}

	/**
	 * Provides the value for the login name.
	 * @return The value for the login name.
	 */
	public String getLogin() {
		return login.getText();
	}

	/**
	 * Provides the value for the password.
	 * @return The value for the password.
	 */
	public String getPassword() {
		return new String(password.getPassword());
	}

	/**
	 * Provides the value of the save password flag.
	 * @return The value of the save password flag.
	 */
	public boolean getSavePassword() {
		return savePassword.isSelected();
	}
}

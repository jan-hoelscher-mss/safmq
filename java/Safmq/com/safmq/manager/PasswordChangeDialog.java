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
 * @author Matt
 *
 */
public class PasswordChangeDialog extends JDialog {
	boolean good = false;
	JPasswordField	password1 = new JPasswordField("",20);
	JPasswordField	password2 = new JPasswordField("",20);
	
	public PasswordChangeDialog(String userName) {
		super(Manager.getInstance(),"Password Change",true);
		JPanel content = new JPanel();
		this.setContentPane(content);
		
		GridBagLayout 		g = new GridBagLayout();
		GridBagConstraints	c = new GridBagConstraints();
		Insets				insets = c.insets;
		
		content.setLayout(g);
		
		c.insets = new Insets(5,5,5,0);
		c.anchor = GridBagConstraints.NORTHWEST;
		c.gridwidth = 2;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 0;
		content.add(new JLabel(UIManager.getIcon("OptionPane.questionIcon")),c);
		
		c.insets = new Insets(5,20,5,5);
		c.anchor = GridBagConstraints.NORTH;
		c.gridwidth = GridBagConstraints.REMAINDER;
		content.add(new JLabel("Please enter the new password for \""+userName+"\""),c);
		
		JPanel sub = new JPanel();
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		content.add(sub,c);
		sub.setLayout(g);
		
		c.insets = insets; 
		c.anchor = GridBagConstraints.EAST;
		c.gridwidth = GridBagConstraints.RELATIVE;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.weighty = 0;
		sub.add(new JLabel("Password:"),c);
		
		c.anchor = GridBagConstraints.WEST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 0;
		sub.add(password1,c);
		
		c.anchor = GridBagConstraints.EAST;
		c.gridwidth = GridBagConstraints.RELATIVE;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.weighty = 0;
		sub.add(new JLabel("Confirmation:"),c);
		
		c.anchor = GridBagConstraints.WEST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 0;
		sub.add(password2,c);
		
		
		JPanel p;
		c.anchor = GridBagConstraints.NORTHWEST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 0;
		content.add(p = new JPanel(),c);
		p.setLayout(g = new GridBagLayout());
		
		Action ok;
		c.insets = new Insets(5,5,5,10);
		c.gridwidth = GridBagConstraints.RELATIVE;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.weighty = 0;
		c.anchor = GridBagConstraints.NORTHEAST;
		p.add(new JButton(ok = new SimpleAction("OK",(Integer)null,true) { public void actionPerformed(ActionEvent e) { onOK(); } }),c);
		content.getInputMap(JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT).put(KeyStroke.getKeyStroke(KeyEvent.VK_ENTER,0),ok);
		content.getActionMap().put(ok,ok);
		
		Action cancel;
		c.insets = new Insets(5,0,5,5);
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 0;
		c.anchor = GridBagConstraints.NORTHEAST;
		p.add(new JButton(cancel = new SimpleAction("Cancel",(Integer)null,true) { public void actionPerformed(ActionEvent e) { setVisible(false); } }),c);
		content.getInputMap(JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT).put(KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE,0),cancel);
		content.getActionMap().put(cancel,cancel);

		pack();
		this.setResizable(false);
	}
	
	/**
	 * Incase called instead of "show()".
	 */
	public void setVisible(boolean show) {
		if (show) {
			password1.setText("");
			password2.setText("");
			good = false;
		}
		super.setVisible(show);
	}
	
	void onOK() {
		String pwd1 = new String(password1.getPassword());
		String pwd2 = new String(password2.getPassword());
		
		if (pwd1.equals(pwd2)) {
			good = true;
			setVisible(false);	
		} else {
			JOptionPane.showMessageDialog(this,"The passwords entered do not match","Password Error",JOptionPane.ERROR_MESSAGE);	
		}
	}
	
	/**
	 * @return Returns the good.
	 */
	public boolean isGood() {
		return good;
	}
	
	public String getPassword() {
		return new String(password1.getPassword());
	}
}

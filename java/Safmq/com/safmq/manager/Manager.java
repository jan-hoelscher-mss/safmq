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
		
Created on May 12, 2005
*/
package com.safmq.manager;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;

import javax.swing.Action;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JSplitPane;
import javax.swing.UIManager;

/**
 * @author matt
 *
 */
public class Manager extends JFrame implements Runnable {
	ServerList		servers;
	ServerTreeView	treeView;
	JPanel			controlView = new JPanel();

	private static Manager	theManager = null;

	Action	newQueueAction;
	Action	newUserAction;
	Action	newGroupAction;

	private Manager() {
		super("SAFMQ Manager");
		
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		
		servers = new ServerList();
		treeView = new ServerTreeView(servers);

		JMenuBar	bar = new JMenuBar();
		
		JMenu		file = new JMenu("File");
		file.setMnemonic(KeyEvent.VK_F);
		
		file.add(new JMenuItem(new SimpleAction("New Server", new Integer(KeyEvent.VK_N), true) {
			public void actionPerformed(ActionEvent e){ addQueueServer(); } 
		}));
		file.addSeparator();
		file.add(treeView.getNewQueueAction());
		file.add(treeView.getNewUserAction());
		file.add(treeView.getNewGroupAction());
		file.addSeparator();
		file.add(treeView.getDeleteAction());
		file.addSeparator();
		file.add(new SimpleAction("Exit", new Integer(KeyEvent.VK_X), true) {
			public void actionPerformed(ActionEvent e){ System.exit(0); } 
		});
		
		bar.add(file);
		this.setJMenuBar(bar);

		controlView.setLayout(new GridLayout());
		controlView.setMinimumSize(new Dimension(50,50));
		controlView.setPreferredSize(new Dimension(400,400));

		JSplitPane sp = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, treeView, controlView);
		sp.setOpaque(true);
		setContentPane(sp);
		this.pack();
		this.setVisible(true);
	}
	
	public static synchronized Manager getInstance() {
		if (theManager == null)
			theManager = new Manager();
		return theManager;
	}

	public void addQueueServer() {
		NewServerDialog	dlg = new NewServerDialog();
		dlg.setVisible(true);
		if (dlg.isGood()) {
			String login = dlg.getLogin();
			String password = dlg.getPassword();
			String server = dlg.getServer();
			
			if (server.length() > 0) {
				//if (login.length() == 0)
				//	login = null;
				//if (password.length() == 0)
				//	password = null;
				addQueueServer(server,login,password,dlg.getSavePassword());	
			}
		}
	}
	
	public void addQueueServer(String serverurl, String username, String password, boolean storepassword) {
		servers.addServer(serverurl,username,password,storepassword);
		treeView.refreshRoot();
	}
	
	public void deleteQueueServer(Server s) {
		servers.deleteServer(s);
		treeView.refreshRoot();
	}
	
	/**
	 * Provides access to the tree view pane.
	 * 
	 * @return the pane representing the tree view
	 */
	public ServerTreeView getTreeView() {
		return treeView;
	}


	public void setControlView(Component c) {
		//controlView.setViewportView(c);
		controlView.removeAll();
		controlView.add(c);
		controlView.validate();
	}
	
	public void handleNetworkError(Server svr) {
		JOptionPane.showMessageDialog(this,"An error occurred communicating with safmq://"+svr.getName(),
														"Network Error",JOptionPane.ERROR_MESSAGE);
		servers.handleNetworkError(svr);
		treeView.resetChildren(svr, "Connection Closed by Server");
	}
	
	public void handleNetworkError(String msg, Server svr) {
		JOptionPane.showMessageDialog(this,msg,"Network Error",JOptionPane.ERROR_MESSAGE);
		servers.handleNetworkError(svr);
		treeView.resetChildren(svr, "Connection Closed by Server");
	}
	
	public void storeList() {
		servers.store();	
	}
	
	public void run() {
		servers.load();
	}

	public static void main(String args[]) {
		try {
			UIManager.setLookAndFeel("com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
			JFrame.setDefaultLookAndFeelDecorated(true);
		} catch (Exception e) {
			e.printStackTrace();
		}
		Manager.getInstance().run();
	}


}

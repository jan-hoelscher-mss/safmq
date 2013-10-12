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
		
Created on May 18, 2005
*/
package com.safmq.manager;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.Vector;

import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTable;
import javax.swing.KeyStroke;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;

import com.safmq.manager.queuetable.QueueModel;
import com.safmq.manager.queuetable.QueuePermissionsTableModel;

/**
 * @author matt
 */
public class QueuePermissionEditor extends JPanel implements MouseListener, TableModelListener, ListSelectionListener {
	Queue 						queue;
	
	JTable						permissionTable;
	QueuePermissionsTableModel	permissionModel;
	
	JButton						addActor;
	
	JTable						dataTable;
	QueueModel					dataModel;
	
	
	Action applyAction;
	Action reloadAction;
	Action deleteActorAction;
	
	public QueuePermissionEditor(Queue queue) {
		JButton			reload;
		JButton			apply;

		this.queue = queue;

		// Create a general keyboard action		
		deleteActorAction = new SimpleAction("Delete Actor", new Integer(KeyEvent.VK_D),true) {
			public void actionPerformed(ActionEvent e) { deleteSelection(); }
		};

		// Create Actions for the buttons
		applyAction = new SimpleAction("Apply",new Integer(KeyEvent.VK_A),false) {
			public void actionPerformed(ActionEvent e) { doApply(); }
		};
		apply = new JButton(applyAction);
		
		reloadAction = new SimpleAction("Reload",new Integer(KeyEvent.VK_R),false) {
			public void actionPerformed(ActionEvent e) { doReload(); }
		};
		reload = new JButton(reloadAction);

		// Begin setting up the layout.
		GridBagLayout		g = new GridBagLayout();
		GridBagConstraints	c = new GridBagConstraints();
		
		JPanel	securityPane = new JPanel();
		
		securityPane.setLayout(g);
		
		// Setup the table and add the table to the window
		permissionModel = new QueuePermissionsTableModel(queue);
		permissionModel.addTableModelListener(this);
		permissionTable = new JTable(permissionModel);
		JScrollPane tablepane = new JScrollPane(permissionTable);
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		c.gridwidth = GridBagConstraints.REMAINDER;
		securityPane.add(tablepane,c);
		permissionTable.addMouseListener(this); // TO Enable Popup menu click detection
		permissionTable.getSelectionModel().addListSelectionListener(this);
		
		permissionTable.getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_DELETE,0), deleteActorAction);
		
		// Set the constraints for use with the buttons
		c.anchor = GridBagConstraints.EAST;
		c.insets = new Insets(0,5,0,45);
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1;
		c.weighty = 0.0;
		c.gridwidth = 1;
		
		// Add the "Add Actor" button
		addActor = new JButton(new SimpleAction("Add Actor", new Integer(KeyEvent.VK_C), true) {
			public void actionPerformed(ActionEvent e) {
				doAddActor();
			}
		});
		securityPane.add(addActor,c);
		
		// Add the "Reload" button
		c.insets = new Insets(0,0,0,10);
		c.weightx = 0;
		c.gridwidth = GridBagConstraints.RELATIVE;
		securityPane.add(reload,c);

		// Add the "Apply" button.
		c.gridwidth = GridBagConstraints.REMAINDER;
		securityPane.add(apply,c);
		
		
		JTabbedPane	tab = new JTabbedPane();
		tab.addTab("Security", securityPane);
		
		dataTable = new JTable(dataModel = new QueueModel(queue.server.getCon(), queue.getName()));
		JScrollPane	qdata = new JScrollPane(dataTable);
		tab.addTab("Data", qdata);
		
		this.setLayout(new GridBagLayout());
		c = new GridBagConstraints();
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.weightx = 1;
		c.weighty = 1;
		c.fill = GridBagConstraints.BOTH;
		
		this.add(tab, c);
	}

	public void doReload() {
		queue.reload();
	}
	public void doApply() {
		queue.saveChanges();
		applyAction.setEnabled(false);
		reloadAction.setEnabled(false);
	}
	public void doAddActor() {
		try {
			QueueAddActor dlg = new QueueAddActor(queue);
			dlg.setVisible(true);
			if (dlg.isGood()) {
				Object newActors[] = dlg.getNewActors();
				for(int x=0; x<newActors.length; x++) {
					queue.getPermissions().add(newActors[x]);				
				}
				permissionModel.fireTableStructureChanged();
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void deleteSelection() {
		int row = permissionTable.getSelectedRow();
		Vector perms = queue.getPermissions();
		if (row >= 0 && row < perms.size()) {
			int result = JOptionPane.showConfirmDialog(Manager.getInstance(),
					"Are you sure you wish to remove \"" + permissionModel.getValueAt(row,QueuePermissionsTableModel.NAME_COL)+"\"?",
					"Permission Deletion", JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE);
			if (result == JOptionPane.YES_OPTION) {		
				applyAction.setEnabled(true);
				reloadAction.setEnabled(true);
				permissionTable.getActionMap().remove(deleteActorAction);
				queue.deletePerm((Queue.perms)perms.get(row));
				permissionModel.notifyRowDeletion(row);
			}
		}
	}	

	/**
	 * Handles mouse down events in the table, especially the right-click events.
	 * @see java.awt.event.MouseListener#mouseClicked(java.awt.event.MouseEvent)
	 */
	public void mouseClicked(MouseEvent e) {}
	public void mouseEntered(MouseEvent e) {}
	public void mouseExited(MouseEvent e) {}
	public void mousePressed(MouseEvent e) {}
	public void mouseReleased(MouseEvent e) {
		if (e.isPopupTrigger()) {
			// TODO: Implement a menu to add/remove actors from the perms list
		}
	}

	/**
	 * Handles changes to the 
	 */
	public void tableChanged(TableModelEvent e) {
		applyAction.setEnabled(true);
		reloadAction.setEnabled(true);
	}
	
	/**
	 * Enables the delete actor action in response to a new selection taking place.
	 * 
	 * @see javax.swing.event.ListSelectionListener#valueChanged(javax.swing.event.ListSelectionEvent)
	 */
	public void valueChanged(ListSelectionEvent e) {
		if (permissionTable.getSelectedRow() >= 0)
			permissionTable.getActionMap().put(deleteActorAction,deleteActorAction);
		else
			permissionTable.getActionMap().remove(deleteActorAction);
	}

}

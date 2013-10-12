package com.safmq.manager.UserEditorPanels;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.KeyEvent;

import javax.swing.Action;
import javax.swing.DefaultListModel;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.KeyStroke;

import com.safmq.manager.IconListRenderer;
import com.safmq.manager.User;
import com.safmq.manager.UserEditor;

public class BasicInfoPanel extends JPanel  {
	JCheckBox	modUsers = new JCheckBox("Add/Modify Users");
	JCheckBox	modGroups = new JCheckBox("Add/Modify Groups");
	JCheckBox	modQueues = new JCheckBox("Add/Modify Queues");
	JList		groupsList = new JList(new DefaultListModel());

	public BasicInfoPanel(User user, Action deleteGroupAction, UserEditor ed) {
		// Setup the data panel ///////////////////////////////////////////////
		GridBagConstraints c = new GridBagConstraints();
		Insets origInsets = c.insets;
		
		this.setLayout(new GridBagLayout());
		c.insets = origInsets;
		c.anchor = GridBagConstraints.NORTHWEST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.weightx = 1;
		c.weighty = 0;
		
		modUsers.setOpaque(false);
		modGroups.setOpaque(false);
		modQueues.setOpaque(false);

		this.add(modUsers,c);
		this.add(modGroups,c);
		this.add(modQueues,c);
		
		c.insets = new Insets(5,0,0,0);
		this.add(new JLabel("Groups:"),c);
		c.insets = new Insets(0,0,0,10);
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		groupsList.setVisibleRowCount(-1);
		groupsList.setLayoutOrientation(JList.VERTICAL_WRAP);
		groupsList.setCellRenderer(new IconListRenderer());
		this.add(new JScrollPane(groupsList),c);
		groupsList.getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_DELETE,0),deleteGroupAction);
		groupsList.getActionMap().put(deleteGroupAction,deleteGroupAction);
		
		modUsers.addItemListener(ed);
		modGroups.addItemListener(ed);
		modQueues.addItemListener(ed);
	}
	
	public void setModUsers(boolean modUsers) {
		this.modUsers.setSelected(modUsers);
	}
	public boolean getModUsers() {
		return modUsers.isSelected();
	}
	
	public void setModGroups(boolean modGroups) {
		this.modGroups.setSelected(modGroups);
	}
	public boolean getModGroups() {
		return modGroups.isSelected();
	}

	public void setModQueues(boolean modQueues) {
		this.modQueues.setSelected(modQueues);
	}
	public boolean getModQueues() {
		return modQueues.isSelected();
	}
	public DefaultListModel getGroupListModel() {
		return (DefaultListModel)groupsList.getModel();
	}
	
	public boolean deleteSelectedGroup() {
		int sel = groupsList.getSelectedIndex();
		if (sel >= 0) {
			DefaultListModel model = (DefaultListModel)groupsList.getModel();
			model.remove(sel);
			return true;
		}
		return false;
	}
}

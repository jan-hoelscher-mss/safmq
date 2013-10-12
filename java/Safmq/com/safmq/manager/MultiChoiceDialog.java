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
public class MultiChoiceDialog extends JDialog {
	boolean good = false;
	JList	list;
	
	MultiChoiceDialog(String message, String title, Object choices[], ListCellRenderer renderer) {
		super(Manager.getInstance(),title,true);
		
		JPanel content = new JPanel();
		
		this.setContentPane(content);
		
		list = new JList(choices);
		if (renderer != null)
			list.setCellRenderer(renderer);
		list.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
		list.setVisibleRowCount(10);
		list.setLayoutOrientation(JList.VERTICAL_WRAP);

		GridBagLayout 		g = new GridBagLayout();
		GridBagConstraints 	c = new GridBagConstraints();
		//Insets				insets = c.insets;
		content.setLayout(g);
		
		c.insets = new Insets(5,5,5,5);
		c.anchor = GridBagConstraints.NORTHWEST;
		c.gridwidth = GridBagConstraints.RELATIVE;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 0;
		content.add(new JLabel(UIManager.getIcon("OptionPane.questionIcon")),c);
		
		c.insets = new Insets(5,5,5,5);
		c.gridwidth = GridBagConstraints.REMAINDER;
		content.add(new JLabel(message),c);
		
		c.insets = new Insets(0,5,0,5);
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.gridwidth = GridBagConstraints.REMAINDER;
		JScrollPane sp;
		content.add(sp = new JScrollPane(list),c);
		
		Dimension d = sp.getPreferredSize();
		d = new Dimension(400>d.getHeight()*3?400:(int)d.getHeight()*3,(int)d.getHeight());
		list.setPreferredSize(d);
		list.setMinimumSize(d);
		d = new Dimension((int)d.getWidth()+20,(int)d.getHeight()+20);
		sp.setPreferredSize(d);
		sp.setMinimumSize(d);
		
		JPanel p;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
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
		c.insets = new Insets(5,0,5,0);
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0;
		c.weighty = 0;
		c.anchor = GridBagConstraints.NORTHEAST;
		p.add(new JButton(cancel = new SimpleAction("Cancel",(Integer)null,true) { public void actionPerformed(ActionEvent e) { setVisible(false); } }),c);
		content.getInputMap(JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT).put(KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE,0),cancel);
		content.getActionMap().put(cancel,cancel);
		
		pack();
		setResizable(false);
	}
	
	/**
	 * Incase called instead of "show()".
	 */
	public void setVisible(boolean show) {
		if (show) good = false;
		super.setVisible(show);
	}
	
	void onOK() {
		good = true;
		setVisible(false);	
	}
	
	/**
	 * @return Returns the good.
	 */
	public boolean isGood() {
		return good;
	}
	
	public Object[] getChoices() {
		return list.getSelectedValues();
	}
}

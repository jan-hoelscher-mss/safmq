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

import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.Icon;
import javax.swing.JList;
import javax.swing.JScrollPane;
import javax.swing.ListModel;

/**
 * @author matt
 */
public class IconListView extends JScrollPane {
	interface Activateable {
		public void activate();	
	}
	
	JList				list;
	IconListRenderer	ivr;

	IconListView(ListModel model) {
		init(model);
	}
	
	IconListView(ListModel model, Icon defaultIcon) {
		init(model);
		ivr.setDefaultIcon(defaultIcon);
	}
	
	void init(ListModel model) {
		list = new JList(model);
		list.setCellRenderer(ivr = new IconListRenderer());
		this.setViewportView(list);
		list.addMouseListener(new MouseAdapter() {
			public void mouseClicked(MouseEvent e) {
				if (e.getClickCount() == 2) {
					int index = list.locationToIndex(e.getPoint());
					handleDoubleClick(index);
				 }
			}
		});
		list.setLayoutOrientation(JList.VERTICAL_WRAP);
		list.setVisibleRowCount(-11);
	}
	
	public void handleDoubleClick(int index) {
		Object element = list.getModel().getElementAt(index);
		
		if (element instanceof Activateable) 
			((Activateable)element).activate();
	}
}

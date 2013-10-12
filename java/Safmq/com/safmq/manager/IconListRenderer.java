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

import java.awt.Component;

import javax.swing.DefaultListCellRenderer;
import javax.swing.Icon;
import javax.swing.JList;

public class IconListRenderer extends DefaultListCellRenderer {
	Icon	defaultIcon;
		
	public Component getListCellRendererComponent(
		 JList list,
		 Object value,
		 int index,
		 boolean isSelected,
		 boolean cellHasFocus)
	 {
		super.getListCellRendererComponent(list,value,index,isSelected,cellHasFocus);
		if (value instanceof IconItem) {
			setIcon(((IconItem)value).getIcon());	
		} else if (defaultIcon != null) {
			setIcon(defaultIcon);
		}
		return this;
	 }
		 
	 public void setDefaultIcon(Icon defaultIcon) {
		this.defaultIcon = defaultIcon;
	 }
}
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

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Vector;

import javax.swing.Action;
import javax.swing.Icon;

/**
 * @author matt
 */
public abstract class SimpleAction implements Action {
	boolean 	enabled = false;
	Vector		pcl = new Vector();
	Integer 	acclKey = null;
	String		name = null;
	Icon		icon = null;
	
	public SimpleAction() {
	}
	
	public SimpleAction(String name, Integer acclKey, boolean enabled) {
		this.name = name;
		this.acclKey = acclKey;
		this.enabled = enabled;
	}
	
	public SimpleAction(String name, Icon icon, boolean enabled) {
		this.name = name;
		this.icon = icon;
		this.enabled = enabled;	
	}
	
	public SimpleAction(boolean enabled) {
		this.enabled = enabled;
	}
	
	public void addPropertyChangeListener(PropertyChangeListener listener) { 
		pcl.add(listener);
	}
	
	public Object getValue(String key) {
		if (key.equals(Action.MNEMONIC_KEY))	return acclKey;
		else if (key.equals(Action.NAME))		return name;
		else if (key.equals(Action.SMALL_ICON)) return icon;
		return null;
	}
	
	public boolean isEnabled() { 
		return enabled;
	}
	
	public void putValue(String key, Object value) {
		PropertyChangeEvent e = null;
		if (key.equals(Action.MNEMONIC_KEY)) {
			e = new PropertyChangeEvent(this,Action.MNEMONIC_KEY,acclKey,value);
			acclKey = (Integer)value;
		} else if (key.equals(Action.NAME)) {
			e = new PropertyChangeEvent(this,Action.NAME,name,value);
			name = value.toString();
		} else if (key.equals(Action.SMALL_ICON)) {
			e = new PropertyChangeEvent(this,Action.SMALL_ICON,icon,value);
			icon = (Icon)value;
		}
		for(int x=0; e!=null && x<pcl.size(); x++) ((PropertyChangeListener)pcl.get(x)).propertyChange(e);
	}
	
	public void removePropertyChangeListener(PropertyChangeListener listener) { 
		pcl.remove(listener); 
	}
	
	public void setEnabled(boolean b) {
		PropertyChangeEvent	e = new PropertyChangeEvent(this,"enabled",new Boolean(enabled), new Boolean(b));
		enabled = b;
		for(int x=0; e!=null && x<pcl.size(); x++) ((PropertyChangeListener)pcl.get(x)).propertyChange(e);
	}
}
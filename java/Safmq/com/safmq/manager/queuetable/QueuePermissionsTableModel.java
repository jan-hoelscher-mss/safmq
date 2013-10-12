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
package com.safmq.manager.queuetable;

import javax.swing.table.AbstractTableModel;

import com.safmq.manager.Queue;

/**
 * @author matt
 */
public class QueuePermissionsTableModel extends AbstractTableModel {
	Queue queue;
	
	public final static int NAME_COL = 0,
							READ_COL = 1,
							WRITE_COL = 2,
							MOD_SECURITY_COL = 3,
							DESTROY_COL = 4;
	
	public final static String COLUMN_NAMES[] = {
					"Name",
					"Read",
					"Write",
					"Modify Security",
					"Destroy"
	};
	
	
	public QueuePermissionsTableModel(Queue queue) {
		this.queue = queue;	
	}

	public int getColumnCount() {
		return 5;
	}

	public int getRowCount() {
		return queue.getPermissions().size();
	}

	public Object getValueAt(int rowIndex, int columnIndex) {
		Queue.perms perms = (Queue.perms)queue.getPermissions().get(rowIndex);
		switch (columnIndex) {
			case NAME_COL:
				return perms.getName() + (perms.isGroup() ? " (group)" : " (user)");
			case READ_COL:
				return perms.getRead();
			case WRITE_COL:
				return perms.getWrite();
			case MOD_SECURITY_COL:
				return perms.getChangeSecurity();
			case DESTROY_COL:
				return perms.getDestroy();
		}
		return null;
	}

	public Class getColumnClass(int c) {
		switch (c) {
			case NAME_COL:
				return String.class;
			default:
				return Boolean.class;
		}
	}

	public String getColumnName(int column) {
		return COLUMN_NAMES[column];
	}

	/**
	 * @see javax.swing.table.TableModel#isCellEditable(int, int)
	 */
	public boolean isCellEditable(int rowIndex, int columnIndex) {
		return columnIndex > 0;
	}

	/**
	 */
	public void setValueAt(Object aValue, int rowIndex, int columnIndex) {
		Queue.perms perms = (Queue.perms)queue.getPermissions().get(rowIndex);
		switch (columnIndex) {
			case READ_COL:
				perms.setRead((Boolean)aValue);
				perms.setModified();
				this.fireTableCellUpdated(rowIndex,columnIndex);
				break;
			case WRITE_COL:
				perms.setWrite((Boolean)aValue);
				perms.setModified();
				this.fireTableCellUpdated(rowIndex,columnIndex);
				break;
			case MOD_SECURITY_COL:
				perms.setChangeSecurity((Boolean)aValue);
				perms.setModified();
				this.fireTableCellUpdated(rowIndex,columnIndex);
				break;
			case DESTROY_COL:
				perms.setDestroy((Boolean)aValue);
				perms.setModified();
				this.fireTableCellUpdated(rowIndex,columnIndex);
				break;
		}
	}
	
	public void notifyRowDeletion(int row) {
		fireTableRowsDeleted(row,row);	
	}
}

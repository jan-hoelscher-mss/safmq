package com.safmq.manager.UserEditorPanels;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.ItemSelectable;
import java.awt.event.ActionEvent;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.util.Vector;

import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.KeyStroke;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableModel;

import com.safmq.X509Identity;
import com.safmq.manager.NewIdentityDialog;
import com.safmq.manager.SimpleAction;
import com.safmq.manager.User;

public class IdentityPanel extends JPanel implements ItemSelectable {
	Vector		itemListeners = new Vector();
	User		user;
	Vector		adds = new Vector();
	Vector		removes = new Vector();
	JTable		identTable;
	
	Action deleteIdentityAction = new SimpleAction("Delete Identity", (Integer)null, true) {
		public void actionPerformed(ActionEvent e) {
			doDeleteIdentity();
		}
	};
	
	Action addIdentityAction = new SimpleAction("Add Identity", (Integer)null, true) {
		public void actionPerformed(ActionEvent e) {
			doAddIdentity();
		}
	};
	
	class IDTable extends AbstractTableModel {
		Vector ids;

		public IDTable(Vector ids) {
			this.ids = ids;
		}

		public int getColumnCount() {
			return 2;
		}

		public int getRowCount() {
			return ids.size();
		}

		public String getColumnName(int column) {
			return (column == 0) ? "Subject DN" : "Issuer DN";
		}

		public Object getValueAt(int rowIndex, int columnIndex) {
			X509Identity id = (X509Identity) ids.get(rowIndex);
			return (columnIndex == 0) ? id.getSubjectDN() : id.getIssuerDN();
		}

		public Class getColumnClass(int columnIndex) {
			return String.class;
		}
	}

	public IdentityPanel(User user) {
		this.user = user;
		
		this.setLayout(new GridBagLayout());

		GridBagConstraints 	c = new GridBagConstraints();
		Insets				origInsets = c.insets;
		
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.fill = GridBagConstraints.NONE;
		c.anchor = GridBagConstraints.NORTHEAST;
		c.weightx = 1;
		c.weighty = 0;
		c.insets = new Insets(0,0,2,5);
		JButton	addButton = new JButton(addIdentityAction);
		this.add(addButton, c);
		
		c.insets = origInsets;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.anchor = GridBagConstraints.NORTHWEST;
		c.fill = GridBagConstraints.BOTH;
		c.weightx = 1;
		c.weighty = 1;
		TableModel model = new IDTable(user.getIdentities());
		identTable = new JTable(model);
		this.add(new JScrollPane(identTable), c);
		
		identTable.setCellSelectionEnabled(false);
		identTable.setColumnSelectionAllowed(false);
		identTable.setRowSelectionAllowed(true);
		identTable.getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_DELETE,0),deleteIdentityAction);
		identTable.getActionMap().put(deleteIdentityAction,deleteIdentityAction);
	}
	
	public Vector getAdds() {
		return adds;
	}
	
	public Vector getRemoves() {
		return removes;
	}
	
	public void doDeleteIdentity() {
		int sel = identTable.getSelectedRow();
		Vector ids = user.getIdentities();
		if (sel >= 0 && sel < ids.size()) {
			X509Identity id = (X509Identity)ids.get(sel);
			
			adds.remove(id); // remove from the add list if it's there
			if (removes.indexOf(id) < 0) // add to the remove list
				removes.add(id);
			ids.remove(sel);
			
			IDTable m = (IDTable)identTable.getModel();
			m.fireTableRowsDeleted(sel,sel);
			fireItemChanged();
		}
	}
	
	public void doAddIdentity() {
		NewIdentityDialog	dlg = new NewIdentityDialog();
		dlg.setVisible(true);
		if (dlg.isGood()) {
			String subjectDN = dlg.getSubjectDN();
			String issuerDN = dlg.getIssuerDN();
			
			if (subjectDN.length() > 0 && issuerDN.length() > 0) {
				X509Identity	id = new X509Identity(subjectDN, issuerDN);
				Vector			ids = user.getIdentities();
				
				removes.remove(id); // remove from the remove list.
				adds.add(id);		// add to the add list
				ids.add(id);		// add to the display list
				
				IDTable m = (IDTable)identTable.getModel();
				m.fireTableRowsInserted(ids.size()-1,ids.size()-1);
				fireItemChanged();
			}
		}
	}
	
	public void addItemListener(ItemListener listener) {
		itemListeners.add(listener);
	}
	
	public Object[] getSelectedObjects() {
		return null;
	}

	public void removeItemListener(ItemListener listener) {
		itemListeners.remove(listener);
	}
	
	protected void fireItemChanged() {
		ItemEvent e = new ItemEvent(this, ItemEvent.ITEM_STATE_CHANGED, this, ItemEvent.SELECTED);
		for(int x=0; x<itemListeners.size(); x++) {
			((ItemListener)itemListeners.get(x)).itemStateChanged(e);
		}
	}


}

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
		
Created on May 17, 2005
*/
package com.safmq.manager;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.util.Vector;

import javax.swing.Action;
import javax.swing.Icon;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTree;
import javax.swing.KeyStroke;
import javax.swing.event.TreeExpansionEvent;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.event.TreeWillExpandListener;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.ExpandVetoException;
import javax.swing.tree.TreePath;



class ServerTreeCellRenderer extends DefaultTreeCellRenderer {
	public Component getTreeCellRendererComponent(JTree tree, Object value, boolean sel, boolean expanded, boolean leaf, int row, boolean hasFocus) {
		super.getTreeCellRendererComponent(tree,value,sel,expanded,leaf,row,hasFocus);
		Object val = ((DefaultMutableTreeNode)value).getUserObject();
		if (val instanceof IconItem) {
			Icon icon = ((IconItem)val).getIcon();
			if (icon != null)
				if(!tree.isEnabled())
					setDisabledIcon(icon);
				else
					setIcon(icon);
		}
		return this;
	}
}

/**
 * @author matt
 */
public class ServerTreeView extends JScrollPane implements TreeSelectionListener, TreeWillExpandListener {
	DefaultMutableTreeNode	root;
	JTree					tree;
	DefaultTreeModel		model;

	Action 					newQueueAction;
	Action					newUserAction;
	Action					newGroupAction;
	Action					deleteAction;
	
	public ServerTreeView(Object rootObject) {
		tree = new JTree();
		setViewportView(tree);
		root = new DefaultMutableTreeNode(rootObject);
		model = (DefaultTreeModel)tree.getModel();
		model.setRoot(root);
		root.add(new DefaultMutableTreeNode("No Servers Configured"));
		tree.setRootVisible(true);
		tree.setShowsRootHandles(true);
		tree.addTreeSelectionListener(this);
		tree.addTreeWillExpandListener(this);
		
		tree.setCellRenderer(new ServerTreeCellRenderer());
		
		newQueueAction = new SimpleAction("New Queue", new Integer(KeyEvent.VK_Q), false) {
			public void actionPerformed(ActionEvent e)	{ addQueue(); } 
		};
		newUserAction = new SimpleAction("New User", new Integer(KeyEvent.VK_U), false) {
			public void actionPerformed(ActionEvent e)	{ addUser(); } 
		};
		newGroupAction = new SimpleAction("New Group", new Integer(KeyEvent.VK_G), false) {
			public void actionPerformed(ActionEvent e)	{ addGroup(); }
		};
		deleteAction = new SimpleAction("Delete", new Integer(KeyEvent.VK_D), false) {
			public void actionPerformed(ActionEvent e)	{ deleteSelected(); }	
		};
		
		tree.getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_DELETE,0),deleteAction);
	}
	
	/**
	 * Locates the node containing <code>userobj</code> and removes all the children
	 * of that node and replaces them with a new node with <code>child</code> as the 
	 * user data.  This method is typically used to reset the contents of a node containing
	 * a referece to a <code>Server</code>.  All the children of that node are replaced
	 * with the new object passed as <code>child</code>.
	 * 
	 * @param userobj 	The data from the node to be located
	 * @param child 	The new data for the single child of the node to be located.
	 */
	public void resetChildren(Object userobj, Object child) {
		DefaultMutableTreeNode n;
		for(int x=0;x<root.getChildCount();x++){
			n = (DefaultMutableTreeNode)root.getChildAt(x);
			if (n.getUserObject().equals(userobj)) {
				n.removeAllChildren();
				n.add(new DefaultMutableTreeNode(child));
				model.nodeStructureChanged(n);
				break;
			}
		}
	}
	
	/**
	 * Refereshes the root node's list of children.
	 */
	public void refreshRoot() {
		ServerList sl = (ServerList)root.getUserObject();
		sl.setNodeChildren(tree, root);
		tree.setSelectionPath(null);
		
		TreePath tp = new TreePath(root.getPath());
		tree.expandPath(tp);
		tree.setSelectionPath(tp);
	}
	
	public void activatePath(TreePath path) {
		tree.setSelectionPath(null);
		tree.expandPath(path);
		tree.setSelectionPath(path);
	}
	
	/**
	 * Called when a node in the <code>tree</code> is selected.
	 * 
	 * @param event Used to determine which node has been selected.
	 * 
	 * @see javax.swing.event.TreeSelectionListener#valueChanged(javax.swing.event.TreeSelectionEvent)
	 */
	public void valueChanged(TreeSelectionEvent event) {
		TreePath tp = event.getPath();
		if (tp != null) {
			Object opath[] = tp.getPath();
			DefaultMutableTreeNode	n = (DefaultMutableTreeNode)opath[opath.length-1];
			if (n.getUserObject() instanceof ServerTreeViewSelectionListener) {
				((ServerTreeViewSelectionListener)n.getUserObject()).doSelection(tree, n);
			}

			if (opath.length >= 2 && (((DefaultMutableTreeNode)opath[1]).getUserObject() instanceof Server)) {
				newQueueAction.setEnabled(true);			
				newUserAction.setEnabled(true);			
				newGroupAction.setEnabled(true);
			} else {
				newQueueAction.setEnabled(false);			
				newUserAction.setEnabled(false);			
				newGroupAction.setEnabled(false);
			}
			
			Object userObject = n.getUserObject();
			if (userObject != null && userObject instanceof Deleteable) {
				deleteAction.setEnabled(true);
				deleteAction.putValue(Action.NAME,"Delete "+((Deleteable)userObject).getDescription());
				tree.getActionMap().put(deleteAction,deleteAction);
			} else {
				deleteAction.setEnabled(false);
				deleteAction.putValue(Action.NAME,"Delete");	
				tree.getActionMap().remove(deleteAction);
			}
			
			deleteAction.setEnabled(n.getUserObject() instanceof Deleteable);
		} else {
			newQueueAction.setEnabled(false);			
			newUserAction.setEnabled(false);			
			newGroupAction.setEnabled(false);
			deleteAction.setEnabled(false);	
			deleteAction.putValue(Action.NAME,"Delete");	
			tree.getActionMap().remove(deleteAction);
		}
	}

	/**
	 * Called when a branch in the <code>tree</code> is being expanded.
	 * 
	 * @param event Used to determine which node is being expanded
	 * 
	 * @see javax.swing.event.TreeWillExpandListener#treeWillExpand(javax.swing.event.TreeExpansionEvent)
	 */
	public void treeWillExpand(TreeExpansionEvent event) throws ExpandVetoException {
		Object					opath[] = event.getPath().getPath();
		DefaultMutableTreeNode	n = (DefaultMutableTreeNode)opath[opath.length-1];
		Object					userObject = n.getUserObject();
		
		if (userObject instanceof ServerTreeViewExpansionListener) {
			((ServerTreeViewExpansionListener)userObject).willExpand(tree, n);
		}
	}
	
	/**
	 * Called when a branch in the <code>tree</code> is being collapsed.
	 * 
	 * @see javax.swing.event.TreeWillExpandListener#treeWillCollapse(javax.swing.event.TreeExpansionEvent)
	 */
	public void treeWillCollapse(TreeExpansionEvent event) throws ExpandVetoException {	}
	/**
	 * @return
	 */
	public Action getNewGroupAction() {
		return newGroupAction;
	}

	/**
	 * @return
	 */
	public Action getNewQueueAction() {
		return newQueueAction;
	}

	/**
	 * @return
	 */
	public Action getNewUserAction() {
		return newUserAction;
	}

	public void selectNodeInstance(DefaultMutableTreeNode parent, Class c) {
		DefaultMutableTreeNode node;	
		for(int x=0;x<parent.getChildCount();x++) {
			node = (DefaultMutableTreeNode)parent.getChildAt(x);
			if (node.getUserObject().getClass() == c) {
				tree.setSelectionPath(null);
				tree.setSelectionPath(new TreePath(node.getPath()));				
			}
		}
	}

	public void addQueue() {
		TreePath path = tree.getSelectionPath();
		if (path != null) {
			Object objPath[] = path.getPath();
			if (objPath.length >= 2) { // contains at least one server record
				String 					name = JOptionPane.showInputDialog(Manager.getInstance(),
															"Please enter the name of the new queue",
															"Create a New Queue",
															JOptionPane.QUESTION_MESSAGE);
				if (name != null && name.length() > 0) {
					DefaultMutableTreeNode	node = (DefaultMutableTreeNode)objPath[1];
					Server 					server = (Server)node.getUserObject();
					if (server.addQueue(name)) {
						selectNodeInstance(node,QueueList.class);
					}
				}
			}
		}
	}
	
	public void addUser() {
		TreePath path = tree.getSelectionPath();
		if (path != null) {
			Object objPath[] = path.getPath();
			if (objPath.length >= 2) { // contains at least one server record
				//Object		options[] = {"User Name", "Description", "Password"};
				NewUserDialog	dlg = new NewUserDialog();
				dlg.setVisible(true);
				if (dlg.isGood()) {
					String login = dlg.getLogin();
					String desc = dlg.getDescription();
					String password = dlg.getPassword();
					
					if (login.length() > 0) {
						DefaultMutableTreeNode	node = (DefaultMutableTreeNode)objPath[1];
						Server 					server = (Server)node.getUserObject();
						if (server.addUser(login,desc,password)) {
							selectNodeInstance(node,UserList.class);
						}
					}
				}
			}
		}
	}
	
	public void addGroup() {
		TreePath path = tree.getSelectionPath();
		if (path != null) {
			Object objPath[] = path.getPath();
			if (objPath.length >= 2) { // contains at least one server record
				String 					name = JOptionPane.showInputDialog(Manager.getInstance(),
												"Please enter the name of the new group", 
												"Create a new Group",
												JOptionPane.QUESTION_MESSAGE);
				if (name != null && name.length() > 0) {
					DefaultMutableTreeNode	node = (DefaultMutableTreeNode)objPath[1];
					Server 					server = (Server)node.getUserObject();
					if (server.addGroup(name)) {
						selectNodeInstance(node,GroupList.class);
					}
				}
			}
		}
	}
	
	public void deleteSelected() {
		TreePath path = tree.getSelectionPath();
		if (path != null) {
			Object objPath[] = path.getPath();
			DefaultMutableTreeNode	node = (DefaultMutableTreeNode)objPath[objPath.length-1];
			Deleteable				deleteable = (Deleteable)node.getUserObject();
			if (deleteable.delete()) {
				// change the selection path to the above item
				Vector	newPath = new Vector();
				for(int x=0;x<objPath.length-1;x++)
					newPath.add(objPath[x]);				
				tree.setSelectionPath(new TreePath(newPath.toArray()));
			}
		}
	}
	/**
	 * Provides access to the delete action.
	 * @return The delete action.
	 */
	public Action getDeleteAction() {
		return deleteAction;
	}

}

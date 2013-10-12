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
		
Created on May 25, 2005
*/
package com.safmq.manager;

import java.util.Vector;

import javax.swing.AbstractListModel;
import javax.swing.JTree;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.DefaultTreeSelectionModel;
import javax.swing.tree.TreePath;


/**
 * @author matt
 */
public abstract class AbstractManagerList 
		extends AbstractListModel 
		implements ServerTreeViewExpansionListener, ServerTreeViewSelectionListener, IconListView.Activateable, TreePathable
{
	Vector	list = new Vector();
	
	/**
	 * Method for reloading the list
	 * 
	 * @return
	 */
	public abstract boolean reload();
	
	/**
	 * Called when a node is first added to the list.
	 * 
	 * @param n The node which was just added.
	 */
	public void onAddNode(DefaultMutableTreeNode n) {
	}
	
	public Vector getList() {
		return list;
	}
	
	/**
	 * @see com.safmq.manager.ServerTreeViewExpansionListener#willExpand(javax.swing.JTree, javax.swing.tree.DefaultMutableTreeNode)
	 */
	public boolean willExpand(JTree tree, DefaultMutableTreeNode container) {
		if (reload()) {
			setNodeChildren(tree, container);
			TreePath tp = ((DefaultTreeSelectionModel)tree.getSelectionModel()).getSelectionPath();
			if (tp != null) {
				Object objPath[] = tp.getPath();
				if (((DefaultMutableTreeNode)objPath[objPath.length-1]).getUserObject() == this) {
					Manager.getInstance().setControlView(new IconListView(this));
				}
			}
			return true;
		}
		return false;
	}

	protected boolean setNodeChildren(JTree tree, DefaultMutableTreeNode container) {
		DefaultMutableTreeNode	n;
		DefaultMutableTreeNode	child;
		DefaultTreeModel		treeModel = (DefaultTreeModel)tree.getModel();
		int						x;
		Vector					tmp = (Vector)getList().clone();
		
		//System.out.println(this.getClass().getName()+".setNodeChildren()===================================");
		if (tmp.size() > 0) {
			boolean removed = false;
			for(x = 0; x < container.getChildCount(); ) {
				child = (DefaultMutableTreeNode)container.getChildAt(x);
				if (!tmp.removeElement(child.getUserObject()) || (child.getUserObject() instanceof String)) {
					int		idx[] = {x};
					Object	nodes[] = {child};
					
					container.remove(x);
					treeModel.nodesWereRemoved(container,idx,nodes);
					removed = true;
				} else {
					x++;
				}
			}
			
			for(x = 0; x < tmp.size(); x++) {
				int idx[] = new int[1]; 
				container.add(n = new DefaultMutableTreeNode(tmp.get(x)));
				
				idx[0] = container.getChildCount()-1;
				treeModel.nodesWereInserted(container,idx);
				onAddNode(n);
				if (tmp.get(x) instanceof TreePathable)
					((TreePathable)tmp.get(x)).setPath(new TreePath(n.getPath()));
			}
			return removed || x > 0;
		}
		return false;
	}
	
	/**
	 * @see com.safmq.manager.ServerTreeViewSelectionListener#doSelection(JTree,DefaultMutableTreeNode)
	 */
	public void doSelection(JTree tree, DefaultMutableTreeNode container) {
		if (reload()) {
			setNodeChildren(tree, container);
			Manager.getInstance().setControlView(new IconListView(this));	
		}		
	}


	/**
	 * @see javax.swing.ListModel#getElementAt(int)
	 */
	public Object getElementAt(int index) {
		return getList().get(index);
	}

	/**
	 * @see javax.swing.ListModel#getSize()
	 */
	public int getSize() {
		return getList().size();
	}


	/**
	 * @see com.safmq.manager.IconListView.Activateable#activate()
	 */
	public void activate() {
		if (getPath() != null) {
			Manager.getInstance().getTreeView().activatePath(getPath());
		}		
	}


	TreePath treePath = null;
	/**
	 * @see com.safmq.manager.TreePathable#getPath()
	 */
	public TreePath getPath() {
		return treePath;
	}

	/**
	 * @see com.safmq.manager.TreePathable#setPath(javax.swing.tree.TreePath)
	 */
	public void setPath(TreePath path) {
		treePath = path;
	}
}

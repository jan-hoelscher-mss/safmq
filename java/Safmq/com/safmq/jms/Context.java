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


This software implements a Java interface to SAFMQ (see http://safmq.sourceforge.net).

*/
package com.safmq.jms;

import java.net.URI;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.TreeMap;
import java.util.Vector;

import javax.naming.Binding;
import javax.naming.CompositeName;
import javax.naming.InvalidNameException;
import javax.naming.Name;
import javax.naming.NameAlreadyBoundException;
import javax.naming.NameClassPair;
import javax.naming.NameNotFoundException;
import javax.naming.NameParser;
import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.NotContextException;
import javax.naming.OperationNotSupportedException;
import javax.naming.directory.InvalidAttributeValueException;
import javax.net.ssl.SSLContext;

import com.safmq.MQBuilder;
import com.safmq.MQConnection;
import com.safmq.Safmq;

public class Context implements javax.naming.Context, javax.naming.NameParser {
	public static final String SSL_CONTEXT = "com.safmq.jms.SSLContext";

	URI 			serverURI;
	String			userName;
	String			password;
	SSLContext		ctx = null;
	Hashtable		environment;
	MQConnection	con = null;
	
	TreeMap			extBindings = new TreeMap();
	
	public class SafmqNamingEnum implements javax.naming.NamingEnumeration {
		ArrayList	l;
		int			x = 0;
		
		SafmqNamingEnum(ArrayList list) {
			l = list;
		}
		
		public boolean hasMoreElements() {
			return hasMore();
		}
		public boolean hasMore() {
			return x < l.size();
		}
		public Object nextElement() {
			return next();
		}
		public Object next() {
			return l.get(x++);
		}
		public void close() {
		}
	}
	
	Context(Hashtable environment) throws NamingException {
		this.environment = environment;
		try {
			serverURI = new URI((String)environment.get(Context.PROVIDER_URL));
			userName = (String)environment.get(Context.SECURITY_PRINCIPAL);	
			password = (String)environment.get(Context.SECURITY_CREDENTIALS);
			ctx = (SSLContext)environment.get(SSL_CONTEXT);
			
			con = MQBuilder.buildConnection(serverURI, userName, password, ctx);	
		} catch (Exception e) {
			throw new InvalidAttributeValueException(e.toString());
		}
	}
	
	// For Subcontext to avoid allocating a connection tot he server.
	Context() {
	}
	
	/**
	 * A class to represent the results of an attempt to locate a
	 * subcontext or bound object.
	 */
	class subCtxResult {
		javax.naming.Context 	ctx = null;
		boolean					target = false;
		boolean					empty = false;
		Object					o = null;
		String					name = "";
		String					subName = "/";
	}
	
	/*
	 * Locates the most signifigant name
	 */
	int topName(Name name) {
		int idx = 0;
		while (idx < name.size() && name.get(idx).equals("")) {
			idx ++;
		}
		return idx;
	}
	
	/*
	 * Locates the least signifigant name
	 */
	int bottomName(Name name) {
		int idx = name.size() - 1;
		while (idx > 0 && name.get(idx).equals("")) {
			idx--;
		}
		return idx;
	}
		
	/*
	 * Locates the named item and sets some flags concerning its
	 * type, status as a target and whether it is a subcontext
	 * 
	 * @param name
	 * 
	 * @return The results
	 */
	subCtxResult locateSubContext(Name name) {
		// produce a list of queues, connection factories, and subcontexts
		int tnidx = topName(name);
		int bnidx = bottomName(name);
		
		subCtxResult res = new subCtxResult();
		res.target = tnidx == bnidx;
		res.empty = tnidx > bnidx;
		
		if (tnidx <= bnidx) {
			res.o = extBindings.get(res.name = name.get(tnidx));
			if (res.o != null && res.o instanceof javax.naming.Context)
				res.ctx = (javax.naming.Context) res.o;
			if (bnidx > tnidx)
				res.subName = name.getSuffix(tnidx + 1).toString();
		}
		return res;
	}
	
	
	
	public Name parse(String name) throws NamingException {
		return new CompositeName(name);
	}
	
	public Object addToEnvironment(String propName, Object propVal) throws NamingException {
		return environment.put(propName, propVal);
	}

	public void bind(Name name, Object obj) throws NamingException {
		subCtxResult sub = locateSubContext(name);
		if (sub.target) {
			if (sub.o != null)
				throw new NameAlreadyBoundException(name + " is already bound in this context");
			
			if (sub.empty)
				throw new InvalidNameException("Cannot bind to an empty name");
			
			extBindings.put(sub.name, obj);
		} else if (sub.ctx != null) {
			sub.ctx.bind(sub.subName, obj);
		} else {
			throw new NotContextException(name + " is not a subcontext");
		}
	}

	public void bind(String name, Object obj) throws NamingException {
		bind(parse(name), obj);
	}

	public void close() throws NamingException {
		try {
			NamingEnumeration ne = listBindings("/");
			Object				o;
			while (ne.hasMoreElements()) {
				o = ne.nextElement();
				if (o instanceof javax.naming.Context) {
					((javax.naming.Context)o).close();
				}
			}
		} finally {
			if (con != null) {
				con.Close();
				con = null;
			}
		}
	}

	public Name composeName(Name name, Name prefix) throws NamingException {
		return parse(composeName(name.toString(), prefix.toString()));
	}

	public String composeName(String name, String prefix) throws NamingException {
		return prefix + "/" + name;
	}

	public javax.naming.Context createSubcontext(Name name) throws NamingException {
		subCtxResult	sub = locateSubContext(name);
		if (sub.target) {
			if (sub.o != null)
				throw new NameAlreadyBoundException(name + " is already bound");
			SubContext ctx = new SubContext(environment, name.toString());
			extBindings.put(sub.name, ctx);
			return ctx;
		} else {
			if (sub.ctx != null) {
				return sub.ctx.createSubcontext(sub.subName);
			}
		}
		throw new NameNotFoundException(name + " was not found in the current context");
	}

	public javax.naming.Context createSubcontext(String name) throws NamingException {
		return createSubcontext(parse(name));
	}

	public void destroySubcontext(Name name) throws NamingException {
		subCtxResult	sub = locateSubContext(name);
		if (sub.target) {
			if (sub.ctx != null) {
				extBindings.remove(sub.ctx);
				sub.ctx.close();
			} else {
				throw new NotContextException(name + " is not a Subcontext");
			}
		} else if (sub.ctx != null) {
			sub.ctx.destroySubcontext(sub.subName);
		} else {
			throw new NameNotFoundException(name + "\" was not found in the current context");
		}
	}

	public void destroySubcontext(String name) throws NamingException {
		destroySubcontext(parse(name));
	}

	public Hashtable getEnvironment() throws NamingException {
		return environment;
	}

	public String getNameInNamespace() throws NamingException {
		return ""; // NOTE: empty string for full name
	}

	public NameParser getNameParser(Name name) throws NamingException {
		subCtxResult	sub = locateSubContext(name);
		if (sub.ctx != null)
			return sub.ctx.getNameParser(sub.subName);
		else if (sub.o != null && !sub.target)
			throw new NotContextException(name + " is not a subcontext");
		return this;	
	}

	public NameParser getNameParser(String name) throws NamingException {
		return getNameParser(parse(name));
	}

	public NamingEnumeration list(Name name) throws NamingException {
		// produce a list of queues, connection factories, and subcontexts
		subCtxResult	sub = locateSubContext(name);
		if (sub.ctx != null)
			return sub.ctx.list(sub.subName);
		else if (sub.o != null)
			throw new NotContextException(name + " is not a subcontext");
		else if (sub.empty) {
			ArrayList		list = new ArrayList();
			list.add(new NameClassPair("ConnectionFactory", javax.jms.ConnectionFactory.class.getName()));
			list.add(new NameClassPair("QueueConnectionFactory", javax.jms.QueueConnectionFactory.class.getName()));
			
			Vector	queues = new Vector();
			int ec = con.EnumerateQueues(queues);
			if (ec == Safmq.EC_NOERROR) {
				MQConnection.QueueData	qd;
				for(int x = 0; x < queues.size(); x++) {
					qd = (MQConnection.QueueData)queues.get(x);
					list.add(new NameClassPair("\"" + qd.getName() + "\"", javax.jms.Queue.class.getName()));
				}
			}
			
			Iterator i = extBindings.keySet().iterator();
			while (i.hasNext()) {
				list.add(new NameClassPair(i.next().toString(), javax.naming.Context.class.getName()));
			}
			return new SafmqNamingEnum(list);
		}
		throw new NameNotFoundException(name + " is not bound in this context");
	}

	public NamingEnumeration list(String name) throws NamingException {
		return list(parse(name));
	}
	
	public NamingEnumeration listBindings(Name name) throws NamingException {
		// produce a list of queues, connection factories, and subcontexts
		subCtxResult	sub = locateSubContext(name);
		if (sub.ctx != null)
			return sub.ctx.listBindings(sub.subName);
		else if (sub.o != null)
			throw new NotContextException(name + " is not a subcontext");
		else if (sub.empty) {
			ArrayList	list = new ArrayList();
			
			list.add(new Binding("ConnectionFactory", new com.safmq.jms.ConnectionFactory(serverURI, userName, password, ctx)));
			list.add(new Binding("QueueConnectionFactory", new com.safmq.jms.QueueConnectionFactory(serverURI, userName, password, ctx)));
			
			Vector	queues = new Vector();
			int ec = con.EnumerateQueues(queues);
			if (ec == Safmq.EC_NOERROR) {
				MQConnection.QueueData	qd;
				for(int x = 0; x < queues.size(); x++) {
					qd = (MQConnection.QueueData)queues.get(x);
					try {
						list.add(new Binding("\"" + qd.getName() + "\"", new com.safmq.jms.Queue(new URI(serverURI + "/" + qd.getName()))));
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			}
			
			Iterator i = extBindings.keySet().iterator();
			while (i.hasNext()) {
				String s = (String)i.next();
				list.add(new Binding(s, extBindings.get(s)));
			}
			
			return new SafmqNamingEnum(list);
		}
		throw new NameNotFoundException(name + " is not bound in this context");
	}
		

	public NamingEnumeration listBindings(String name) throws NamingException {
		return listBindings(parse(name));
	}

	public Object lookup(Name name) throws NamingException {
		// lookup the value found
		subCtxResult	sub = locateSubContext(name);

		if (sub.ctx != null)
			return sub.ctx.lookup(sub.subName);
		else if (sub.o != null && sub.target)
			return sub.o; // what we're lookin for.
		else if (sub.o != null && !sub.target) // be explicit
			throw new NotContextException(name + " is not a subcontext");
		else if (sub.empty) {
			return new Context(environment);
		} else { // sub.empty == false && o == null
			String itemName = sub.name;
			if (itemName.equals("ConnectionFactory"))
				return new com.safmq.jms.ConnectionFactory(serverURI, userName, password, ctx);
			
			if (itemName.equals("QueueConnectionFactory"))
				return new com.safmq.jms.QueueConnectionFactory(serverURI, userName, password, ctx);
		
			// Look For the Queue
			Vector	queues = new Vector();
			int ec = con.EnumerateQueues(queues);
			if (ec == Safmq.EC_NOERROR) {
				MQConnection.QueueData	qd;
				for(int x = 0; x < queues.size(); x++) {
					qd = (MQConnection.QueueData)queues.get(x);
					try {
						if (qd.getName().equals(itemName))
							return new com.safmq.jms.Queue(new URI(serverURI + "/" + qd.getName()));
					} catch (Exception e) {
						// Probably won't happen, but...
						e.printStackTrace();
					}
				}
			}
		}
		throw new NameNotFoundException(name + " is not bound in this context");
	}

	public Object lookup(String name) throws NamingException {
		return lookup(parse(name));
	}

	public Object lookupLink(Name name) throws NamingException {
		return lookup(name); // NOTE: Does not support links
	}

	public Object lookupLink(String name) throws NamingException {
		return lookupLink(parse(name));
	}

	public void rebind(Name name, Object obj) throws NamingException {
		throw new OperationNotSupportedException("SAFMQ Context does not support user bindings");
	}

	public void rebind(String name, Object obj) throws NamingException {
		rebind(parse(name), obj);
	}

	public Object removeFromEnvironment(String propName) throws NamingException {
		return environment.remove(propName);
	}

	public void rename(Name oldName, Name newName) throws NamingException {
		throw new OperationNotSupportedException("SAFMQ Context does not support user bindings");
	}

	public void rename(String oldName, String newName) throws NamingException {
		rename(parse(oldName), parse(newName));
	}

	public void unbind(Name name) throws NamingException {
		subCtxResult	sub = locateSubContext(name);
		if (sub.ctx != null) {
			if (sub.target) {
				sub.ctx.close();
				extBindings.remove(sub.ctx);
			} else {
				sub.ctx.unbind(sub.subName);
			}
		} else if (sub.o != null) {
			if (sub.target) {
				extBindings.remove(sub.o);
			} else {
				throw new NotContextException(name + " is not a subcontext");
			}
		} else {
			throw new NameNotFoundException(name + " is not bound in this context");
		}
	}

	public void unbind(String name) throws NamingException {
		unbind(parse(name));
	}
}

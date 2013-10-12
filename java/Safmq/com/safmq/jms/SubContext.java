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

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Iterator;

import javax.naming.Binding;
import javax.naming.Name;
import javax.naming.NameClassPair;
import javax.naming.NameNotFoundException;
import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.NotContextException;

public class SubContext extends com.safmq.jms.Context {
	String myName;
	
	SubContext(Hashtable env, String name) throws NamingException {
		this.environment = env;
		myName = name;
	}

	public NamingEnumeration list(Name name) throws NamingException {
		// produce a list of queues, connection factories, and subcontexts
		subCtxResult	sub = locateSubContext(name);
		if (sub.ctx != null)
			return sub.ctx.list(sub.subName);
		else if (sub.o != null)
			throw new NotContextException(name + " is not a subcontext");
		else if (sub.empty) {
			ArrayList	list = new ArrayList();
			Iterator	i = extBindings.keySet().iterator();
			
			while (i.hasNext()) {
				list.add(new NameClassPair(i.next().toString(), javax.naming.Context.class.getName()));
			}
		
			return new SafmqNamingEnum(list);
		}
		throw new NameNotFoundException(name + " was not found in this context");
	}

	public NamingEnumeration listBindings(Name name) throws NamingException {
		// produce a list of queues, connection factories, and subcontexts
		subCtxResult	sub = locateSubContext(name);
		
		if (sub.ctx != null) {
			return sub.ctx.listBindings(sub.subName);
		} else if (sub.o != null) {
			throw new NotContextException(name + " is not a subcontext");
		} else if (sub.empty) {
			ArrayList	list = new ArrayList();
			Iterator 	i = extBindings.keySet().iterator();
			while (i.hasNext()) {
				String s = (String)i.next();
				list.add(new Binding(s, extBindings.get(s)));
			}
			
			return new SafmqNamingEnum(list);
		}
		throw new NameNotFoundException(name + " was not found in this context");
	}

	public Object lookup(Name name) throws NamingException {
		// produce a list of queues, connection factories, and subcontexts
		subCtxResult	sub = locateSubContext(name);
		
		if (sub.empty)
			return this;
		
		if (sub.ctx != null && !sub.target)
			return sub.ctx.lookup(sub.subName);
		else if (sub.o != null && !sub.target)
			throw new NotContextException(name + " is not a subcontext");
		else if (sub.o == null)
			throw new NameNotFoundException(name + " is not bound in this context");
		
		return sub.o;
	}
}

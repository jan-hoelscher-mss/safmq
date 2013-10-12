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

import java.util.Enumeration;

import javax.jms.JMSException;

public class ConnectionMetaData implements javax.jms.ConnectionMetaData {

	public int getJMSMajorVersion() throws JMSException {
		return 1;
	}

	public int getJMSMinorVersion() throws JMSException {
		return 1;
	}

	public String getJMSProviderName() throws JMSException {
		return "SAFMQ";
	}

	public String getJMSVersion() throws JMSException {
		return "1.1";
	}

	public Enumeration getJMSXPropertyNames() throws JMSException {
		return new Enumeration() {
			public boolean hasMoreElements() { return false; }
			public Object nextElement() { return null; }
		};
	}

	public int getProviderMajorVersion() throws JMSException {
		return 0;
	}

	public int getProviderMinorVersion() throws JMSException {
		return 6;
	}

	public String getProviderVersion() throws JMSException {
		return "0.6.0";
	}

}

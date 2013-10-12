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
		
Created on May 27, 2005
*/
package com.safmq.manager;

import java.io.Serializable;

/**
 * @author matt
 */
public class ServerSpec implements Serializable {
	String name = null;
	String user = null;
	String password = null;
	boolean storePassword = false;

	public ServerSpec() {
	}
	public ServerSpec(String name, String user, String password, boolean storePassword) {
		this.name = name;
		this.user = user;
		this.storePassword = storePassword;
		if (storePassword)
			this.password = password;
		else
			this.password = null;
	}
	/**
	 * @return
	 */
	public String getName() {
		return name;
	}

	/**
	 * @return
	 */
	public String getPassword() {
		return password;
	}

	/**
	 * @return
	 */
	public boolean isStorePassword() {
		return storePassword;
	}

	/**
	 * @return
	 */
	public String getUser() {
		return user;
	}

	/**
	 * @param string
	 */
	public void setName(String string) {
		name = string;
	}

	/**
	 * @param string
	 */
	public void setPassword(String string) {
		password = string;
	}

	/**
	 * @param b
	 */
	public void setStorePassword(boolean b) {
		storePassword = b;
	}

	/**
	 * @param string
	 */
	public void setUser(String string) {
		user = string;
	}
}

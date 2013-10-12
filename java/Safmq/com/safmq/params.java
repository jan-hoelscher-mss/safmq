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

Internal classes used to define message formats. 
*/
package com.safmq;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.Vector;

class paramutil {
	public static void arrayCopy(byte dest[], String src) {
		int	x;
		for(x=0;x<dest.length;x++)
			dest[x] = 0;
		
		if (src != null) {
			byte tmp[] = src.getBytes();
			for(x=0;x<dest.length && x<tmp.length;x++)
				dest[x] = tmp[x];
		}
	}
}

class LOGIN_PARAMS {
	public LOGIN_PARAMS() {
		major_version = Safmq.SAFMQ_PROTOCOL_MAJOR_VERSION;
		minor_version = Safmq.SAFMQ_PROTOCOL_MINOR_VERSION;
	}
	
	public byte username[] = new byte[Safmq.USER_NAME_LENGTH];
	public byte password[] = new byte[Safmq.PASSWORD_LENGTH];
	public int major_version;
	public int minor_version;
	
	void write(DataOutput out) throws IOException {
		out.writeInt(username.length + password.length + Safmq.SIZE_INT + Safmq.SIZE_INT);
		
		out.write(username);
		out.write(password);
		out.writeInt(major_version);
		out.writeInt(minor_version);
	}
}
	
class USER_SET_PASSWORD_PARAMS{
	public byte username[] = new byte[Safmq.USER_NAME_LENGTH];
	public byte password[] = new byte[Safmq.PASSWORD_LENGTH];
	void write(DataOutput out) throws IOException {
		out.writeInt(username.length + password.length);
		
		out.write(username);
		out.write(password);
	}
}

class USER_CREATE_PARAMS {
	public byte username[] = new byte[Safmq.USER_NAME_LENGTH];
	public byte password[] = new byte[Safmq.PASSWORD_LENGTH];
	public byte description[] = new byte[Safmq.DESCRIPTION_LENGTH];
	
	void write(DataOutput out) throws IOException {
		out.writeInt(username.length + password.length + description.length);
		
		out.write(username);
		out.write(password);
		out.write(description);
	}
}

class USER_SET_PERMS_PARAMS {
	public byte	username[] = new byte[Safmq.USER_NAME_LENGTH];
	public byte	modify_queues;
	public byte	modify_users;
	public byte	modify_groups;
	
	void write(DataOutput out) throws IOException {
		out.writeInt(username.length + 3);
		
		out.write(username);
		out.write(modify_queues);
		out.write(modify_users);
		out.write(modify_groups);
	}
}

class USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS {
	public byte username[] = new byte[Safmq.USER_NAME_LENGTH];

	void write(DataOutput out) throws IOException {
		out.writeInt(username.length);
		
		out.write(username);
	}
}

class GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS {
	public byte groupname[] = new byte[Safmq.GROUP_NAME_LENGTH];
	void write(DataOutput out) throws IOException {
		out.writeInt(groupname.length);
		
		out.write(groupname);
	}
}

class GROUP_SET_PERMS_PARAMS {
	public byte	groupname[] = new byte[Safmq.GROUP_NAME_LENGTH];
	public byte	modify_queues;
	public byte	modify_users;
	public byte	modify_groups;

	void write(DataOutput out) throws IOException {
		out.writeInt(groupname.length + 3);
		
		out.write(groupname);
		out.write(modify_queues);
		out.write(modify_users);
		out.write(modify_groups);
	}
}

class GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS {
	public byte groupname[] = new byte[Safmq.GROUP_NAME_LENGTH];
	public byte username[] = new byte[Safmq.USER_NAME_LENGTH];
	void write(DataOutput out) throws IOException {
		out.writeInt(groupname.length + username.length);
		
		out.write(groupname);
		out.write(username);
	}
}

class QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS {
	public byte queuename[] = new byte[Safmq.QNAME_LENGTH];
	
	void write(DataOutput out) throws IOException {
		out.writeInt(queuename.length);
		
		out.write(queuename);
	}
}

class QUEUE_SET_USER_PERMS_PARAMS {
	public byte queuename[] = new byte[Safmq.QNAME_LENGTH];
	public byte username[] = new byte[Safmq.USER_NAME_LENGTH];
	public byte	read;
	public byte	write;
	public byte	destroy;
	public byte	change_security;

	void write(DataOutput out) throws IOException {
		out.writeInt(queuename.length + username.length + 4);
		
		out.write(queuename);
		out.write(username);
		out.write(read);
		out.write(write);
		out.write(destroy);
		out.write(change_security);
	}
}

class QUEUE_DEL_USER_PERMS_PARAM {
	public byte queuename[] = new byte[Safmq.QNAME_LENGTH];
	public byte username[] = new byte[Safmq.USER_NAME_LENGTH];

	void write(DataOutput out) throws IOException {
		out.writeInt(queuename.length + username.length);
		
		out.write(queuename);
		out.write(username);
	}
}

class QUEUE_SET_GROUP_PERMS_PARAMS {
	public byte queuename[] = new byte[Safmq.QNAME_LENGTH];
	public byte groupname[] = new byte[Safmq.GROUP_NAME_LENGTH];
	public byte	read;
	public byte	write;
	public byte	destroy;
	public byte	change_security;

	void write(DataOutput out) throws IOException {
		out.writeInt(queuename.length + groupname.length + 4);
		
		out.write(queuename);
		out.write(groupname);
		out.write(read);
		out.write(write);
		out.write(destroy);
		out.write(change_security);
	}
}

class QUEUE_DEL_GROUP_PERMS_PARAM {
	public byte queuename[] = new byte[Safmq.QNAME_LENGTH];
	public byte groupname[] = new byte[Safmq.GROUP_NAME_LENGTH];

	void write(DataOutput out) throws IOException {
		out.writeInt(queuename.length + groupname.length);
		
		out.write(queuename);
		out.write(groupname);
	}
}

class QUEUE_CLOSE_OPEN_CURSOR_PARAMS {
	/*QueueHandle*/int queueID;

	void write(DataOutput out) throws IOException {
		out.writeInt(Safmq.SIZE_INT);
		
		out.writeInt(queueID);
	}
}

class ENQUEUE_PARAMS {
	/*QueueHandle*/int 		queueID;
	public QueueMessage 	msg;

	void write(DataOutput out) throws IOException {
		out.writeInt(Safmq.SIZE_INT + msg.getSize());
		
		out.writeInt(queueID);
		msg.write(out);
	}
}


class RETRIEVE_PEEK_FRONT_PARAMS {
	/*QueueHandle*/int	queueID;
	public byte 		retrievebody;
	public int 			timeoutseconds;
	
	void write(DataOutput out) throws IOException {
		out.writeInt(Safmq.SIZE_INT + 1 + Safmq.SIZE_INT);
		
		out.writeInt(queueID);
		out.write(retrievebody);
		out.writeInt(timeoutseconds);
	}
}

class RETRIEVE_ACK_PARAMS {
	/*QueueHandle*/int queueID;
	public UUID msgID;
	void write(DataOutput out) throws IOException {
		out.writeInt(Safmq.SIZE_INT + msgID.getSize());

		out.writeInt(queueID);
		msgID.write(out);
	}
}

class RETRIEVE_ID_PEEK_ID_PARAMS {
	/*QueueHandle*/int queueID;
	public byte retrievebody;
	public UUID reciptID;
	public int timeoutseconds;
	
	void write(DataOutput out) throws IOException {
		out.writeInt(Safmq.SIZE_INT + 1 + reciptID.getSize() + Safmq.SIZE_INT);

		out.writeInt(queueID);
		out.write(retrievebody);
		reciptID.write(out);
		out.writeInt(timeoutseconds);
	}
}

class PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS {
	/*QueueHandle*/int queueID;
	public byte retrievebody;
	/*CursorHandle*/int	cursorID;
	
	void write(DataOutput out) throws IOException {
		out.writeInt(Safmq.SIZE_INT + 1 + Safmq.SIZE_INT);

		out.writeInt(queueID);
		out.write(retrievebody);
		out.writeInt(cursorID);
	}
}

class CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS {
	/*QueueHandle*/int queueID;
	/*CursorHandle*/int	cursorID;
	
	void write(DataOutput out) throws IOException {
		out.writeInt(Safmq.SIZE_INT + Safmq.SIZE_INT);

		out.writeInt(queueID);
		out.writeInt(cursorID);
	}
}

class SEEK_ID_PARAMS {
	/*QueueHandle*/int queueID;
	/*CursorHandle*/int	cursorID;
	public UUID reciptID;
	public int timeoutseconds;

	void write(DataOutput out) throws IOException {
		out.writeInt(Safmq.SIZE_INT + Safmq.SIZE_INT + reciptID.getSize() + Safmq.SIZE_INT);

		out.writeInt(queueID);
		out.writeInt(cursorID);
		reciptID.write(out);
		out.writeInt(timeoutseconds);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Server Response Structures
///////////////////////////////////////////////////////////////////////////////
class LOGIN_RESPONSE {
	int major_version;
	int minor_version;
	
	void read(DataInput in)  throws IOException {
		major_version = in.readInt();
		minor_version = in.readInt();
	}
}

class RESPONSE_RESPONSE {
	public RESPONSE_RESPONSE() {
		errorcode = Safmq.EC_NOERROR;
	}
	public RESPONSE_RESPONSE(int err) {
		errorcode = err;
	}

	/*int*/int	errorcode;
	
	void read(DataInput in) throws IOException {
		errorcode = in.readInt();
	}
}

class QUEUE_CONFIG_ENTRY {
	public byte	queuename[] = new byte[Safmq.QNAME_LENGTH];
	public byte	owner[] = new byte[Safmq.USER_NAME_LENGTH];

	void read(DataInput in) throws IOException {
		in.readFully(queuename);
		in.readFully(owner);
	}
}

class ENUM_QUEUES_RESPONSE {
	public ENUM_QUEUES_RESPONSE(){
		errorcode = Safmq.EC_NOERROR;
	}
	public ENUM_QUEUES_RESPONSE(int err) {
		errorcode = err;
	}

	/*int*/int	errorcode;
	public int nQueues;
	// Followed by up to nQueues: QUEUE_CONFIG_ENTRY queue_data;
}

class ENUM_USERS_RESPONSE {
	public ENUM_USERS_RESPONSE(int nUsers) {
		errorcode = Safmq.EC_NOERROR;
		this.nUsers = nUsers;
	}

	/*int*/int	errorcode;
	public int nUsers;
	// Followed by up to nUsers: byte username[] = new byte[Safmq.USER_NAME_LENGTH];
}

class ENUM_GROUPS_RESPONSE {
	public ENUM_GROUPS_RESPONSE(int nGroups) {
		errorcode = Safmq.EC_NOERROR;
		this.nGroups = nGroups; 
	}

	/*int*/int	errorcode;
	public int nGroups;
	// Followed by up to nGroups: byte username[] = new byte[Safmq.USER_NAME_LENGTH];
}


class GROUP_GET_USERS_RESPONSE {
	public GROUP_GET_USERS_RESPONSE(int nUsers) {
		errorcode = Safmq.EC_NOERROR;
		this.nUsers = nUsers; 
	} 

	/*int*/int	errorcode;
	public int nUsers;
	//byte	username[] = new byte[Safmq.1][] = new byte[Safmq.USER_NAME_LENGTH]; // repeats upto nUsers times
}

class USER_GET_GROUPS_RESPONSE {
	public USER_GET_GROUPS_RESPONSE(int nGroups) {
		errorcode = Safmq.EC_NOERROR;
		this.nGroups = nGroups; 
	} 

	/*int*/int	errorcode;
	public int nGroups;
	//byte	groupname[] = new byte[Safmq.1][] = new byte[Safmq.GROUP_NAME_LENGTH]; // repeats upto nUsers times
}


class OPEN_QUEUE_RESPOSNE {
	public OPEN_QUEUE_RESPOSNE() {
		errorcode = Safmq.EC_NOERROR;
	}

	/*int*/int errorcode;
	/*ServiceThread::QueueHandle*/ int	queueID;
}

class ENQUEUE_RESPONSE_DATA {
	public ENQUEUE_RESPONSE_DATA() {}
	public ENQUEUE_RESPONSE_DATA(UUID id, int ts) {
		msgID = id;
		timestamp = ts;
	}
	public UUID msgID;
	public int timestamp;
	
	void read(DataInput in) throws IOException {
		msgID.read(in);
		timestamp = in.readInt();
	}
}

class ENQUEUE_RESPONSE {
	public ENQUEUE_RESPONSE() {
		errorcode = Safmq.EC_NOERROR;
	}
	public ENQUEUE_RESPONSE(UUID id, int ts) {
		errorcode = Safmq.EC_NOERROR;
		data = new ENQUEUE_RESPONSE_DATA(id,ts);
	}

	/*int*/int	errorcode;
	public ENQUEUE_RESPONSE_DATA data;
}

class RETRIEVE_RESPONSE {
	public RETRIEVE_RESPONSE() {
		errorcode = Safmq.EC_NOERROR;
	}
	
	/*int*/int	errorcode;
	
	public QueueMessage  msg;
}

class OPEN_CURSOR_RESPONSE {
	public OPEN_CURSOR_RESPONSE() {
		errorcode = Safmq.EC_NOERROR;
	}
	public OPEN_CURSOR_RESPONSE(/*QStorage::CursorHandle*/int cursor) {
		errorcode = Safmq.EC_NOERROR;
		cursorID = cursor;
	}

	/*int*/int errorcode;
	/*QStorage::CursorHandle*/int cursorID;
}

class USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data {
	public byte	modify_queues;
	public byte	modify_users;
	public byte	modify_groups;
	
	void read(DataInput in) throws IOException {
		modify_queues = in.readByte();
		modify_users = in.readByte();
		modify_groups = in.readByte();
	}
}

class USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE {
	public USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE() {
		errorcode = Safmq.EC_NOERROR;
	}

	public int errorcode;
	public USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data	data;
}


class QUEUE_PERM_DATA {
	public static final int MAX_ENTITY_LENGTH = 20;
	
	public byte	entityname[] = new byte [QUEUE_PERM_DATA.MAX_ENTITY_LENGTH];
	public byte	isgroup;
	public byte	read;
	public byte	write;
	public byte	destroy;
	public byte	change_security;
	
	void read(DataInput in) throws IOException {
		in.readFully(entityname);
		isgroup = in.readByte();
		read = in.readByte();
		write = in.readByte();
		destroy = in.readByte();
		change_security = in.readByte();
	}
}

class QUEUE_ENUM_PERMS_RESPOSNE {
	public QUEUE_ENUM_PERMS_RESPOSNE(int nPerms) {
		errorcode = Safmq.EC_NOERROR;
		this.nPerms = nPerms;
	}

	public int	errorcode;
	public int nPerms;
//	QUEUE_PERM_DATA	perms[] = new byte[Safmq.0];
}

class ADD_USER_IDENTITY_PARAMS {
	public String	subjectDN;
	public String	issuerDN;
	public String	username;

	public ADD_USER_IDENTITY_PARAMS(String subjectDN, String issuerDN, String username) {
		this.subjectDN = subjectDN;
		this.issuerDN = issuerDN;
		this.username = username;
	}
	
	void write(DataOutput out) throws IOException {
		// note (12) size of 3, 4 byte integers
		int size = (12) + subjectDN.length() + issuerDN.length() + username.length();
		out.writeInt(size);
		out.writeInt(subjectDN.length());
		out.writeInt(issuerDN.length());
		out.writeInt(username.length());
		
		out.write(subjectDN.getBytes());
		out.write(issuerDN.getBytes());
		out.write(username.getBytes());
	}
}

class REMOVE_USER_IDENTITY {
	public String	subjectDN;
	public String	issuerDN;

	public REMOVE_USER_IDENTITY(String subjectDN, String issuerDN) {
		this.subjectDN = subjectDN;
		this.issuerDN = issuerDN;
	}
	
	void write(DataOutput out) throws IOException {
		// note (8) size of 2, 4 byte integers
		int size = (8) + subjectDN.length() + issuerDN.length();
		out.writeInt(size);
		out.writeInt(subjectDN.length());
		out.writeInt(issuerDN.length());
		
		out.write(subjectDN.getBytes());
		out.write(issuerDN.getBytes());
	}
}

class ENUM_USER_IDENTITY {
	public String	username;
	
	public ENUM_USER_IDENTITY(String username) {
		this.username = username;
	}
	
	void write(DataOutput out) throws IOException {
		// note (4) size of 1, 4 byte integers
		int size = (4) +  username.length();
		out.writeInt(size);
		out.writeInt(username.length());
		
		out.write(username.getBytes());
	}
}

class QUEUE_STATS_PARAMS {
	int	queueID;
	byte includeStorageBytes;
	byte includeMessageBytes;

	public QUEUE_STATS_PARAMS(QueueHandle qh, boolean includeStorageBytes, boolean includeMessageBytes) {
		queueID = qh.handle;
		this.includeStorageBytes = (byte)(includeStorageBytes ? 1 : 0); 
		this.includeMessageBytes = (byte)(includeMessageBytes ? 1 : 0); 
	}
	
	void write(DataOutput out) throws IOException {
		int size = 4 + 1 + 1;
		out.writeInt(size);
		out.writeInt(queueID);
		out.writeByte(includeStorageBytes);
		out.writeByte(includeMessageBytes);
	}
}

class QUEUE_STATS_RESPONSE {
	int/*ErrorCode*/	errorcode = Safmq.EC_ERROR;
	QueueStatistics		stats = new QueueStatistics();
	
	void read(DataInput input) throws IOException {
		errorcode = input.readInt();
		if (errorcode == Safmq.EC_NOERROR) {
			stats.messageCount = ((long)input.readInt()) & 0x00FFFFFFFF;
			stats.storageBytes = ((long)input.readInt()) & 0x00FFFFFFFF;
			stats.messageBytes = ((long)input.readInt()) & 0x00FFFFFFFF;

			stats.enqueued10second = input.readInt();
			stats.enqueued60second = input.readInt();
			stats.enqueued300second = input.readInt();

			stats.retrieved10second = input.readInt();
			stats.retrieved60second = input.readInt();
			stats.retrieved300second = input.readInt();

			stats.peeked10second = input.readInt();
			stats.peeked60second = input.readInt();
			stats.peeked300second = input.readInt();
		}
	}
}

class QUEUE_CREATE_TEMP_RESPONSE {
	public int/*ErrorCode*/	errorcode;
	public int/*QueueHandle*/	queueID;
	public String				queueName;
	
	void read(DataInput i) throws IOException {
		errorcode = i.readInt();
		if (errorcode == Safmq.EC_NOERROR) {
			queueID = i.readInt();
			int length = i.readInt();
			byte[] buf = new byte[length];
			i.readFully(buf);
			queueName = new String(buf);
		}
	}
}

class CLIENT_INFO_RESPONSE
{
	public int/*ErrorCode*/	errorcode;
	public Vector clientInfo;

	void read (DataInput input) throws IOException {
		errorcode = Safmq.EC_NETWORKERROR;
		errorcode = input.readInt();
		if (errorcode == Safmq.EC_NOERROR) {
			int ciCount = input.readInt();
			ClientInfo ci;
			byte[] buf = new byte[4096];

			for(int x = 0; x < ciCount; ++x) {
				int nameLen = input.readInt();
				ci = new ClientInfo();

				input.readFully(buf, 0, nameLen);
				ci.username = new String(buf, 0, nameLen);

				ci.address = input.readInt();
				ci.port = input.readShort();
				clientInfo.add(ci);
			}
			
		}			
	}
}


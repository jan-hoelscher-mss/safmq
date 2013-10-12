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


Created on Aug 9, 2007

*/
using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace safmq.net
{
    internal struct ioutil {
        internal static void paddedWrite(BinaryWriter w, byte[] bytes, int padding, byte padByte) {
            w.Write(bytes, 0, Math.Min(bytes.Length, padding));
            for (int x = 0; x < (padding - bytes.Length); x++)
                w.Write(padByte);
        }

        /**
         * Internal method to calculate the length of a zero terminated string (zstring).
         * @param src The zero terminated string as a byte array.
         * @return The number of bytes input the string.
         */
        internal static int length(byte[] src) {
            int len;
            for (len = 0; len < src.Length && src[len] != 0; len++) { }
            return len;
        }
    }

    internal struct LOGIN_PARAMS
    {
        internal string username;
        internal string password;
        public int major_version;
        public int minor_version;

        internal LOGIN_PARAMS(string username, string password)
        {
		    major_version = Safmq.SAFMQ_PROTOCOL_MAJOR_VERSION;
		    minor_version = Safmq.SAFMQ_PROTOCOL_MINOR_VERSION;
            this.username = username;
            this.password = password;
	    }
    	
	    internal void Write(BinaryWriter output) {
            output.Write((int)(Safmq.USER_NAME_LENGTH + Safmq.PASSWORD_LENGTH + Safmq.SIZE_INT + Safmq.SIZE_INT));

            byte[] un = Encoding.UTF8.GetBytes(username.ToCharArray());
            byte[] pass = Encoding.UTF8.GetBytes(password.ToCharArray());

            ioutil.paddedWrite(output, un, Safmq.USER_NAME_LENGTH, 0);
            ioutil.paddedWrite(output, pass, Safmq.PASSWORD_LENGTH, 0);
		    output.Write(major_version);
		    output.Write(minor_version);
	    }
    }

    internal struct LOGIN_RESPONSE {
	    internal int major_version;
	    internal int minor_version;
    	
	    internal void Read(BinaryReader input) {
            major_version = input.ReadInt32();
            minor_version = input.ReadInt32();
	    }
    }

    internal struct QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS {
        internal string queuename;

        internal void Write(BinaryWriter output) {
            output.Write((int)Safmq.QNAME_LENGTH);
            byte[] qn = Encoding.UTF8.GetBytes(queuename.ToCharArray());
            ioutil.paddedWrite(output, qn, Safmq.QNAME_LENGTH, 0);
        }
    }

    internal struct QUEUE_CLOSE_OPEN_CURSOR_PARAMS {
	    internal QueueHandle queueID;

	    internal void Write(BinaryWriter output) {
		    output.Write((int)Safmq.SIZE_INT);
		    output.Write((int)queueID.handle);
	    }
    }

    internal struct ENQUEUE_PARAMS {
	    internal QueueHandle 	queueID;
	    internal QueueMessage 	msg;

	    internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.SIZE_INT + msg.getSize()));
		    output.Write(queueID.handle);
		    msg.Write(output);
	    }
    }

    internal struct ENQUEUE_RESPONSE_DATA {
	    internal ENQUEUE_RESPONSE_DATA(UUID id, int ts) {
		    msgID = id;
		    timestamp = ts;
	    }
	    internal UUID msgID;
	    internal int timestamp;
    	
	    internal void Read(BinaryReader input) {
		    msgID.Read(input);
		    timestamp = input.ReadInt32();
	    }
    }

    internal struct RETRIEVE_PEEK_FRONT_PARAMS {
	    internal QueueHandle	queueID;
	    internal byte 		    retrievebody;
	    internal int 			timeoutseconds;
    	
	    internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.SIZE_INT + 1 + Safmq.SIZE_INT));
    		
		    output.Write(queueID.handle);
		    output.Write(retrievebody);
		    output.Write(timeoutseconds);
	    }
    }

    internal struct RETRIEVE_ACK_PARAMS {
        internal QueueHandle queueID;
        internal UUID msgID;

        internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.SIZE_INT + msgID.getSize()));
		    output.Write(queueID.handle);
		    msgID.Write(output);
	    }
    }


    internal struct RETRIEVE_ID_PEEK_ID_PARAMS {
	    internal QueueHandle queueID;
	    internal byte retrievebody;
	    internal UUID reciptID;
	    internal int timeoutseconds;

        internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.SIZE_INT + 1 + reciptID.getSize() + Safmq.SIZE_INT));

		    output.Write(queueID.handle);
		    output.Write(retrievebody);
		    reciptID.Write(output);
		    output.Write(timeoutseconds);
	    }
    }

    internal struct PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS {
	    internal QueueHandle queueID;
	    internal byte retrievebody;
	    internal CursorHandle cursorID;

        internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.SIZE_INT + 1 + Safmq.SIZE_INT));

		    output.Write(queueID.handle);
		    output.Write(retrievebody);
		    output.Write(cursorID.handle);
	    }
    }

    internal struct CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS {
	    internal QueueHandle queueID;
	    internal CursorHandle	cursorID;

        internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.SIZE_INT + Safmq.SIZE_INT));

		    output.Write(queueID.handle);
		    output.Write(cursorID.handle);
	    }
    }

    internal struct SEEK_ID_PARAMS {
	    internal QueueHandle    queueID;
	    internal CursorHandle	cursorID;
	    internal UUID reciptID;
	    internal int timeoutseconds;

	    internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.SIZE_INT + Safmq.SIZE_INT + reciptID.getSize() + Safmq.SIZE_INT));

		    output.Write(queueID.handle);
		    output.Write(cursorID.handle);
		    reciptID.Write(output);
		    output.Write(timeoutseconds);
	    }
    }

    internal struct USER_CREATE_PARAMS {
        internal string username;
        internal string password;
        internal string description;

    	
	    internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.USER_NAME_LENGTH + Safmq.PASSWORD_LENGTH + Safmq.DESCRIPTION_LENGTH));

            byte[] un = Encoding.UTF8.GetBytes(username.ToCharArray());


            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(username.ToCharArray()), Safmq.USER_NAME_LENGTH, 0);
            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(password.ToCharArray()), Safmq.PASSWORD_LENGTH, 0);
            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(description.ToCharArray()), Safmq.DESCRIPTION_LENGTH, 0);
	    }
    }

    internal struct USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS {
        internal string username;

	    internal void Write(BinaryWriter output) {
		    output.Write(Safmq.USER_NAME_LENGTH);
            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(username.ToCharArray()), Safmq.USER_NAME_LENGTH, 0);
        }
    }

    internal struct USER_SET_PERMS_PARAMS {
        internal string username;
	    internal byte	modify_queues;
	    internal byte	modify_users;
	    internal byte	modify_groups;
    	
	    internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.USER_NAME_LENGTH + 3));

            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(username.ToCharArray()), Safmq.USER_NAME_LENGTH, 0);
            output.Write(modify_queues);
		    output.Write(modify_users);
            output.Write(modify_groups);
	    }
    }

    internal struct USER_SET_PASSWORD_PARAMS {
        internal string username;
        internal string password;

	    internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.USER_NAME_LENGTH + Safmq.PASSWORD_LENGTH));
            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(username.ToCharArray()), Safmq.USER_NAME_LENGTH, 0);
            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(password.ToCharArray()), Safmq.PASSWORD_LENGTH, 0);
	    }
    }

    internal struct GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS {
        internal string groupname;

	    internal void Write(BinaryWriter output) {
		    output.Write((int)Safmq.GROUP_NAME_LENGTH);
		    ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(groupname.ToCharArray()), Safmq.GROUP_NAME_LENGTH, 0);
	    }
    }

    internal struct GROUP_SET_PERMS_PARAMS {
        internal string groupname;
	    internal byte	modify_queues;
	    internal byte	modify_users;
	    internal byte	modify_groups;

	    internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.GROUP_NAME_LENGTH + 3));
    		
		    ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(groupname.ToCharArray()), Safmq.GROUP_NAME_LENGTH, 0);
		    output.Write(modify_queues);
		    output.Write(modify_users);
		    output.Write(modify_groups);
	    }
    }

    internal struct GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS {
        internal string groupname;
        internal string username;

	    internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.GROUP_NAME_LENGTH+Safmq.USER_NAME_LENGTH));
    		
		    ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(groupname.ToCharArray()), Safmq.GROUP_NAME_LENGTH, 0);
		    ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(username.ToCharArray()), Safmq.USER_NAME_LENGTH, 0);
	    }
    }

    internal struct QUEUE_SET_USER_PERMS_PARAMS {
        internal string queuename;
        internal string username;
	    internal byte	read;
	    internal byte	write;
	    internal byte	destroy;
	    internal byte	change_security;

	    internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.QNAME_LENGTH + Safmq.USER_NAME_LENGTH + 4));
    		
		    ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(queuename.ToCharArray()), Safmq.QNAME_LENGTH, 0);
		    ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(username.ToCharArray()), Safmq.USER_NAME_LENGTH, 0);
		    output.Write(read);
		    output.Write(write);
		    output.Write(destroy);
		    output.Write(change_security);
	    }
    }

    internal struct QUEUE_DEL_USER_PERMS_PARAM {
        internal string queuename;
        internal string username;

	    internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.QNAME_LENGTH+Safmq.USER_NAME_LENGTH));

            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(queuename.ToCharArray()), Safmq.QNAME_LENGTH, 0);
            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(username.ToCharArray()), Safmq.USER_NAME_LENGTH, 0);
        }
    }

    internal struct QUEUE_SET_GROUP_PERMS_PARAMS {
        internal string queuename;
        internal string groupname;
        internal byte   read;
	    internal byte	write;
	    internal byte	destroy;
	    internal byte	change_security;

	    internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.QNAME_LENGTH + Safmq.GROUP_NAME_LENGTH + 4));
    		
            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(queuename.ToCharArray()), Safmq.QNAME_LENGTH, 0);
            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(groupname.ToCharArray()), Safmq.GROUP_NAME_LENGTH, 0);
		    output.Write(read);
		    output.Write(write);
		    output.Write(destroy);
		    output.Write(change_security);
	    }
    }

    internal struct QUEUE_DEL_GROUP_PERMS_PARAM {
	    internal string queuename;
        internal string groupname;

	    internal void Write(BinaryWriter output) {
		    output.Write((int)(Safmq.QNAME_LENGTH + Safmq.GROUP_NAME_LENGTH));
    		
            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(queuename.ToCharArray()), Safmq.QNAME_LENGTH, 0);
            ioutil.paddedWrite(output, Encoding.UTF8.GetBytes(groupname.ToCharArray()), Safmq.GROUP_NAME_LENGTH, 0);
	    }
    }

    internal struct USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE_data {
	    internal  byte	modify_queues;
	    internal  byte	modify_users;
	    internal  byte	modify_groups;
    	
	    internal void Read(BinaryReader input) {
		    modify_queues = input.ReadByte();
		    modify_users = input.ReadByte();
		    modify_groups = input.ReadByte();
	    }
    }


    internal struct QUEUE_CONFIG_ENTRY {
	    internal byte[]	queuename;
        internal byte[] owner;

	    internal void Read(BinaryReader input) {
		    queuename = input.ReadBytes(Safmq.QNAME_LENGTH);
            owner = input.ReadBytes(Safmq.USER_NAME_LENGTH);
	    }
    }

    internal class QUEUE_PERM_DATA {
	    internal const int MAX_ENTITY_LENGTH = 20;

        internal QUEUE_PERM_DATA() {
            en = new byte[MAX_ENTITY_LENGTH];
        }

        internal byte[] en;
        internal string entityname;
	    internal byte	isgroup;
	    internal byte	read;
	    internal byte	write;
	    internal byte	destroy;
	    internal byte	change_security;
    	
	    internal void Read(BinaryReader input) {
		    input.Read(en, 0, en.Length);
            entityname = new string(Encoding.UTF8.GetChars(en, 0, ioutil.length(en)));
		    isgroup = input.ReadByte();
		    read = input.ReadByte();
		    write = input.ReadByte();
		    destroy = input.ReadByte();
            change_security = input.ReadByte();
	    }
    }

	class ADD_USER_IDENTITY_PARAMS {
		public string	subjectDN;
		public string	issuerDN;
		public string	username;

		public ADD_USER_IDENTITY_PARAMS(string subjectDN, string issuerDN, string username) {
			this.subjectDN = subjectDN;
			this.issuerDN = issuerDN;
			this.username = username;
		}
		
		internal void Write(BinaryWriter output) {
			// note (12) size of 3, 4 byte inputtegers
			int size = (12) + subjectDN.Length + issuerDN.Length + username.Length;
			output.Write(size);
			output.Write(subjectDN.Length);
			output.Write(issuerDN.Length);
			output.Write(username.Length);
			
			output.Write(Encoding.UTF8.GetBytes(subjectDN.ToCharArray()));
			output.Write(Encoding.UTF8.GetBytes(issuerDN.ToCharArray()));
			output.Write(Encoding.UTF8.GetBytes(username.ToCharArray()));
		}
	}

	class REMOVE_USER_IDENTITY {
		public string	subjectDN;
		public string	issuerDN;

		public REMOVE_USER_IDENTITY(string subjectDN, string issuerDN) {
			this.subjectDN = subjectDN;
			this.issuerDN = issuerDN;
		}

		internal void Write(BinaryWriter output) {
			// note (8) size of 2, 4 byte inputtegers
			int size = (8) + subjectDN.Length + issuerDN.Length;
			output.Write(size);
			output.Write(subjectDN.Length);
			output.Write(issuerDN.Length);
			
			output.Write(Encoding.UTF8.GetBytes(subjectDN.ToCharArray()));
			output.Write(Encoding.UTF8.GetBytes(issuerDN.ToCharArray()));
		}
	}

	class ENUM_USER_IDENTITY {
		public string	username;
		
		public ENUM_USER_IDENTITY(string username) {
			this.username = username;
		}

		internal void Write(BinaryWriter output) {
			// note (4) size of 1, 4 byte inputtegers
			int size = (4) +  username.Length;
			output.Write(size);
			output.Write(username.Length);
			output.Write(Encoding.UTF8.GetBytes(username.ToCharArray()));
		}
	}

	class QUEUE_STATS_PARAMS {
		int	queueID;
		byte includeStorageBytes;
		byte includeMessageBytes;

		public QUEUE_STATS_PARAMS(QueueHandle qh, bool includeStorageBytes, bool includeMessageBytes) {
			queueID = qh.handle;
			this.includeStorageBytes = (byte)(includeStorageBytes ? 1 : 0); 
			this.includeMessageBytes = (byte)(includeMessageBytes ? 1 : 0); 
		}
		
		internal void Write(BinaryWriter output) {
			int size = 4 + 1 + 1;
			output.Write(size);
			output.Write(queueID);
			output.Write(includeStorageBytes);
			output.Write(includeMessageBytes);
		}
	}

	class QUEUE_STATS_RESPONSE {
		internal ErrorCode			errorcode = ErrorCode.EC_ERROR;
		internal QueueStatistics	stats = new QueueStatistics();
		
		internal void Read(BinaryReader input) {
			errorcode = (ErrorCode)input.ReadInt32();
			if (errorcode == ErrorCode.EC_NOERROR) {
				stats.messageCount = ((long)input.ReadInt32()) & 0x00FFFFFFFF;
				stats.storageBytes = ((long)input.ReadInt32()) & 0x00FFFFFFFF;
				stats.messageBytes = ((long)input.ReadInt32()) & 0x00FFFFFFFF;

				stats.enqueued10second = input.ReadInt32();
				stats.enqueued60second = input.ReadInt32();
				stats.enqueued300second = input.ReadInt32();

				stats.retrieved10second = input.ReadInt32();
				stats.retrieved60second = input.ReadInt32();
				stats.retrieved300second = input.ReadInt32();

				stats.peeked10second = input.ReadInt32();
				stats.peeked60second = input.ReadInt32();
				stats.peeked300second = input.ReadInt32();
			}
		}
	}

	class QUEUE_CREATE_TEMP_RESPONSE {
		public ErrorCode	errorcode;
		public QueueHandle	queueID = new QueueHandle();
		public String		queueName;
		
		internal void Read(BinaryReader input) {
			errorcode = ErrorCode.EC_NETWORKERROR;
			errorcode = (ErrorCode)input.ReadInt32();
			if (errorcode == ErrorCode.EC_NOERROR) {
				queueID.handle = input.ReadInt32();
				int length = input.ReadInt32();
				byte[] buf = new byte[length];
				input.Read(buf, 0, buf.Length);
				queueName = Encoding.ASCII.GetString(buf);
			}
		}
	}

	class CLIENT_INFO_RESPONSE
	{
		public ErrorCode/*ErrorCode*/	errorcode;
		public List<ClientInfo> clientInfo;

		internal void Read (BinaryReader input) {
			errorcode = ErrorCode.EC_NETWORKERROR;
			errorcode = (ErrorCode)input.ReadInt32();
			if (errorcode == ErrorCode.EC_NOERROR) {
				int ciCount = input.ReadInt32();
				ClientInfo ci;
				byte[] buf = new byte[4096];

				for(int x = 0; x < ciCount; ++x) {
					int nameLen = input.ReadInt32();
					ci = new ClientInfo();

					int read = input.Read(buf, 0, nameLen);
					if (read != nameLen)
						throw new IOException("Unable to read all input");
					ci.username = Encoding.ASCII.GetString(buf, 0, nameLen);

					ci.address = input.ReadInt32();
					ci.port = input.ReadInt16();
					clientInfo.Add(ci);
				}
				
			}			
		}
	}
}

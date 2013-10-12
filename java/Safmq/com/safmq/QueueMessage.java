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

Created on Mar 21, 2005
*/
package com.safmq;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Represents a message to be sent or retrieved from a SAFMQ message queue
 * server.
 * 
 * <p>In the simplest use of the <code>QueueMessage</code> object, clients
 * would only need to instantiate an instance then write to the body represented
 * by an <code>OutputStream</code> then the message could be sent to the server.
 * An example follows:</p>
 * 
 * <pre>
 * 	MessageQueue	queue = MQBuilder.buildMessageQueue("//localhost/foo","user","password");
 * 	QueueMessage	msg = new QueueMessage();
 * 	PrintWriter	w = new PrintWriter(new OutputStreamWriter(msg.getOutputStream()));
 * 
 * 	w.write("Hello World!");
 * 	w.flush();

 * 	queue.Enqueue(msg);
 * 	queue.Close();
 * </pre>
 * 
 * <p>Facilitating two way communications, the <code>QueueMessage</code> class
 * provides a facility via a "Recipt ID" in which responding programs can place
 * the original message's id into the recipt id of the response so that the
 * querier can wait for a response.  Additionaly, SAFMQ provides the ability to
 * prescribe a "Time-To-Live" for a message query so that in the case that
 * the responding program is not able to retrieve the message and respond in time,
 * the caller can be notified by the SAFMQ server.  An example follows:</p>
 * 
 * <p>The Client:</p>
 * <pre>
 * 	MessageQueue	queue = MQBuilder.buildMessageQueue("//localhost/foo","user","password");
 * 	QueueMessage	msg = new QueueMessage();
 * 	PrintWriter	w = new PrintWriter(new OutputStreamWriter(msg.getOutputStream()));
 * 
 * 	w.write("Hello World!");
 * 	w.flush();
 * 
 * 	msg.setLable("Query");
 * 	msg.setTimeToLiveSeconds(5); // allow 5 seconds before an auto response
 * 	msg.setTTLErrorWanted(true);
 * 	msg.setResponseQueueName("//localhost/foo");
 * 	if (queue.Enqueue(msg) == Safmq.EC_NOERROR) {
 * 		UUID id = msg.getMessageID(); // generated via the call to Enqueue()
 * 		msg = new QueueMessage();
 * 		if (queue.RetrieveID(true,id,-1,msg) == Safmq.EC_NOERROR) {
 * 			if (msg.getMessageClass() == Safmq.MC_SYSTEMERRMSG) {
 * 				System.out.println("The message errored out");
 * 			}
 * 			InputStream in = msg.getInputStream();
 * 			byte		data[] = new byte[1024];
 * 			int			read;
 * 			while ( (read=in.read(data)) > 0) {
 * 				System.out.write(data,0,read);
 * 			}
 * 		}
 *	}
 * 	queue.close();
 * </pre> 
 * 
 * <p>The Server:</p>
 * <pre>
 * 	MessageQueue	queue = MQBuilder.buildMessageQueue("//localhost/foo","user","password");
 * 	QueueMessage	msg = new QueueMessage();
 * 	
 * 	while ( queue.Retrieve(true,-1,msg) == Safmq.EC_NOERROR ) {
 * 		QueueMessage response = new QueueMessage();
 * 		PrintWriter	w = new PrintWriter(new OutputStreamWriter(response.getOutputStream()));
 * 		
 * 		w.write("Back at ya!");
 * 		w.flush();
 * 		
 * 		response.setReciptID(msg.getMesasgeID());
 * 		response.setLabel("Response");
 * 
 * 		MessageQueue	responseQueue = MQBuilder.buildMessageQueue(msg.getResponseQueueName(),"user","password");
 * 
 * 		responseQueue.Enqueue(response);
 * 		
 * 		responseQueue.Close();
 * 		msg = new QueueMessage();
 * 	}
 * 	queue.Close();
 * </pre>
 * 
 * 
 * @author Matt
 */
public class QueueMessage {
	static byte[] EMPTY = {};
	//private static final UUID NUL_UUID = new UUID();
	
	UUID	messageID			= new UUID();
	byte	messageClass		= Safmq.MC_USERMSG;
	byte	messagePriority		= Safmq.MP_STANDARD;
	byte	label[]				= new byte[Safmq.MSGLBL_LENGTH];
	int		timeStamp			= 9;
	int		timeToLiveSeconds	= -1;
	byte	ttlErrorWanted		= 0;
	byte	responseQueueName[]	= new byte[Safmq.QNAME_LENGTH];
	UUID	reciptID			= new UUID();
	byte	bodyType			= Safmq.BT_NONE;
	int		bodySize			= -1;
	
	bodyStream body 			= null;
	byte bodyBytes[]			= EMPTY;
	
	class bodyStream extends ByteArrayOutputStream {
		public void close() throws IOException {
			super.close();
			bodyBytes = toByteArray();
			body = null;
		}
	}
	
	/**
	 * Default Constructor.
	 */
	public QueueMessage() {
	}
	
	/**
	 * Determines the size of the QueueMessage when written to a
	 * stream.
	 * 
	 * @return The size in bytes of the object when written to a stream.
	 */
	int getSize() {
		return 	messageID.getSize() + 
			1 + 				//byte	messageClass
			1 + 				//byte	messagePriority
			label.length + 
			Safmq.SIZE_INT + 	//int		timeStamp			= 9;
			Safmq.SIZE_INT + 	// int		timeToLiveSeconds	= -1;
			1 + 				//byte	ttlErrorWanted		= 0;
			responseQueueName.length +
			reciptID.getSize() + 
			1 + 				//byte	bodyType			= Safmq.BT_NONE;
			Safmq.SIZE_INT + 	//int		bodySize			= -1;
			getBufferSize();
	}
	
	byte[] getBufferBytes() {
		return body != null ? body.toByteArray() : bodyBytes;
	}
	
	/**
	 * Package protected member to write the message to a DataOutput stream.
	 * 
	 * @param	out	The stream to which the data is to be written
	 * @throws	IOException	In the case that the stream experiences an error
	 * 						while writing data.
	 */
	void write(DataOutput out) throws IOException {
		messageID.write(out);
		out.writeByte(messageClass);
		out.writeByte(messagePriority);
		out.write(label);
		out.writeInt(timeStamp);
		out.writeInt(timeToLiveSeconds);
		out.writeByte(ttlErrorWanted);
		out.write(responseQueueName);
		reciptID.write(out);
		out.writeByte(bodyType);
		bodySize = getBufferSize();
		out.writeInt(bodySize);
		out.write(getBufferBytes());
	}

	/**
	 * Package protected member to read the mesage from a DataOutput stream.
	 * 
	 * @param	in				The stream from which the data is to be read
	 * @param	retrievebody	A flag indicating the body of the message should
	 * 							be retrieved.
	 * 
	 * 
	 * @throws	IOException	In the case that the stream experiences an error
	 * 						while reading data.
	 */
	void read(DataInput in, boolean retrievebody) throws IOException {
		messageID.read(in);
		messageClass = in.readByte();
		messagePriority = in.readByte();
		in.readFully(label);
		timeStamp = in.readInt();
		timeToLiveSeconds = in.readInt();
		ttlErrorWanted = in.readByte();
		in.readFully(responseQueueName);
		reciptID.read(in);
		bodyType = in.readByte();
		bodySize = in.readInt();
		body = null;
		bodyBytes = new byte[bodySize];
		if (retrievebody)
			in.readFully(bodyBytes);
	}

	/**
	 * Provides access to the message id, this value is generated by a successful
	 * call to <code>MQConnection.Enqueue(QueueHandle,QueueMessage)</code>.
	 * 
	 * @return Returns the messageID
	 */
	public UUID getMessageID() {
		return messageID;
	}
		
	/**
	 * Provides access to the size of the message body.
	 * 
	 * @return Returns the bodySize.
	 */
	public int getBodySize() {
		return (bodySize == -1) ? getBufferSize() : bodySize;
	}
	/**
	 * Provides the size of the buffer allocated.
	 * @return Returns the size of the allocated buffer.
	 */
	public int getBufferSize() {
		return body != null ? body.size() : bodyBytes.length;
	}
	
	/**
	 * Resets the contents of the body.
	 */
	public void resetBody() {
		body = null;
		bodyBytes = EMPTY;
	}
	
	/**
	 * Provides an output stream to write data into the <code>QueueMessage</code>
	 * message body.
	 * 
	 * @return An output stream for writing the message's body.
	 */
	public OutputStream getOutputStream() {
		if (body == null)
			body = new bodyStream();
		return body;
	}
	
	/**
	 * Provides an input stream to read the data in the message's body.
	 * 
	 * @return An input sream to read the message's body.
	 */
	public InputStream getInputStream() {
		return new ByteArrayInputStream(getBufferBytes());
	}
	
	/**
	 * The body type as set by the member <code>setBodyType</code>.
	 * 
	 * @return Returns the bodyType.
	 */
	public byte getBodyType() {
		return bodyType;
	}
	/**
	 * Sets the message's body type.
	 * 
	 * @param bodyType The bodyType to set.
	 * @see Safmq#BT_LONG
	 * @see Safmq#BT_SHORT
	 * @see Safmq#BT_CHAR
	 * @see Safmq#BT_TEXT
	 * @see Safmq#BT_WTEXT
	 * @see Safmq#BT_BINARY
	 * @see Safmq#BT_NONE
	 * 
	 */
	public void setBodyType(byte bodyType) {
		switch (bodyType) {
			case Safmq.BT_LONG:
			case Safmq.BT_SHORT:
			case Safmq.BT_CHAR:
			case Safmq.BT_TEXT:
			case Safmq.BT_WTEXT:
			case Safmq.BT_BINARY:
			case Safmq.BT_NONE:
				break;
			default:
				throw new IllegalArgumentException("A value of ("+bodyType+") is not a valid body type value");
		}
		this.bodyType = bodyType;
	}
	/**
	 * Provides the label of the message.
	 * @return Returns the label.
	 */
	public String getLabel() {
		return new String(label,0,length(label));
	}
	/**
	 * Sets the label of the message.
	 * @param label The label to set.
	 */
	public void setLabel(String label) {
		byte	tmp[] = label.getBytes();
		int		x;
		for(x=0;x<this.label.length;x++)
			this.label[x] = 0;
		for(x=0;x<this.label.length && x<tmp.length;x++)
			this.label[x] = tmp[x];
	}
	/**
	 * Provides the message's priority
	 * @return Returns the messagePriority.
	 */
	public byte getMessagePriority() {
		return messagePriority;
	}
	/**
	 * Sets the priority of the message.
	 * @param messagePriority The messagePriority to set.
	 * @see Safmq#MP_STANDARD
	 * @see Safmq#MP_LOW
	 * @see Safmq#MP_MEDIUMLOW
	 * @see Safmq#MP_MEDIUM
	 * @see Safmq#MP_MEDIUMHIGH
	 * @see Safmq#MP_HIGH
	 * @see Safmq#MP_HIGHEST
	 */
	public void setMessagePriority(byte messagePriority) {
		switch (messagePriority) {
			case Safmq.MP_LOW:
			case Safmq.MP_MEDIUMLOW:
			case Safmq.MP_MEDIUM:
			case Safmq.MP_MEDIUMHIGH:
			case Safmq.MP_HIGH:
			case Safmq.MP_HIGHEST:
				break;
			default:
				throw new IllegalArgumentException("A value of ("+messagePriority+") is not a valid message priority value");
		}
		this.messagePriority = messagePriority;
	}
	/**
	 * Provides the recipt id of this message
	 * @return Returns the recipt id.
	 */
	public UUID getReciptID() {
		return reciptID;
	}
	/**
	 * Sets the recipt id of this message
	 * @param reciptID The recipt id of this message
	 */
	public void setReciptID(UUID reciptID) {
		this.reciptID = reciptID;
	}
	/**
	 * Provides the response queue name for this message
	 * @return Returns the name of the response queue.
	 */
	public String getResponseQueueName() {
		return new String(responseQueueName,0,length(responseQueueName));
	}
	/**
	 * Sets the response queue name of this message
	 * @param responseQueueName The name of the response queue
	 */
	public void setResponseQueueName(String responseQueueName) {
		byte tmp[] = responseQueueName.getBytes();
		int	x;
		for(x=0;x<this.responseQueueName.length;x++)
			this.responseQueueName[x] = 0;
		for(x=0;x<this.responseQueueName.length && x < tmp.length;x++)
			this.responseQueueName[x] = tmp[x];
	}
	/**
	 * Provides the timestamp for this message.
	 * @return Returns the time stamp.
	 */
	public long getTimeStamp() {
		return (long)timeStamp * 1000;
	}
	/**
	 * Sets the timestamp for this message.
	 * @param timeStamp The time stamp for this message
	 */
	public void setTimeStamp(long timeStamp) {
		this.timeStamp = (int)(timeStamp/1000);
	}
	/**
	 * Provides the time to live in seconds for this message.
	 * @return Returns the timeToLiveSeconds.
	 */
	public int getTimeToLiveSeconds() {
		return timeToLiveSeconds;
	}
	/**
	 * Sets the time to live in seconds for this message
	 * @param timeToLiveSeconds The number of seconds before this message should be purged
	 */
	public void setTimeToLiveSeconds(int timeToLiveSeconds) {
		this.timeToLiveSeconds = timeToLiveSeconds;
	}
	/**
	 * Provids the flag indicating this message desires auto generated TTL messages.
	 * @return Returns the the flag indicating auto generated TTL messages are wanted.
	 */
	public boolean getTTLErrorWanted() {
		return ttlErrorWanted != 0;
	}
	/**
	 * Sets the flag indicating the auto generated TTL error message is wanted.
	 * @param errorWanted The flag indicating the TTL auto error message is wanted
	 */
	public void setTTLErrorWanted(boolean errorWanted) {
		ttlErrorWanted = (byte)(errorWanted ? 1 : 0);
	}
	/**
	 * Provides the message class
	 * @return Returns the message class.
	 * @see Safmq#MC_SYSTEMERRMSG
	 * @see Safmq#MC_USERMSG
	 */
	public byte getMessageClass() {
		return messageClass;
	}
	
	/**
	 * Internal method to calculate the length of a zero terminated string (zstring).
	 * @param src The zero terminated string as a byte array.
	 * @return The number of bytes in the string.
	 */
	int length(byte src[]) {
		int len;
		for(len=0; len < src.length && src[len] != 0; len++) { }
		return len;
	}
}

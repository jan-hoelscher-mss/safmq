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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;

import javax.jms.JMSException;
import javax.jms.MessageEOFException;
import javax.jms.MessageNotWriteableException;

import com.safmq.QueueMessage;

public class BytesMessage extends com.safmq.jms.Message implements javax.jms.BytesMessage {
	DataInputStream in = null;
	DataOutputStream out = null;
	ByteArrayOutputStream buffer = null;

	BytesMessage(QueueMessage msg) throws JMSException {
		super(msg);
		try {
			allocInputStream();
			allocOutputStream();
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	void allocInputStream() throws Exception {
		if (msg.getBodySize() > 0)
			in = new DataInputStream(msg.getInputStream());
		in = new DataInputStream(new ByteArrayInputStream(new byte[0]));
	}

	void allocOutputStream() throws Exception {
		out = new DataOutputStream(buffer = new ByteArrayOutputStream());
	}

	QueueMessage getMessage() throws Exception {
		out.flush();
		byte[] data = buffer.toByteArray();
		msg.resetBody();
		msg.getOutputStream().write(data);
		return msg;
	}

	public long getBodyLength() throws JMSException {
		return msg.getBodySize();
	}

	public boolean readBoolean() throws JMSException {
		try {
			return in.readBoolean();
		} catch (EOFException e) {
			throw new MessageEOFException(e.toString());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public byte readByte() throws JMSException {
		try {
			return in.readByte();
		} catch (EOFException e) {
			throw new MessageEOFException(e.toString());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public int readBytes(byte[] value) throws JMSException {
		try {
			return in.read(value);
		} catch (EOFException e) {
			throw new MessageEOFException(e.toString());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public int readBytes(byte[] value, int length) throws JMSException {
		try {
			return in.read(value, 0, length);
		} catch (EOFException e) {
			throw new MessageEOFException(e.toString());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}


	public char readChar() throws JMSException {
		try {
			return in.readChar();
		} catch (EOFException e) {
			throw new MessageEOFException(e.toString());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public double readDouble() throws JMSException {
		try {
			return in.readDouble();
		} catch (EOFException e) {
			throw new MessageEOFException(e.toString());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public float readFloat() throws JMSException {
		try {
			return in.readFloat();
		} catch (EOFException e) {
			throw new MessageEOFException(e.toString());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public int readInt() throws JMSException {
		try {
			return in.readInt();
		} catch (EOFException e) {
			throw new MessageEOFException(e.toString());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public long readLong() throws JMSException {
		try {
			return in.readLong();
		} catch (EOFException e) {
			throw new MessageEOFException(e.toString());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public Object readObject() throws JMSException {
		throw new JMSException("Not Implemented");
		//		try {
		//			return in.readObject();
		//		} catch (EOFException e) {
		//			throw new MessageEOFException(e.toString());
		//		} catch (Exception e) {
		//			JMSException je = new JMSException(e.toString());
		//			je.setLinkedException(e);
		//			throw je;
		//		}
	}

	public short readShort() throws JMSException {
		try {
			return in.readShort();
		} catch (EOFException e) {
			throw new MessageEOFException(e.toString());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public String readUTF() throws JMSException {
		throw new JMSException("Not Implemented");
		//		try {
		//			return (String)in.readObject();
		//		} catch (EOFException e) {
		//			throw new MessageEOFException(e.toString());
		//		} catch (Exception e) {
		//			JMSException je = new JMSException(e.toString());
		//			je.setLinkedException(e);
		//			throw je;
		//		}
	}

	public int readUnsignedByte() throws JMSException {
		try {
			int b = in.readByte();
			return (b & 0x00FF);
		} catch (EOFException e) {
			throw new MessageEOFException(e.toString());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public int readUnsignedShort() throws JMSException {
		try {
			int s = in.readShort();
			return (s & 0x00FFFF);
		} catch (EOFException e) {
			throw new MessageEOFException(e.toString());
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	// TODO: Revisit this method
	public void reset() throws JMSException {
		try {
			allocInputStream();
			allocOutputStream();
		} catch (Exception e) {
			JMSException je = new JMSException(e.toString());
			je.setLinkedException(e);
			throw je;
		}
	}

	public void writeBoolean(boolean value) throws JMSException {
		try {
			out.writeBoolean(value);
		} catch (Exception e) {
			throw new MessageNotWriteableException(e.toString());
		}
	}

	public void writeByte(byte value) throws JMSException {
		try {
			out.writeByte(value);
		} catch (Exception e) {
			throw new MessageNotWriteableException(e.toString());
		}
	}

	public void writeBytes(byte[] value) throws JMSException {
		try {
			out.write(value);
		} catch (Exception e) {
			throw new MessageNotWriteableException(e.toString());
		}
	}

	public void writeBytes(byte[] value, int offset, int length) throws JMSException {
		try {
			out.write(value, offset, length);
		} catch (Exception e) {
			throw new MessageNotWriteableException(e.toString());
		}
	}

	public void writeChar(char value) throws JMSException {
		try {
			out.writeChar(value);
		} catch (Exception e) {
			throw new MessageNotWriteableException(e.toString());
		}
	}

	public void writeDouble(double value) throws JMSException {
		try {
			out.writeDouble(value);
		} catch (Exception e) {
			throw new MessageNotWriteableException(e.toString());
		}
	}

	public void writeFloat(float value) throws JMSException {
		try {
			out.writeFloat(value);
		} catch (Exception e) {
			throw new MessageNotWriteableException(e.toString());
		}
	}

	public void writeInt(int value) throws JMSException {
		try {
			out.writeInt(value);
		} catch (Exception e) {
			throw new MessageNotWriteableException(e.toString());
		}
	}

	public void writeLong(long value) throws JMSException {
		try {
			out.writeLong(value);
		} catch (Exception e) {
			throw new MessageNotWriteableException(e.toString());
		}
	}

	public void writeObject(Object value) throws JMSException {
		throw new JMSException("Not Implemented");
		//		try {
		//			out.writeObject(value);
		//		} catch (Exception e) {
		//			throw new MessageNotWriteableException(e.toString());
		//		}
	}

	public void writeShort(short value) throws JMSException {
		try {
			out.writeShort(value);
		} catch (Exception e) {
			throw new MessageNotWriteableException(e.toString());
		}
	}

	public void writeUTF(String value) throws JMSException {
		throw new JMSException("Not Implemented");
		//		try {
		//			out.writeObject(value);
		//		} catch (Exception e) {
		//			throw new MessageNotWriteableException(e.toString());
		//		}
	}

	public void clearBody() throws JMSException {
		// TODO Clear the body of the message
	}
}

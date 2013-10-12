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

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

/**
 An object to represent a universally unique identifier (UUID).
*/
public class UUID {
	public int		d1 = 0;
	public short	d2 = 0;
	public short	d3 = 0;
	public byte		d4[] = new byte[8];
	
	/**
	Default Constructor
	*/
	public UUID() {
		for(int x=0;x<d4.length;x++)
			d4[x] = 0;
	}
	
	/**
	Constructs the UUID from the passed values
	*/
	public UUID(int d1, short d2, short d3, byte d4[]) {
		this.d1 = d1;
		this.d2 = d2;
		this.d3 = d3;
		for(int x=0;x<d4.length;x++)
			this.d4[x] = d4[x];
	}
	
	/**
	 * Constructs from the String representation of a UUID.
	 * @param src
	 * @throws NumberFormatException
	 * @throws IllegalArgumentException
	 */
	public UUID(String src) throws NumberFormatException, IllegalArgumentException {
		fromString(src);
	}
	
//	/**
//	 * Constructs from a Java 5 java.util.UUID.
//	 * 
//	 * @param uuid A Java 5 UUID value.
//	 */
//	public UUID(java.util.UUID uuid) {
//		long	high = uuid.getMostSignificantBits();
//		long	low = uuid.getLeastSignificantBits();
//		
//		d1 = (int)((high >> 32) & 0x00FFFFFFFF);
//		d2 = (short)((high >> 16) & 0x00FFFF);
//		d3 = (short)(high & 0x00FFFF);
//		for (int x = 0; x < d4.length; x++) {
//			d4[d4.length - x - 1] = (byte)((low >> (x*8)) & 0x00FF); 
//		}
//	}
	
	/**
	 * Writes the UUID to a DataOutput object.
	 * 
	 * @param out The output object
	 * 
	 * @throws IOException Thrown by a write call to the DataOutput object.
	 */
	public void write(DataOutput out) throws IOException {
		out.writeInt(d1);
		out.writeShort(d2);
		out.writeShort(d3);
		out.write(d4);
	}
	
	/**
	 * Reads the UUID from the DataInput.
	 * 
	 * @param in The input object
	 * @throws IOException Thrown by a read call to the DataInput object.
	 */
	public void read(DataInput in) throws IOException {
		d1 = in.readInt();
		d2 = in.readShort();
		d3 = in.readShort();
		in.readFully(d4);
	}
	
	/**
	Makes a clone copy of this object
	@return a duplicated copy of this UUID object
	*/
	public Object clone() {
		return new UUID(d1,d2,d3,d4);
	}
	
	/**
	Compares this instance of UUID to another object.
	@return true If this instance is equal to the passed instance,
					wherein the passed instance is also a UUID.
	*/
	public boolean equals(Object o) {
		if (o instanceof UUID) {
			UUID u = (UUID)o;
			return this.d1 == u.d1 &&
					this.d2 == u.d2 &&
					this.d3 == u.d3 &&
					this.d4[0] == u.d4[0] &&
					this.d4[1] == u.d4[1] &&
					this.d4[2] == u.d4[2] &&
					this.d4[3] == u.d4[3] &&
					this.d4[4] == u.d4[4] &&
					this.d4[5] == u.d4[5] &&
					this.d4[6] == u.d4[6] &&
					this.d4[7] == u.d4[7];
		}
		return false;
	}
	
	void fromString(String src) throws IllegalArgumentException, NumberFormatException{
		//                            1         2         3 
		//                  0123456789012345678901234567890123456
		// Standard format "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
		if (src.length() >= 36) {
			d1 = Integer.parseInt(src.substring(0,8), 16);
			d2 = Short.parseShort(src.substring(9,13), 16);
			d3 = Short.parseShort(src.substring(14,18), 16);
			d4[0] = Byte.parseByte(src.substring(19,21), 16);
			d4[1] = Byte.parseByte(src.substring(21,23), 16);
			for (int x = 2; x < d4.length; x++) {
				d4[x] = Byte.parseByte(src.substring(22 + x, 24 + x), 16);
			}
		} else {
			throw new IllegalArgumentException("length of the UUID string must be at least 36 characters long");
		}
	}
	
	/**
	Prepares a string representation of this instance of UUID.
	@return A string representation of the UUID.
	*/
	public String toString() {
		String zeros="00000000";
		
		String d1 = Integer.toHexString(this.d1);
		String d2 = Integer.toHexString(this.d2 & 0x0FFFF);
		String d3 = Integer.toHexString(this.d3 & 0x0FFFF);
		String d4_0 = Integer.toHexString(d4[0] & 0x0FF);
		String d4_1 = Integer.toHexString(d4[1] & 0x0FF);
		String res;
		
		res = zeros.substring(0,8-d1.length()) + d1 + "-" 
				+ zeros.substring(0,4-d2.length()) + d2 + "-"
				+ zeros.substring(0,4-d3.length()) + d3 + "-"
				+ zeros.substring(0,2-d4_0.length()) + d4_0
				+ zeros.substring(0,2-d4_1.length()) + d4_1 + "-";
		
		for(int x=2;x<d4.length;x++) {
			String t;
			t = Integer.toHexString(d4[x] & 0x0FF);
			res += zeros.substring(0,2-t.length()) + t;
 		}
		return res;
	}
	
	/**
	 * Calculates the size for binary serialization.
	 * 
	 * @return 16 in all cases.
	 */
	int getSize() {
		return Safmq.SIZE_INT + Safmq.SIZE_SHORT + Safmq.SIZE_SHORT + d4.length;	
	}
	

}

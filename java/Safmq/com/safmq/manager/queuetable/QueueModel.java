package com.safmq.manager.queuetable;

import java.util.Date;
import java.util.Vector;

import javax.swing.table.AbstractTableModel;

import com.safmq.CursorHandle;
import com.safmq.MQConnection;
import com.safmq.QueueHandle;
import com.safmq.QueueMessage;
import com.safmq.Safmq;
import com.safmq.UUID;

public class QueueModel extends AbstractTableModel {
	Vector	messages = new Vector();
	Vector	bodies = new Vector();
	
	static final UUID	nil = new UUID();
	
	static final int MAX_MSGS = 50;
	
	static final int
		MESSAGE_ID_COL = 0,
		RECIPT_ID_COL = 1,
		RESPONSE_QUEUE_COL = 2,
		TIMESTAMP_COL = 3,
		LABEL_COL = 4,
		BODY_COL = 5;
	
	static final String columns[] = {
		"Message ID",
		"Receipt ID",
		"Response Queue",
		"Timestamp",
		"Label",
		"Body",
	};
	
	public QueueModel(MQConnection con, String queueName) {
		QueueHandle	h = new QueueHandle();
		CursorHandle cur = new CursorHandle();
		
		if (con.OpenQueue(queueName,h) == Safmq.EC_NOERROR) {
			QueueMessage msg;
			
			if (con.OpenCursor(h, cur) == Safmq.EC_NOERROR) {
				for(int x = 0; x < MAX_MSGS; x++) {
					msg = new QueueMessage();
					if (con.PeekCursor(h, true, cur, msg) == Safmq.EC_NOERROR) {
						messages.add(msg);
						
						
						byte body[] = new byte[50];
						int read = 0;
						try { read = msg.getInputStream().read(body); } catch (Exception e) {}
						bodies.add(new String(body, 0, read) + ((read == 50) ? "...":""));
						con.AdvanceCursor(h, cur);
					} else 
						break;
				}
			}
			con.CloseQueue(h);
		}
	}
	
	public int getColumnCount() {
		return columns.length;
	}

	public int getRowCount() {
		return messages.size();
	}

	public String getColumnName(int column) {
		return columns[column];
	}

	public Object getValueAt(int row, int col) {
		QueueMessage msg = (QueueMessage)messages.get(row);
		switch (col) {
			case MESSAGE_ID_COL:
				return msg.getMessageID().toString();
			case RECIPT_ID_COL: 
				if (msg.getReciptID() != null) {
					UUID recipt = msg.getReciptID();
					if (!recipt.equals(nil))
						return msg.getReciptID();
				}
				return "";
			case RESPONSE_QUEUE_COL:
				return msg.getResponseQueueName();
			case TIMESTAMP_COL:
				return new Date(msg.getTimeStamp());
			case LABEL_COL:
				return msg.getLabel();
			case BODY_COL:
				return bodies.get(row);
		}
		return "";
	}

}

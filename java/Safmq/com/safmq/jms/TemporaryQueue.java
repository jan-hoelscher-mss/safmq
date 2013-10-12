package com.safmq.jms;

import java.net.URI;

import com.safmq.MessageQueue;


public class TemporaryQueue extends Queue implements javax.jms.TemporaryQueue {
	MessageQueue que;
	
	TemporaryQueue(URI uri, MessageQueue que) {
		super(uri);
		this.que = que;
	}
	
	public void delete() {
		que.Close();
	}
}

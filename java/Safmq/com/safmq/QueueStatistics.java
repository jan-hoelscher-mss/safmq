package com.safmq;

/**
Set of queue performance statistics.  When queried, all fields except
<code>storageBytes</code> and <code>messageBytes</code> are returned.
The byte count fields (<code>storageBytes</code> and <code>messageBytes</code>)
require sequential access to the queue and/or file system and thus
cause a performance penalty to acquire.
*/
public class QueueStatistics {
	/** Number of messages in a queue */
	public long messageCount = 0;
	/** Number of bytes on disk, includes purged messages still on disk */
	public long storageBytes = 0;
	/** Number of bytes in the queue, does not included purged messages */
	public long messageBytes = 0;

	/** Number of messages enqueued in the last 10 seconds */
	public int enqueued10second = 0;
	/** Number of messages enqueued in the last 60 seconds */
	public int enqueued60second = 0;
	/** Number of messages enqueued in the last 300 seconds */
	public int enqueued300second = 0;

	/** Number of messages retrieved in the last 10 seconds */
	public int retrieved10second = 0;
	/** Number of messages retrieved in the last 60 seconds */
	public int retrieved60second = 0;
	/** Number of messages retrieved in the last 300 seconds */
	public int retrieved300second = 0;

	/** Number of messages "peeked" in the last 10 seconds */
	public int peeked10second = 0;
	/** Number of messages "peeked" in the last 60 seconds */
	public int peeked60second = 0;
	/** Number of messages "peeked" in the last 300 seconds */
	public int peeked300second = 0;
}

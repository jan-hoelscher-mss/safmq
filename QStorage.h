/*
 Copyright 2004-2008 Matthew J. Battey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software distributed
	under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied. See the License for the
	specific language governing permissions and limitations under the License.




This software implements a platform independent Store and Forward Message Queue.
*/
#if !defined(_QSTORAGE_H_)
#define _QSTORAGE_H_

#pragma warning(disable: 4786)

#include "safmq.h"
#include "Mutex.h"
#include "safmq-Signal.h"
#include "randpq.h"
#include "Stats.h"

#include <fstream>
#include <memory.h>
#include <set>
#include <map>

using namespace safmq;

/**
Operator to compare two uuid's.  If the two uuid's are identical
then operator function returns true.

@param lhs The first uuid to compare
@param rhs The second uuid to compare
@return true If the two uuid's are equal
*/
inline bool operator==(const uuid& lhs, const uuid& rhs) {
	return memcmp(&lhs, &rhs, sizeof(uuid)) == 0;
}

/**
Operator to compare two uuid's.  If the first uuid is less than
the second then the operator function return strue

@param lhs The first uuid to compare
@param rhs The second uuid to compare
@return true If the first uuid is less than the second
*/
inline bool operator<(const uuid& lhs, const uuid& rhs) {
	return memcmp(&lhs, &rhs, sizeof(uuid)) < 0;
}


/**
A class containing in-memory data to index the on-disk data in a message queue.
*/
struct QueueIndex {
	/// The priority of the message
	safmq::MessagePriorityEnum		MessagePriority;
	/// The identification of the message
	uuid					MessageID;
	/// The class of the message
	MessageClassEnum		MessageClass;
	/// The return-id of the message
	uuid					ReciptID;
	/// The time the message was created
	time_t					Timestamp;
	/// The number of seconds this message has to live
	SAFMQ_INT32					TTL;
	/// The id of the file the message resides in
	SAFMQ_INT32					FileID;
	/// The offset into the file where the message resides
	std::fstream::off_type	DataOffset;
	/// A flag indicating the message has been or is about to be purged from disk
	bool					Purged;



	QueueIndex() { 
		DataOffset = 0;
		FileID = 0;
		MessageClass = MC_USERMSG;
		MessagePriority = MP_STANDARD;
		MessageID = UUID_NULL;
		ReciptID = UUID_NULL;
		Purged = true;
	}
	QueueIndex(const QueueIndex& src) {
		copy(src);
	}
	QueueIndex(MessagePriorityEnum MessagePriority, uuid MessageID, MessageClassEnum MessageClass, uuid ReciptID, time_t Timestamp, SAFMQ_INT32 TTL, SAFMQ_INT32 FileID, std::fstream::off_type DataOffset, bool Purged) {
		this->MessagePriority	= MessagePriority;
		this->MessageID			= MessageID;
		this->MessageClass		= MessageClass;
		this->ReciptID			= ReciptID;
		this->Timestamp			= Timestamp;
		this->TTL				= TTL;
		this->FileID			= FileID;
		this->DataOffset		= DataOffset;
		this->Purged			= Purged;
	}

	QueueIndex& operator=(const QueueIndex& src) {
		copy(src);
		return *this;
	}

	bool operator<(const QueueIndex& rhs) const {
		return MessagePriority > rhs.MessagePriority;
	}

	void copy(const QueueIndex& src) {
		MessagePriority	= src.MessagePriority;
		MessageID		= src.MessageID;
		MessageClass	= src.MessageClass;
		ReciptID		= src.ReciptID;
		Timestamp		= src.Timestamp;
		TTL				= src.TTL;
		FileID			= src.FileID;
		DataOffset		= src.DataOffset;
		Purged			= src.Purged;
	}
};

/**
Basic definition for data storage into 

@field MsgMarker			Always equal to 'QMsg' inorder to accurately locate the beginning of a message.
@field Purged				Flag indicating whether the message has been purged from the queue and should be ignored.
@field Length				Length of this record
@field Msg					The message data
*/
#pragma pack(1)
struct QMsgStorageHdr {
	QMsgStorageHdr() : MsgMarker(0), Purged(false), Length(0xFFFFFFFF) { }

	SAFMQ_INT32		MsgMarker;
	bool			Purged;
	SAFMQ_UINT32	Length;
};
#pragma pack()



namespace safmq {
/**
Class to control queue storage...
*/
class QStorage 
{
public: //behavior
	typedef ra_priority_queue<QueueIndex>				IndexQueue;
	typedef	std::map<SAFMQ_FILE_ID, size_t>				FileUtilizationMap;
	typedef SAFMQ_INT32									CursorHandle;
	typedef std::map<CursorHandle, IndexQueue::cursor>	CursorHandleMap;
	typedef std::set<uuid>								IDSet;

	QStorage(const std::string& name, const std::string& storagedirectory, SAFMQ_INT32 maxfilesize=40*1024*1024);
	virtual ~QStorage();


	const std::string& getName() { return name; }

	ErrorCode GetStatistics(QueueStatistics& stats, bool includeStorageBytes, bool includeMessageBytes);

	/**
	Place a message on the queue.
	*/
	ErrorCode Enqueue(QueueMessage& msg, QStorage::CursorHandle& enqCursorID);
	ErrorCode FinalizeEnqueue(QStorage::CursorHandle finalizeID);
	ErrorCode CancelEnqueue(QStorage::CursorHandle cancelID);

	ErrorCode BeginRetrieve(bool retrievebody, QueueMessage& msg, QStorage::CursorHandle& retrieveID);
	ErrorCode BeginRetrieveID(bool retrievebody, const uuid& id, QueueMessage& msg, QStorage::CursorHandle& retrieveID);
	// TODO: ErrorCode BeginRetrievePriority(bool retrievebody, MessagePriorityenum priority, QueueMessage& msg, QStorage::CursorHandle& retrieveID);
	ErrorCode CancelRetrieve(QStorage::CursorHandle retrieveID);
	ErrorCode FinalizeRetrieve(QStorage::CursorHandle retrieveID);
	ErrorCode FinalizeRetrieveCursor(QStorage::CursorHandle retrieveID);

	ErrorCode PeekID(bool retrievebody, const uuid& id, QueueMessage& msg);
	ErrorCode PeekFront(bool retrievebody, QueueMessage& msg);

	ErrorCode OpenCursor(QStorage::CursorHandle& cursorID);
	ErrorCode CloseCursor(QStorage::CursorHandle cursorID);
	ErrorCode CursorCopy(QStorage::CursorHandle& dest, QStorage::CursorHandle& src);
	ErrorCode AdvanceCursor(QStorage::CursorHandle cursorID);
	ErrorCode SeekID(const uuid& id, QStorage::CursorHandle cursorID);


	// TODO: ErrorCode SeekFront(QStorage::CursorHandle cursorID);
	// TODO: ErrorCode SeekFirstPriority(MessagePriorityEnum priority, QStorage::CursorHandle cursorID);
	ErrorCode TestCursor(QStorage::CursorHandle cursorID);
	ErrorCode PeekCursor(bool retrievebody, QStorage::CursorHandle cursorID, QueueMessage& msg);
	ErrorCode BeginRetrieveCursor(bool retrievebody, QStorage::CursorHandle cursorID, QueueMessage& msg);

	ErrorCode WaitForID(const uuid& id, SAFMQ_INT32 timeout, QStorage::CursorHandle& messageCursor);
	ErrorCode WaitFront(SAFMQ_INT32 timeout);
	
	ErrorCode TTLPurge();
protected: // behavior
	void init();
	void recover();
	void recover_file(SAFMQ_FILE_ID fileid);
	SAFMQ_FILE_ID getFileIDFromName(const std::string& name);
	std::string& getFileNameFromID(std::string& dest, SAFMQ_INT32 fileid);
	std::string& getCurrentFileName(std::string& dest);
	QStorage::CursorHandle nextCursorID();

	ErrorCode statMessage(QStorage::CursorHandle cursorID, bool purged);
	ErrorCode statMessage(QStorage::IndexQueue::cursor& idxcur, bool purged);
	ErrorCode testCursor(QStorage::CursorHandle cursorID, QStorage::CursorHandleMap::iterator& cur);
	ErrorCode peekCursor(bool retrievebody, QStorage::CursorHandle cursorID, QueueMessage& msg);
	ErrorCode peekCursor(bool retrievebody, const QStorage::IndexQueue::cursor& idxcur, QueueMessage& message);
	ErrorCode openCursor(QStorage::CursorHandle& cursorID);
	ErrorCode closeCursor(QStorage::CursorHandle cursorID);
	ErrorCode seekID(const uuid& id, QStorage::CursorHandle cursorID);
	ErrorCode retrieveCursor(QStorage::CursorHandle cursorID, QueueMessage& message);

	SAFMQ_UINT32 getStorageBytes();
	SAFMQ_UINT32 getMessageBytes();

private: // behavior

public: // data
protected: // data
	IndexQueue					index;
	FileUtilizationMap			fileutilization;
	CursorHandleMap				cursors;
	SAFMQ_FILE_ID				currentfileid;
	std::string					storagedirectory;
	size_t						maxfilesize;
	Mutex						qmtx;
	IDSet						waitset;
	Signal						enqsignal;
	Signal						idsignal;
	std::string					name;

	bool						hasTTL;

	Stats						statistics;

	static const std::string	DIRSEP;
private: // data
};

} // end of safmq namespace

#endif // !defined(_QSTORAGE_H_)

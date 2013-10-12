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
#include "QStorage.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
//#include <unistd.h>
#include <stdio.h>
#include "DirectoryList.h"
#include "SystemDelivery.h"
#include "QManager.h"
#include "Log.h"

using namespace std;

const SAFMQ_INT32 MESSAGE_MARKER='QMsg';

extern QManager* theQueueManager;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QStorage::QStorage(const std::string& name, const std::string& storagedirectory, SAFMQ_INT32 maxfilesize) : enqsignal(false), idsignal(true), hasTTL(false)
{
	this->name = name;
	this->storagedirectory = storagedirectory;
	this->maxfilesize = maxfilesize;
	init();
}

QStorage::~QStorage()
{

}

void QStorage::init()
{
	currentfileid = 0;
	recover();
}

class list_pred {
public:
	bool operator()(DirListItem lhs, DirListItem rhs) const {
//		return lhs.c_time < rhs.c_time || (lhs.c_time==rhs.c_time && lhs.fname < rhs.fname);
		std::stringstream	slhs, srhs;
		SAFMQ_FILE_ID	ulhs, urhs;
		slhs << lhs.fname;
		slhs >> std::hex >> ulhs;
		srhs << rhs.fname;
		srhs >> std::hex >> urhs;
		return ulhs < urhs;
	}
};

void QStorage::recover()
{
	DirectoryList	files(storagedirectory, "*.q");
	std::sort(files.begin(),files.end(), list_pred());

	DirectoryList::iterator i;

	for(i = files.begin(); i != files.end(); i++) {
		currentfileid = getFileIDFromName(i->fname);
		recover_file(currentfileid);
	}
}

void QStorage::recover_file(SAFMQ_FILE_ID fileid)
{
	std::string				fname;

	std::fstream			f;
	QMsgStorageHdr			hdr;
	QMessage				msg;
	int						sizmsg=sizeof(msg);
	std::fstream::off_type	offset, tmpoff;

	f.open(getFileNameFromID(fname, fileid).c_str(), std::ios::in|std::ios::binary);
	f.seekg(0);
	if (f.good()) {
		while (f.good()) {
			hdr = QMsgStorageHdr();
			msg = QMessage();
			offset = f.tellg();
			f.read((char*)&hdr, sizeof(hdr));
			if (f.good()) {
				if (hdr.MsgMarker == MESSAGE_MARKER) {
					if (!hdr.Purged) {
						f.read((char*)&msg, sizeof(msg));
						if (f.good()) {
							if (msg.TimeToLiveSeconds > 0)
								hasTTL = true;
							index.enqueue(QueueIndex((MessagePriorityEnum)msg.MessagePriority, msg.MessageID, (MessageClassEnum)msg.MessageClass, msg.ReciptID, msg.TimeStamp, msg.TimeToLiveSeconds, fileid, offset, false));
							fileutilization[fileid]++;
						} else {
							// failed to read message header!
							break;
						}
						tmpoff = f.tellg();
						f.seekg(msg.BodySize, std::ios::cur);
						if ((std::fstream::off_type)f.tellg() != tmpoff + msg.BodySize)
							break;
					} else {
						tmpoff = f.tellg();
						f.seekg(hdr.Length, std::ios::cur);
						if ((std::fstream::off_type)f.tellg() != tmpoff+hdr.Length) 
							break; // failed to read far enough..
					}
				} else {
					// TODO: try to sync until another message is found...

					break;
				}
			}
		}
		f.close();
	}
}

SAFMQ_FILE_ID QStorage::getFileIDFromName(const std::string& name)
{
	SAFMQ_FILE_ID	val;
	int e = name.find_last_of(".");
	int b = name.find_last_of(DIRSEP);
	if (b == -1)
		b = 0;
	bufstream	s(128);
	s << name.substr(b,e-b);
	s >> std::hex >> std::uppercase;
	s >> val;
	return val;
}

std::string& QStorage::getFileNameFromID(std::string& dest, SAFMQ_INT32 fileid)
{
	std::stringstream	s;
	s << storagedirectory << DIRSEP << std::hex << std::setfill('0') << std::setw((sizeof(SAFMQ_INT32)*2)) << fileid << ".q" << std::ends;
	dest = s.str();

//	char	buffer[1024];
//	sprintf(buffer,"%s%s%08lx.q",storagedirectory.c_str(), DIRSEP, fileid);
//	dest = buffer;

	return dest;
}

std::string& QStorage::getCurrentFileName(std::string& dest)
{
	return getFileNameFromID(dest, currentfileid);
}

QStorage::CursorHandle  QStorage::nextCursorID()
{
	QStorage::CursorHandleMap::reverse_iterator i = cursors.rbegin();
	if (i != cursors.rend())
		return i->first + 1;
	return 0;
}

ErrorCode QStorage::statMessage(QStorage::CursorHandle cursorID, bool purged)
{
	CursorHandleMap::iterator	cur;
	ErrorCode	ret = testCursor(cursorID, cur);
	if (ret == EC_NOERROR) {
		statMessage(cur->second,purged);
	}
	return ret;
}

ErrorCode QStorage::statMessage(QStorage::IndexQueue::cursor& idxcur, bool purged)
{
	ErrorCode	ret = EC_NOERROR;
	std::string		fname;
	std::fstream	*f;
	
	//f.open(getFileNameFromID(fname,idxcur->FileID).c_str(), std::ios::in | std::ios::out | std::ios::binary);
	f = theQueueManager->GetStreamCache()->get(getFileNameFromID(fname,idxcur->FileID), std::ios::in | std::ios::out | std::ios::binary);
	if (f->good()) {
		f->seekg(idxcur->DataOffset, std::ios::beg);
		if (f->good() && (std::fstream::off_type)f->tellg() == idxcur->DataOffset) {
			QMsgStorageHdr	hdr;
			
			f->read((char*)&hdr,sizeof(hdr));
			if (f->good()) {
				if (hdr.MsgMarker == MESSAGE_MARKER) {
					hdr.Purged = purged;
					f->seekp(idxcur->DataOffset, std::ios::beg);
					if (f->good() && (std::fstream::off_type)f->tellp() == idxcur->DataOffset) {
						f->write((char*)&hdr, sizeof(hdr));
						idxcur->Purged = hdr.Purged;
					} else
						ret = EC_DOESNOTEXIST; // TODO Make a better error code
				} else
					ret = EC_DOESNOTEXIST; // TODO: Make a better error code
			} else
				ret = EC_DOESNOTEXIST; // TODO: Make a better error code
		} else
			ret = EC_DOESNOTEXIST; // TODO: make a better error code
		//f.close();
		theQueueManager->GetStreamCache()->release(f);
	} else
		ret = EC_DOESNOTEXIST; // TODO: make a better error code
	return ret;
}

ErrorCode QStorage::testCursor(QStorage::CursorHandle cursorID, QStorage::CursorHandleMap::iterator& cur)
{
	cur = cursors.find(cursorID);
	if (cur != cursors.end()) {
		if ( cur->second ) {
			if (cur->second != index.end()) {
				return EC_NOERROR;
			}
			return EC_NOMOREMESSAGES;
		}
		return EC_CURSORINVALIDATED;
	}
	return EC_NOTOPEN;
}

ErrorCode QStorage::peekCursor(bool retrievebody, QStorage::CursorHandle cursorID, QueueMessage& message)
{
	CursorHandleMap::iterator	cur;
	ErrorCode					ret = testCursor(cursorID, cur);
	if (ret == EC_NOERROR) {
		return peekCursor(retrievebody, cur->second, message);
	}
	return ret;
}

#ifdef unix
#define _stat stat
#endif

ErrorCode QStorage::peekCursor(bool retrievebody, const QStorage::IndexQueue::cursor& idxcur, QueueMessage& message)
{
	ErrorCode	ret = EC_NOERROR;
	if (!idxcur->Purged) {
		std::string		fname;
		std::fstream	*f;

		//f.open(getFileNameFromID(fname,idxcur->FileID).c_str(), std::ios::in | std::ios::out | std::ios::binary);
		f = theQueueManager->GetStreamCache()->get(getFileNameFromID(fname,idxcur->FileID), std::ios::in | std::ios::out | std::ios::binary);
		if (f->good()) {
			f->seekg(idxcur->DataOffset, std::ios::beg);
			if (f->good() && (std::fstream::off_type)f->tellg() == idxcur->DataOffset) {
				QMsgStorageHdr	hdr;
				QMessage		msg;

				memset(&msg, 0, sizeof(QMessage));
				errno = 0;
				f->read((char*)&hdr,sizeof(hdr));
				if (f->good()) {
					if (hdr.MsgMarker != MESSAGE_MARKER) {
						Log::getLog()->Info("Message Marker not equal, expected: %ld got: %ld", MESSAGE_MARKER, hdr.MsgMarker);
					}
//
//					Log::getLog()->Info("%ld: File Name: %s", GetCurrentThreadId(), getFileNameFromID(fname,idxcur->FileID).c_str());
//					Log::getLog()->Info("%ld: File Record Length: %ld", GetCurrentThreadId(), hdr.Length);
//					Log::getLog()->Info("%ld: Offset from beginning of file: %ld", GetCurrentThreadId(), idxcur->DataOffset);
//					struct _stat	stats;
//					_stat(getFileNameFromID(fname,idxcur->FileID).c_str(), &stats);
//					Log::getLog()->Info("%ld: File Size: %ld", GetCurrentThreadId(), stats.st_size);
//					Log::getLog()->Info("%ld: Attempt to read till: %ld", GetCurrentThreadId(), idxcur->DataOffset + sizeof(msg));

					if (hdr.Length >= sizeof(QMessage)) {
						f->read((char*)&msg, sizeof(msg));
						if (f->good()) {
							message.msg = msg;
							if (retrievebody && msg.BodySize > 0) {
								// retrieve the body
								bufstream	*buf = message.getBufferStream();
								buf->reserve(msg.BodySize);
								buf->seekg(0);
								buf->seekp(msg.BodySize);
								if ((bufstream::off_type)buf->tellp() != msg.BodySize) {
									// Weird Error here!
									Log::getLog()->Info("QStorage::peekCursor--Unable to seek to the end of the message buffer");
								}
								f->read(buf->buffer(), msg.BodySize);
							}
						} else {
							// read error on message
							int err = errno;

							Log::getLog()->Info("QStorage::peekCursor--read error reading message header");

//							Log::getLog()->Info("%ld: strerror: %s", GetCurrentThreadId(), strerror(err));
//							Log::getLog()->Info("%ld: Offset: %ld", GetCurrentThreadId(), idxcur->DataOffset);
//							Log::getLog()->Info("%ld: tellg: %ld", GetCurrentThreadId(), (streamoff)f->tellg());
//							Log::getLog()->Info("%ld: tellp: %ld", GetCurrentThreadId(), (streamoff)f->tellp());
//							Log::getLog()->Info("%ld: rdstate: 0x%03x", GetCurrentThreadId(), f->rdstate());
//							Log::getLog()->Info("%ld: eof: %ld", GetCurrentThreadId(), f->eof());
//							Log::getLog()->Info("%ld: good: %ld", GetCurrentThreadId(), f->good());
//							Log::getLog()->Info("%ld: fail: %ld", GetCurrentThreadId(), f->fail());
//							Log::getLog()->Info("%ld: bad: %ld", GetCurrentThreadId(), f->bad());
						
							ret = EC_DOESNOTEXIST;
						}
					} else {
						// Data error on message header
						Log::getLog()->Info("QStorage::peekCursor--data error--message length too small, length found: %ld", hdr.Length);
						ret = EC_DOESNOTEXIST;
					}
				} else {
					// read error on file record hdr
					Log::getLog()->Info("QStorage::peekCursor--read error reading header");
					ret = EC_DOESNOTEXIST;
				}
			} else {
				// seek error on message
				Log::getLog()->Info("QStorage::peekCursor--unable to seek message location");
				ret = EC_DOESNOTEXIST;
			}
			//f.close();
			theQueueManager->GetStreamCache()->release(f);
		}
	} else {
		ret = EC_CURSORINVALIDATED;
	}
	return ret;
}

ErrorCode QStorage::openCursor(QStorage::CursorHandle& cursorID)
{
	IndexQueue::cursor c = index.begin();

	while ((bool)c && c != index.end() && (c->Purged)) {
		++c;
	}
	if ((bool)c) {
		cursors[cursorID=nextCursorID()] = c;
		return EC_NOERROR;
	}
	return EC_CURSORINVALIDATED;
}

ErrorCode QStorage::closeCursor(QStorage::CursorHandle cursorID)
{
	CursorHandleMap::iterator	i = cursors.find(cursorID);
	if (i != cursors.end()) {
		cursors.erase(i);
		return EC_NOERROR;
	}
	return EC_NOTOPEN;
}

ErrorCode QStorage::seekID(const uuid& id, QStorage::CursorHandle cursorID)
{
	CursorHandleMap::iterator	cur;
	ErrorCode					ret = testCursor(cursorID, cur);
	if (ret == EC_NOERROR) {
		bool						found = false;
		IndexQueue::cursor			tmp = cur->second;

		if (ret == EC_NOERROR) {
			while (!found && tmp != index.end()) {
				if (tmp->ReciptID == id) {
					found = true;
					cur->second = tmp;
				} else
					++(tmp);
			}
			if (!found) 
				ret = EC_NOMOREMESSAGES;
		}
	}
	return ret;
}

SAFMQ_UINT32 QStorage::getStorageBytes()
{
	SAFMQ_UINT32	totalBytes = 0;
	string			filename;
	struct STAT		stats;

	for(FileUtilizationMap::iterator i = fileutilization.begin(); i != fileutilization.end(); ++i) {
		getFileNameFromID(filename, i->first);

		memset(&stats, 0, sizeof(stats));
		if (STAT(filename.c_str(), &stats) == 0) {
			totalBytes += (SAFMQ_UINT32)stats.st_size;
		}
	}


	return totalBytes;
}

SAFMQ_UINT32 QStorage::getMessageBytes()
{
	SAFMQ_UINT32				totalBytes = 0;
	QueueMessage				msg;
	CursorHandle				cur;
	CursorHandleMap::iterator	iCur;
	ErrorCode					ec = openCursor(cur);

	if (ec == EC_NOERROR) {
		ec = testCursor(cur, iCur); // Get the internal representation of the cursor

		while (ec == EC_NOERROR && iCur->second != index.end()) {
			ec = peekCursor(false, iCur->second, msg);
			if (ec == EC_NOERROR) {
				totalBytes += sizeof(QMsgStorageHdr) + sizeof(QMessage) + msg.getBodySize();
				++(iCur->second);
			}
		}
	}
	closeCursor(cur);

	return totalBytes;
}


ErrorCode QStorage::GetStatistics(QueueStatistics& stats, bool includeStorageBytes, bool includeMessageBytes)
{
	MutexLock	lock(&qmtx);
	ErrorCode	ret = EC_NOERROR;

	stats.messageCount = (SAFMQ_UINT32)index.size();
	stats.storageBytes = includeStorageBytes ? getStorageBytes() : 0;
	stats.messageBytes = includeMessageBytes ? getMessageBytes() : 0;

	_1060300 counts = statistics.enqueueCounts();
	stats.enqueued10second = counts._10;
	stats.enqueued60second = counts._60;
	stats.enqueued300second = counts._300;

	counts = statistics.dequeueCounts();
	stats.retrieved10second = counts._10;
	stats.retrieved60second = counts._60;
	stats.retrieved300second = counts._300;

	counts = statistics.peekCounts();
	stats.peeked10second = counts._10;
	stats.peeked60second = counts._60;
	stats.peeked300second = counts._300;

	return ret;
}


ErrorCode QStorage::Enqueue(QueueMessage& msg, QStorage::CursorHandle& enqCursorID)
{
	MutexLock		lock(&qmtx);
	ErrorCode		ret = EC_NOERROR;
	std::string		fname;
	struct STAT		stats;
	QMsgStorageHdr	hdr;
	std::fstream	*f;
	bool			good_write;

	// set the record in the file as not purged, but set the in memory index purged
	// to allow for transaction enqueueing.
	hdr.MsgMarker = MESSAGE_MARKER;
	hdr.Purged = false;
	hdr.Length = sizeof(msg.msg) + msg.getBufferSize();
	msg.msg.BodySize = msg.getBufferSize();

	f = theQueueManager->GetStreamCache()->get(getCurrentFileName(fname), std::ios::out|std::ios::app|std::ios::binary);
	if (f->good()) {
		f->seekp(0,std::ios::end); // prime the seek/tell position to tell where we are in the file
		std::fstream::off_type off = f->tellp();
		// Initially mark the message as purged, and then commit the write
		QueueIndex	qi(msg.getMessagePriority(), msg.getMessageID(), msg.getMessageClass(),
							msg.getReciptID(), msg.getTimeStamp(), msg.getTimeToLiveSeconds(),
							currentfileid, f->tellp(), true);

		f->write((const char*)&hdr, sizeof(hdr));
		if ((good_write=f->good())) {
			f->write((const char*)&msg.msg, sizeof(msg.msg));
			if ((good_write=f->good())) {
				f->write(msg.getBufferStream()->buffer(), msg.getBufferStream()->length());
				good_write=f->good();
			} else {
				// TODO: Use a better mechanism than Info
				Log::getLog()->Info("ERROR: Error writing to file: %s", getCurrentFileName(fname).c_str());
				ret = EC_FILESYSTEMERROR;
			}
		} else {
			// TODO: Use a better mechanism than Info
			Log::getLog()->Info("ERROR: Error writing to file: %s", getCurrentFileName(fname).c_str());
			ret = EC_FILESYSTEMERROR;
		}
		f->flush();
		theQueueManager->GetStreamCache()->release(f);

		if (good_write) {
			fileutilization[currentfileid]++;

			// check to see if the queue has any ttl records
			if (msg.getTimeToLiveSeconds() > 0)
				hasTTL = true;
			// set the cursor to the cursor id.
			cursors[enqCursorID=nextCursorID()] = index.enqueue(qi);

		}
		// if the current file has reached the max file size, increment to a new file
		memset(&stats, 0, sizeof(stats));
		if (STAT(fname.c_str(), &stats) == 0 && (size_t)stats.st_size > maxfilesize) {
			currentfileid++;
		}
	} else {
		int err = errno;
		// TODO: Use a better reporting mechanism than Info, this is a very serious error if it is not due to configuration!
		Log::getLog()->Info("ERROR: Error unable to open file: %s; %s", getCurrentFileName(fname).c_str(), strerror(err));
		ret = EC_FILESYSTEMERROR;
		// Release the allocated file.
		theQueueManager->GetStreamCache()->release(f);
	}
	return ret;
}

/**
Mark the message as not purged in the in memory index.
@param finalizeID A cursor pointing to the message to be made ready for reading
*/
ErrorCode QStorage::FinalizeEnqueue(QStorage::CursorHandle finalizeID)
{
	MutexLock	lock(&qmtx);
	ErrorCode ret = EC_NOERROR;

	CursorHandleMap::iterator cur = cursors.find(finalizeID);
	if (cur != cursors.end() && (bool)cur->second) {
		cur->second->Purged = false;

		IDSet::iterator waiter = waitset.find(cur->second->ReciptID);
		if (waiter != waitset.end()) { // if someone is waiting on the id then signal the id
			waitset.erase(waiter);
			idsignal.Set();
		} else { // otherwise signal the other waiters.
			enqsignal.Set();
		}
		closeCursor(finalizeID);
	} else {
		ret = EC_DOESNOTEXIST;
	}

	statistics.messageEnqueued();
	ServerStats::getServerStats()->messageEnqueued();
	return ret;	
}


static std::string trim(const std::string& src)
{
	std::string::size_type b = src.find_first_not_of(" \t\r\n");
	std::string::size_type e = src.find_last_not_of(" \t\r\n");

	if (b <= e && b != std::string::npos)
		return src.substr(b, e-b+1);

	return "";
}

/**
Mark the message as purged and remove the reference from the in memory index.
@param cancelID A cursor pointing to the message to be removed from the queue
			as it's transaction was canceled.
*/
ErrorCode QStorage::CancelEnqueue(QStorage::CursorHandle cancelID)
{
	MutexLock	lock(&qmtx);
	ErrorCode	ret = statMessage(cancelID,true);
	if (ret == EC_NOERROR) {
		CursorHandleMap::iterator cur = cursors.find(cancelID);
		if (cur != cursors.end() && (bool)cur->second) {
			FileUtilizationMap::iterator	util = 	fileutilization.find(cur->second->FileID);
			if (util != fileutilization.end()) {
				--(util->second);
				if (util->second <= 0) {
					// Remove the file, there are no more messages in it.
					std::string	fname;
					//::remove(getFileNameFromID(fname,cur->second->FileID).c_str());
					theQueueManager->GetStreamCache()->remove(getFileNameFromID(fname,cur->second->FileID));
				}
			}
			index.erase(cur->second); // erase the record from the in-memory queue
		} else {
			// fstream	f("foo.log",ios::app);
			// time_t t = time(NULL);
			// string	d = ctime(&t);
			// f << trim(d) << ": Cancel Enqueue cursor not found" << endl;
			// f.close();
		}
		closeCursor(cancelID);
	}
	return EC_NOERROR;
}

ErrorCode QStorage::BeginRetrieve(bool retrievebody, QueueMessage& msg, QStorage::CursorHandle& retrieveID)
{
	MutexLock	lock(&qmtx);
	
	ErrorCode ret = openCursor(retrieveID);
	if (ret == EC_NOERROR) {
		ret = peekCursor(retrievebody,retrieveID,msg);
		if (ret == EC_NOERROR) {
			CursorHandleMap::iterator cur = cursors.find(retrieveID);
			if (cur != cursors.end() && cur->second) // NOTE: This if should never fail as it was already tested in peekCursor
				cur->second->Purged = true;
		} else {
			closeCursor(retrieveID);
			retrieveID = -1;
		}
	}
	return ret;
}

ErrorCode QStorage::BeginRetrieveID(bool retrievebody, const uuid& id, QueueMessage& msg, QStorage::CursorHandle& retrieveID)
{
	MutexLock	lock(&qmtx);
	ErrorCode	ret = openCursor(retrieveID);
	if (ret == EC_NOERROR) {
		ret = seekID(id,retrieveID);
		if (ret == EC_NOERROR) {
			ret = peekCursor(retrievebody,retrieveID,msg);
			if (ret == EC_NOERROR) {
				CursorHandleMap::iterator cur = cursors.find(retrieveID);
				if ( cur != cursors.end() && cur->second) // NOTE: This if should never fail as it was already tested in peekCursor
					cur->second->Purged = true;
			}
		} else {
			closeCursor(retrieveID);
			retrieveID = -1;
		}
	}
	return ret;
}

/**
Mark the message as not purged in the in memory index, as its transaction was
not comitted (canceled).
@param cancelID A cursor pointing to the message to be made ready for reading
*/
ErrorCode QStorage::CancelRetrieve(QStorage::CursorHandle cancelID)
{
	MutexLock	lock(&qmtx);
	ErrorCode ret = EC_NOERROR;

	CursorHandleMap::iterator cur = cursors.find(cancelID);
	if (cur != cursors.end() && cur->second) {
		cur->second->Purged = false;
		closeCursor(cancelID);
		enqsignal.Set();
	} else {
		ret = EC_DOESNOTEXIST;
	}

	return ret;
}

/**
Mark the message as purged and remove the reference from the in memory index.
@param finalizeID A cursor pointing to the message to be removed from the queue
			as it's transaction was completed.
*/
ErrorCode QStorage::FinalizeRetrieve(QStorage::CursorHandle finalizeID)
{
	MutexLock	lock(&qmtx);
	ErrorCode	ret = statMessage(finalizeID,true);
	if (ret == EC_NOERROR) {
		CursorHandleMap::iterator cur = cursors.find(finalizeID);
		if (cur != cursors.end() && (bool)cur->second) {
			FileUtilizationMap::iterator	util = 	fileutilization.find(cur->second->FileID);
			if (util != fileutilization.end()) {
				--(util->second);
				if (util->second <= 0) {
					// Remove the file, there are no more messages in it.
					std::string	fname;
					//::remove(this->getFileNameFromID(fname,cur->second->FileID).c_str());
					theQueueManager->GetStreamCache()->remove(getFileNameFromID(fname,cur->second->FileID));
				}
			}
			index.erase(cur->second); // erase the record from the in-memory queue
		} else {
			// TODO: Produce an error here.

			// fstream	f("foo.log",ios::app);
			// time_t t = time(NULL);
			// string	d = ctime(&t);
			// f << trim(d) << ": Finalize Retrieve cursor not found" << endl;
			// f.close();
		}
		closeCursor(finalizeID);
	}
	statistics.messageDequeued();
	ServerStats::getServerStats()->messageDequeued();
	return EC_NOERROR;
}

ErrorCode QStorage::FinalizeRetrieveCursor(QStorage::CursorHandle retrieveID)
{
	MutexLock	lock(&qmtx);
	ErrorCode	ret = statMessage(retrieveID,true);
	if (ret == EC_NOERROR) {
		CursorHandleMap::iterator cur = cursors.find(retrieveID);
		if (cur != cursors.end() && (bool)cur->second) {
			FileUtilizationMap::iterator	util = 	fileutilization.find(cur->second->FileID);
			if (util != fileutilization.end()) {
				--(util->second);
				if (util->second <= 0) {
					// Remove the file, there are no more messages in it.
					std::string	fname;
					//::remove(this->getFileNameFromID(fname,cur->second->FileID).c_str());
					theQueueManager->GetStreamCache()->remove(getFileNameFromID(fname,cur->second->FileID));
				}
			}
			cur->second = index.erase(cur->second); // erase the record from the in-memory queue and advance the cursor
		}
	}
	statistics.messageDequeued();
	ServerStats::getServerStats()->messageDequeued();
	return EC_NOERROR;
}

ErrorCode QStorage::PeekID(bool retrievebody, const uuid& id, QueueMessage& msg)
{
	MutexLock	lock(&qmtx);
	CursorHandle peekHandle;
	ErrorCode	ret = openCursor(peekHandle);
	if (ret == EC_NOERROR) {
		ret = seekID(id,peekHandle);
		if (ret == EC_NOERROR) {
			ret = peekCursor(retrievebody,peekHandle,msg);
		}
		closeCursor(peekHandle);
	}
	statistics.messagePeeked();
	ServerStats::getServerStats()->messagePeeked();
	return ret;
}

ErrorCode QStorage::PeekFront(bool retrievebody, QueueMessage& msg)
{
	MutexLock		lock(&qmtx);
	CursorHandle	cur;
	ErrorCode		ret = openCursor(cur);

	if (ret == EC_NOERROR) {
		ret = peekCursor(retrievebody,cur,msg);
		closeCursor(cur);
	}
	statistics.messagePeeked();
	ServerStats::getServerStats()->messagePeeked();
	return ret;
}

ErrorCode QStorage::OpenCursor(QStorage::CursorHandle& cursorID)
{
	MutexLock	lock(&qmtx);
	return openCursor(cursorID);
}

ErrorCode QStorage::CloseCursor(QStorage::CursorHandle cursorID)
{
	MutexLock	lock(&qmtx);
	return closeCursor(cursorID);
}

ErrorCode QStorage::CursorCopy(QStorage::CursorHandle& dest, QStorage::CursorHandle& src)
{
	MutexLock					lock(&qmtx);
	CursorHandleMap::iterator	psrc, pdest;
	ErrorCode					ret;
	
	ret = testCursor(src,psrc);
	if (ret == EC_NOERROR) {
		ret = testCursor(dest,pdest);
		if (ret == EC_NOERROR || ret == EC_NOMOREMESSAGES) {
			ret = EC_NOERROR;
			pdest->second = psrc->second;
		}
	}

	return ret;
}

ErrorCode QStorage::AdvanceCursor(QStorage::CursorHandle cursorID)
{
	MutexLock					lock(&qmtx);
	CursorHandleMap::iterator	i;
	ErrorCode					ret = testCursor(cursorID, i);
	if (ret == EC_NOERROR)
		++(i->second);
	return ret;
}

ErrorCode QStorage::SeekID(const uuid& id, QStorage::CursorHandle cursorID)
{
	MutexLock	Lock(&qmtx);
	return seekID(id,cursorID);
}

ErrorCode QStorage::TestCursor(QStorage::CursorHandle cursorID)
{
	MutexLock					lock(&qmtx);
	CursorHandleMap::iterator	i;
	return testCursor(cursorID, i);
}

ErrorCode QStorage::PeekCursor(bool retrievebody, QStorage::CursorHandle cursorID, QueueMessage& message)
{
	MutexLock	lock(&qmtx);
	statistics.messagePeeked();
	ServerStats::getServerStats()->messagePeeked();
	return peekCursor(retrievebody,cursorID,message);
}

ErrorCode QStorage::BeginRetrieveCursor(bool retrievebody, QStorage::CursorHandle cursorID, QueueMessage& msg)
{
	MutexLock	lock(&qmtx);
	ErrorCode	ret = peekCursor(retrievebody,cursorID,msg);
	if (ret == EC_NOERROR) {
		CursorHandleMap::iterator cur = cursors.find(cursorID);
		if (cur != cursors.end() && cur->second)
			cur->second->Purged = true;
	}
	return ret;
}

ErrorCode QStorage::WaitForID(const uuid& id, SAFMQ_INT32 timeout, QStorage::CursorHandle& messageCursor)
{
	ErrorCode err;

	{
		MutexLock lock(&qmtx);
		err = openCursor(messageCursor);
		if (err == EC_NOERROR) {
			err = seekID(id,messageCursor);
			if (err == EC_NOERROR)
				return EC_NOERROR;
			closeCursor(messageCursor);
			messageCursor = -1;
		} else
			return err;
		waitset.insert(id);
	}

	Signal::Result	waitres;
	time_t			start = time(NULL);
	time_t			now = start;

	while ( timeout != 0 && (timeout < 0 || ((timeout - (now-start)) > 0) )) {
		if (timeout < 0)
			waitres = idsignal.WaitFor(-1);
		else 
			waitres = idsignal.WaitFor((timeout - (SAFMQ_INT32)(now-start)) * 1000);

		if (waitres == Signal::SIGNAL_TIMEOUT || waitres == Signal::SIGNAL_ERROR) {
			break;
		} else {
			MutexLock lock(&qmtx);
			if (waitset.find(id) == waitset.end()) {
				err = openCursor(messageCursor);
				if (err == EC_NOERROR) {
					err = seekID(id,messageCursor);
					if (err != EC_NOERROR)
						closeCursor(messageCursor);
				}
				return err;
			}
		}
		now = time(NULL);
	}

	{
		MutexLock	lock(&qmtx);
		waitset.erase(id);
	}
	if (timeout == 0)
		return EC_NOMOREMESSAGES;
	return EC_TIMEDOUT;
}

ErrorCode QStorage::WaitFront(SAFMQ_INT32 timeout)
{
	Signal::Result waitres;

	if (timeout < 0)
		waitres = enqsignal.WaitFor(-1);
	else
		waitres = enqsignal.WaitFor(timeout * 1000);

	if (waitres == Signal::SIGNAL_TIMEOUT)
		return EC_TIMEDOUT;
	else if (waitres == Signal::SIGNAL_ERROR)
		return EC_ERROR;
	return EC_NOERROR;
}

ErrorCode QStorage::TTLPurge()
{
	MutexLock			lock(&qmtx);
	time_t				now = time(NULL);
	IndexQueue::cursor	idx;
	bool				erased;
	ErrorCode			err = EC_NOERROR;

	if (hasTTL) {
		// Reset teh has ttl flag
		hasTTL = false;
		for(idx=index.begin(); (bool)idx && idx!=index.end(); ) {
			erased = false;
			// NOTE: For a 1 second timeout, it could be as muc as 1.999999 seconds
			// NOTE: before the message is timed out.
			if ((bool)idx) {
				QueueIndex	x = *idx;
				if (!idx->Purged && idx->TTL > 0) {
					if ((now - idx->Timestamp) >= idx->TTL) {
						SAFMQ_INT32 TTL = idx->TTL;
						time_t Timestamp = idx->Timestamp;
						time_t diff = now - Timestamp;
						QueueMessage	msg;
						err = peekCursor(false, idx, msg);
						if (err == EC_NOERROR) {
							err = statMessage(idx,true);

							Log::getLog()->Message(Log::ttl, getName().c_str(), &msg);

							if (err == EC_NOERROR) {
								FileUtilizationMap::iterator	util = 	fileutilization.find(idx->FileID);
								if (util != fileutilization.end()) {
									--(util->second);
									if (util->second <= 0) {
										// Remove the file, there are no more messages in it.
										std::string	fname;
										//::remove(this->getFileNameFromID(fname,idx->FileID).c_str());
										theQueueManager->GetStreamCache()->remove(getFileNameFromID(fname,idx->FileID));
									}
								}
								idx = index.erase(idx); // erase the record from the in-memory queue and advance the cursor
								erased = true;

								if (msg.getTTLErrorWanted()) {
									SystemMessage	errmsg;
									std::string			str;

									SystemConnection::BuildErrorMsg(errmsg,EC_TTLEXPIRED, msg.getMessageID());

									msg.getResponseQueueName(str);

									errmsg.setResponseQueueName(str);
									SystemDelivery::Enqueue(errmsg);
								}
							}
						}
					} else {
						// There is still a non expired TTL
						// Note that we should check again.
						hasTTL = true;
					}
				} // end if not purged and ttl > 0
				if (!erased)
					++idx;
			}
		}
	}
	return err;
}

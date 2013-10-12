#ifndef _CURSORCMDS_H_
#define _CURSORCMDS_H_

#include "Command.h"
#include "ServiceThread.h"
#include "comdefs.h"
#include "server_operators.h"
#include "QAccessControl.h"
#include "Log.h"

namespace safmq {

extern const char* EC_Decode(ErrorCode);

class AdvanceCursorCmd : public Command {
protected:
	CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS	params;
	ErrorCode response;

public:
	AdvanceCursorCmd(ServiceThread* pservice) : Command(pservice), response(EC_ERROR) {	}
	virtual ~AdvanceCursorCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new AdvanceCursorCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}

	virtual void writeResponse(std::ostream& o) {
		o << RESPONSE_RESPONSE(response) << std::flush;
	}

	virtual int perform() {
		if (service->loggedIn()) {
			QStorage* pqueue = service->getOpenQueue(params.queueID);
			if (pqueue) {
				response = pqueue->AdvanceCursor(params.cursorID);
			} else {
				response = EC_NOTOPEN;
			}
			return 0;
		}
		return -1;
	}
};
class OpenCursorCmd : public Command {
protected:
	QUEUE_CLOSE_OPEN_CURSOR_PARAMS	params;
	QStorage::CursorHandle			cursor;
	ErrorCode						response;

public:
	OpenCursorCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~OpenCursorCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new OpenCursorCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}

	virtual void writeResponse(std::ostream& o) {
		if (response == EC_NOERROR)
			o << OPEN_CURSOR_RESPONSE(cursor) << std::flush;
		else
			o << RESPONSE_RESPONSE(response) << std::flush;
	}

	virtual int perform() {
		if (service->loggedIn()) {
			QStorage* pqueue = service->getOpenQueue(params.queueID);
			if (pqueue) {
				if (SecurityControl::getSecurityControl()->testQueueOperation(service->getUsername(), pqueue->getName(), SecurityControl::READ) == SecurityControl::GRANTED) {
					// Open the cursor
					response = pqueue->OpenCursor(cursor);
					if (response == EC_NOERROR) {
						// Add the cursor to the list of open cursors
						service->addCursor(params.queueID, cursor);
						Log::getLog()->Info("Opened cursor %ld for queue:%s", (int)cursor, pqueue->getName().c_str());
					} 
				} else {
					response = EC_NOTAUTHORIZED;
				}
			} else {
				response = EC_NOTOPEN;
			}
			return 0;
		}
		return -1;
	}
};

class CloseCursorCmd : public Command {
protected:
	CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS	params;
	ErrorCode response;
public:
	CloseCursorCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~CloseCursorCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new CloseCursorCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}

	virtual void writeResponse(std::ostream& o) {
		o << RESPONSE_RESPONSE(response) << std::flush;
	}

	virtual int perform() {
		if (service->loggedIn()) {
			QStorage* pqueue = service->getOpenQueue(params.queueID);
			if (pqueue) {
				// Close the cursor
				response = pqueue->CloseCursor(params.cursorID);
				service->removeCursor(params.queueID, params.cursorID);
			} else {
				response = EC_NOTOPEN;
			}
			return 0;
		}
		return -1;
	}
};

class SeekIDCmd : public Command {
protected:
	SEEK_ID_PARAMS	params;
	ErrorCode response;
public:
	SeekIDCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~SeekIDCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new SeekIDCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}

	virtual void writeResponse(std::ostream& o) {
		o << RESPONSE_RESPONSE(response) << std::flush;
	}

	virtual int perform() {
		// TODO: If the queue becomes is completely empty when 
		// TODO: The ID being waited for returns, the cursor waited on
		// TODO: Can become invalidated, this can be a serious error
		// TODO: when using two queues for two way communicaiton and the
		// TODO: return queue is usually empty.  Or becomes empty while in
		// TODO: a call for WaitForID(), possibly
		if (service->loggedIn()) {
			QStorage* pqueue = service->getOpenQueue(params.queueID);
			if (pqueue) {
				QStorage::CursorHandle	h;
				response = pqueue->WaitForID(params.reciptID, params.timeoutseconds, h);
				pqueue->CloseCursor(h);
				if (response == EC_NOERROR)
					response = pqueue->SeekID(params.reciptID, params.cursorID);
			} else {
				response = EC_NOTOPEN;
			}
			return 0;
		}
		return -1;
	}
};

class TestCursorCmd : public Command {
protected:
	CLOSE_CURSOR_ADVANCE_CURSOR_TEST_CURSOR_PARAMS	params;
	ErrorCode response;
public:
	TestCursorCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~TestCursorCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new TestCursorCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}

	virtual void writeResponse(std::ostream& o) {
		o << RESPONSE_RESPONSE(response) << std::flush;
	}

	virtual int perform() {
		if (service->loggedIn()) {
			QStorage* pqueue = service->getOpenQueue(params.queueID);
			if (pqueue) {
				response = pqueue->TestCursor(params.cursorID);
			} else {
				response = EC_NOTOPEN;
			}
			return 0;
		}
		return -1;
	}
};
}

#endif

#ifndef _MESSAGECMDS_H_
#define _MESSAGECMDS_H_

#include "Command.h"
#include "ServiceThread.h"
#include "comdefs.h"
#include "server_operators.h"
#include "QAccessControl.h"
#include "Log.h"
#include "TransactionManager.h"

namespace safmq {
	
extern const char* EC_Decode(ErrorCode);

class EnqueueCmd : public Command {
protected:
	QueueMessage		msg;
	ENQUEUE_PARAMS		params;
	ENQUEUE_RESPONSE	resp;

	ErrorCode			response;
public:
	EnqueueCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {
		params.msg = &msg;
	}
	virtual ~EnqueueCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new EnqueueCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		if (response == EC_NOERROR) {
			o << resp << std::flush;
		} else {
			o << RESPONSE_RESPONSE(response) << std::flush;
		}
	}
	virtual int perform() {
		ServiceThread::QueueHandle queueID;
		
		queueID = params.queueID;
		if (service->loggedIn()) {
			QStorage* pqueue = service->getOpenQueue(queueID);
			if (pqueue) {
				if ( (msg.getMessageClass() != MC_SYSTEMERRMSG && SecurityControl::getSecurityControl()->testQueueOperation(service->getUsername(), pqueue->getName(), SecurityControl::WRITE) == SecurityControl::GRANTED)
					|| (msg.getMessageClass() == MC_SYSTEMERRMSG && SecurityControl::getSecurityControl()->testQueueOperation(service->getUsername(), pqueue->getName(), SecurityControl::SYSTEM_WRITE) == SecurityControl::GRANTED) ) {

					if (msg.getMessageClass() != MC_SYSTEMERRMSG || (msg.getBodyType() == BT_NONE && msg.getBodySize() == 0)) {
						uuidgen(&msg.msg.MessageID);

						time_t now;
						time(&now);

						msg.msg.TimeStamp = (SAFMQ_INT32)now;
						
						QStorage::CursorHandle	enqcur;
						response = pqueue->Enqueue(msg,enqcur);
						if (response == EC_NOERROR) {
							service->getTransactionManager()->Enqueue(queueID,msg.getMessageID(),enqcur);
							resp = ENQUEUE_RESPONSE(msg.getMessageID(), (SAFMQ_INT32)msg.getTimeStamp());
						}
					} else {
						response = EC_WRONGMESSAGETYPE;
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

class RetrieveCmd : public Command {
protected:
	RETRIEVE_PEEK_FRONT_PARAMS	params;
	RETRIEVE_RESPONSE	resp;
	QStorage::CursorHandle		retrieve_cur;
	QStorage* pqueue;
	ErrorCode response;
public:
	RetrieveCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR), pqueue(NULL) {}
	virtual ~RetrieveCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new RetrieveCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		if (response == EC_NOERROR) {
			resp.errorcode = htonl(response);
			o.write((char*)&resp.errorcode,sizeof(resp.errorcode)) << resp.msg.msg;
			if (o.good()) { // wrote the response code
				if (params.retrievebody)
					o.write((const char*)resp.msg.getBufferStream()->buffer(), resp.msg.getBufferSize());
				if (o.good()) { // potentially wrote the body
					o.flush();
					if (o.good()) { // successfually std::flushed the message
						// The message was sent mark the transaction manager with a successful begin retrieve
						service->getTransactionManager()->BeginRetrieve(params.queueID,resp.msg.getMessageID(),retrieve_cur);
					} else {
						pqueue->CancelRetrieve(retrieve_cur);
					}
				} else {
					pqueue->CancelRetrieve(retrieve_cur);
				}
			} else {
				pqueue->CancelRetrieve(retrieve_cur);
			}
		} else {
			o << RESPONSE_RESPONSE(response) << std::flush;
		}
	}
	
	// NOTE: perform() currently blocks until a message is found or the timeout is acheived
	virtual int perform() {
		if (service->loggedIn()) {
			pqueue = service->getOpenQueue(params.queueID);
			if (pqueue) {
				if (SecurityControl::getSecurityControl()->testQueueOperation(service->getUsername(), pqueue->getName(), SecurityControl::READ) == SecurityControl::GRANTED) {
					bool				retrieved = false;
					time_t				start = time(NULL);
					time_t				now  = start;

					while ( !retrieved ) {
						response = pqueue->BeginRetrieve(params.retrievebody!=0, resp.msg, retrieve_cur);
						if (response == EC_NOERROR) {
							// The mesage was successfully found on the queue.
							retrieved = true;
						} else {
							if (response == EC_NOMOREMESSAGES && params.timeoutseconds) {
								if (params.timeoutseconds > 0 && params.timeoutseconds - (now-start) > 0) {
									response = pqueue->WaitFront(params.timeoutseconds - (SAFMQ_INT32)(now-start));
								} else if (params.timeoutseconds < 0) {
									response = pqueue->WaitFront(-1);
								} else {
									response = EC_TIMEDOUT;
								}

								if (response == EC_TIMEDOUT) {
									// timeout expired
									retrieved = true;
								}
							} else {
								// No messages and timeout expired
								retrieved = true;
							}
						}
						now = time(NULL);
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

class RetrieveAckCmd : public Command {
protected:
	RETRIEVE_ACK_PARAMS	params;
	ErrorCode response;
public:
	RetrieveAckCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~RetrieveAckCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new RetrieveAckCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			response = service->getTransactionManager()->FinalizeRetrieve(params.queueID,params.msgID);
			return 0;
		}
		return -1;
	}
};
class RetrieveCursorAckCmd : public Command {
protected:
	RETRIEVE_ACK_PARAMS	params;
	ErrorCode response;
public:
	RetrieveCursorAckCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~RetrieveCursorAckCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new RetrieveCursorAckCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			response = service->getTransactionManager()->FinalizeRetrieve(params.queueID,params.msgID);
			return 0;
		}
		return -1;
	}
};
class RetrieveIDCmd : public Command {
protected:
	QStorage::CursorHandle		retrieve_cur;
	QStorage*					pqueue;
	RETRIEVE_RESPONSE			resp;
	RETRIEVE_ID_PEEK_ID_PARAMS	params;
	ErrorCode					response;
public:
	RetrieveIDCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~RetrieveIDCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new RetrieveIDCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		if (response == EC_NOERROR) {
			resp.errorcode = htonl(response);
			o.write((char*)&resp.errorcode,sizeof(resp.errorcode)) << resp.msg.msg;
			if (o.good()) {
				if (params.retrievebody)
					o.write(resp.msg.getBufferStream()->buffer(), resp.msg.getBufferSize());
				if (o.good()) {
					o.flush();
					if (o.good()) {
						service->getTransactionManager()->BeginRetrieve(params.queueID, resp.msg.getMessageID(), retrieve_cur);
					} else {
						pqueue->CancelRetrieve(retrieve_cur);
					}
				} else {
					pqueue->CancelRetrieve(retrieve_cur);
				}
			} else {
				pqueue->CancelRetrieve(retrieve_cur);
			}
		} else {
			o << RESPONSE_RESPONSE(response) << std::flush;
		}
	}
	virtual int perform() {
		if (service->loggedIn()) {
			pqueue = service->getOpenQueue(params.queueID);
			if (pqueue) {
				if (SecurityControl::getSecurityControl()->testQueueOperation(service->getUsername(), pqueue->getName(), SecurityControl::READ) == SecurityControl::GRANTED) {
					response = pqueue->BeginRetrieveID(params.retrievebody!=0,params.reciptID,resp.msg,retrieve_cur);
					if (response == EC_NOMOREMESSAGES && params.timeoutseconds) {
						response = pqueue->WaitForID(params.reciptID, params.timeoutseconds, retrieve_cur);
						if (response == EC_NOERROR) {
							response = pqueue->BeginRetrieveCursor(params.retrievebody!=0, retrieve_cur, resp.msg);
						}
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

class PeekIDCmd : public Command {
protected:
	RETRIEVE_ID_PEEK_ID_PARAMS	params;
	RETRIEVE_RESPONSE			resp;
	ErrorCode					response;
public:
	PeekIDCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~PeekIDCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new PeekIDCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}

	virtual void writeResponse(std::ostream& o) {
		if (response == EC_NOERROR) {
			resp.errorcode = htonl(response);
			o.write((char*)&resp.errorcode,sizeof(resp.errorcode)) << resp.msg.msg;
			if (o.good()) {
				if (params.retrievebody)
					o.write(resp.msg.getBufferStream()->buffer(), resp.msg.getBufferSize());
				if (o.good()) {
					o.flush();
				}
			}
		} else {
			o << RESPONSE_RESPONSE(response) << std::flush;
		}
	}

	virtual int perform() {
		if (service->loggedIn()) {
			QStorage* pqueue = service->getOpenQueue(params.queueID);
			if (pqueue) {
				if (SecurityControl::getSecurityControl()->testQueueOperation(service->getUsername(), pqueue->getName(), SecurityControl::READ) == SecurityControl::GRANTED) {
					response = pqueue->PeekID(params.retrievebody!=0,params.reciptID,resp.msg);

					if (response == EC_NOMOREMESSAGES && params.timeoutseconds) {
						QStorage::CursorHandle	message;
						response = pqueue->WaitForID(params.reciptID, params.timeoutseconds, message);

						if (response == EC_NOERROR) {
							response = pqueue->PeekCursor(params.retrievebody!=0, message, resp.msg);
							pqueue->CloseCursor(message);
						}
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

class PeekFrontCmd : public Command {
protected:
	RETRIEVE_RESPONSE			resp;
	RETRIEVE_PEEK_FRONT_PARAMS	params;
	ErrorCode					response;
public:
	PeekFrontCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~PeekFrontCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new PeekFrontCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		if (response == EC_NOERROR) {
			resp.errorcode = htonl(response);
			o.write((char*)&resp.errorcode,sizeof(resp.errorcode)) << resp.msg.msg;
			if (o.good()) {
				if (params.retrievebody)
					o.write(resp.msg.getBufferStream()->buffer(), resp.msg.getBufferSize());
				if (o.good())
					o.flush();
			}
		} else {
			o << RESPONSE_RESPONSE(response) << std::flush;
		}
	}
	virtual int perform() {
		if (service->loggedIn()) {
			QStorage* pqueue = service->getOpenQueue(params.queueID);
			if (pqueue) {
				if (SecurityControl::getSecurityControl()->testQueueOperation(service->getUsername(), pqueue->getName(), SecurityControl::READ) == SecurityControl::GRANTED) {
					bool				retrieved = false;
					time_t				start = time(NULL);
					time_t				now = start;

					while ( !retrieved) { 
						response = pqueue->PeekFront(params.retrievebody!=0,resp.msg);
						if (response == EC_NOERROR) {
							// message found
							retrieved = true;
						}  else {
							if (response == EC_NOMOREMESSAGES && params.timeoutseconds) {
								if (params.timeoutseconds > 0 && params.timeoutseconds - (now-start) > 0)						
									response = pqueue->WaitFront(params.timeoutseconds - (SAFMQ_INT32)(now-start));
								else if (params.timeoutseconds < 0)
									response = pqueue->WaitFront(-1);
								else
									response = EC_TIMEDOUT;

								if (response == EC_TIMEDOUT) {
									// Timeout expired
									retrieved = true;
								}
							} else {
								// No messages and timeout expired
								retrieved = true;
							}
						}
						now = time(NULL);
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


class EnqueueForwardCmd : public Command {
protected:
	ServiceThread::QueueHandle	queueID;
	QueueMessage				msg;
	ENQUEUE_PARAMS				params;
	ENQUEUE_RESPONSE			resp;
	ErrorCode					response;
public:
	EnqueueForwardCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {
		params.msg = &msg;
	}
	virtual ~EnqueueForwardCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new EnqueueForwardCmd(pservice); }


	virtual void readRequest(std::istream& i) { 
		i >> params;
	}

	virtual void writeResponse(std::ostream& o) {
		if (response == EC_NOERROR) {
			o << resp << std::flush;
		} else {
			o << RESPONSE_RESPONSE(response) << std::flush;
		}
	}
	virtual int perform() {
		queueID = params.queueID;
		if (service->loggedIn()) {
			if (stricmp(pcfg->getParam(ACCEPT_FORWARDS_PARAM,"no").c_str(),"yes") == 0) {

				if (msg.getMessageClass() != MC_SYSTEMERRMSG) {
					QStorage* pqueue = service->getOpenQueue(params.queueID);
					if (pqueue) {
						if (SecurityControl::getSecurityControl()->testQueueOperation(service->getUsername(), pqueue->getName(), SecurityControl::WRITE) == SecurityControl::GRANTED) {
							// NOTE: Since this is a forward, don't set the message id or timestamp,
							//       basically the only difference from a regular enqueue.
							QStorage::CursorHandle enqcur;
							response = pqueue->Enqueue(msg,enqcur);
							if (response == EC_NOERROR) {
								service->getTransactionManager()->Enqueue(queueID,msg.getMessageID(),enqcur);
								resp = ENQUEUE_RESPONSE(msg.getMessageID(), (SAFMQ_INT32)msg.getTimeStamp());
							}
						} else {
							response = EC_NOTAUTHORIZED;
						}
					} else {
						response = EC_NOTOPEN;
					}
				} else {
					response = EC_WRONGMESSAGETYPE;
				}
			} else {
				response = EC_FORWARDNOTALLOWED;
			}
			return 0;
		} 
		return -1;
	}
};

class PeekCursorCmd : public Command {
protected:
	PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS	params;
	RETRIEVE_RESPONSE					resp;
	ErrorCode							response;
public:
	PeekCursorCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~PeekCursorCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new PeekCursorCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		if (response == EC_NOERROR) {
			resp.errorcode = htonl(response);
			o.write((char*)&resp.errorcode,sizeof(resp.errorcode)) << resp.msg.msg;
			if (o.good()) {
				if (params.retrievebody)
					o.write(resp.msg.getBufferStream()->buffer(), resp.msg.getBufferSize());
				if (o.good())
					o.flush();
			}
		} else {
			o << RESPONSE_RESPONSE(response) << std::flush;
		}
	}
	virtual int perform() {
		if (service->loggedIn()) {
			QStorage* pqueue = service->getOpenQueue(params.queueID);

			if (pqueue) {
				response = pqueue->PeekCursor(params.retrievebody!=0,params.cursorID, resp.msg);
			} else {
				response = EC_NOTOPEN;
			}
			return 0;
		}
		return -1;
	}
};

class RetrieveCursorCmd : public Command {
protected:
	ErrorCode							response;
	QStorage*							pqueue;
	PEEK_CURSOR_RETRIEVE_CURSOR_PARAMS	params;
	RETRIEVE_RESPONSE					resp;
public:
	RetrieveCursorCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~RetrieveCursorCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new RetrieveCursorCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		if (response == EC_NOERROR) {
			resp.errorcode = htonl(response);
			o.write((char*)&resp.errorcode,sizeof(resp.errorcode)) << resp.msg.msg;
			if (o.good()) {
				if (params.retrievebody)
					o.write(resp.msg.getBufferStream()->buffer(), resp.msg.getBufferSize());
				if (o.good()) {
					o.flush();
					if (o.good()) {
						// Begin Retreive Cursor
						QStorage::CursorHandle	trans_cur;
						response = pqueue->OpenCursor(trans_cur);
						if (response == EC_NOERROR || response == EC_NOMOREMESSAGES) {
							// Advance the cursor to the next position
							// Done to be able to retrieve cursor again...
							if ((response = pqueue->CursorCopy(trans_cur, params.cursorID)) == EC_NOERROR) {
								pqueue->AdvanceCursor(params.cursorID);
						 		service->getTransactionManager()->BeginRetrieve(params.queueID,resp.msg.getMessageID(),trans_cur);
						 	}
						}
					} else {
						pqueue->CancelRetrieve(params.cursorID);
					}
				} else {
					pqueue->CancelRetrieve(params.cursorID);
				}
			} else {
				pqueue->CancelRetrieve(params.cursorID);
			}
		} else {
			o << RESPONSE_RESPONSE(response) << std::flush;
		}
	}

	virtual int perform() {
		if (service->loggedIn()) {
			pqueue = service->getOpenQueue(params.queueID);
			if (pqueue) {
				response = pqueue->BeginRetrieveCursor(params.retrievebody!=0,params.cursorID, resp.msg);
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

#ifndef _XACTCMDS_H_
#define _XACTCMDS_H_
#include "Command.h"
#include "ServiceThread.h"
#include "comdefs.h"
#include "server_operators.h"
#include "QAccessControl.h"
#include "Log.h"
#include "TransactionManager.h"

namespace safmq {

extern const char* EC_Decode(ErrorCode);

class BeginTransactionCmd : public Command {
protected:
	ErrorCode response;
public:
	BeginTransactionCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~BeginTransactionCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new BeginTransactionCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		SAFMQ_INT32 zero;
		i.read((char*)&zero, sizeof(zero));
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		// TODO: Change this to verify logged in status
		bool ok = service->getTransactionManager()->BeginTransaction();
		if (ok) {
			response = EC_NOERROR;
		} else {
			response = EC_ALREADYOPEN;
		}
		return 0;
	}
};
class CommitTransactionCmd : public Command {
public:
	CommitTransactionCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~CommitTransactionCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new CommitTransactionCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		SAFMQ_INT32 zero;
		i.read((char*)&zero,sizeof(zero));
	}

	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(EC_NOERROR) << std::flush;
	}

	virtual int perform() {
		// TODO: Change this to verify logged in status
		service->getTransactionManager()->CommitTransaction();
		return 0;
	}
};

class RollbackTransactionCmd : public Command {
protected:
public:
	RollbackTransactionCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~RollbackTransactionCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new RollbackTransactionCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		SAFMQ_INT32 zero;
		i.read((char*)&zero,sizeof(zero));
	}
	virtual void writeResponse(std::ostream& o) { 
		// TODO: Change this to verify logged in status
		o << RESPONSE_RESPONSE(EC_NOERROR) << std::flush;
	}
	virtual int perform() {
		service->getTransactionManager()->RollbackTransaction();
		return 0;
	}
};

class EndTransactionCmd : public Command {
protected:
	ErrorCode response;
public:
	EndTransactionCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~EndTransactionCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new EndTransactionCmd(pservice); }


	virtual void readRequest(std::istream& i) { 
		SAFMQ_INT32 zero;
		i.read((char*)&zero,sizeof(zero));
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		// TODO: Change this to verify logged in status
		bool ok = service->getTransactionManager()->EndTransaction();
		if (ok) {
			response = EC_NOERROR;
		} else {
			response = EC_NOTOPEN;
		}
		return 0;
	}
};

}
#endif

#ifndef _SERVER_CMDS_H_
#define _SERVER_CMDS_H_

#include "Command.h"
#include "ServiceThread.h"
#include "comdefs.h"
#include "server_operators.h"
#include "QAccessControl.h"
#include "Stats.h"

class ClientInfoCmd : public Command {
protected:
	CLIENT_INFO_RESPONSE	response;

public:
	ClientInfoCmd(ServiceThread* pservice) : Command(pservice) {
		response.errorcode = EC_NOERROR;
	}

	virtual ~ClientInfoCmd() {}

	static Command* allocate(ServiceThread* pservice) { 
		return new ClientInfoCmd(pservice);
	}

	virtual void readRequest(std::istream& i) { 
	}

	virtual void writeResponse(std::ostream& o) { 
		o << response << std::flush;
	}

	virtual int perform() {
		if (service->loggedIn()) {
			ClientRegistry::GetClientInfo(response.clientInfo);
			return 0;
		}
		return -1;
	}
};

class ServerStatsCmd : public Command {
protected:
	QUEUE_STATS_RESPONSE response;

public:
	ServerStatsCmd(ServiceThread* pservice) : Command(pservice) {
		response.errorcode = EC_NOERROR;
	}

	virtual ~ServerStatsCmd() {}

	static Command* allocate(ServiceThread* pservice) { 
		return new ServerStatsCmd(pservice);
	}

	virtual void readRequest(std::istream& i) { 
	}

	virtual void writeResponse(std::ostream& o) { 
		o << response << std::flush;
	}

	virtual int perform() {
		if (service->loggedIn()) {
			_1060300 counts = ServerStats::getServerStats()->enqueueCounts();
			response.stats.enqueued10second = counts._10;
			response.stats.enqueued60second = counts._60;
			response.stats.enqueued300second = counts._300;

			counts = ServerStats::getServerStats()->dequeueCounts();
			response.stats.retrieved10second = counts._10;
			response.stats.retrieved60second = counts._60;
			response.stats.retrieved300second = counts._300;

			counts = ServerStats::getServerStats()->peekCounts();
			response.stats.peeked10second = counts._10;
			response.stats.peeked60second = counts._60;
			response.stats.peeked300second = counts._300;
			return 0;
		}
		return -1;
	}
};
#endif

#ifndef _QUEUECMDS_H_
#define _QUEUECMDS_H_

#include "Command.h"
#include "ServiceThread.h"
#include "comdefs.h"
#include "server_operators.h"
#include "QAccessControl.h"
#include "Log.h"
#include "Stats.h"
#include <sstream>

namespace safmq {

extern const char* EC_Decode(ErrorCode);

class QueueCreateCmd : public Command {
protected:
	QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	params;
	ErrorCode response;
public:
	QueueCreateCmd(ServiceThread *pservice) : Command(pservice), response(EC_ERROR) {}
	virtual ~QueueCreateCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new QueueCreateCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->testActorOperation(service->getUsername(),SecurityControl::QUEUE);
			if (res == SecurityControl::GRANTED) {
				std::string name = std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str();
				response = theQueueManager->CreateQueue(name, service->getUsername());

				if (response == EC_NOERROR) {
					SecurityControl::getSecurityControl()->setInitialQueueControl(service->getUsername(), name);

					Log::getLog()->Queue(Log::created, 
						service->getUsername().c_str(),
						service->getPeername(),
						std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str());
				}
			} else {
				Log::getLog()->Queue(Log::error, 
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str(),
					"create",
					EC_Decode(EC_NOTAUTHORIZED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};

class QueueDeleteCmd : public Command {
protected:
	QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS params;
	ErrorCode response;
public:
	QueueDeleteCmd(ServiceThread *pservice) : Command(pservice), response(EC_ERROR) {}
	virtual ~QueueDeleteCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new QueueDeleteCmd(pservice); }


	virtual void readRequest(std::istream& i) { 
		i >> params;
	}

	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}

	virtual int perform() {
		if (service->loggedIn()) {
			std::string	qname = std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str();

			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->testQueueOperation(service->getUsername(), qname, SecurityControl::DESTROY);
			if (res == SecurityControl::GRANTED) {
				response = theQueueManager->DeleteQueue(qname);

				if (response == EC_NOERROR) {
					SecurityControl::getSecurityControl()->purgeQSecurity(service->getUsername(),qname);

					Log::getLog()->Queue(Log::deleted, 
						service->getUsername().c_str(),
						service->getPeername(),
						std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str());
				}
			} else {
				Log::getLog()->Queue(Log::error, 
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str(),
					"create",
					EC_Decode(EC_NOTAUTHORIZED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};
class QueueSetUserPermsCmd : public Command {
protected:
	QUEUE_SET_USER_PERMS_PARAMS	params;
	ErrorCode response;
public:
	QueueSetUserPermsCmd(ServiceThread *pservice) : Command(pservice), response(EC_ERROR) {}
	virtual ~QueueSetUserPermsCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new QueueSetUserPermsCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			std::string qname = std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str();
			std::string user = std::string(params.username,SAFMQ_USER_NAME_LENGTH).c_str();

			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->setQAccessControlUser(service->getUsername(),qname,user,
						QAccessControl(params.read!=0,params.write!=0,params.destroy!=0,params.change_security!=0));
			if (res == SecurityControl::GRANTED) {
				Log::getLog()->Queue(Log::acl, 
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str(),
					"user",
					user.c_str(),
					params.read!=0,
					params.write!=0,
					params.destroy!=0,
					params.change_security!=0);

				response = EC_NOERROR;
			} else {
				Log::getLog()->Queue(Log::error, 
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str(),
					"acl",
					EC_Decode(EC_NOTAUTHORIZED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};
class QueueDeleteUserPermsCmd : public Command {
protected:
	ErrorCode response;
	QUEUE_DEL_USER_PERMS_PARAM	params;
public:
	QueueDeleteUserPermsCmd(ServiceThread *pservice) : Command(pservice), response(EC_ERROR) {}
	virtual ~QueueDeleteUserPermsCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new QueueDeleteUserPermsCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			std::string qname = std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str();
			std::string user = std::string(params.username,SAFMQ_USER_NAME_LENGTH).c_str();

			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->deleteQAccessControlUser(service->getUsername(),qname,user);
			if (res == SecurityControl::GRANTED) {
				Log::getLog()->Queue(Log::aclclear, 
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str(),
					"user",
					user.c_str());


				response = EC_NOERROR;
			} else if (res == SecurityControl::FAILED) {
				Log::getLog()->Queue(Log::error, 
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str(),
					"acl clear",
					EC_Decode(EC_DOESNOTEXIST));

				response = EC_DOESNOTEXIST;
			} else {
				Log::getLog()->Queue(Log::error, 
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str(),
					"acl clear",
					EC_Decode(EC_NOTAUTHORIZED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};
class QueueSetGroupPermsCmd : public Command {
protected:
	QUEUE_SET_GROUP_PERMS_PARAMS	params;
	ErrorCode response;
public:
	QueueSetGroupPermsCmd(ServiceThread *pservice) : Command(pservice), response(EC_ERROR) {}
	virtual ~QueueSetGroupPermsCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new QueueSetGroupPermsCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			std::string qname = std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str();
			std::string group = std::string(params.groupname,SAFMQ_GROUP_NAME_LENGTH).c_str();

			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->setQAccessControlGroup(service->getUsername(),qname,group,
						QAccessControl(params.read!=0,params.write!=0,params.destroy!=0,params.change_security!=0));
			if (res == SecurityControl::GRANTED) {
				Log::getLog()->Queue(Log::acl, 
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str(),
					"group",
					group.c_str(),
					params.read!=0,
					params.write!=0,
					params.destroy!=0,
					params.change_security!=0);

				response = EC_NOERROR;
			} else {
				Log::getLog()->Queue(Log::error, 
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str(),
					"acl",
					EC_Decode(EC_NOTAUTHORIZED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};
class QueueDelGroupPermsCmd : public Command {
protected:
	QUEUE_DEL_GROUP_PERMS_PARAM	params;
	ErrorCode response;
public:
	QueueDelGroupPermsCmd(ServiceThread *pservice) : Command(pservice), response(EC_ERROR) {}
	virtual ~QueueDelGroupPermsCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new QueueDelGroupPermsCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			std::string qname = std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str();
			std::string group = std::string(params.groupname,SAFMQ_GROUP_NAME_LENGTH).c_str();

			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->deleteQAccessControlGroup(service->getUsername(),qname,group);
			if (res == SecurityControl::GRANTED) {
				Log::getLog()->Queue(Log::aclclear, 
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str(),
					"group",
					group.c_str());

				response = EC_NOERROR;
			} else if (res == SecurityControl::FAILED) {
				Log::getLog()->Queue(Log::error, 
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str(),
					"acl clear",
					EC_Decode(EC_DOESNOTEXIST));

				response = EC_DOESNOTEXIST;
			} else {
				Log::getLog()->Queue(Log::error, 
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str(),
					"acl clear",
					EC_Decode(EC_NOTAUTHORIZED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};
class QueueEnumPermsCmd : public Command {
protected:
	QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS	params;
	PERM_LIST	perms;
	ErrorCode response;
public:
	QueueEnumPermsCmd(ServiceThread *pservice) : Command(pservice), response(EC_ERROR) {}
	virtual ~QueueEnumPermsCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new QueueEnumPermsCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << QUEUE_ENUM_PERMS_RESPOSNE(perms.size());
		
		for(PERM_LIST::iterator i = perms.begin(); o.good() && i != perms.end(); i++) {
			QUEUE_PERM_DATA data;
			memset(&data, 0, sizeof(data));
			std::char_traits<char>::copy(data.entityname,i->key.id.c_str(),std::min(i->key.id.length(),(size_t)MAX_ENTITY_LENGTH));
			data.isgroup = i->key.type == ControlKey::GROUP;
			data.read = i->ctl.getRead();
			data.write = i->ctl.getWrite();
			data.destroy = i->ctl.getDestroy();
			data.change_security = i->ctl.getChangeSecurity();
			
			o << data;
		}			
		if (o.good())
			o << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			std::string qname = std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str();
			SecurityControl::getSecurityControl()->enumerateQueuePermissions(qname, perms);
			return 0;
		}
		return -1;
	}

};
class QueueOpenCmd : public Command {
protected:
	QUEUE_OPEN_QUEUE_DELETE_QUEUE_CREATE_QUEUE_ENUM_PERMS_PARAMS params;
	ServiceThread::QueueHandle openedQueue;
	ErrorCode response;
public:
	QueueOpenCmd(ServiceThread *pservice) : Command(pservice), response(EC_ERROR) {}
	virtual ~QueueOpenCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new QueueOpenCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		if (response == EC_NOERROR) {
			OPEN_QUEUE_RESPOSNE	resp;
			resp.queueID = openedQueue;
			o << resp << std::flush;
		} else {
			o << RESPONSE_RESPONSE(response) << std::flush;
		}
	}
	virtual int perform() {
		if (service->loggedIn()) {
			std::string	qname = std::string(params.queuename,SAFMQ_QNAME_LENGTH).c_str();
			
			if (qname != FORWARD_QUEUE_NAME || stricmp(pcfg->getParam(ENABLE_FOWARDING_PARAM,"no").c_str(),"yes")==0) {
				if (SecurityControl::getSecurityControl()->testQueueOperation(service->getUsername(), qname, SecurityControl::READ) == SecurityControl::GRANTED ||
						SecurityControl::getSecurityControl()->testQueueOperation(service->getUsername(), qname, SecurityControl::WRITE) == SecurityControl::GRANTED ||
							SecurityControl::getSecurityControl()->testQueueOperation(service->getUsername(), qname, SecurityControl::SYSTEM_WRITE) == SecurityControl::GRANTED ) {

					QStorage* pqueue = theQueueManager->GetQueue(qname);
					if (pqueue != NULL) {
						openedQueue = service->registerQueue(pqueue);
						response = EC_NOERROR;
					} else {
						response = EC_DOESNOTEXIST;
					}
				} else {
					response = EC_NOTAUTHORIZED;
				}
			} else {
				response = EC_FORWARDNOTALLOWED;
			}
			return 0;
		}
		return -1;
	}
};
class QueueCloseCmd : public Command {
protected:
	QUEUE_CLOSE_OPEN_CURSOR_PARAMS	params;
	ErrorCode response;
public:
	QueueCloseCmd(ServiceThread *pservice) : Command(pservice), response(EC_ERROR) {}
	virtual ~QueueCloseCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new QueueCloseCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			response = service->shutdownQueue(params.queueID);
			return 0;
		}
		return -1;
	}
};

class EnumQueuesCmd : public Command {
private:
	std::vector<QManager::QData>		names;

public:
	EnumQueuesCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~EnumQueuesCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new EnumQueuesCmd(pservice); }

	void writeResponse(std::ostream& o) {
		ENUM_QUEUES_RESPONSE				resp;
		resp.nQueues = names.size();
		o << resp;
		if (o.good()) {
			QUEUE_CONFIG_ENTRY	ent;
			std::vector<QManager::QData>::iterator	name;

			for(name = names.begin(); o.good() && name != names.end(); name++) {
				::memset(&ent,0,sizeof(ent));
				std::char_traits<char>::copy(ent.queuename, name->queuename.c_str(), std::min(name->queuename.length(),(size_t)SAFMQ_QNAME_LENGTH));
				std::char_traits<char>::copy(ent.owner, name->owner.c_str(), std::min(name->owner.length(),(size_t)SAFMQ_USER_NAME_LENGTH));
				o << ent;
			}
			o << std::flush;
		}
	}
	virtual int perform() {
		if (service->loggedIn()) {
			//std::vector<QManager::QData>::iterator	name;
			theQueueManager->EnumQueues(names);
			//for(name = names.begin(); name != names.end();) {
			//	if (false) { // TODO: Test visibility permissions
			//		name = names.erase(name);
			//	} else
			//		name++;
			//}
			return 0;
		}
		return -1;
	}
};

class QueueCreateTemporaryCmd : public Command {
protected:
	QUEUE_CREATE_TEMP_RESPONSE response;

public:
	QueueCreateTemporaryCmd(ServiceThread* pservice) : Command(pservice) {
		response.errorcode = EC_NOERROR;
	}

	virtual ~QueueCreateTemporaryCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new QueueCreateTemporaryCmd(pservice); }


	virtual void readRequest(std::istream& i) { 
		// NOTE: No Parameters
	}

	virtual void writeResponse(std::ostream& o) { 
		o << response << std::flush;
	}
	/*
	1) Create A temporary queue
	2) Setup initial access control
	3) Open the queue
	4) Return queue name and handle to user.
	*/
	virtual int perform() {
		if (service->loggedIn()) {
			uuid qn;
			std::stringstream buf;

			uuidgen(&qn);
			buf << qn;
			response.queueName = buf.str();

			response.errorcode = theQueueManager->CreateQueue(response.queueName, service->getUsername(), true, true);
			if (response.errorcode == EC_NOERROR) {
				SecurityControl::getSecurityControl()->setInitialQueueControl(service->getUsername(), response.queueName);
				SecurityControl::getSecurityControl()->setQAccessControlGroup(service->getUsername(), response.queueName, safmq_all_users, QAccessControl(false,true,false,false));


				QStorage* pqueue = theQueueManager->GetQueue(response.queueName);
				if (pqueue != NULL) {
					response.queueID = service->registerQueue(pqueue);
					response.errorcode = EC_NOERROR;
				} else {
					response.errorcode = EC_DOESNOTEXIST;
				}

				Log::getLog()->Queue(Log::created, 
					service->getUsername().c_str(),
					service->getPeername(),
					response.queueName.c_str());
			} else {
				response.queueName = "";
			}
			return 0;
		}
		return -1;
	}
};

class QueueStatsCmd : public Command {
protected:
	QUEUE_STATS_RESPONSE response;
	QUEUE_STATS_PARAMS request;

public:
	QueueStatsCmd(ServiceThread* pservice) : Command(pservice) {
		response.errorcode = EC_NOERROR;
	}

	virtual ~QueueStatsCmd() {}

	static Command* allocate(ServiceThread* pservice) { 
		return new QueueStatsCmd(pservice);
	}

	virtual void readRequest(std::istream& i) { 
		i >> request;
	}

	virtual void writeResponse(std::ostream& o) { 
		o << response << std::flush;
	}

	virtual int perform() {
		if (service->loggedIn()) {
			QStorage* pqueue = service->getOpenQueue(request.queueID);
			if (pqueue) {
				response.errorcode = pqueue->GetStatistics(response.stats, request.includeStorageBytes != 0, request.includeMessageBytes != 0);
			} else {
				response.errorcode = EC_NOTOPEN;
			}
			return 0;
		}
		return -1;
	}
};



}

#endif

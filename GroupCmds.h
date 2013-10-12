#ifndef _GROUPCMDS_H_
#define _GROUPCMDS_H_

#include "Command.h"
#include "ServiceThread.h"
#include "comdefs.h"
#include "server_operators.h"
#include "QAccessControl.h"
#include "Log.h"

namespace safmq {
	
extern const char* EC_Decode(ErrorCode);

class GroupCreateCmd : public Command {
protected:
	GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	params;
	ErrorCode response;
public:
	GroupCreateCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~GroupCreateCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new GroupCreateCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->createGroup(service->getUsername(), std::string(params.groupname,SAFMQ_GROUP_NAME_LENGTH).c_str());
			if (res == SecurityControl::GRANTED) {
				Log::getLog()->Group(Log::created, 
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str());

				response = EC_NOERROR;
			} else if (res == SecurityControl::FAILED) {
				Log::getLog()->Group(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str(),
					"create",
					EC_Decode(EC_ALREADYDEFINED));

				response = EC_ALREADYDEFINED;
			} else {
				Log::getLog()->Group(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str(),
					"create",
					EC_Decode(EC_ALREADYDEFINED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};
class GroupDeleteCmd : public Command {
protected:
	GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	params;
	ErrorCode response;
public:
	GroupDeleteCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~GroupDeleteCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new GroupDeleteCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->deleteGroup(service->getUsername(), std::string(params.groupname,SAFMQ_GROUP_NAME_LENGTH).c_str());
			if (res == SecurityControl::GRANTED){
				Log::getLog()->Group(Log::deleted,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str());

				response = EC_NOERROR;
			} else if (res == SecurityControl::FAILED) {
				Log::getLog()->Group(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str(),
					"delete",
					EC_Decode(EC_DOESNOTEXIST));

				response = EC_DOESNOTEXIST;
			} else {
				Log::getLog()->Group(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str(),
					"delete",
					EC_Decode(EC_NOTAUTHORIZED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};
class EnumGroupsCmd : public Command {
protected:
	GROUPS groups;
public:
	EnumGroupsCmd(ServiceThread* pservice): Command(pservice) {}
	virtual ~EnumGroupsCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new EnumGroupsCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
	}
	virtual void writeResponse(std::ostream& o) { 
		o << ENUM_GROUPS_RESPONSE(groups.size());
		if (o.good()) {
			char	groupname[SAFMQ_GROUP_NAME_LENGTH];
			for(GROUPS::iterator g=groups.begin();o.good() && g!=groups.end();g++) {
				memset(groupname,0,sizeof(groupname));
				std::char_traits<char>::copy(groupname,g->c_str(),std::min(g->length(),(size_t)SAFMQ_GROUP_NAME_LENGTH));
				htonwstr(groupname,SAFMQ_GROUP_NAME_LENGTH);
				o.write((char*)groupname,sizeof(groupname));
			}
			if (o.good())
				o.flush();
		}
	}
	virtual int perform() {
		if (service->loggedIn()) {
			SecurityControl::getSecurityControl()->enumerateGroups(groups);
			return 0;
		}
		return -1;
	}
};
class GroupAddUserCmd : public Command {
protected:
	GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS	params;
	ErrorCode response;
public:
	GroupAddUserCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~GroupAddUserCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new GroupAddUserCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->addUserGroup(service->getUsername(), std::string(params.username,SAFMQ_USER_NAME_LENGTH).c_str(), std::string(params.groupname,SAFMQ_GROUP_NAME_LENGTH).c_str());
			if (res == SecurityControl::GRANTED) {
				Log::getLog()->Group(Log::useradd,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str(),
					std::string(params.username,SAFMQ_USER_NAME_LENGTH).c_str());

				response = EC_NOERROR;
			} else if (res == SecurityControl::FAILED) {
				Log::getLog()->Group(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str(),
					"delete user",
					EC_Decode(EC_DOESNOTEXIST));

				response = EC_DOESNOTEXIST;
			} else {
				Log::getLog()->Group(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str(),
					"delete user",
					EC_Decode(EC_NOTAUTHORIZED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};
class GroupDeleteUserCmd : public Command {
protected:
	GROUP_ADD_USER_GROUP_DELETE_USER_PARAMS	params;
	ErrorCode response;
public:
	GroupDeleteUserCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~GroupDeleteUserCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new GroupDeleteUserCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->deleteUserGroup(service->getUsername(), std::string(params.username,SAFMQ_USER_NAME_LENGTH).c_str(), std::string(params.groupname,SAFMQ_GROUP_NAME_LENGTH).c_str());
			if (res == SecurityControl::GRANTED) {
				Log::getLog()->Group(Log::userdel,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str(),
					std::string(params.username,SAFMQ_USER_NAME_LENGTH).c_str());

				response = EC_NOERROR;
			} else {
				Log::getLog()->Group(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str(),
					"delete user",
					EC_Decode(EC_NOTAUTHORIZED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};
class GroupGetUsersCmd : public Command {
protected:
	GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	params;
	USERS	users;
	ErrorCode response;
public:
	GroupGetUsersCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~GroupGetUsersCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new GroupGetUsersCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		char	name[SAFMQ_USER_NAME_LENGTH];
		o << GROUP_GET_USERS_RESPONSE(users.size());
		for(USERS::iterator i = users.begin(); o.good() && i != users.end(); i++) {
			memset(name,0,sizeof(name));
			std::char_traits<char>::copy(name,i->first.c_str(),std::min(i->first.length(),(size_t)SAFMQ_USER_NAME_LENGTH));
			htonwstr(name,SAFMQ_USER_NAME_LENGTH);
			o.write((char*)name,sizeof(name));
		}
		if ( o.good() )
			o.flush();
	}
	virtual int perform() {
		if (service->loggedIn()) {
			SecurityControl::getSecurityControl()->enumerateGroupUsers(std::string(params.groupname,SAFMQ_GROUP_NAME_LENGTH).c_str(), users);
			return 0;
		}
		return -1;
	}
};
class GroupSetPermsCmd : public Command {
protected:
	GROUP_SET_PERMS_PARAMS	params;
	ErrorCode response;
public:
	GroupSetPermsCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~GroupSetPermsCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new GroupSetPermsCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			std::string	group = std::string(params.groupname,SAFMQ_GROUP_NAME_LENGTH).c_str();
			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->setActorControlGroup(service->getUsername(),group,ActorControl(params.modify_queues!=0,params.modify_users!=0,params.modify_groups!=0));
			if (res == SecurityControl::GRANTED) {
				Log::getLog()->Group(Log::acl,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str(),
					params.modify_queues!=0,
					params.modify_users!=0,
					params.modify_groups!=0);

				response = EC_NOERROR;
			} else if (res == SecurityControl::FAILED) {
				Log::getLog()->Group(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str(),
					"acl",
					EC_Decode(EC_DOESNOTEXIST));

				response = EC_DOESNOTEXIST;
			} else {
				Log::getLog()->Group(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.groupname, SAFMQ_GROUP_NAME_LENGTH).c_str(),
					"acl",
					EC_Decode(EC_NOTAUTHORIZED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};
class GroupGetPermsCmd : public Command {
protected:
	GROUP_CREATE_GROUP_DELETE_GROUP_GET_USERS_GROUP_GET_PERMS_PARAMS	params;
	USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE resp;
	ErrorCode response;
public:
	GroupGetPermsCmd(ServiceThread* pservice): Command(pservice), response(EC_ERROR) {}
	virtual ~GroupGetPermsCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new GroupGetPermsCmd(pservice); }

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
		if (service->loggedIn()) {
			std::string	group = std::string(params.groupname,SAFMQ_GROUP_NAME_LENGTH).c_str();
			ActorControl	ctl;
			if (SecurityControl::getSecurityControl()->getActorControlGroup(service->getUsername(),group,ctl) == SecurityControl::GRANTED) {
				resp.data.modify_queues = ctl.getModifyQueues();
				resp.data.modify_groups = ctl.getModifyGroups();
				resp.data.modify_users = ctl.getModifyUsers();
				response = EC_NOERROR;
			} else {
				response = EC_DOESNOTEXIST;
			}
			return 0;
		}
		return -1;
	}
};

}
#endif

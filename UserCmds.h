#ifndef _USERCMDS_H_
#define _USERCMDS_H_

#include "Command.h"
#include "ServiceThread.h"
#include "comdefs.h"
#include "server_operators.h"
#include "QAccessControl.h"
#include "Log.h"

namespace safmq {
	
extern const char* EC_Decode(ErrorCode);

class UserCreateCmd : public Command
{
protected:
	USER_CREATE_PARAMS	params;
	ErrorCode			response;
public:
	UserCreateCmd(ServiceThread* pservice) : Command(pservice), response(EC_NOERROR) {}
	virtual ~UserCreateCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new UserCreateCmd(pservice); }

	
	virtual void readRequest(std::istream& i) { 
		i >> params;
	}

	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}

	virtual int perform() {
		if (service->loggedIn()) {
			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->createUser(service->getUsername(), 
											std::string(params.username,	SAFMQ_USER_NAME_LENGTH).c_str(), 
											std::string(params.password,	SAFMQ_PASSWORD_LENGTH).c_str(), 
											std::string(params.description,	SAFMQ_DESCRIPTION_LENGTH).c_str());
			if (res == SecurityControl::GRANTED) {
				// new user succesfully created
				Log::getLog()->User(Log::created, 
						service->getUsername().c_str(),
						service->getPeername(),
						std::string(params.username, SAFMQ_USER_NAME_LENGTH).c_str(),
						std::string(params.description, SAFMQ_DESCRIPTION_LENGTH).c_str());

				response = EC_NOERROR;
			} else if (res == SecurityControl::FAILED) {
				// new user rejected ID already exists
				Log::getLog()->User(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.username, SAFMQ_USER_NAME_LENGTH).c_str(),
					"create",
					EC_Decode(EC_ALREADYDEFINED));

				response = EC_ALREADYDEFINED;
			} else {
				// user not authorized to create new user accounts
				Log::getLog()->User(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.username, SAFMQ_USER_NAME_LENGTH).c_str(),
					"create",
					EC_Decode(EC_NOTAUTHORIZED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};


class UserDeleteCmd : public Command
{
protected:
	USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS	params;
	ErrorCode response;
public:
	UserDeleteCmd(ServiceThread* pservice) : Command(pservice), response(EC_NOERROR) {}
	virtual ~UserDeleteCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new UserDeleteCmd(pservice); }


	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) {
		o << RESPONSE_RESPONSE(response) << std::flush;
	}

	virtual int perform() {
		if (service->loggedIn()) {
			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->deleteUser(service->getUsername(), std::string(params.username, SAFMQ_USER_NAME_LENGTH).c_str());
			if (res == SecurityControl::GRANTED) {
				Log::getLog()->User(Log::deleted,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.username, SAFMQ_USER_NAME_LENGTH).c_str());

				response = EC_NOERROR;
			} else if (res == SecurityControl::FAILED) {
				Log::getLog()->User(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.username, SAFMQ_USER_NAME_LENGTH).c_str(),
					"delete",
					EC_Decode(EC_DOESNOTEXIST));

				response = EC_DOESNOTEXIST;
			} else {
				Log::getLog()->User(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.username, SAFMQ_USER_NAME_LENGTH).c_str(),
					"delete",
					EC_Decode(EC_NOTAUTHORIZED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}	
};

class EnumUsersCmd : public Command
{
protected:
	USERS users;

public:
	EnumUsersCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~EnumUsersCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new EnumUsersCmd(pservice); }

	
	virtual void readRequest(std::istream& i) { }

	virtual void writeResponse(std::ostream& o) { 
		o << ENUM_USERS_RESPONSE(users.size());
		if (o.good()) {
			char uname[SAFMQ_USER_NAME_LENGTH];
			char desc[SAFMQ_DESCRIPTION_LENGTH];

			for(USERS::iterator u=users.begin();o.good() && u!=users.end();u++) {
				memset(uname,0,sizeof(uname));
				memset(desc,0,sizeof(desc));
				std::char_traits<char>::copy(uname,u->first.c_str(),std::min(u->first.length(),(size_t)SAFMQ_USER_NAME_LENGTH));
				std::char_traits<char>::copy(desc,u->second.c_str(),std::min(u->second.length(),(size_t)SAFMQ_DESCRIPTION_LENGTH));
				htonwstr(uname,SAFMQ_USER_NAME_LENGTH);
				htonwstr(desc,SAFMQ_DESCRIPTION_LENGTH);
				o.write((char*)uname,sizeof(uname));
				o.write((char*)desc,sizeof(desc));
			}
			if (o.good())
				o << std::flush;
		}
	}

	virtual int perform() {
		if (service->loggedIn()) {
			SecurityControl::getSecurityControl()->enumerateUsers(users);
			return 0;
		}
		return -1;
	}
};
class UserSetPermsCmd : public Command
{
protected:
	USER_SET_PERMS_PARAMS	params;
	ErrorCode				response;
public:
	UserSetPermsCmd(ServiceThread* pservice) : Command(pservice), response(EC_NOERROR) {}
	virtual ~UserSetPermsCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new UserSetPermsCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			std::string	user = std::string(params.username,SAFMQ_USER_NAME_LENGTH).c_str();
			
			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->setActorControlUser(
						service->getUsername(),
						user,
						ActorControl(params.modify_queues!=0, params.modify_users!=0, params.modify_groups!=0));

			if (res == SecurityControl::GRANTED) {
				Log::getLog()->User(Log::acl,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.username, SAFMQ_USER_NAME_LENGTH).c_str(),
					params.modify_queues!=0,
					params.modify_users!=0,
					params.modify_groups!=0);

				response = EC_NOERROR;
			} else if (res == SecurityControl::FAILED) {
				Log::getLog()->User(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.username, SAFMQ_USER_NAME_LENGTH).c_str(),
					"access control",
					EC_Decode(EC_DOESNOTEXIST));

				response = EC_DOESNOTEXIST;
			} else {
				Log::getLog()->User(Log::error,
					service->getUsername().c_str(),
					service->getPeername(),
					std::string(params.username, SAFMQ_USER_NAME_LENGTH).c_str(),
					"access control",
					EC_Decode(EC_NOTAUTHORIZED));

				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};
class UserGetPermsCmd : public Command
{
protected:
	USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS	params;
	USER_GET_PERMS_GROUP_GET_PERMS_RESPONSE				resp;
public:
	UserGetPermsCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~UserGetPermsCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new UserGetPermsCmd(pservice); }


	virtual void readRequest(std::istream& i) { 
		i >> params;
	}

	virtual void writeResponse(std::ostream& o) { 
		if (resp.errorcode == EC_NOERROR)
			o << resp << std::flush;
		else
			o << RESPONSE_RESPONSE(resp.errorcode) << std::flush;
	}

	virtual int perform() {
		if (service->loggedIn()) {
			std::string	user = std::string(params.username,SAFMQ_USER_NAME_LENGTH).c_str();
			ActorControl	ctl;
			if (SecurityControl::getSecurityControl()->getActorControlUser(service->getUsername(),user,ctl) == SecurityControl::GRANTED) {
				resp.errorcode = EC_NOERROR;
				resp.data.modify_queues = ctl.getModifyQueues();
				resp.data.modify_groups = ctl.getModifyGroups();
				resp.data.modify_users = ctl.getModifyUsers();
			} else {
				resp.errorcode = EC_DOESNOTEXIST;
			}
			return 0;
		}
		return -1;
	}
};
class UserGetGroupsCmd : public Command
{
	USER_DELETE_USER_GET_PERMS_USER_GET_GROUPS_PARAMS	params;
	GROUPS												groups;
public:
	UserGetGroupsCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~UserGetGroupsCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new UserGetGroupsCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}

	virtual void writeResponse(std::ostream& o) { 
		USER_GET_GROUPS_RESPONSE	resp(groups.size());
		char						groupname[SAFMQ_GROUP_NAME_LENGTH];
		o << resp;

		for(GROUPS::iterator i = groups.begin(); o.good() && i!= groups.end(); i++) {
			memset(groupname,0,sizeof(groupname));
			std::char_traits<char>::copy(groupname,i->c_str(),std::min(i->length(),(size_t)SAFMQ_GROUP_NAME_LENGTH));
			htonwstr(groupname,SAFMQ_GROUP_NAME_LENGTH);
			o.write((char*)groupname, sizeof(groupname));
		}
		if (o.good())
			o << std::flush;
	}

	virtual int perform() {
		if (service->loggedIn()) {
			std::string	user = std::string(params.username, SAFMQ_USER_NAME_LENGTH).c_str();
			SecurityControl::getSecurityControl()->enumerateUserGroups(user, groups);
			return 0;
		}
		return -1;
	}
};
class UserSetPasswordCmd : public Command
{
protected:
	USER_SET_PASSWORD_PARAMS	params;
	ErrorCode					response;

public:
	UserSetPasswordCmd(ServiceThread* pservice) : Command(pservice), response(EC_NOERROR) {}
	virtual ~UserSetPasswordCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new UserSetPasswordCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			std::string	user = std::string(params.username,SAFMQ_USER_NAME_LENGTH).c_str();
			std::string passwd = std::string(params.password,SAFMQ_PASSWORD_LENGTH).c_str();

			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->changePassword(service->getUsername(), user, passwd);
			if (res == SecurityControl::GRANTED)
				response = EC_NOERROR;
			else if (res == SecurityControl::FAILED)
				response = EC_DOESNOTEXIST;
			else
				response = EC_NOTAUTHORIZED;
			return 0;
		}
		return -1;
	}
};
class UserAddIdentityCmd : public Command
{
protected:
	ADD_USER_IDENTITY_PARAMS	params;
	ErrorCode					response;
public:
	UserAddIdentityCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~UserAddIdentityCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new UserAddIdentityCmd(pservice); }

	virtual void readRequest(std::istream& i) {
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->addUserIdentity(service->getUsername(), params.subjectDN, params.issuerDN, params.username);
			if (res == SecurityControl::GRANTED) {
				// TODO: Log this event
				response = EC_NOERROR;
			} else if (res == SecurityControl::FAILED) {
				// TODO: Log this event
				response = EC_DOESNOTEXIST;
			} else {
				// TODO: Log this event
				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};
class UserDelIdentityCmd : public Command
{
protected:
	REMOVE_USER_IDENTITY	params;
	ErrorCode				response;
public:
	UserDelIdentityCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~UserDelIdentityCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new UserDelIdentityCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) { 
		o << RESPONSE_RESPONSE(response) << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			SecurityControl::SecurityResult res = SecurityControl::getSecurityControl()->removeIdentity(service->getUsername(), params.subjectDN, params.issuerDN);
			if (res == SecurityControl::GRANTED) {
				// TODO: Log this event
				response = EC_NOERROR;
			} else if (res == SecurityControl::FAILED) {
				// TODO: Log this event
				response = EC_DOESNOTEXIST;
			} else {
				// TODO: Log this event
				response = EC_NOTAUTHORIZED;
			}
			return 0;
		}
		return -1;
	}
};
class UserEnumIdentityCmd : public Command
{
protected:
	ENUM_USER_IDENTITY  params;
	IDENTITIES			ids;
public:
	UserEnumIdentityCmd(ServiceThread* pservice) : Command(pservice) {}
	virtual ~UserEnumIdentityCmd() {}

	static Command* allocate(ServiceThread* pservice) { return new UserEnumIdentityCmd(pservice); }

	virtual void readRequest(std::istream& i) { 
		Log::getLog()->Info("Reading USER_ENUM_IDENT params");
		i >> params;
	}
	virtual void writeResponse(std::ostream& o) {
		Log::getLog()->Info("Writing USER_ENUM_IDENT result, count:%ld", ids.size());
		o << RESPONSE_RESPONSE(EC_NOERROR) << ids << std::flush;
	}
	virtual int perform() {
		if (service->loggedIn()) {
			Log::getLog()->Info("Getting list of identities for user: %s", params.username.c_str());
			SecurityControl::getSecurityControl()->enumerateUserIdentities(params.username, ids);
			return 0;
		}
		return -1;
	}
};

}

#endif

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
#if !defined(QACCESSCONTROL_H)
#define QACCESSCONTROL_H

#pragma warning(disable: 4786)


#include <set>
#include <map>
#include <vector>
#include "Mutex.h"
#include <string>
#include "safmq_defs.h"

class QAccessControl  
{
public:
	QAccessControl(bool rd = false, bool wr = false, bool destroy = false, bool change_security = false) {
		this->rd = rd;
		this->wr = wr;
		this->destroy = destroy;
		this->change_security = change_security;
	}
	
	QAccessControl(const QAccessControl& src) {
		operator=(src);
	}
	
	virtual ~QAccessControl() { }

	const QAccessControl& operator=(const QAccessControl& src) {
		this->rd = src.rd;
		this->wr = src.wr;
		this->destroy = src.destroy;
		this->change_security = src.change_security;
		return *this;
	}

	bool getRead() const { return rd; }
	bool getWrite() const { return wr; }
	bool getDestroy() const { return destroy; }
	bool getChangeSecurity() const { return change_security; }

protected:
	bool	rd;
	bool	wr;
	bool	destroy;
	bool	change_security;
};

class ActorControl
{
public:
	ActorControl(bool queues=false, bool users=false, bool groups=false) {
		this->queues = queues;
		this->users = users;
		this->groups = groups;
	}
	
	ActorControl(const ActorControl& src) {
		operator=(src);
	}
	
	virtual ~ActorControl() {}
	
	const ActorControl& operator=(const ActorControl& src) {
		queues = src.queues;
		users = src.users;
		groups = src.groups;
		return *this;
	}

	bool getModifyQueues() const { return queues; }
	bool getModifyUsers() const { return users; }
	bool getModifyGroups() const { return groups; }
protected:
	bool	queues;
	bool	users;
	bool	groups;
};

class ControlKey
{
public:
	enum CKType { NONE, USER, GROUP };

	ControlKey(const std::string& id="", CKType type=NONE) {
		this->id = id;
		this->type = type;
	}
	ControlKey(const ControlKey& src) {
		operator=(src);
	}
	virtual ~ControlKey() {}
	const ControlKey& operator=(const ControlKey& src) {
		id = src.id;
		type = src.type;
		return *this;
	}
	bool operator<(const ControlKey& src) const {
		int cmp = id.compare(src.id);
		if (cmp < 0 && id.length() == 0)
			cmp = 1;
		return cmp < 0 || (cmp == 0 && (int)type < (int)src.type);
	}
public:
	std::string	id;
	CKType	type;
};

class QueueControlKey
{
public:
	QueueControlKey(const std::string& que="", const ControlKey& key = ControlKey()) {
		this->que = que;
		this->key = key;
	}
	QueueControlKey(const QueueControlKey& src) {
		operator=(src);
	}
	virtual ~QueueControlKey() {}
	const QueueControlKey& operator=(const QueueControlKey& src) {
		que = src.que;
		key = src.key;
		return *this;
	}
	bool operator<(const QueueControlKey& src) const {
		int cmp = que.compare(src.que);
		return cmp < 0 || (cmp == 0 && key < src.key);
	}
public:
	std::string		que;
	ControlKey	key;
};


struct DescPass {
	DescPass(const std::string& desc, const std::string& passwd) : description(desc.c_str()), password(passwd.c_str()) {}
	DescPass(const DescPass& pwd) : description(pwd.description.c_str()), password(pwd.password.c_str()) {}

	std::string description;
	std::string password;
};




typedef std::map<QueueControlKey, QAccessControl>	QUEUE_ACL;
typedef std::map<ControlKey, ActorControl>			ACTOR_ACL;
typedef std::multimap<std::string, std::string>		USER_GROUPS;
typedef std::map<std::string, DescPass>				USER_PASSWORDS;
typedef std::set<std::string>						GROUPS;
typedef std::map<std::string,std::string>			USERS;
typedef std::vector<safmq::X509Identity>			IDENTITIES;
typedef std::map<safmq::X509Identity, std::string>	IDENTITY_MAP;

struct PERM_ENTRY {
	PERM_ENTRY() {}
	PERM_ENTRY(const PERM_ENTRY& src) {
		operator=(src);
	}
	PERM_ENTRY(const ControlKey& key, const QAccessControl& ctl) {
		this->key = key;
		this->ctl = ctl;
	}
	const PERM_ENTRY& operator=(const PERM_ENTRY& src) {
		key = src.key;
		ctl = src.ctl;
		return *this;
	}
	ControlKey		key;
	QAccessControl	ctl;
};

typedef std::vector<PERM_ENTRY>						PERM_LIST;

class SecurityControl
{
protected:
	SecurityControl();
	virtual ~SecurityControl();
public:
	enum QOperation		{ DESTROY, READ, WRITE, CHANGE_SECURITY, SYSTEM_WRITE };
	enum ActorOperation	{ USER, GROUP, QUEUE };

	enum SecurityResult	{ FAILED, DENIED, GRANTED };

	static SecurityControl* getSecurityControl();

	SecurityResult testQueueOperation(const std::string& actor, const std::string& que, SecurityControl::QOperation operation);
	SecurityResult testActorOperation(const std::string& actor, SecurityControl::ActorOperation operation);

	SecurityResult verifyPassword(const std::string& user, const std::string& password);

	SecurityResult createUser(const std::string& actor, const std::string& user, const std::string& password, const std::string& description);
	SecurityResult changePassword(const std::string& actor, const std::string& user, const std::string& password);
	SecurityResult deleteUser(const std::string& actor, const std::string& user);

	SecurityResult identifyUser(const std::string& subjectDN, const std::string& issuerDN, std::string& identifiedUser);
	SecurityResult addUserIdentity(const std::string& actor, const std::string& subjectDN, const std::string& issuerDN, const std::string& username);
	SecurityResult removeIdentity(const std::string& actor, const std::string& subjectDN, const std::string& issuerDN);
	void enumerateUserIdentities(const std::string& username, IDENTITIES& );


	SecurityResult createGroup(const std::string& actor, const std::string& group);
	SecurityResult deleteGroup(const std::string& actor, const std::string& group);
	SecurityResult addUserGroup(const std::string& actor, const std::string& user, const std::string& group);
	SecurityResult deleteUserGroup(const std::string& actor, const std::string& user, const std::string& group);

	SecurityResult setActorControlUser(const std::string& actor, const std::string& user, const ActorControl& ctl);
	SecurityResult setActorControlGroup(const std::string& actor, const std::string& group, const ActorControl& ctl);

	SecurityResult getActorControlUser(const std::string& actor, const std::string& user, ActorControl& ctl);
	SecurityResult getActorControlGroup(const std::string& actor, const std::string& group, ActorControl& ctl);

	SecurityResult setInitialQueueControl(const std::string& actor, const std::string& que);
	SecurityResult setQAccessControlUser(const std::string& actor, const std::string& que, const std::string& user, const QAccessControl& ctl);
	SecurityResult setQAccessControlGroup(const std::string& actor, const std::string& que, const std::string& group, const QAccessControl& ctl);
	SecurityResult deleteQAccessControlUser(const std::string& actor, const std::string& que, const std::string& user);
	SecurityResult deleteQAccessControlGroup(const std::string& actor, const std::string& que, const std::string& group);

	SecurityResult purgeQSecurity(const std::string& actor, const std::string& que);

	void enumerateGroupUsers(const std::string& group, USERS& users);
	void enumerateUserGroups(const std::string& user, GROUPS& group_names);
	
	void enumerateGroups(GROUPS& groups);
	void enumerateUsers(USERS& users);
	
	void enumerateQueuePermissions(const std::string& que, PERM_LIST& perms);

protected:
	void loadSecurityLists();
	void loadGroups();
	void loadUsers();
	void loadUserGroups();
	void loadActorACL();
	void loadQueueACL();
	void loadIdentities();

	void storeSecurityLists();
	void storeGroups();
	void storeUsers();
	void storeUserGroups();
	void storeActorACL();
	void storeQueueACL();
	void storeIdentities();

	SecurityControl::SecurityResult testQueueOperation(QOperation op, const QAccessControl& ctl);
	SecurityControl::SecurityResult testActorOperation(SecurityControl::ActorOperation op, const ActorControl& ctl);

	static SecurityControl* theSecurityControl;
	static Mutex			scmtx;

	QUEUE_ACL		qacl;
	ACTOR_ACL		actoracl;
	USER_GROUPS		user_groups;
	USER_PASSWORDS	user_passwords;
	GROUPS			groups;
	IDENTITY_MAP	identities;

	std::string		cfg_dir;

	static std::string	group_file, user_file, user_group_file, actoracl_file, qacl_file, identity_file;
};

#endif // !defined(QACCESSCONTROL_H)

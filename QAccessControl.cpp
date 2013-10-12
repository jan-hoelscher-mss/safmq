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
#include "QAccessControl.h"
#include "DirectoryList.h"
#include "utilities.h"
#include "main.h"
#include "Log.h"

#include <fstream>
#include <iostream>
#include <stdlib.h>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const static std::string admin_name("admin");
const static std::string admin_desc("default admin account");
const static std::string admin_def_passwd("");
const static std::string admin_group("administration");

const static std::string	identityTokens(":");
const static char			identityEscape = '\\';


SecurityControl*	SecurityControl::theSecurityControl = NULL;
Mutex				SecurityControl::scmtx;

std::string			SecurityControl::group_file(DIRSEP"group.cfg"), 
					SecurityControl::user_file(DIRSEP"user.cfg"),
					SecurityControl::user_group_file(DIRSEP"user_group.cfg"),
					SecurityControl::actoracl_file(DIRSEP"aacl.cfg"),
					SecurityControl::qacl_file(DIRSEP"qacl.cfg"),
					SecurityControl::identity_file(DIRSEP"identity.cfg");

SecurityControl::SecurityControl()
{
	
	cfg_dir = pcfg->getParam(CONFIG_DIR_PARAM,"queues");
	if (cfg_dir.length() == 0)
		cfg_dir = "queues";

	loadSecurityLists();

	// NOTE: Maybe this should go in load users and load actor acl
	if (user_passwords.find(admin_name) == user_passwords.end()) {
		user_passwords.insert(USER_PASSWORDS::value_type(admin_name,DescPass(admin_desc,admin_def_passwd)));
		actoracl[ControlKey(admin_name,ControlKey::USER)] = ActorControl(true,true,true);
	}
	if (groups.find(admin_group) == groups.end()) {
		groups.insert(admin_group);
		actoracl[ControlKey(admin_group,ControlKey::GROUP)] = ActorControl(true,true,true);
		if (user_passwords.find(admin_name) != user_passwords.end())
			user_groups.insert(USER_GROUPS::value_type(admin_name,admin_group));
	}
}

SecurityControl::~SecurityControl()
{
}

SecurityControl* SecurityControl::getSecurityControl()
{
	MutexLock	lock(&scmtx);
	if (!theSecurityControl)
		return (theSecurityControl=new SecurityControl());
	return theSecurityControl;
}

SecurityControl::SecurityResult SecurityControl::testQueueOperation(const std::string& actor, const std::string& que, SecurityControl::QOperation operation)
{
	MutexLock	lock(&scmtx);
	SecurityResult		access = DENIED;

	if (operation == SYSTEM_WRITE && actor == system_user) {
		access = GRANTED;
	} else {
		// First test the actor, then test the group
		QUEUE_ACL::iterator	handle = qacl.find(QueueControlKey(que, ControlKey(actor, ControlKey::USER)));
		
		if (handle != qacl.end())
			access = testQueueOperation(operation, handle->second);

		if (access == DENIED && actor == admin_name && user_passwords.find(admin_name) != user_passwords.end())
			access = GRANTED;

		USER_GROUPS::iterator	grp;
		if (access == DENIED) {
			for(grp = user_groups.find(actor); access == DENIED && grp != user_groups.end() && grp->first == actor; ++grp) {
				handle = qacl.find(QueueControlKey(que, ControlKey(grp->second,ControlKey::GROUP)));
				if (handle != qacl.end())
					access = testQueueOperation(operation, handle->second);
				if (access == DENIED && grp->second == admin_group && groups.find(admin_group) != groups.end())
					access = GRANTED;
			}
		}


		if (access == DENIED) {
			handle = qacl.find(QueueControlKey(que, ControlKey(safmq_all_users, ControlKey::GROUP)));
			if (handle != qacl.end())
				access = testQueueOperation(operation, handle->second);
		}
	}
	
	return access;
}
SecurityControl::SecurityResult SecurityControl::testQueueOperation(QOperation op, const QAccessControl& ctl)
{
	bool	allowed = false;
	switch (op) {
		case DESTROY:			allowed = ctl.getDestroy(); break;
		case READ:				allowed = ctl.getRead(); break;
		case WRITE:				allowed = ctl.getWrite(); break;
		case CHANGE_SECURITY:	allowed = ctl.getChangeSecurity(); break;
	}
	return allowed ? GRANTED : DENIED;
}

SecurityControl::SecurityResult SecurityControl::testActorOperation(const std::string& actor, SecurityControl::ActorOperation operation)
{
	MutexLock	lock(&scmtx);
	// First test the actor, then test the group
	ACTOR_ACL::iterator	handle = actoracl.find(ControlKey(actor,ControlKey::USER));
	SecurityResult		access = DENIED;
	
	if (handle != actoracl.end())
		access = testActorOperation(operation, handle->second);
	
	USER_GROUPS::iterator	grp;
	if (access == DENIED)
		for(grp = user_groups.find(actor); access == DENIED && grp != user_groups.end() && grp->first == actor; ++grp) {
			handle = actoracl.find(ControlKey(grp->second,ControlKey::GROUP));
			if (handle != actoracl.end())
				access = testActorOperation(operation, handle->second);
		}
	return access;
}
SecurityControl::SecurityResult SecurityControl::testActorOperation(SecurityControl::ActorOperation op, const ActorControl& ctl)
{
	bool allowed = false;
	switch (op) {
		case USER:			allowed = ctl.getModifyUsers(); break;
		case GROUP:			allowed = ctl.getModifyGroups(); break;
		case QUEUE:			allowed = ctl.getModifyQueues(); break;
	}
	return allowed ? GRANTED : DENIED;
}

SecurityControl::SecurityResult SecurityControl::verifyPassword(const std::string& user, const std::string& password)
{
	MutexLock		lock(&scmtx);
	SecurityResult	access = DENIED;

	if (user == system_user) {
 		access = GRANTED;
	} else {
		USER_PASSWORDS::iterator	iuser = user_passwords.find(user);
		
		if (iuser != user_passwords.end() && iuser->second.password == password)
			access = GRANTED;
	}
	return access;
}

SecurityControl::SecurityResult SecurityControl::createUser(const std::string& actor, const std::string& user, const std::string& password, const std::string& description)
{
	if (user == system_user) {
		return FAILED;
	} else {
		SecurityResult	access = testActorOperation(actor,USER);
		if (access == GRANTED) {
			MutexLock lock(&scmtx);
			USER_PASSWORDS::iterator	iuser = user_passwords.find(user);
			if (iuser != user_passwords.end())
				return FAILED;
			user_passwords.insert(USER_PASSWORDS::value_type(user,DescPass(description,password)));
			storeSecurityLists();
		}
		return access;
	}
}

SecurityControl::SecurityResult SecurityControl::changePassword(const std::string& actor, const std::string& user, const std::string& password)
{
	SecurityResult	access = testActorOperation(actor,USER);
	if (access == GRANTED || actor==user) {
		MutexLock lock(&scmtx);
		USER_PASSWORDS::iterator	iuser = user_passwords.find(user);
		if (iuser == user_passwords.end())
			return FAILED;
		iuser->second.password = password;
		storeSecurityLists();
		access = GRANTED;
	}
	return access;
}

SecurityControl::SecurityResult SecurityControl::deleteUser(const std::string& actor, const std::string& user)
{
	SecurityResult	access = testActorOperation(actor,USER);
	if (access == GRANTED) {
		MutexLock lock(&scmtx);
		USER_PASSWORDS::iterator	iuser = user_passwords.find(user);
		if (iuser == user_passwords.end())

			return FAILED;
		user_passwords.erase(iuser);
		
		// Delete the user's group affilation
		user_groups.erase(user);
		
		// Delete the user's actor control settings
		ACTOR_ACL::iterator	aacl = actoracl.find(ControlKey(user,ControlKey::USER));
		if (aacl != actoracl.end())
			actoracl.erase(aacl);
	
		//Delete user's que controls
		QUEUE_ACL::iterator	iq;
		for(iq = qacl.begin(); iq != qacl.end(); ) {
			if (iq->first.key.type == ControlKey::USER && iq->first.key.id == user) {
				QUEUE_ACL::iterator tmp = iq++;
				qacl.erase(tmp);
			} else {
				++iq;
			}
		}


		// erase the digitial certificate identities.
		IDENTITIES				ids;
		IDENTITIES::iterator	ids_itr;
		IDENTITY_MAP::iterator	id_itr;

		enumerateUserIdentities(user, ids);
		for(ids_itr = ids.begin(); ids_itr != ids.end(); ids_itr++) {
			id_itr = identities.find(*ids_itr);
			if (id_itr != identities.end()) {
				identities.erase(id_itr);
			}
		}


		storeSecurityLists();
	}
	return access;
}

SecurityControl::SecurityResult SecurityControl::identifyUser(const std::string& subjectDN, const std::string& issuerDN, std::string& identifiedUser)
{
	IDENTITY_MAP::iterator i = identities.find(safmq::X509Identity(subjectDN, issuerDN));

	if (i != identities.end()) {
		identifiedUser = i->second;
		return GRANTED;
	}

	safmq::Log::getLog()->Info("Subject: \"%s\"", subjectDN.c_str());
	safmq::Log::getLog()->Info("issuerDN: \"%s\"", issuerDN.c_str());
	for(i = identities.begin(); i != identities.end(); ++i) {
		safmq::Log::getLog()->Info("Identity Subject: \"%s\"", i->first.subjectDN.c_str());
		safmq::Log::getLog()->Info("Identity issuerDN: \"%s\"", i->first.issuerDN.c_str());
	}

	return FAILED;
}

SecurityControl::SecurityResult SecurityControl::addUserIdentity(const std::string& actor, const std::string& subjectDN, const std::string& issuerDN, const std::string& username)
{
	SecurityResult	access = testActorOperation(actor,USER);
	if (access == GRANTED) {
		identities[safmq::X509Identity(subjectDN, issuerDN)] = username;
		storeSecurityLists();
	}
	return access;
}

SecurityControl::SecurityResult SecurityControl::removeIdentity(const std::string& actor, const std::string& subjectDN, const std::string& issuerDN)
{
	SecurityResult	access = testActorOperation(actor,USER);
	if (access == GRANTED) {
		IDENTITY_MAP::iterator	i = identities.find(safmq::X509Identity(subjectDN, issuerDN));
		if (i != identities.end()) {
			identities.erase(i);
		}
		storeSecurityLists();
	}
	return access;
}

void SecurityControl::enumerateUserIdentities(const std::string& username, IDENTITIES& ids)
{
	IDENTITY_MAP::iterator	i;

	ids.clear();
	for(i = identities.begin(); i != identities.end(); i++) {
		if (i->second == username) {
			ids.push_back(i->first);
		}
	}
}

SecurityControl::SecurityResult SecurityControl::createGroup(const std::string& actor, const std::string& group)
{
	SecurityResult	access = testActorOperation(actor,GROUP);
	if (access == GRANTED) {
		MutexLock lock(&scmtx);
		GROUPS::iterator	grp = groups.find(group);
		if (grp != groups.end())
			return FAILED;
		groups.insert(group);
		storeSecurityLists();
	}
	return access;
}

SecurityControl::SecurityResult SecurityControl::deleteGroup(const std::string& actor, const std::string& group)
{
	SecurityResult	access = testActorOperation(actor,GROUP);
	if (access == GRANTED) {
		MutexLock lock(&scmtx);
		GROUPS::iterator	grp = groups.find(group);
		if (grp == groups.end())
			return FAILED;

		groups.erase(grp);

		// Delete the groups's users
		USER_GROUPS::iterator	ug;
		for(ug = user_groups.begin(); ug != user_groups.end(); ) {
			if (ug->second == group) {
				USER_GROUPS::iterator	tmp = ug++;
				user_groups.erase(tmp);
			} else {
				++ug;
			}
		}

		// Delete the user's actor control settings
		ACTOR_ACL::iterator	aacl = actoracl.find(ControlKey(group,ControlKey::GROUP));
		if (aacl != actoracl.end())
			actoracl.erase(aacl);
	
		//Delete user's que controls
		QUEUE_ACL::iterator	iq;
		for(iq = qacl.begin(); iq != qacl.end(); ) {
			if (iq->first.key.type == ControlKey::GROUP && iq->first.key.id == group) {
				QUEUE_ACL::iterator tmp = iq++;
				qacl.erase(tmp);
			} else {
				++iq;
			}
		}

		storeSecurityLists();
	}
	return access;
}

SecurityControl::SecurityResult SecurityControl::addUserGroup(const std::string& actor, const std::string& user, const std::string& group)
{
	SecurityResult	access = testActorOperation(actor,GROUP);
	if (access == GRANTED) {
		MutexLock lock(&scmtx);

		if (groups.find(group) == groups.end())
			return FAILED; // group does not exist
		
		bool					found = false;
		USER_GROUPS::iterator	ug;
		for(ug = user_groups.find(user); !found && ug != user_groups.end() && ug->first == user; ug++)
			found = ug->second == group;
		if (!found) {
			user_groups.insert(USER_GROUPS::value_type(user,group));
			storeSecurityLists();
		}
	}
	return access;
}

SecurityControl::SecurityResult SecurityControl::deleteUserGroup(const std::string& actor, const std::string& user, const std::string& group)
{
	SecurityResult	access = testActorOperation(actor,GROUP);
	if (access == GRANTED) {
		MutexLock lock(&scmtx);
		USER_GROUPS::iterator	ug;

		for(ug = user_groups.find(user); ug != user_groups.end() && ug->first == user; ug++) {
			if (ug->second == group) {
				user_groups.erase(ug);
				storeSecurityLists();
				break;
			}
		}
	}
	return access;
}

SecurityControl::SecurityResult SecurityControl::setActorControlUser(const std::string& actor, const std::string& user, const ActorControl& ctl)
{
	SecurityResult	access = testActorOperation(actor,USER);
	if (access == GRANTED) {
		MutexLock lock(&scmtx);
		if (user_passwords.find(user) == user_passwords.end())
			return FAILED;

		actoracl[ControlKey(user,ControlKey::USER)] = ctl;
		storeSecurityLists();
	}
	return access;
}

SecurityControl::SecurityResult SecurityControl::setActorControlGroup(const std::string& actor, const std::string& group, const ActorControl& ctl)
{
	SecurityResult	access = testActorOperation(actor,GROUP);
	if (access == GRANTED) {
		MutexLock lock(&scmtx);
		if (groups.find(group) == groups.end())
			return FAILED;

		actoracl[ControlKey(group,ControlKey::GROUP)] = ctl;
		storeSecurityLists();
	}
	return access;
}

SecurityControl::SecurityResult SecurityControl::getActorControlUser(const std::string& actor, const std::string& user, ActorControl& ctl)
{
	MutexLock lock(&scmtx);
	ACTOR_ACL::iterator	i = actoracl.find(ControlKey(user,ControlKey::USER));
	if (i != actoracl.end()) {
		ctl = i->second;
		return GRANTED;
	}
	return FAILED;
}

SecurityControl::SecurityResult SecurityControl::getActorControlGroup(const std::string& actor, const std::string& group, ActorControl& ctl)
{
	MutexLock lock(&scmtx);
	ACTOR_ACL::iterator	i = actoracl.find(ControlKey(group,ControlKey::GROUP));
	if (i != actoracl.end()) {
		ctl = i->second;
		return GRANTED;
	}
	return FAILED;
}


SecurityControl::SecurityResult SecurityControl::setInitialQueueControl(const std::string& actor, const std::string& que)
{
	MutexLock lock(&scmtx);
	qacl[QueueControlKey(que,ControlKey(actor,ControlKey::USER))] = QAccessControl(true,true,true,true);
	storeSecurityLists();
	return GRANTED;
}

SecurityControl::SecurityResult SecurityControl::setQAccessControlUser(const std::string& actor, const std::string& que, const std::string& user, const QAccessControl& ctl)
{
	SecurityResult access = testQueueOperation(actor,que,CHANGE_SECURITY);
	if (access == GRANTED) {
		MutexLock lock(&scmtx);
		qacl[QueueControlKey(que,ControlKey(user,ControlKey::USER))] = ctl;
		storeSecurityLists();
	}
	return access;
}

SecurityControl::SecurityResult SecurityControl::setQAccessControlGroup(const std::string& actor, const std::string& que, const std::string& group, const QAccessControl& ctl)
{
	SecurityResult access = testQueueOperation(actor,que,CHANGE_SECURITY);
	if (access == GRANTED) {
		MutexLock lock(&scmtx);
		qacl[QueueControlKey(que,ControlKey(group,ControlKey::GROUP))] = ctl;
		storeSecurityLists();
	}

	return access;
}

SecurityControl::SecurityResult SecurityControl::deleteQAccessControlUser(const std::string& actor, const std::string& que, const std::string& user)
{
	SecurityResult access = testQueueOperation(actor,que,CHANGE_SECURITY);
	if (access == GRANTED) {
		MutexLock lock(&scmtx);
		QUEUE_ACL::iterator	qa = qacl.find(QueueControlKey(que,ControlKey(user,ControlKey::USER)));
		if (qa == qacl.end())
			return FAILED;
		qacl.erase(qa);
		storeSecurityLists();
	}
	return access;
}

SecurityControl::SecurityResult SecurityControl::deleteQAccessControlGroup(const std::string& actor, const std::string& que, const std::string& group)
{
	SecurityResult access = testQueueOperation(actor,que,CHANGE_SECURITY);
	if (access == GRANTED) {
		MutexLock lock(&scmtx);
		QUEUE_ACL::iterator	qa = qacl.find(QueueControlKey(que,ControlKey(group,ControlKey::GROUP)));
		if (qa == qacl.end())
			return FAILED;
		qacl.erase(qa);
		storeSecurityLists();
	}
	return access;
}

SecurityControl::SecurityResult SecurityControl::purgeQSecurity(const std::string& actor, const std::string& que)
{
// NOTE: Should come after a check to see if delete is ok, no reason to check twice.
//	SecurityResult access = testActorOperation(actor,QUEUE);
//	if (access == GRANTED) {
		MutexLock	lock(&scmtx);
//		QueueControlKey			idx(que);
//		QUEUE_ACL::iterator	f = qacl.lower_bound(idx);
//		QUEUE_ACL::iterator	l = f;
		
//		while(l != qacl.end() && l->first.que == que) {
//			l++;
//		}
//		qacl.erase(f,l);
		qacl.erase(QueueControlKey(que));
		storeSecurityLists();
//	}
	return GRANTED;
}

void SecurityControl::enumerateGroupUsers(const std::string& group, USERS& users)
{
	MutexLock lock(&scmtx);
	users.clear();
	USER_GROUPS::iterator	ug;
	for(ug = user_groups.begin(); ug != user_groups.end(); ug++)
		if (ug->second == group)
			users.insert(USERS::value_type(ug->first,_WS("")));
}

void SecurityControl::enumerateUserGroups(const std::string& user, GROUPS& group_names)
{
	MutexLock lock(&scmtx);
	group_names.clear();
	USER_GROUPS::iterator	ug;
	for(ug = user_groups.find(user); ug != user_groups.end() && ug->first == user; ug++)
		group_names.insert(ug->second);
}

void SecurityControl::enumerateGroups(GROUPS& group_names)
{
	MutexLock lock(&scmtx);
	group_names.clear();
	GROUPS::iterator	gp;
	for(gp = groups.begin(); gp != groups.end(); gp++)
		group_names.insert(*gp);
}

void SecurityControl::enumerateUsers(USERS& users)
{
	MutexLock lock(&scmtx);
	users.clear();
	USER_PASSWORDS::iterator up;
	for(up = user_passwords.begin(); up != user_passwords.end(); up++)
		users.insert(USERS::value_type(up->first, up->second.description));
}

void SecurityControl::enumerateQueuePermissions(const std::string& que, PERM_LIST& perms)
{
	MutexLock	lock(&scmtx);
	perms.clear();
	QUEUE_ACL::iterator i;
	for(i = qacl.lower_bound(QueueControlKey(que)); i != qacl.end() && i->first.que == que; i++) {
		perms.push_back(PERM_ENTRY(i->first.key,i->second));
	}
}


void SecurityControl::loadSecurityLists()
{
	loadGroups();
	loadUsers();
	loadUserGroups();
	loadActorACL();
	loadQueueACL();
	loadIdentities();
}

void SecurityControl::loadGroups()
{
	fstream	in((cfg_dir+group_file).c_str(), ios::in);
	std::string					line;

	groups.clear();
	getline(in,line);
	while (in.good()) {
		// NOTE: trim can be a problem if char is a wice character
		groups.insert(CStringHelper::trim(line));
		getline(in,line);
	}
	in.close();
}
void SecurityControl::loadUsers()
{
	fstream	in((cfg_dir+user_file).c_str(), ios::in);
	std::string					line;
	std::vector<std::string>	vals;

	user_passwords.clear();
	getline(in,line);

	while (in.good()) {
		CStringHelper::tokenize(line,_WS(":"),vals);
		// NOTE: The next few lines is a problem if char ever becomes wide character
		if (vals.size() == 1)
			user_passwords.insert(USER_PASSWORDS::value_type(CStringHelper::trim(vals[0]),DescPass(_WS(""),_WS(""))));
		else if (vals.size() == 2)
			user_passwords.insert(USER_PASSWORDS::value_type(CStringHelper::trim(vals[0]),DescPass(_WS(""),CStringHelper::trim(vals[1])) ) );
		else if (vals.size() == 3)
			user_passwords.insert(USER_PASSWORDS::value_type(CStringHelper::trim(vals[0]),DescPass(CStringHelper::trim(vals[2]),CStringHelper::trim(vals[1])) ) );
		getline(in,line);
	}
	in.close();
}

void SecurityControl::loadUserGroups()
{
	fstream	in((cfg_dir+user_group_file).c_str(), ios::in);
	std::string					line;
	std::vector<std::string>	vals;

	user_groups.clear();
	getline(in,line);
	while (in.good()) {
		CStringHelper::tokenize(line,_WS(":"),vals);
		// NOTE: The next line is a problem if char ever becomes wide character
		if (vals.size() == 2)
			user_groups.insert(USER_GROUPS::value_type(CStringHelper::trim(vals[0]),CStringHelper::trim(vals[1])));
		getline(in,line);
	}
	in.close();
}

void SecurityControl::loadActorACL()
{
	fstream	in((cfg_dir+actoracl_file).c_str(), ios::in);
	std::string					line;
	std::vector<std::string>	vals;

	actoracl.clear();
	getline(in,line);
	while (in.good()) {
		CStringHelper::tokenize(line,_WS(":"),vals);
		if (vals.size() == 5) {
			// NOTE: The next line is a problem if char ever becomes a wide character
			actoracl.insert(ACTOR_ACL::value_type(
						ControlKey(CStringHelper::trim(vals[0]), (ControlKey::CKType)atoi(CStringHelper::trim(vals[1]).c_str())),
						ActorControl(atoi(CStringHelper::trim(vals[2]).c_str()) != 0,
										atoi(CStringHelper::trim(vals[3]).c_str()) != 0,
										atoi(CStringHelper::trim(vals[4]).c_str()) != 0) ) );
		}
		getline(in,line);
	}
	in.close();
}

void SecurityControl::loadQueueACL()
{
	fstream	in((cfg_dir+qacl_file).c_str(), ios::in);
	std::string					line;
	std::vector<std::string>	vals;

	qacl.clear();
	getline(in,line);
	while (in.good()) {
		CStringHelper::tokenize(line,_WS(":"),vals);
		if (vals.size() == 7) {
			// NOTE: The next line is a problem if char ever becomes a wide character
			qacl.insert(QUEUE_ACL::value_type(
						QueueControlKey(CStringHelper::trim(vals[0]), ControlKey(CStringHelper::trim(vals[1]), (ControlKey::CKType)atoi(CStringHelper::trim(vals[2]).c_str()))),
						QAccessControl(atoi(CStringHelper::trim(vals[3]).c_str()) != 0,
										atoi(CStringHelper::trim(vals[4]).c_str()) != 0,
										atoi(CStringHelper::trim(vals[5]).c_str()) != 0,
										atoi(CStringHelper::trim(vals[6]).c_str()) != 0) ) );
		}
		getline(in,line);
	}
	in.close();
}

void SecurityControl::storeSecurityLists()
{
	storeUsers();
	storeGroups();
	storeUserGroups();
	storeActorACL();
	storeQueueACL();
	storeIdentities();
}

void SecurityControl::storeGroups()
{
	fstream	out((cfg_dir+group_file).c_str(), ios::out);
	GROUPS::iterator	g;
	for(g=groups.begin();g!=groups.end();g++)
		out << *g << endl;
	out.close();
}

void SecurityControl::storeUsers()
{
	fstream	out((cfg_dir+user_file).c_str(), ios::out);
	USER_PASSWORDS::iterator	up;
	for(up=user_passwords.begin();up!=user_passwords.end();up++)
		out << up->first << _WS(":") << up->second.password << _WS(":") << up->second.description << endl;
	out.close();
}
void SecurityControl::storeUserGroups()
{
	fstream	out((cfg_dir+user_group_file).c_str(), ios::out);
	USER_GROUPS::iterator	ug;
	for(ug=user_groups.begin();ug!=user_groups.end();ug++)
		out << ug->first << _WS(":") << ug->second << endl;
	out.close();
}
void SecurityControl::storeActorACL()
{
	fstream	out((cfg_dir+actoracl_file).c_str(), ios::out);
	ACTOR_ACL::iterator		i;
	for(i = actoracl.begin(); i != actoracl.end(); i++) {
		out << i->first.id << _WS(":") << i->first.type << _WS(":") << i->second.getModifyQueues() << _WS(":") << i->second.getModifyUsers() << _WS(":") << i->second.getModifyGroups() << endl;
	}
	out.close();
}
void SecurityControl::storeQueueACL()
{
	fstream	out((cfg_dir+qacl_file).c_str(), ios::out);
	QUEUE_ACL::iterator		i;
	for(i = qacl.begin(); i != qacl.end(); i++) {
		out << i->first.que << _WS(":") << i->first.key.id << _WS(":") << i->first.key.type << _WS(":") << i->second.getRead() << _WS(":") << i->second.getWrite() << _WS(":") << i->second.getDestroy() << _WS(":") << i->second.getChangeSecurity() << endl;
	}
	out.close();
}

void SecurityControl::loadIdentities()
{
	fstream	in((cfg_dir+identity_file).c_str(), ios::in);
	std::string					line;
	std::vector<std::string>		params;

	identities.clear();

	std::getline(in, line);
	while (in.good()) {
		// NOTE: trim may be a problem with wide characters
		line = CStringHelper::trim(line);
		if (line.length() && line[0] != '#') { // allow for comments
			CStringHelper::tokenizeEsc(line, ':', identityEscape, params);
			if (params.size() == 3) {
				if (params[0].length() && params[1].length() && params[2].length()) {
					identities[safmq::X509Identity(params[0],params[1])] = params[2];

				}
			}
		}
		std::getline(in, line);
	}

	in.close();
}


void SecurityControl::storeIdentities()
{
	IDENTITY_MAP::iterator	i;
	std::string			buf;

	fstream	out((cfg_dir+identity_file).c_str(), ios::out);
	for(i = identities.begin(); i != identities.end(); i++) {
		out << CStringHelper::escape(i->first.subjectDN, identityTokens, identityEscape, buf);
		out << _WS(":");
		out << CStringHelper::escape(i->first.issuerDN, identityTokens, identityEscape, buf);
		out << _WS(":");
		out << i->second; // the user name
		out << endl;
	}
	out.close();
}


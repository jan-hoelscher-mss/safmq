#pragma warning(disable: 4996)
#include <time.h>
#include "main.h"
#include "Command.h"
#include "QManager.h"

extern QManager*	theQueueManager;

#include "UserCmds.h"
#include "GroupCmds.h"
#include "CursorCmds.h"
#include "XactCmds.h"
#include "QueueCmds.h"
#include "MessageCmds.h"
#include "ServerCmds.h"

#include "comdefs.h"

namespace safmq {
	typedef std::map<SAFMQ_COM_CMD, Command::Allocator>	CommandMap;
}

using namespace safmq;

CommandMap::value_type _CommandsV1[] = {
	CommandMap::value_type(ENUM_QUEUES,				EnumQueuesCmd::allocate),
	CommandMap::value_type(SERVER_STATS,			ServerStatsCmd::allocate),
	CommandMap::value_type(CLIENT_INFO,				ClientInfoCmd::allocate),
	CommandMap::value_type(USER_CREATE,				UserCreateCmd::allocate),
	CommandMap::value_type(USER_DELETE,				UserDeleteCmd::allocate),
	CommandMap::value_type(ENUM_USERS,				EnumUsersCmd::allocate),
	CommandMap::value_type(USER_SET_PERMS,			UserSetPermsCmd::allocate),
	CommandMap::value_type(USER_GET_PERMS,			UserGetPermsCmd::allocate),
	CommandMap::value_type(USER_GET_GROUPS,			UserGetGroupsCmd::allocate),
	CommandMap::value_type(USER_SET_PASSWORD,		UserSetPasswordCmd::allocate),
	CommandMap::value_type(USER_ADD_IDENT,			UserAddIdentityCmd::allocate),
	CommandMap::value_type(USER_DEL_IDENT,			UserDelIdentityCmd::allocate),
	CommandMap::value_type(USER_ENUM_IDENT,			UserEnumIdentityCmd::allocate),
	CommandMap::value_type(GROUP_CREATE,			GroupCreateCmd::allocate),
	CommandMap::value_type(GROUP_DELETE,			GroupDeleteCmd::allocate),
	CommandMap::value_type(ENUM_GROUPS,				EnumGroupsCmd::allocate),
	CommandMap::value_type(GROUP_ADD_USER,			GroupAddUserCmd::allocate),
	CommandMap::value_type(GROUP_DELETE_USER,		GroupDeleteUserCmd::allocate),
	CommandMap::value_type(GROUP_GET_USERS,			GroupGetUsersCmd::allocate),
	CommandMap::value_type(GROUP_SET_PERMS,			GroupSetPermsCmd::allocate),
	CommandMap::value_type(GROUP_GET_PERMS,			GroupGetPermsCmd::allocate),
	CommandMap::value_type(QUEUE_CREATE,			QueueCreateCmd::allocate),
	CommandMap::value_type(QUEUE_DELETE,			QueueDeleteCmd::allocate),
	CommandMap::value_type(QUEUE_SET_USER_PERMS,	QueueSetUserPermsCmd::allocate),
	CommandMap::value_type(QUEUE_DEL_USER_PERMS,	QueueDeleteUserPermsCmd::allocate),
	CommandMap::value_type(QUEUE_SET_GROUP_PERMS,	QueueSetGroupPermsCmd::allocate),
	CommandMap::value_type(QUEUE_DEL_GROUP_PERMS,	QueueDelGroupPermsCmd::allocate),
	CommandMap::value_type(QUEUE_ENUM_PERMS,		QueueEnumPermsCmd::allocate),
	CommandMap::value_type(QUEUE_OPEN,				QueueOpenCmd::allocate),
	CommandMap::value_type(QUEUE_CLOSE,				QueueCloseCmd::allocate),
	CommandMap::value_type(QUEUE_STATS,				QueueStatsCmd::allocate),
	CommandMap::value_type(ENQUEUE,					EnqueueCmd::allocate),
	CommandMap::value_type(RETRIEVE,				RetrieveCmd::allocate),
	CommandMap::value_type(RETRIEVE_ACK,			RetrieveAckCmd::allocate),
	CommandMap::value_type(RETRIEVE_CURSOR_ACK,		RetrieveCursorAckCmd::allocate),
	CommandMap::value_type(RETRIEVE_ID,				RetrieveIDCmd::allocate),
	CommandMap::value_type(PEEK_ID,					PeekIDCmd::allocate),
	CommandMap::value_type(PEEK_FRONT,				PeekFrontCmd::allocate),
	CommandMap::value_type(ENQUEUE_FORWARD,			EnqueueForwardCmd::allocate),
	CommandMap::value_type(OPEN_CURSOR,				OpenCursorCmd::allocate),
	CommandMap::value_type(CLOSE_CURSOR,			CloseCursorCmd::allocate),
	CommandMap::value_type(ADVANCE_CURSOR,			AdvanceCursorCmd::allocate),
	CommandMap::value_type(SEEK_ID,					SeekIDCmd::allocate),
	CommandMap::value_type(TEST_CURSOR,				TestCursorCmd::allocate),
	CommandMap::value_type(PEEK_CURSOR,				PeekCursorCmd::allocate),
	CommandMap::value_type(RETRIEVE_CURSOR,			RetrieveCursorCmd::allocate),
	CommandMap::value_type(BEGIN_TRANSACTION,		BeginTransactionCmd::allocate),
	CommandMap::value_type(COMMIT_TRANSACTION,		CommitTransactionCmd::allocate),
	CommandMap::value_type(ROLLBACK_TRANSACTION,	RollbackTransactionCmd::allocate),
	CommandMap::value_type(END_TRANSACTION,			EndTransactionCmd::allocate), 
	CommandMap::value_type(QUEUE_CREATE_TEMP,		QueueCreateTemporaryCmd::allocate),
};

CommandMap commandsV1(_CommandsV1, _CommandsV1 + (sizeof(_CommandsV1)/sizeof(*_CommandsV1)));

class CommandsV1 : public CommandFactory {
public:
	CommandsV1() {}
public:
	virtual Command* build(int commandID, ServiceThread* service) {
		CommandMap::iterator i = commandsV1.find((SAFMQ_COM_CMD)commandID);
		if (i != commandsV1.end()) {
			return (i->second)(service);
		}
		return NULL;
	}
} v1Factory;

CommandFactory* CommandFactory::getFactory(int version)
{
	switch (version) {
		case 1:
			return &v1Factory;
	}
	// TODO: Return the current version factory;
	return &v1Factory;
}


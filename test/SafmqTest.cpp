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

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestResult.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TextTestProgressListener.h>
#include <memory.h>
#include <string>
#include <signal.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <time.h>
#include "../lib/MQConnection.h"
#include "../lib/MQFactory.h"
#include "test-aid.h"

using namespace safmq;
using namespace std;
using namespace test_aid;
using namespace tcpsocket;

#define LENGTH(s) (sizeof(s)/sizeof(*s))

#define Assert(msg, test) CPPUNIT_ASSERT_MESSAGE(string(msg).c_str(), (test))

#define TEST(test,function) new CppUnit::TestCaller<test>(#test"::"#function, &test::function)

#define CONNECT_CATCH(s) \
		catch (MQFactoryException& e) { Assert(string(s) + e.what(), false); } \
		catch (SocketException& e) { Assert(e.what(), false); } \
		catch (ErrorCode e) { Assert(string() + e, false); } \
		/*catch (...) { Assert("CAUGHT ...", false); } */

#define CONNECT_CATCH_EX(s) \
		catch (MQFactoryException& e) { Assert(string(s) + e.what(), false); } \
		catch (SocketException& e) { Assert(e.what(), false); } \
		/*catch (...) { Assert("CAUGHT ...", false); } */

typedef _<MQConnection> MQConnectionPtr;

typedef _<QueueMessage>	QueueMessagePtr;

typedef _<SSLContext>	SSLContextPtr;


// Test Data
const std::string TEST_QUEUE[] = { "test", "foo", "bar", "thequeue", "notthequeue" };
const std::string TEST_USERS[] = { "bob", "mary", "pat", "charlie", "jack", "kris" };
//const std::string TEST_PASSWD[] = TEST_USERS;
//const std::string TEST_DESC[] = TEST_USERS;
//
#define TEST_PASSWD TEST_USERS
#define TEST_DESC TEST_USERS
const std::string TEST_GROUPS[] = { "them", "us", "others", "natives", "aliens" };
const std::string SPECIAL_MESSAGE = "special message";

std::string host = "localhost";
std::string port = "";
std::string sslport = "";

class SafmqTest : public CppUnit::TestCase {
protected:	
	std::string user;
	std::string password;
	std::string address;
	std::string simpleAddress;

public:
	SafmqTest() throw (std::exception) {
		address = "safmq://admin:@" + ::host + ::port;
		simpleAddress = "safmq://" + ::host + ::port;
	}

	
	static CppUnit::TestSuite* GetSuite() {
		CppUnit::TestSuite* suite = new CppUnit::TestSuite();

		suite->addTest(TEST(SafmqTest,test_CreateQueue));
		suite->addTest(TEST(SafmqTest,test_DeleteQueue));
		suite->addTest(TEST(SafmqTest,test_CreateUser));
		suite->addTest(TEST(SafmqTest,test_SetPassword));
		suite->addTest(TEST(SafmqTest,test_UserPermissions));
		suite->addTest(TEST(SafmqTest,test_DeleteUser));
		suite->addTest(TEST(SafmqTest,test_CreateGroup));
		suite->addTest(TEST(SafmqTest,test_GroupPermissions));
		suite->addTest(TEST(SafmqTest,test_DeleteGroup));
		suite->addTest(TEST(SafmqTest,test_EnumerateQueues));
		suite->addTest(TEST(SafmqTest,test_EnumerateUsers));
		suite->addTest(TEST(SafmqTest,test_EnumerateGroups));
		suite->addTest(TEST(SafmqTest,test_GroupAddUser_GroupDeleteUser_GroupGetUsers));
		suite->addTest(TEST(SafmqTest,test_UserGetGroups));
		suite->addTest(TEST(SafmqTest,test_QueueSetUserPermission_QueueDeleteUserPermission_QueueEnumeratePermissions_QueueSetGroupPermission_QueueDeleteGroupPermission));
		suite->addTest(TEST(SafmqTest,test_OpenQueue_Enqueue_Retrieve_CloseQueue));
		suite->addTest(TEST(SafmqTest,test_PeekID_PeekFront_OpenCursor_PeekCursor_SeekID_RetrieveCursor_AdvanceCursor_TestCurosr_CloseCursor_RetrieveID));
		suite->addTest(TEST(SafmqTest,test_Transactions));
		suite->addTest(TEST(SafmqTest,test_EnqueueWithRelay));
		suite->addTest(TEST(SafmqTest,test_TemporaryQueue));
		return suite;
	}

	void tearDown() throw (std::exception) {
		TestCase::tearDown();
	}


	virtual MQConnection* connectToServer(const std::string& uri, const std::string& user, const std::string& password) {
		try {
			MQConnection* con = MQFactory::BuildConnection(uri, user, password);
			Assert("Successfully Connected", con != NULL);
			return con;
			Assert("CAUGHT ...", false);
		} CONNECT_CATCH("Connecting to Server:");

		return NULL;
	}

	void createEmptyQueue(MQConnection* con, std::string queueName) {
		ErrorCode ec;
		ec = con->DeleteQueue(queueName);
		CPPUNIT_ASSERT_MESSAGE("Failed to delete queue: ", (ec == EC_NOERROR || ec == EC_DOESNOTEXIST));

		ec = con->CreateQueue(queueName);
		Assert("Failed to create queue: " + queueName + " ec: " + ec, ec == EC_NOERROR);
	}

	void createNewUser(MQConnection* con, std::string userName, std::string description, std::string password) {
		ErrorCode ec = con->DeleteUser(userName);
		Assert("Failed to remove user prior to create: " + userName, (ec == EC_NOERROR || ec == EC_DOESNOTEXIST));
		ec = con->CreateUser(userName, password, description);
		Assert("Failed to create user: " + userName + " ec: " + ec, ec == EC_NOERROR);
	}

	void createNewGroup(MQConnection* con, std::string groupName) {
		ErrorCode ec = con->DeleteGroup(groupName);
		Assert("Failed to remove group prior to create: " + groupName, (ec == EC_NOERROR || ec == EC_DOESNOTEXIST));
		ec = con->CreateGroup(groupName);
		Assert("Failed to create group: " + groupName + " ec: " + ec, ec == EC_NOERROR);
	}

	void addAllUsers(MQConnection* con) {
		for (int x = 0; x < LENGTH(TEST_USERS); x++) {
			createNewUser(con, TEST_USERS[x], TEST_DESC[x], TEST_PASSWD[x]);
		}
	}

	void addAllGroups(MQConnection* con) {
		for (int x = 0; x < LENGTH(TEST_GROUPS); x++) {
			createNewGroup(con, TEST_GROUPS[x]);
		}
	}

	void addAllQueues(MQConnection* con) {
		for (int x = 0; x < LENGTH(TEST_QUEUE); x++) {
			createEmptyQueue(con, TEST_QUEUE[x]);
		}
	}

	void sendMessages(MQConnection* con, MQConnection::QueueHandle handle, int count, int idxStart) {
		QueueMessage msg;
		ErrorCode ec;
		
		for(int x=0;x<count;x++) {{
			QueueMessage msg;
			*msg.getBufferStream() << (x+idxStart) << endl;
			ec = con->Enqueue(handle, msg);
			Assert(string() + "Unable to send message: " + (x + idxStart) + " ec: " + ec, ec == EC_NOERROR);
		}}
	}

	void test_CreateQueue() {
		MQConnectionPtr con = connectToServer(address, "", "");
		createEmptyQueue(con, TEST_QUEUE[0]);
	}

	void test_DeleteQueue() {
		MQConnectionPtr con = connectToServer(address, "", "");
		createEmptyQueue(con, TEST_QUEUE[0]);

		ErrorCode ec = con->DeleteQueue(TEST_QUEUE[0]);
		Assert("Failed to delete queue: " + TEST_QUEUE[0], ec == EC_NOERROR);
	}
	void test_CreateUser() {
		MQConnectionPtr con = connectToServer(address, "", "");
		createNewUser(con, TEST_USERS[0], TEST_DESC[0], TEST_PASSWD[0]);
	}
	
	void test_SetPassword() {
		MQConnectionPtr con = connectToServer(address, "", "");
		createNewUser(con, TEST_USERS[0], TEST_DESC[0], TEST_PASSWD[0]);
		
		std::string newPwd = "new password";
		ErrorCode ec = con->SetPassword(TEST_USERS[0], newPwd);
		Assert("SetPassword", ec == EC_NOERROR);
		
		delete con.release();
		con = connectToServer(simpleAddress, TEST_USERS[0], newPwd);
	}
	

	void test_UserPermissions() {
		MQConnectionPtr con = connectToServer(address, "", "");
		createNewUser(con, TEST_USERS[0], TEST_DESC[0], TEST_PASSWD[0]);
		ErrorCode ec;
		for (int x = 7; x >= 0; x--) {
			ec = con->UserSetPermissions(TEST_USERS[0], (x & 0x04) != 0, (x & 0x2) != 0, (x & 0x1) != 0);
			Assert("Failed to set permissions for: " + TEST_USERS[0], ec == EC_NOERROR);

			bool mqueues, musers, mgroups;
			ec = con->UserGetPermissions(TEST_USERS[0], mqueues, musers, mgroups);
			Assert("Failed to retrive permissions for: " + TEST_USERS[0], ec == EC_NOERROR);
			Assert("Modify queues incorrectly set: " + x, mqueues == ((x & 0x04) != 0));
			Assert("Modify users incorrectly set: " + x, musers == ((x & 0x02) != 0));
			Assert("Modify groups incorrectly set: " + x, mgroups == ((x & 0x01) != 0));
		}
	}

	void test_DeleteUser() {
		MQConnectionPtr con = connectToServer(address, "", "");
		createNewUser(con, TEST_USERS[0], TEST_DESC[0], TEST_PASSWD[0]);

		ErrorCode ec = con->DeleteUser(TEST_USERS[0]);
		Assert("Failed to delete user: " + TEST_USERS[0], ec == EC_NOERROR);
	}

	void test_CreateGroup() {
		MQConnectionPtr con = connectToServer(address, "", "");
		createNewGroup(con, TEST_GROUPS[0]);
	}

	void test_GroupPermissions() {
		MQConnectionPtr con = connectToServer(address, "", "");
		createNewGroup(con, TEST_GROUPS[0]);
		ErrorCode ec;
		for (int x = 7; x >= 0; x--) {
			ec = con->GroupSetPermissions(TEST_GROUPS[0], (x & 0x04) != 0, (x & 0x2) != 0, (x & 0x1) != 0);
			Assert("Failed to set permissions for: " + TEST_GROUPS[0], ec == EC_NOERROR);
			bool mq,mu,mg;
			ec = con->GroupGetPermissions(TEST_GROUPS[0], mq,mu,mg);
			Assert("Failed to retrive permissions for: " + TEST_GROUPS[0], ec == EC_NOERROR);
			Assert("Modify queues incorrectly set: " + x, mq == ((x & 0x04) != 0));
			Assert("Modify users incorrectly set: " + x, mu == ((x & 0x02) != 0));
			Assert("Modify groups incorrectly set: " + x, mg == ((x & 0x01) != 0));
		}
	}

	void test_DeleteGroup() {
		MQConnectionPtr con = connectToServer(address, "", "");
		createNewGroup(con, TEST_GROUPS[0]);
		ErrorCode ec = con->DeleteGroup(TEST_GROUPS[0]);
		Assert("Failed to delete group: " + TEST_GROUPS[0], ec == EC_NOERROR);
	}

	void test_EnumerateQueues() {
		MQConnectionPtr con = connectToServer(address, "", "");

		addAllQueues(con);
		MQConnection::QDATA_VECTOR qnames;
		ErrorCode ec = con->EnumerateQueues(qnames);
		Assert("Couldn't enumerate queues", ec == EC_NOERROR);

		for (int x = 0; x < LENGTH(TEST_QUEUE); x++) {
			bool found = false;
			for (int y = 0; y < qnames.size(); y++) {
				MQConnection::QData& qd = qnames[y];
			if (qd.queuename == TEST_QUEUE[x])
					found = true;
			}
			Assert("Queue not enumerated: " + TEST_QUEUE[0], found);
		}
	}

	void test_EnumerateUsers() {
		MQConnectionPtr con = connectToServer(address, "", "");

		addAllUsers(con);
		MQConnection::USER_VECTOR unames;
		ErrorCode ec = con->EnumerateUsers(unames);
		Assert("Couldn't enumerate users", ec == EC_NOERROR);

		for (int x = 0; x < LENGTH(TEST_USERS); x++) {
			bool found = false;
			for (int y = 0; y < unames.size(); y++) {
				MQConnection::NameDesc& ud = unames[y];
				if (ud.name == (TEST_USERS[x]))
					found = true;
			}
			Assert("User not enumerated: " + TEST_USERS[0], found);
		}
	}

	void test_EnumerateGroups() {
		MQConnectionPtr con = connectToServer(address, "", "");

		addAllGroups(con);
		MQConnection::NAME_VECTOR gnames;
		ErrorCode ec = con->EnumerateGroups(gnames);
		Assert("Couldn't enumerate groups", ec == EC_NOERROR);

		for (int x = 0; x < LENGTH(TEST_GROUPS); x++) {
			bool found = false;
			for (int y = 0; y < gnames.size(); y++) {
				std::string& group = gnames[y];
				if (group == (TEST_GROUPS[x]))
					found = true;
			}
			Assert("Group not enumerated: " + TEST_GROUPS[0], found);
		}
	}

	void test_GroupAddUser_GroupDeleteUser_GroupGetUsers() {
		MQConnectionPtr con = connectToServer(address, "", "");

		addAllGroups(con);
		addAllUsers(con);

		ErrorCode ec;
		for (int x = 0; x < 3; x++) {
			ec = con->GroupAddUser(TEST_GROUPS[0], TEST_USERS[x]);
			Assert("Unable to add user: " + TEST_USERS[x] + " to group: " + TEST_GROUPS[x], ec == EC_NOERROR);
		}

		MQConnection::NAME_VECTOR users;
		ec = con->GroupGetUsers(TEST_GROUPS[0], users);
		Assert("Unable to get users in group: " + TEST_GROUPS[0], ec == EC_NOERROR);

		for (int x = 0; x < 3; x++) {
			Assert("User not found in group: " + TEST_USERS[x], contains(users, TEST_USERS[x]));
		}

		ec = con->GroupDeleteUser(TEST_GROUPS[0], TEST_USERS[1]);
		Assert("Unable to delete user: " + TEST_USERS[1] + " from group: " + TEST_GROUPS[0], ec == EC_NOERROR);

		ec = con->GroupGetUsers(TEST_GROUPS[0], users);
		Assert("Unable to get users in group: " + TEST_GROUPS[0], ec == EC_NOERROR);
		Assert("User found in group after delete: " + TEST_USERS[1], !contains(users, TEST_USERS[1]));
	}

	void test_UserGetGroups() {
		MQConnectionPtr con = connectToServer(address, "", "");

		addAllGroups(con);
		addAllUsers(con);

		ErrorCode ec;
		for (int x = 0; x < 3; x++) {
			ec = con->GroupAddUser(TEST_GROUPS[x], TEST_USERS[0]);
			Assert("Unable to add user: " + TEST_USERS[x] + " to group: " + TEST_GROUPS[x], ec == EC_NOERROR);
		}

		MQConnection::NAME_VECTOR groups;
		ec = con->UserGetGroups(TEST_USERS[0], groups);
		Assert("Unable to get groups for user: " + TEST_USERS[0], ec == EC_NOERROR);

		for (int x = 0; x < 3; x++) {
			Assert("Group not found: " + TEST_GROUPS[x] + " for user: " + TEST_USERS[0], contains(groups, TEST_GROUPS[x]));
		}

		ec = con->GroupDeleteUser(TEST_GROUPS[1], TEST_USERS[0]);
		Assert("Unable to delete user: " + TEST_USERS[1] + " from group: " + TEST_GROUPS[0], ec == EC_NOERROR);

		ec = con->UserGetGroups(TEST_GROUPS[0], groups);
		Assert("Unable to get groups for user: " + TEST_USERS[0], ec == EC_NOERROR);
		Assert("User found in group after delete: " + TEST_USERS[0], !contains(groups, TEST_GROUPS[1]));
	}
	
	
	void test_QueueSetUserPermission_QueueDeleteUserPermission_QueueEnumeratePermissions_QueueSetGroupPermission_QueueDeleteGroupPermission() {
		MQConnectionPtr con = connectToServer(address, "", "");
			
		addAllGroups(con);
		addAllUsers(con);
		addAllQueues(con);
		
		ErrorCode ec;
		for(int x=15;x>=0;x--) {
			ec = con->QueueSetUserPermission(TEST_QUEUE[0], TEST_USERS[0], (x&0x08)!=0,(x&0x04)!=0,(x&0x02)!=0,(x&0x01)!=0);
			Assert("Unable to set user permissions: " + x, ec == EC_NOERROR);

			ec = con->QueueSetGroupPermission(TEST_QUEUE[0], TEST_GROUPS[0], (x&0x08)!=0,(x&0x04)!=0,(x&0x02)!=0,(x&0x01)!=0);
			Assert("Unable to set group permissions: " + x, ec == EC_NOERROR);
				
			std::vector<QueuePermissions> perms;
			ec = con->QueueEnumeratePermissions(TEST_QUEUE[0], perms);
			Assert("Unable to get permissions: " + TEST_QUEUE[0], ec == EC_NOERROR);
			
			for(int y=0;y<perms.size();y++) {
				QueuePermissions& perm = perms[y];
				if(perm.isgroup && perm.entity == (TEST_GROUPS[0])) {
					Assert("Group read perm incorrect", ((x&0x08)!=0) == perm.read); 
					Assert("Group wirte perm incorrect", ((x&0x04)!=0) == perm.write); 
					Assert("Group destroy perm incorrect", ((x&0x02)!=0) == perm.destroy); 
					Assert("Group change security perm incorrect", ((x&0x01)!=0) == perm.change_security); 
				} else if (!perm.isgroup&& perm.entity == (TEST_USERS[0])) {
					Assert("user read perm incorrect", ((x&0x08)!=0) == perm.read); 
					Assert("user wirte perm incorrect", ((x&0x04)!=0) == perm.write); 
					Assert("user destroy perm incorrect", ((x&0x02)!=0) == perm.destroy); 
					Assert("user change security perm incorrect", ((x&0x01)!=0) == perm.change_security); 
				}
			}
		}
			
		ec = con->QueueDeleteGroupPermission(TEST_QUEUE[0], TEST_GROUPS[0]);
		Assert("unable to delete permission for group:" + TEST_GROUPS[0], ec == EC_NOERROR);

		ec = con->QueueDeleteGroupPermission(TEST_QUEUE[0], TEST_GROUPS[0]);
		Assert("able to delete permission for group:" + TEST_GROUPS[0], ec != EC_NOERROR);
		
		ec = con->QueueDeleteUserPermission(TEST_QUEUE[0], TEST_USERS[0]);
		Assert("unable to delete permission for user:" + TEST_USERS[0], ec == EC_NOERROR);

		ec = con->QueueDeleteUserPermission(TEST_QUEUE[0], TEST_USERS[0]);
		Assert("able to delete permission for user:" + TEST_USERS[0], ec != EC_NOERROR);
		
	}

	void test_OpenQueue_Enqueue_Retrieve_CloseQueue()  {
		MQConnectionPtr con = connectToServer(address, "", "");
			
		addAllGroups(con);
		addAllUsers(con);
		addAllQueues(con);

		MQConnection::QueueHandle handle;
		ErrorCode ec = con->OpenQueue(TEST_QUEUE[0], handle);
		Assert("Unable to open queue: " + TEST_QUEUE[0], ec == EC_NOERROR);
		
		sendMessages(con, handle, 5, 0);
		
		for(int x=0;x<5;x++) {{
			QueueMessage msg;

			ec = con->Retrieve(handle, true, 0, msg);
			Assert("Unable to retrieve message: " + x, ec == EC_NOERROR);
		
			std::string s;
			getline(*msg.getBufferStream(), s);
			
			Assert("Incorrect message retrieved: " + x, parseInt(s) == x);
		}}
		
		ec = con->CloseQueue(handle);
		Assert("Unable to close queue", ec == EC_NOERROR);
	}	
	
	void test_PeekID_PeekFront_OpenCursor_PeekCursor_SeekID_RetrieveCursor_AdvanceCursor_TestCurosr_CloseCursor_RetrieveID()  {
		MQConnectionPtr con = connectToServer(address, "", "");
			
		addAllGroups(con);
		addAllUsers(con);
		addAllQueues(con);

		MQConnection::QueueHandle handle;
		ErrorCode ec = con->OpenQueue(TEST_QUEUE[0], handle);
		Assert("Unable to open queue: " + TEST_QUEUE[0], ec == EC_NOERROR);
		
		sendMessages(con, handle, 5, 0);
		QueueMessagePtr msg = new QueueMessage();
		*msg->getBufferStream() << SPECIAL_MESSAGE;
		uuid uuid;
		uuid.guid.d1 = 123456789;
		uuid.guid.d2 = 4321;
		uuid.guid.d3 = 1234;
		msg->setReciptID(uuid);
		
		ec = con->Enqueue(handle, *msg);
		Assert("Unable to write special message", ec == EC_NOERROR);
		
		sendMessages(con, handle, 5, 5);
		
		msg = new QueueMessage();
		ec = con->PeekID(handle, true, uuid, 0, *msg);
		Assert("Unable to PeekID", ec == EC_NOERROR);
		std::string txt;
		getline(*msg->getBufferStream(), txt);

		Assert(string() + "Incorrect PeekID Message: " + msg->getReciptID(), (txt == (SPECIAL_MESSAGE)) && (uuid == (msg->getReciptID())));
			
		msg = new QueueMessage();
		ec = con->PeekFront(handle, true, 0, *msg);
		Assert("Unable to PeekFront", ec == EC_NOERROR);
		getline(*msg->getBufferStream(), txt);
		Assert("Incorrect PeekFront Message: body=" + txt, txt == ("0"));
			
		MQConnection::CursorHandle cur;

		ec = con->OpenCursor(handle, cur);
		Assert("Unable to OpenCursor", ec == EC_NOERROR);

		ec = con->SeekID(handle, uuid, 0, cur);
		Assert(string() + "Unable to SeekID ec:" + ec, ec == EC_NOERROR);
		msg = new QueueMessage();
		ec = con->PeekCursor(handle, true, cur, *msg);
		Assert("Unable to PeekCursor", ec == EC_NOERROR);
		getline(*msg->getBufferStream(), txt);
		Assert("Incorrect PeekCursor Message: " + msg->getReciptID(), txt == (SPECIAL_MESSAGE) && uuid == (msg->getReciptID()));

		ec = con->AdvanceCursor(handle, cur);
		Assert("Unable to AdvanceCursor", ec == EC_NOERROR);
		msg = new QueueMessage();
		ec = con->PeekCursor(handle, true, cur, *msg);
		Assert("Unable to PeekCursor (after advance)", ec == EC_NOERROR);
		getline(*msg->getBufferStream(), txt);
		Assert("Incorrect PeekCursor (after advance) Message: " + msg->getReciptID(), txt == ("5"));
	
		msg = new QueueMessage();
		ec = con->RetrieveCursor(handle, true, cur, *msg);
		Assert("Unable to RetrieveCursor", ec == EC_NOERROR);
		getline(*msg->getBufferStream(), txt);
		Assert("Incorrect PeekCursor (after RetrieveCursor) Message: " + msg->getReciptID(), txt == ("5"));
		
		ec = con->TestCursor(handle, cur);
		Assert(string() + "Unable to test Cursor, ec: " + ec, ec == EC_NOERROR);
		
		ec = con->CloseCursor(handle, cur);
		Assert(string() + "Unable to CloseCursor, ec: " + ec, ec == EC_NOERROR);
		
		
		msg = new QueueMessage();
		ec = con->RetrieveID(handle, true, uuid, 0, *msg);
		Assert("Unable to RetrieveID", ec == EC_NOERROR);
		getline(*msg->getBufferStream(), txt);
		Assert(string() + "Incorrect RetrieveID Message: " + msg->getReciptID(), txt == (SPECIAL_MESSAGE) && uuid == (msg->getReciptID()));
		
		int bodies[] = {0,1,2,3,4,6,7,8,9}; // 5 was retrieved above.
		for(int x=0;x<LENGTH(bodies);x++) {
			msg = new QueueMessage();
			ec = con->Retrieve(handle, true, 0, *msg);
			Assert("Unable to retrieve message: " + x, ec == EC_NOERROR);
			getline(*msg->getBufferStream(), txt);
			Assert("Incorrect message: " + txt, parseInt(txt) == bodies[x]);
		}
		
		
		ec = con->CloseQueue(handle);
		Assert("Unable to close queue", ec == EC_NOERROR);
			
	}	
	
	void test_Transactions()  {
		MQConnectionPtr con = connectToServer(address, "", "");
			
		addAllGroups(con);
		addAllUsers(con);
		addAllQueues(con);

		MQConnection::QueueHandle handle;
		ErrorCode ec = con->OpenQueue(TEST_QUEUE[0], handle);
		Assert("Unable to open queue: " + TEST_QUEUE[0], ec == EC_NOERROR);
			
		sendMessages(con, handle, 10, 0);

		ec = con->BeginTransaction();
		Assert(string() + "Unable to BeginTransaction:" + ec, ec == EC_NOERROR);
		MQConnection::CursorHandle cur;
			
		ec = con->OpenCursor(handle, cur);
		Assert(string() + "Unable to OpenCursor:" + ec, ec == EC_NOERROR);
			
		ec = con->AdvanceCursor(handle, cur);
		Assert(string() + "Unable to AdvanceCursor:" + ec, ec == EC_NOERROR);
		ec = con->AdvanceCursor(handle, cur);
		Assert(string() + "Unable to AdvanceCursor:" + ec, ec == EC_NOERROR);

		QueueMessagePtr msg = new QueueMessage();
		ec = con->RetrieveCursor(handle, true, cur, *msg);
		Assert(string() + "Unable to RetrieveCursor:" + ec, ec == EC_NOERROR);
		std::string txt;
		getline(*msg->getBufferStream(), txt);
		Assert("Incorrect Message: " + txt + " should be 2", txt == ("2"));
			
		msg = new QueueMessage();
		ec = con->RetrieveCursor(handle, true, cur, *msg);
		Assert(string() + "Unable to RetrieveCursor:" + ec, ec == EC_NOERROR);
		getline(*msg->getBufferStream(), txt);
		Assert("Incorrect Message: " + txt + " should be 3", txt == ("3"));
	
		msg = new QueueMessage();
		ec = con->Retrieve(handle, true, 0, *msg);
		Assert(string() + "Unable to RetrieveCursor:" + ec, ec == EC_NOERROR);
		getline(*msg->getBufferStream(), txt);
		Assert("Incorrect Message: " + txt + " should be 0", txt == ("0"));
		
		int ids[] = {1,4,5,6,7,8,9};
		for(int x=0;x<LENGTH(ids);x++) {
			msg = new QueueMessage();
			ec = con->Retrieve(handle, true, 0, *msg);
			Assert(string() + "Unable to RetrieveCursor:" + ec, ec == EC_NOERROR);
			getline(*msg->getBufferStream(), txt);
			Assert("Incorrect Message: " + txt + " should be " + ids[x], parseInt(txt) == ids[x]);
		}
		
		ec = con->RollbackTransaction();
		Assert(string() + "Unable to RollbackTransaction:" + ec, ec == EC_NOERROR);
		
		for(int x=0;x<8;x++) {
			msg = new QueueMessage();
			ec = con->Retrieve(handle, true, 0, *msg);
			Assert(string() + "Unable to RetrieveCursor:" + ec, ec == EC_NOERROR);
			getline(*msg->getBufferStream(), txt);
			Assert("Incorrect Message: " + txt + " should be " + x, parseInt(txt) == x);
		}
		
		ec = con->CommitTransaction();
		Assert(string() + "Unable to CommitTransaction:" + ec, ec == EC_NOERROR);
		
		for(int x=0;x<2;x++) {
			msg = new QueueMessage();
			ec = con->Retrieve(handle, true, 0, *msg);
			Assert(string() + "Unable to RetrieveCursor:" + ec, ec == EC_NOERROR);
			getline(*msg->getBufferStream(), txt);
			Assert("Incorrect Message: " + txt+ " should be " + (x+8), parseInt(txt) == x+8);
		}
		
		ec = con->RollbackTransaction();
		Assert(string() + "Unable to RollbackTransaction:" + ec, ec == EC_NOERROR);
		
		ec = con->EndTransaction();
		Assert(string() + "Unable to RollbackTransaction:" + ec, ec == EC_NOERROR);
		
		for(int x=0;x<2;x++) {
			msg = new QueueMessage();
			ec = con->Retrieve(handle, true, 0, *msg);
			Assert(string() + "Unable to RetrieveCursor:" + ec, ec == EC_NOERROR);
			getline(*msg->getBufferStream(), txt);
			Assert("Incorrect Message: " + txt+ " should be " +(x+8), parseInt(txt) == x+8);
		}

		msg = new QueueMessage();
		ec = con->Retrieve(handle, true, 0, *msg);
		Assert(string() + "Able to RetrieveCursor:" + ec, ec == EC_NOMOREMESSAGES);
		
		ec = con->CloseQueue(handle);
		Assert(string() + "Unble to CloseQueue:" + ec, ec == EC_NOERROR);
	}	

	void test_EnqueueWithRelay()  {
		MQConnectionPtr con = connectToServer(address, "", "");
		QueueMessagePtr msg = new QueueMessage();
		*msg->getBufferStream() << SPECIAL_MESSAGE;

		addAllQueues(con);
		ErrorCode ec = con->EnqueueWithRelay(address + "/" + TEST_QUEUE[1], *msg);
		Assert(string() + "Unble to EnqueueWithRelay:" + ec, ec == EC_NOERROR);
	
		MQConnection::QueueHandle handle;
		ec = con->OpenQueue(TEST_QUEUE[1], handle);
		Assert(string() + "Unble to OpenQueue:" + ec, ec == EC_NOERROR);
		
		msg = new QueueMessage();
		ec = con->Retrieve(handle, true, 2, *msg); // wait up to 2 seconds for the forward
		Assert(string() + "Unble to Retrieve:" + ec, ec == EC_NOERROR);
		std::string txt;
		getline(*msg->getBufferStream(), txt);
		Assert("Incorrect Message: " + txt, txt == (SPECIAL_MESSAGE));
	
		ec = con->CloseQueue(handle);
		Assert(string() + "Unble to CloseQueue:" + ec, ec == EC_NOERROR);
	}	

	void test_TemporaryQueue() {
		MQConnectionPtr con = connectToServer(address, "", "");
		std::string queueName;
		MQConnection::QueueHandle qh;

		// Try and create a temporary queue
		ErrorCode err = con->CreateTempQueue(queueName, qh);
		//Console.WriteLine("queueName:" + queueName[0]);
		Assert(string() + "Creating Queue:" + err, EC_NOERROR == err);

		// Get permissions for that queue
		std::vector<QueuePermissions> perms;
		err = con->QueueEnumeratePermissions(queueName, perms);
		//for (int x = 0; x < perms.Count; ++x) {
		//	QueuePermissions perm = perms[x];
			//Console.WriteLine(perm.EntityName + ":" + perm.Read + ":" + perm.Write + ":" + perm.Destroy + ":" + perm.ChangeSecurity);
		//}
		Assert(string() + "Unable to load permissions:" + err, perms.size() > 0);

		addAllUsers(con);

		// Try and write to that queue with another user.
		MQConnectionPtr con2 = connectToServer(simpleAddress, TEST_USERS[0], TEST_USERS[0]);
		MQConnection::QueueHandle qh2;
		ErrorCode rc = con2->OpenQueue(queueName, qh2);
		Assert(string() + "Open Temp Queue rc:" + rc, rc == EC_NOERROR);
		QueueMessage msg;
		msg.setLabel("Hello World");
		Assert("Enqueue to temp", EC_NOERROR == con2->Enqueue(qh2, msg));
		con2->CloseQueue(qh2);
	}

};

#ifdef SAFMQ_SSL
#include <openssl/asn1.h>

class SSLSafmqTest : public SafmqTest {
protected:
	std::string user;
	std::string password;
	std::string description;
	std::string subject;
	std::string issuer;


public:
	SSLSafmqTest() throw (std::exception) {
		address = "safmqs://admin:@" + ::host + ::sslport;
		simpleAddress = "safmqs://" + ::host + ::sslport;

		user="npuser";
		password="54321";
		description="passwordless login test user";
		subject = "OU=Testing, O=Safmq Java Test Client";
		issuer = "C=US, ST=Nebraska, L=Omaha, O=SAFMQ, OU=Sample X509 Certificate, CN=Sample SAFMQ X509 Certificate";
	}

	static CppUnit::TestSuite* GetSuite() {
		CppUnit::TestSuite* suite = new CppUnit::TestSuite();
		
		suite->addTest(TEST(SSLSafmqTest,test_CreateQueue));
		suite->addTest(TEST(SSLSafmqTest,test_DeleteQueue));
		suite->addTest(TEST(SSLSafmqTest,test_CreateUser));
		suite->addTest(TEST(SSLSafmqTest,test_SetPassword));
		suite->addTest(TEST(SSLSafmqTest,test_UserPermissions));
		suite->addTest(TEST(SSLSafmqTest,test_DeleteUser));
		suite->addTest(TEST(SSLSafmqTest,test_CreateGroup));
		suite->addTest(TEST(SSLSafmqTest,test_GroupPermissions));
		suite->addTest(TEST(SSLSafmqTest,test_DeleteGroup));
		suite->addTest(TEST(SSLSafmqTest,test_EnumerateQueues));
		suite->addTest(TEST(SSLSafmqTest,test_EnumerateUsers));
		suite->addTest(TEST(SSLSafmqTest,test_EnumerateGroups));
		suite->addTest(TEST(SSLSafmqTest,test_GroupAddUser_GroupDeleteUser_GroupGetUsers));
		suite->addTest(TEST(SSLSafmqTest,test_UserGetGroups));
		suite->addTest(TEST(SSLSafmqTest,test_QueueSetUserPermission_QueueDeleteUserPermission_QueueEnumeratePermissions_QueueSetGroupPermission_QueueDeleteGroupPermission));
		suite->addTest(TEST(SSLSafmqTest,test_OpenQueue_Enqueue_Retrieve_CloseQueue));
		suite->addTest(TEST(SSLSafmqTest,test_PeekID_PeekFront_OpenCursor_PeekCursor_SeekID_RetrieveCursor_AdvanceCursor_TestCurosr_CloseCursor_RetrieveID));
		suite->addTest(TEST(SSLSafmqTest,test_Transactions));
		suite->addTest(TEST(SSLSafmqTest,test_EnqueueWithRelay));
		suite->addTest(TEST(SSLSafmqTest,test_PasswordlessSignon));
		suite->addTest(TEST(SSLSafmqTest,test_EmptyLogin));
		suite->addTest(TEST(SSLSafmqTest,test_EmptyPassword));
		suite->addTest(TEST(SSLSafmqTest,test_BadPassword));
		suite->addTest(TEST(SSLSafmqTest,test_TemporaryQueue));
		suite->addTest(TEST(SSLSafmqTest,test_Certificate));
		return suite;
	}

	virtual MQConnection* connectToServer(std::string uri, std::string user, std::string password) {
		try {
			//SSLContext 		ctx;
			//PrivateKey		pk;
			//X509Certificate	cert;

			//pk.loadPEMFile("../java/cert/safmq_java_test.pem");
			//ctx.setPrivateKey(&pk);
			//cert.loadPEMFile("../java/cert/safmq_java_test.pem");
			//ctx.setX509Certificate(&cert);

			//MQBuilder.setSSLContext(ctx);
			MQConnection* con = MQFactory::BuildConnection(uri, user, password);
			
			Assert("Successfully Connected", con != NULL);
			return con;

		} CONNECT_CATCH("Connecting to Server (safmqs):");
		return NULL;
	}

	/**
	 * Creates a user and associates an x.509 certificate with that user. Then
	 * attempts to login without a password for that user.
	 */
	void test_PasswordlessSignon()  {
		ErrorCode ec;

		MQConnectionPtr con = connectToServer(address,"","");
		
		ec = con->DeleteUser(user);
		Assert("Delete User", ec == safmq::EC_NOERROR || ec == safmq::EC_DOESNOTEXIST);
		
		ec = con->CreateUser(user, password, description);
		Assert("Create User", ec == safmq::EC_NOERROR);
		
		ec = con->AddUserIdentity(subject, issuer, user);
		Assert("AddUserIdentity", ec == safmq::EC_NOERROR);
		
		vector<X509Identity> ids;
		ec = con->EnumerateUserIdentities(user, ids);
		Assert(string() + "EnumerateUserIdentities ec:" + ec + " Val:" + (int)ec, ec == safmq::EC_NOERROR);
		Assert("X.509 ID Count", ids.size() > 0);
		bool idFound = false;
		for(int x = 0; x < ids.size(); x++) {
			X509Identity& id = ids[x];
			if (id.issuerDN == issuer && id.subjectDN == subject)
				idFound = true;
		}
		Assert("Identity found in returned X.509 identities", idFound);
	}
	
	SSLContext* setupSSL()  {
		// Setup the SSL Context using the JKS file created via gen_test_cert in the java/cert directory
		SSLContext 		*ctx = new SSLContext;
		PrivateKey		pk;
		X509Certificate	cert;

		pk.loadPEMFile("../java/cert/safmq_java_test.pem");
		ctx->setPrivateKey(&pk);
		cert.loadPEMFile("../java/cert/safmq_java_test.pem");
		ctx->setX509Certificate(&cert);
        
		MQConnectionPtr con = connectToServer(address,"","");
		ErrorCode ec = con->DeleteUser(user);
		Assert("Delete User", ec == safmq::EC_NOERROR || ec == safmq::EC_DOESNOTEXIST);
		
		ec = con->CreateUser(user, password, description);
		Assert("Create User", ec == safmq::EC_NOERROR);
		
		ec = con->AddUserIdentity(subject, issuer, user);
		Assert("AddUserIdentity", ec == safmq::EC_NOERROR);

		return ctx;
	}
	
	void test_EmptyLogin()  {
		SSLContextPtr ctx = setupSSL();

		try {
			MQConnectionPtr con = MQFactory::BuildConnection(simpleAddress, "", "", ctx);
		} CONNECT_CATCH("SSL Empty userid: ");
	}

	void test_EmptyPassword()  {
		SSLContextPtr ctx = setupSSL();

		try {
			MQConnectionPtr con = MQFactory::BuildConnection(simpleAddress,user,"",ctx);
		} CONNECT_CATCH("SSL Empty Password: ");
		
	}

	void test_BadPassword()  {
		SSLContextPtr ctx = setupSSL();
		MQConnectionPtr con;

		try {
			MQConnectionPtr con = MQFactory::BuildConnection(simpleAddress,user,"12345",ctx);
			Assert("Connected with a bad password", false);
		} catch (ErrorCode ec) {
			Assert(string() + "Reconnect with x.509 ident, name wrong password" + ec, ec == safmq::EC_NOTAUTHORIZED);
		} CONNECT_CATCH_EX("Bad Password Connect:");
		
	}

	class TESTX509 : public X509Certificate {
	public:
		time_t	test_make_time_t(const string& dateTime, int type) {
			return make_time_t(dateTime, type);
		}
	};

	void test_Certificate() {
		TESTX509	cert;

		cert.loadPEMFile("../java/cert/safmq_java_test.pem");

		time_t	t = cert.getNotBefore_time_t();
		char	date[32], date2[32];
		strftime(date, sizeof(date), "%b %e %H:%M:00 %Y GMT", gmtime(&t));
		strftime(date2, sizeof(date2), "%b %e %H:%M:%S %Y GMT", gmtime(&t));
		Assert("Not Before time_t incorrect:\n"
			    "cert.getNotBefore(): " + cert.getNotBefore()
			+ "\n               date: " + date
			+ "\n              date2: " + date2,
			cert.getNotBefore() == date || cert.getNotBefore() == date2);

		t = cert.getNotAfter_time_t();
		strftime(date, sizeof(date), "%b %e %H:%M:00 %Y GMT", gmtime(&t));
		strftime(date2, sizeof(date2), "%b %e %H:%M:%S %Y GMT", gmtime(&t));
		Assert("Not After time_t incorrect:\n"
				"cert.getNotAfter(): " + cert.getNotAfter() + "\n"
				"              date: " + date + "\n"
				"             date2: " + date2,
				cert.getNotAfter() == date || cert.getNotAfter() == date2);

		time_t	now = time(NULL);

		strftime(date, sizeof(date), "%y%m%d%H%M%SZ", gmtime(&now));
		Assert("ASN1_UTCTIME format", cert.test_make_time_t(date, V_ASN1_UTCTIME) == now);

		strftime(date, sizeof(date), "%y%m%d%H%M%S%z", localtime(&now));
		string dateS = date;
		dateS.insert(dateS.length()-2, "'");
		dateS.append("'");
		Assert("ASN1_UTCTIME format", cert.test_make_time_t(dateS, V_ASN1_UTCTIME) == now);

		strftime(date, sizeof(date), "%Y%m%d%H%M%SZ", gmtime(&now));
		Assert("ASN1_UTCTIME format", cert.test_make_time_t(date, V_ASN1_GENERALIZEDTIME) == now);

		strftime(date, sizeof(date), "%Y%m%d%H%M%S%z", localtime(&now));
		dateS = date;
		dateS.insert(dateS.length()-2, "'");
		dateS.append("'");
		Assert("ASN1_UTCTIME format", cert.test_make_time_t(dateS, V_ASN1_GENERALIZEDTIME) == now);
	}
};
#endif


void sigh(int i)
{
	char buffer[80];
	sprintf(buffer, "caught signal: %d", i);
	Assert(buffer, false);
}

void sigint(int i)
{
	Assert("Interupted with CTRL-C", false);
	signal(SIGINT, sigint);
}


int main(int argc, char* argv[])
{
	signal(SIGBUS, sigh);
	signal(SIGSEGV, sigh);
	signal(SIGINT, sigint);

	setvbuf(stdout, NULL, _IONBF, 0);

	if (argc > 1) 
		host = argv[1];
	if (argc > 2) 
		port = string(":") + argv[2];
	if (argc > 3) 
		sslport = string(":") + argv[3];

	CppUnit::TextUi::TestRunner runner;
	runner.eventManager().addListener(new CppUnit::BriefTestProgressListener);

	runner.addTest(SafmqTest::GetSuite());
#ifdef SAFMQ_SSL
	runner.addTest(SSLSafmqTest::GetSuite());
#endif
	runner.run();

	return 0;
}



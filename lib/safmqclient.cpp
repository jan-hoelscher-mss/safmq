// safmqclient.cpp : Defines the entry point for the console application.
//
#pragma warning(disable: 4786)
#include <iostream>
#include <fstream>
#include <iomanip>
#include "MessageQueue.h"
#include "MQConnection.h"
#include "MQFactory.h"
#include "uuidgen/uuidgen.h"
#if defined(SAFMQ_SSL)
	#include "tcpsocket/sslsocket.h"
#endif
//#include "xml/xml.h"
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;
using namespace safmq;
//using namespace XML;

const char* EC_Decode(ErrorCode ec) {
#define ECOUT(v) case v: return #v;
	switch(ec) {
		ECOUT(EC_NOERROR)
		ECOUT(EC_TIMEDOUT)
		ECOUT(EC_NOTAUTHORIZED)
		ECOUT(EC_ALREADYCLOSED)
		ECOUT(EC_DOESNOTEXIST)
		ECOUT(EC_NOTOPEN)
		ECOUT(EC_NETWORKERROR)
		ECOUT(EC_TTLEXPIRED)
		ECOUT(EC_CURSORINVALIDATED)
		ECOUT(EC_DUPLICATEMSGID)
		ECOUT(EC_SERVERUNAVAIL)
		ECOUT(EC_NOMOREMESSAGES)
		ECOUT(EC_FILESYSTEMERROR)
		ECOUT(EC_CANNOTCHANGEGROUP)
		ECOUT(EC_CANNOTCHANGEUSER)
		ECOUT(EC_ALREADYDEFINED)
		ECOUT(EC_NOTIMPLEMENTED)
		ECOUT(EC_QUEUEOPEN)
		ECOUT(EC_NOTLOGGEDIN)
		ECOUT(EC_INVALIDNAME)
		ECOUT(EC_FORWARDNOTALLOWED)
		ECOUT(EC_WRONGMESSAGETYPE)
		ECOUT(EC_UNSUPPORTED_PROTOCOL)
		ECOUT(EC_ERROR)
		default: return "UNKNOWN";
	}
#undef ECOUT
}

//#define SERVER	"localhost"

string trim(const string& src)
{
	int	b = src.find_first_not_of("\r\n\t ");
	int e = src.find_last_not_of("\r\n\t ");

	if (b < e || (e == b && b != string::npos)) {
		return src.substr(b,e-b+1);
	}
	return "";
}

const char* queuename="foobar";
const char* queue2name="foobar2";
const char* username="matt";
const char* groupname="elgroupo";

using namespace tcpsocket;

int main (int argc, char* argv[])
{
	try {

#if defined(SAFMQ_SSL)
		X509Certificate	cert;

		cert.loadPEMFile("f:\\YO!\\stuff\\keys\\cefm-certs\\atlas.pem");

		cout << "Issuer: " << cert.getIssuerName() << endl;
		cout << "Subject: " << cert.getSubjectName() << endl;
		cout << "Not After: " << cert.getNotAfter() << endl;
		cout << "Not Before: " << cert.getNotBefore() << endl;
		cout << "Serial Number: " << cert.getSerialNumber() << endl;

		struct tm now;
		time_t t = cert.getNotBefore_time_t();
		now = *localtime(&t);
		cout << "Not Before: " << asctime(&now);

		t = cert.getNotAfter_time_t();
		now = *localtime(&t);
		cout << "Not After: " << asctime(&now) << flush;
#endif

/*
		MQConnection*				con = MQFactory::BuildConnection("safmq://localhost:9000","admin","");
		ErrorCode					ec;
		MQConnection::QueueHandle	qhandle,q2handle;
		MQConnection::CursorHandle	cursorID;
		uuid						uid;
		QueueMessage				msg, resp;

		memset(&uid,0,sizeof(uid));
		uid.guid.d1 = 42;
		uid.guid.d2 = 63;
		uid.guid.d3 = 74;

		msg.setLabel("Test1");
		msg.setBodyType(BT_TEXT);
		*msg.getBufferStream() << "Hello World!\n";
		//msg.setReciptID(uid);


		con->DeleteQueue(queuename);
		ec = con->CreateQueue(queuename);
		ec = con->OpenQueue(queuename,qhandle);

//		con->DeleteQueue(queue2name);
//		ec = con->CreateQueue(queue2name);
//		ec = con->OpenQueue(queue2name,q2handle);

		ec = con->Enqueue(qhandle,msg);
//		ec = con->Enqueue(q2handle,msg);
		msg.setLabel("Test2");
		ec = con->Enqueue(qhandle,msg);
//		ec = con->Enqueue(q2handle,msg);
		msg.setLabel("Test3");
		ec = con->Enqueue(qhandle,msg);
//		ec = con->Enqueue(q2handle,msg);
		msg.setLabel("Test4");
		ec = con->Enqueue(qhandle,msg);
//		ec = con->Enqueue(q2handle,msg);
		msg.setLabel("Test5");
		ec = con->Enqueue(qhandle,msg);
//		ec = con->Enqueue(q2handle,msg);
		msg.setLabel("Test6");
		ec = con->Enqueue(qhandle,msg);
//		ec = con->Enqueue(q2handle,msg);
		msg.setLabel("Test7");
		ec = con->Enqueue(qhandle,msg);
//		ec = con->Enqueue(q2handle,msg);
		msg.setLabel("Test8");
		msg.setReciptID(uid);
		ec = con->Enqueue(qhandle,msg);
//		ec = con->Enqueue(q2handle,msg);

		ec = con->BeginTransaction();
		cout << "con->BeginTransaction(): " << EC_Decode(ec) << endl;

		ec = con->RetrieveID(qhandle, true, uid, -1, resp);
		cout << "RetrieveID(): " << EC_Decode(ec) << " label: " << resp.getLabel() << " should be: Test8" << endl;

		con->OpenCursor(qhandle,cursorID);
		con->AdvanceCursor(qhandle,cursorID);
		ec = con->RetrieveCursor(qhandle, true, cursorID, resp);
		cout << "RetrieveCursor(): " << EC_Decode(ec) << " label: " << resp.getLabel() << " should be: Test2" << endl;
		ec = con->RetrieveCursor(qhandle, true, cursorID, resp);
		cout << "RetrieveCursor(): " << EC_Decode(ec) << " label: " << resp.getLabel() << " should be: Test3" << endl;


		while ( (ec=con->Retrieve(qhandle, true, 0, resp)) == EC_NOERROR) {
			cout << "Retrieve(): " << EC_Decode(ec) << " label: " << resp.getLabel() << endl;
		}

//		while ( (ec=con->Retrieve(q2handle, true, 0, resp)) == EC_NOERROR) {
//			cout << "q2.Retrieve(): " << EC_Decode(ec) << " label: " << resp.getLabel() << endl;
//		}

		ec = con->RollbackTransaction();
		cout << "con->RollbackTransaction(): " << EC_Decode(ec) << endl;
		ec = con->CommitTransaction();
		cout << "con->CommitTransaction(): " << EC_Decode(ec) << endl;

		cout << "------------------------------------------" << endl;
		while ( (ec=con->Retrieve(qhandle, true, 0, resp)) == EC_NOERROR) {
			cout << "Retrieve(): " << EC_Decode(ec) << " label: " << resp.getLabel() << endl;
		}

//		while ( (ec=con->Retrieve(q2handle, true, 0, resp)) == EC_NOERROR) {
//			cout << "q2.Retrieve(): " << EC_Decode(ec) << " label: " << resp.getLabel() << endl;
//		}

		ec = con->EndTransaction();
		cout << "con->EndTransaction(): " << EC_Decode(ec) << endl;

		con->CloseQueue(qhandle);
		con->DeleteQueue(queuename);
		delete con;
*/

/**
PRIMARY TEST:
		MQConnection*				con = MQFactory::BuildConnection("safmq://localhost:9000","admin","");
		ErrorCode					ec;
		MQConnection::QueueHandle	qhandle;
		MQConnection::CursorHandle	cursorID;
		uuid						uid;
		QueueMessage				msg, resp;

		memset(&uid,0,sizeof(uid));
		uid.guid.d1 = 42;
		uid.guid.d2 = 63;
		uid.guid.d3 = 74;

		msg.setLabel("Test");
		msg.setBodyType(BT_TEXT);
		*msg.getBufferStream() << "Hello World!\n";
		msg.setReciptID(uid);

		cout << "SAFMQ Major Version: " << con->GetServerMajorProtocolVersion() << endl;
		cout << "SAFMQ Minor Version: " << con->GetServerMinorProtocolVersion() << endl;

		ec = con->CreateQueue(queuename);
		cout << "con->CreateQueue("<<queuename<<"): " << EC_Decode(ec) << endl;

		ec = con->OpenQueue(queuename, qhandle);
		cout << "con->OpenQueue("<<queuename<<"," << qhandle <<"): " << EC_Decode(ec) << endl;
		
		ec = con->Enqueue(qhandle, msg);
		cout << "con->Enqueue(" << qhandle << ", " << (void*)&msg << "): " << EC_Decode(ec) << endl;

		ec = con->Enqueue(qhandle, msg);
		cout << "con->Enqueue(" << qhandle << ", " << (void*)&msg << "): " << EC_Decode(ec) << endl;

		ec = con->Enqueue(qhandle, msg);
		cout << "con->Enqueue(" << qhandle << ", " << (void*)&msg << "): " << EC_Decode(ec) << endl;

		ec = con->EnqueueWithRelay("safmq://admin:@localhost/foobar", msg);
		cout << "con->EnqueueWithRelay(safmq://admin:@localhost/foobar, " << (void*)&msg << "): " << EC_Decode(ec) << endl;

		ec = con->OpenCursor(qhandle, cursorID);
		cout << "con->OpenCursor("<<qhandle<<", "<<cursorID<<"): " << EC_Decode(ec) << endl;
		ec = con->SeekID(qhandle, uid, 0, cursorID);
		cout << "con->SeekID(" << qhandle << ", " << uid << ", " << 0 << "," <<cursorID<<"): " << EC_Decode(ec) << endl;

		ec = con->PeekFront(qhandle, true, 0, resp);
		cout << "con->PeekFront(qhandle, true, 0, resp): " << EC_Decode(ec) << endl;
		ec = con->PeekID(qhandle, true, uid, 0, resp);
		cout << "con->PeekFront(qhandle, true, 0, resp): " << EC_Decode(ec) << endl;

		ec = con->PeekCursor(qhandle, true, cursorID, resp);
		cout << "con->PeekCursor(qhandle, true, cursorID, resp): " << EC_Decode(ec) << endl;
		ec = con->TestCursor(qhandle, cursorID);
		cout << "con->TestCursor("<<qhandle<<", "<<cursorID<<"): " << EC_Decode(ec) << endl;
		ec = con->AdvanceCursor(qhandle, cursorID);
		cout << "con->AdvanceCursor("<<qhandle<<", "<<cursorID<<"): " << EC_Decode(ec) << endl;

		ec = con->AdvanceCursor(qhandle, cursorID);
		cout << "con->AdvanceCursor("<<qhandle<<", "<<cursorID<<"): " << EC_Decode(ec) << endl;

		ec = con->RetrieveID(qhandle, true, uid, 0, resp);
		cout << "con->RetrieveID(qhandle, true, uid, 0, resp): " << EC_Decode(ec) << endl;

		ec = con->RetrieveCursor(qhandle, true, cursorID, resp);
		cout << "con->RetrieveCursor(qhandle, true, cursorID, resp): " << EC_Decode(ec) << endl;

		ec = con->Retrieve(qhandle, true, 0, resp);
		cout << "con->Retrieve(qhandle, true, 0, resp): " << EC_Decode(ec) << endl;

		ec = con->CloseCursor(qhandle, cursorID);
		cout << "con->CloseCursor("<<qhandle<<", "<<cursorID<<"): " << EC_Decode(ec) << endl;


		con->CloseQueue(qhandle);
		cout << "con->CloseQueue("<< qhandle <<"): " << EC_Decode(ec) << endl;


		MQConnection::QDATA_VECTOR	qnames;
		MQConnection::USER_VECTOR		users;
		MQConnection::NAME_VECTOR		groups;

		ec = con->EnumerateQueues(qnames);
		cout << "con->EnumerateQueues(qnames): " << EC_Decode(ec) << endl;
		ec = con->EnumerateUsers(users);
		cout << "con->EnumerateUsers(users): " << EC_Decode(ec) << endl;
		ec = con->EnumerateGroups(groups);
		cout << "con->EnumerateGroups(groups): " << EC_Decode(ec) << endl;
		
		ec = con->CreateUser(username, "", "Cool Dude");
		cout << "con->CreateUser(username, '', 'Cool Dude'): " << EC_Decode(ec) << endl;
		ec = con->UserSetPermissions(username, true, true, true);
		cout << "con->UserSetPermissions(username, true, true, true): " << EC_Decode(ec) << endl;
		bool modifyqueues, modifyusers, modifygroups;
		ec = con->UserGetPermissions(username, modifyqueues, modifyusers, modifygroups);
		cout << "con->UserGetPermissions(username, modifyqueues, modifyusers, modifygroups): " << EC_Decode(ec) << endl;
		ec = con->SetPassword(username, "foogoo");
		cout << "con->SetPassword(username, 'foogoo'): " << EC_Decode(ec) << endl;

		ec = con->CreateGroup(groupname);
		cout << "con->CreateGroup(groupname): " << EC_Decode(ec) << endl;
		ec = con->GroupSetPermissions(groupname, true, true, true);
		cout << "con->GroupSetPermissions(groupname, true, true, true): " << EC_Decode(ec) << endl;
		ec = con->GroupGetPermissions(groupname, modifyqueues, modifyusers, modifygroups);
		cout << "con->GroupGetPermissions(groupname, modifyqueues, modifyusers, modifygroups): " << EC_Decode(ec) << endl;

		ec = con->GroupAddUser(groupname, username);
		cout << "con->GroupAddUser(groupname, username): " << EC_Decode(ec) << endl;

		ec = con->GroupDeleteUser(groupname, username);
		cout << "con->GroupDeleteUser(groupname, username): " << EC_Decode(ec) << endl;

		MQConnection::NAME_VECTOR	group_users;
		ec = con->GroupGetUsers(groupname, group_users);
		cout << "con->GroupGetUsers(groupname, group_users): " << EC_Decode(ec) << endl;
		ec = con->UserGetGroups(username, groups);
		cout << "con->UserGetGroups(username, groups): " << EC_Decode(ec) << endl;


		ec = con->QueueSetUserPermission(queuename, username, true, false, true, false);
		cout << "con->QueueSetUserPermission(queuename, username, true, false, true, false): " << EC_Decode(ec) << endl;
		ec = con->QueueSetGroupPermission(queuename, groupname, true, false, true, false);
		cout << "con->QueueSetGroupPermission(queuename, groupname, true, false, true, false): " << EC_Decode(ec) << endl;
		ec = con->QueueDeleteUserPermission(queuename, username);
		cout << "con->QueueDeleteUserPermission(queuename, username): " << EC_Decode(ec) << endl;
		ec = con->QueueDeleteGroupPermission(queuename, groupname);
		cout << "con->QueueDeleteGroupPermission(queuename, groupname): " << EC_Decode(ec) << endl;
		std::vector<QueuePermissions> perms;
		ec = con->QueueEnumeratePermissions(queuename, perms);
		cout << "con->QueueEnumeratePermissions(queuename, perms): " << EC_Decode(ec) << endl;

		ec = con->DeleteGroup(groupname);
		cout << "con->DeleteGroup(groupname): " << EC_Decode(ec) << endl;

		ec = con->DeleteUser(username);
		cout << "con->DeleteUser(username): " << EC_Decode(ec) << endl;

		ec = con->DeleteQueue(queuename);
		cout << "con->DeleteQueue("<< queuename <<"): " << EC_Decode(ec) << endl;
*/

//	} catch (XMLSyntaxException e) {
//		cout << "Error: " << (std::string) e << endl;
//		cout << "Token: '" << e.token() << "'" << endl;
//		cout << "Where: " << e.where() << endl;
//	} catch (XMLException e) {
//		cout << (std::string) e << endl;
	} catch (ErrorCode e) {
		cout << "ERROR=================================================" << endl;
		cout << EC_Decode(e) << endl;
	} catch (MQFactoryException e) {
		cout << "ERROR=================================================" << endl;
		cout << e.what() << endl;
	} catch (tcpsocket::SocketException e) {
		cout << "ERROR=================================================" << endl;
		cout << e.what() << endl;
	}
	return 0;
}


#if 0
int main (int argc, char* argv[])
{
	try {
		MQConnection*	con = MQFactory::BuildConnection("//localhost:9000",username,"");
		con->CreateQueue("tq","");
		con->CreateQueue("tqerr", "");
		delete con;

		MessageQueue*	tq = MQFactory::BuildQueueConnection("//localhost:9000/tq",username,"");
		MessageQueue*	tqerr = MQFactory::BuildQueueConnection("//localhost:9000/tqerr",username,"");

		QueueMessage	snd;

		snd.setResponseQueueName("//localhost:9000/tqerr");
		snd.setTTLErrorWanted(true);
		snd.setTimeToLiveSeconds(1);
		snd.setBodyType(BT_NONE);
		snd.setLabel("Foo");
		tq->Enqueue(snd);

		if (true) {{
			QueueMessage	msg;
			ErrorCode		ec = tqerr->RetreiveID(true,snd.getMessageID(),10,msg);
			if (ec == EC_NOERROR) {
				cout << msg.getLabel() << endl;
				time_t	t = msg.getTimeStamp();
				cout << ctime(&t);
				cout << "Body Type:" << msg.getBodyType() << endl;
				cout << "Message Class: " << msg.getMessageClass() << endl;
			}// else
			//	break;
		}}
		delete tq;
		delete tqerr;
	} catch (XMLSyntaxException e) {
		cout << "Error: " << (std::string) e << endl;
		cout << "Token: '" << e.token() << "'" << endl;
		cout << "Where: " << e.where() << endl;
	} catch (XMLException e) {
		cout << (std::string) e << endl;
	} catch (ErrorCode e) {
		cout << "ERROR=================================================" << endl;
		cout << EC_Decode(e) << endl;
	} catch (MQFactoryException e) {
		cout << "ERROR=================================================" << endl;
		cout << e.what() << endl;
	} catch (tcpsocket::SocketException e) {
		cout << "ERROR=================================================" << endl;
		cout << e.what() << endl;
	}
	return 0;
}








int main(int argc, char* argv[])
{
	bufstream*		buf;
	try {

		MessageQueue	*mq = MQFactory::BuildQueueConnection(argv[1],argv[2],"");

		while (true) {{
			QueueMessage	msg;
			ErrorCode		ec = mq->Retreive(true,-1,msg);

			buf = msg.getBufferStream();
	//		cout.write(buf->buffer(),buf->length());
	//		cout << endl;
	//		cout << buf->length() << endl;

			if (ec == EC_NOERROR) {
				XMLEntity						e(*msg.getBufferStream());
				XMLEntity::XMLEntityPosition	pos;
				std::vector<XML::XMLEntity::XMLNode>	result;

				pos = e.findFirstEntity("auth_number");
				if (pos != XMLEntity::NULLPOS) 
					cout << e[pos].name() << ": " << e[pos].value() << ": ";
				result = e.findEntityByXPath("/claim[1]/claim_number");
				if (result.size())
					cout << result[0].value() << ": ";
				result = e.findEntityByXPath("/claim[1]/claim_status");
				if (result.size())
					cout << result[0].value() << ": ";

				pos = e.findFirstEntity("message");
				if (pos != XMLEntity::NULLPOS)
					cout << trim(e[pos].value());
				cout << endl;
			} else
				break;
		}}
		delete mq;
	} catch (XMLSyntaxException e) {
		
		cout << "buf->gpos():" << (bufstream::pos_type) buf->tellg() << endl;
		cout << "buf->ppos():" << (bufstream::pos_type) buf->tellg() << endl;
		cout << "buf->length(): " << buf->length() << endl;
	

		cout << "Error: " << (std::string) e << endl;
		cout << "Token: '" << e.token() << "'" << endl;
		cout << "Where: " << e.where() << endl;
	} catch (XMLException e) {
		cout << (std::string) e << endl;
	} catch (ErrorCode e) {
		cout << "ERROR=================================================" << endl;
		cout << EC_Decode(e) << endl;
	} catch (MQFactoryException e) {
		cout << "ERROR=================================================" << endl;
		cout << e.what() << endl;
	} catch (tcpsocket::SocketException e) {
		cout << "ERROR=================================================" << endl;
		cout << e.what() << endl;
	}
	return 0;
}



int main(int argc, char* argv[])
{
	try {
		if (argc < 3)
			return -1;

		cout << "Opening :" << argv[1] << endl;

		MessageQueue	*mq = MQFactory::BuildQueueConnection(argv[1],argv[2],"");
		QueueMessage	msg;
		ErrorCode		ec;

		uuid			id;

		id.guid.d1 = 1;
		id.guid.d2 = 2;
		id.guid.d3 = 3;
		memset(id.guid.d4,4,sizeof(id.guid.d4));

		if (argc > 3) {
			for(int xx = 0; xx < atoi(argv[3]) ; xx++) {{
				QueueMessage	msg;
				time_t			t = time(NULL);
			
				msg.setLabel("Test Message");
				msg.setBodyType(BT_TEXT);
				if (argc < 4)
					msg.setReciptID(id);
				*msg.getBufferStream() << ctime(&t);
				ec = mq->Enqueue(msg);
				cout << msg.getMessageID() << " " << EC_Decode(ec) << endl;
			}}
		} else {
			ec = EC_NOERROR;
			int	count = 0;
			MQConnection::CursorHandle	cursor;

			ec = mq->OpenCursor(cursor);
			while (ec == EC_NOERROR) {
				ec = mq->Retreive(true,-1,msg);
				count ++;
				//ec = mq->PeekID(true,id,5,msg);
				//ec = mq->PeekCursor(true,cursor,msg);
				if (ec == EC_NOERROR) {
					time_t t = msg.getTimeStamp();
					//cout << msg.getReciptID() << " ";
					//cout.write(msg.getBufferStream()->buffer(), msg.getBufferStream()->length());
					//cout << "Received: " << count << " " << msg.getMessageID() << " " << " Timestamp:" << ctime(&t) << flush;
					//ec = mq->RetreiveID(true,id,5,msg);
					//cout << msg.getReciptID() << " " << msg.getBufferStream()->rdbuf();
					//cout << endl << flush;
					
					cout << ctime(&t) << flush;
					cout << msg.getLabel() << endl;

					cout << "msg.getBufferStream()->length():" << msg.getBufferStream()->length() << endl;
					cout << "msg.getBodySize():" << msg.getBodySize() << endl;
					//cout.write(msg.getBufferStream()->buffer(), msg.getBufferStream()->length());
					fstream out(("output/"+msg.getLabel()+".xml").c_str(),ios::out);
					out << msg.getBufferStream()->rdbuf() << endl << flush;
					out.close();

					cout << endl;
					//cout.write(msg.getBufferStream()->buffer(), msg.getBufferStream()->length());
					cout.flush();
					//cout << endl;
					//ec = mq->AdvanceCursor(cursor);
				} else {
					time_t t = time(NULL);
					cout << EC_Decode(ec) << " " << ctime(&t) << endl;
				}
			}
			mq->CloseCursor(cursor);
		}
	}  catch (ErrorCode e) {
		cout << "ERROR=================================================" << endl;
		cout << EC_Decode(e) << endl;
	} catch (MQFactoryException e) {
		cout << "ERROR=================================================" << endl;
		cout << e.what() << endl;
	} catch (tcpsocket::SocketException e) {
		cout << "ERROR=================================================" << endl;
		cout << e.what() << endl;
	}

#if 0
	MQConnection*	con = NULL;
	try {
	try {
		MessageQueue *mq = MQFactory::BuildQueueConnection(L"//localhost:9000/foo",Lusername,L"");
		delete mq;
	} catch (ErrorCode ec) {
		cout << "Unable to open: //localhost:9000/foo" << EC_Decode(ec) << endl;
	}
		con = MQFactory::BuildConnection(L"//localhost:9000",Lusername,L"");
		MQConnection::QueueHandle	qh;
		ErrorCode					err;
		int							x;
		uuid						id;
		MQConnection::QDATA_VECTOR	names;
		STRING						tmp,tmp2,tmp3;
		QueuePermissions			perms;
		bufstream					buf;
		fstream						f;

		uuidgen(&id);


		f.open("l:\\development\\tcpsocket\\tcpsocket.cpp", ios::in|ios::binary);
		buf << f.rdbuf();
		f.close();
		err = con->CreateQueue(L"testqueue",L"");
		err = con->OpenQueue(L"testqueue",qh);
		if (err == EC_NOERROR) {
			
			DWORD start = GetTickCount();
			for(x=0;x<1000;x++) {{
				QueueMessage	msg;

				msg.setBodyType(BT_TEXT);
				msg.getBufferStream()->write(buf.buffer(),buf.length());
				err = con->Enqueue(qh,msg);
				if (err != EC_NOERROR)
					break;
				if (x % 250 == 0 && x != 0)
					cout << x << endl;
			}}
			cout << "Enqueue: err: " << EC_Decode(err) << endl;
			DWORD end = GetTickCount();
			cout << x << " messages written in " << fixed << setprecision(3) << (double)(end-start)/1000.0 
				<< " at " << setprecision(3) << x/((double)(end-start)/1000.0) << " " << buf.length() << " byte messages per second." << endl;

		} else {
			cout << "OpenQueue: err: " << EC_Decode(err) << endl;
		}

/*
		err = con->CreateQueue(L"MATTS", L"");
		cout << "MATTS con->CreateQueue(): " << EC_Decode(err) << endl;
		err = con->CreateQueue(L"testqueue", L"");
		cout << "tetqueue con->CreateQueue(): " << EC_Decode(err) << endl;

		err = con->EnumerateQueues(names);
		if (err == EC_NOERROR) {
			for(int x=0;x<names.size();x++) {
				cout << "Queue: \"" << w2acopy(tmp,names[x].queuename) << "\" Owner: \"" << w2acopy(tmp2,names[x].owner) << "\" Group: \"" << w2acopy(tmp3,names[x].group) << "\""<< endl;
			}
		} else {
			cout << "con->enumerateQUeues(): " << EC_Decode(err) << endl;
		}
		


		err= con->OpenQueue(L"MATTS",qh);
		{
		QueueMessage msg;
		msg.setBodyType(BT_TEXT);
		*msg.getBufferStream() << "This is a test" << endl;
		msg.setLabel(L"Deleted Queue Test");
		err = con->Enqueue(qh,msg);
		}
		err = con->DeleteQueue(L"MATTS");
		cout << "con->DeleteQueue(): " << EC_Decode(err) << endl;

		err = con->CloseQueue(qh);


		err = con->DeleteQueue(L"MATTS");
		cout << "con->DeleteQueue(): " << EC_Decode(err) << endl;

		err = con->GetQueuePermissions(L"testqueue", perms);
		cout << "con->GetQueuePermissions(): " << EC_Decode(err) << endl;
		err = con->SetQueuePermissions(L"testqueue", perms);
		cout << "con->SetQueuePermissions(): " << EC_Decode(err) << endl;
*/
		err = con->OpenQueue(L"testqueue", qh);
		if (err == EC_NOERROR) {
/*
			for(x=0;x<10;x++) {
				{
					QueueMessage	msg;
					iostream		*stm;

					msg.setBodyType(BT_TEXT);
					stm = msg.getBufferStream();
					*stm << "This is a test:" << x << endl;
					msg.setLabel(L"My Label for a test");
					if (x==8)
						msg.setReciptID(id);
					err = con->Enqueue(qh,msg);

						

					if (err != EC_NOERROR) {
						cout << "Enqueue ERROR: x:" << x << " error: " << EC_Decode(err) << endl;
						break;
					} else {
						cout << "x: " << x << " msgid: " << msg.getMessageID() << endl;
					}
				}
			}

			
			QueueMessage	front;
			err = con->PeekFront(qh,true,-1,front);
			if (err == EC_NOERROR) {
				cout << "Message id: " << front.getMessageID() << endl;
				cout << "Recipt  id: " << front.getReciptID() << endl;
				cout << "Body:        " << front.getBufferStream()->rdbuf() << flush;
			} else {
				cout << "ERROR: PeekFront: " << EC_Decode(err) <<endl;
			}


			QueueMessage	idmsg;
			err = con->PeekID(qh, true, id, -1, idmsg);
			if (err == EC_NOERROR) {
				cout << "Message id: " << idmsg.getMessageID() << endl;
				cout << "Request id: " << id << endl;
				cout << "Recipt  id: " << idmsg.getReciptID() << endl;
				cout << "Body:        " << idmsg.getBufferStream()->rdbuf() << flush;
			} else {
				cout << "ERROR: PeekID: " << EC_Decode(err) <<endl;
			}

			MQConnection::CursorHandle	seekcur;
			err = con->OpenCursor(qh, seekcur);
			if (err != EC_NOERROR) {
				cout << "ERROR: SeekID: " << EC_Decode(err) <<endl;
				return 0;
			}
			err = con->SeekID(qh, id, -1, seekcur);
			if (err == EC_NOERROR) {
				QueueMessage msg;
				err = con->PeekCursor(qh,true,seekcur, msg);
				if (err == EC_NOERROR) {
					cout << "Message id: " << msg.getMessageID() << endl;
					cout << "Recipt  id: " << msg.getReciptID() << endl;
					cout << "Body:        " << msg.getBufferStream()->rdbuf() << flush;
				} else {
					cout << "ERROR: PeekCursor: " << EC_Decode(err) <<endl;
				}
			} else {
				cout << "ERROR: SeekID: " << EC_Decode(err) <<endl;
			}
		//	con->CloseCursor(qh,seekcur);

*/
			MQConnection::CursorHandle	cursor;
			err = con->OpenCursor(qh, cursor);
			if (err == EC_NOERROR) {
				for(x=0;x<10000;x++) {
					{
						QueueMessage	msg;
						STRING		label;

						#if 1
						err = con->RetreiveCursor(qh, true, cursor, msg);
						if (err == EC_NOERROR) {
							if (x%250==0 && x != 0)
								cout << x << ": " << w2acopy(label, msg.getLabel()) << endl; //<< ": " << msg.getBufferStream()->rdbuf() << flush;
						} else {
							cout << "ERROR: x:" << x << " error: " << EC_Decode(err) << endl;
							break;
						}

						#else
						err = con->PeekCursor(qh, true, cursor, msg);
						if (err == EC_NOERROR) {
							if (x%250==0 && x != 0)
								cout << x << ": " << w2acopy(label, msg.getLabel()) << ": " << msg.getBufferStream()->rdbuf();
							err = con->AdvanceCursor(qh, cursor);
							if (err != EC_NOERROR)	{
								cout << "ERROR: x:" << x << " error: " << EC_Decode(err) << endl;
								break;
							}
						} else {
							cout << "ERROR: x:" << x << " error: " << EC_Decode(err) << endl;
							break;
						}
						#endif
					}
				}
				err = con->TestCursor(qh,cursor);
				cout << "con->TestCursor(cursor): " << EC_Decode(err) << endl;

				err = con->CloseCursor(qh,cursor);
				if (err != EC_NOERROR) {
					cout << "ERROR: Close Cursor: " << EC_Decode(err) << endl;
				}
			} else {
				cout << "Error: OpenCursor: " << EC_Decode(err) << endl;
			}

	
//			err = con->TestCursor(qh,seekcur);
//			cout << "con->TestCursor(seekcur): " << EC_Decode(err) << endl;
		
//			err = con->CloseCursor(qh,seekcur);
//			if (err != EC_NOERROR)
//				cout << "ERROR: CloseCursor (seekcur): " << EC_Decode(err) << endl;

			err = con->CloseQueue(qh);
			if (err != EC_NOERROR)
				cout << "ERROR: CloseQueue: " << EC_Decode(err) << endl;
		} else {
			cout << "Open Queue Error: " << EC_Decode(err) << endl;
		}
	} catch (ErrorCode e) {
		cout << "ERROR=================================================" << endl;
		cout << EC_Decode(e) << endl;
	} catch (MQFactoryException e) {
		cout << "ERROR=================================================" << endl;
		cout << e.what() << endl;
	} catch (tcpsocket::SocketException e) {
		cout << "ERROR=================================================" << endl;
		cout << e.what() << endl;
	}
	delete con;
#endif

 	return 0;
}
#endif



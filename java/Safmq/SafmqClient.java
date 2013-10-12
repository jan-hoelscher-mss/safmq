/*
 * Created on Mar 22, 2005
 *
 */
import java.io.PrintWriter;
import java.net.URI;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import com.safmq.CursorHandle;
import com.safmq.MQBuilder;
import com.safmq.MQConnection;
import com.safmq.QueueHandle;
import com.safmq.QueueMessage;
import com.safmq.Safmq;
import com.safmq.UUID;

class MyTrustManager implements X509TrustManager {
   MyTrustManager() { // constructor
	  // create/load keystore
   }

   public void checkClientTrusted(X509Certificate[] chain,String authType) throws CertificateException {
   		System.out.println(authType+"\n"+chain);
   }

   public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {
		System.out.println(authType+"\n"+chain);
   }

	static X509Certificate certs[] = new X509Certificate[0];

	public X509Certificate[] getAcceptedIssuers() {
		return certs;
   }
}


/**
 * @author Matt
 *
 */
public class SafmqClient {
	static String EC_Decode(int error) {
		switch(error) {
			case Safmq.EC_NOERROR : return "EC_NOERROR"; 
			case Safmq.EC_TIMEDOUT : return "EC_TIMEDOUT";
			case Safmq.EC_NOTAUTHORIZED : return "EC_NOTAUTHORIZED";
			case Safmq.EC_ALREADYCLOSED: return "EC_ALREADYCLOSED";
			case Safmq.EC_DOESNOTEXIST: return "EC_DOESNOTEXIST";
			case Safmq.EC_NOTOPEN: return "EC_NOTOPEN";
			case Safmq.EC_NETWORKERROR: return "EC_NETWORKERROR";
			case Safmq.EC_TTLEXPIRED: return "EC_TTLEXPIRED";
			case Safmq.EC_CURSORINVALIDATED: return "EC_CURSORINVALIDATED";
			case Safmq.EC_DUPLICATEMSGID: return "EC_DUPLICATEMSGID";
			case Safmq.EC_SERVERUNAVAIL: return "EC_SERVERUNAVAIL";
			case Safmq.EC_NOMOREMESSAGES: return "EC_NOMOREMESSAGES";
			case Safmq.EC_FILESYSTEMERROR: return "EC_FILESYSTEMERROR";
			case Safmq.EC_CANNOTCHANGEGROUP: return "EC_CANNOTCHANGEGROUP";
			case Safmq.EC_CANNOTCHANGEUSER: return "EC_CANNOTCHANGEUSER";
			case Safmq.EC_ALREADYDEFINED: return "EC_ALREADYDEFINED";
			case Safmq.EC_NOTIMPLEMENTED: return "EC_NOTIMPLEMENTED";
			case Safmq.EC_QUEUEOPEN: return "EC_QUEUEOPEN";
			case Safmq.EC_NOTLOGGEDIN: return "EC_NOTLOGGEDIN";
			case Safmq.EC_INVALIDNAME: return "EC_INVALIDNAME";
			case Safmq.EC_FORWARDNOTALLOWED: return "EC_FORWARDNOTALLOWED";
			case Safmq.EC_WRONGMESSAGETYPE: return "EC_WRONGMESSAGETYPE";
			case Safmq.EC_UNSUPPORTED_PROTOCOL: return "EC_UNSUPPORTED_PROTOCOL";
			case Safmq.EC_ERROR: return "EC_ERROR";
			default: return "UNKNOWN";
		}	
	}

	public static void main(String[] args) {
		String queuename="foobar";
		//String username="matt";
		//String groupname="elgroupo";

		
		try {
			SSLContext ctx = SSLContext.getInstance("SSL");
			TrustManager myTrustManagers[] = {new MyTrustManager()};
			ctx.init(null,myTrustManagers,null);
			
			MQBuilder.setSSLContext(ctx);
			
			//DateFormat df = DateFormat.getDateTimeInstance(DateFormat.SHORT,DateFormat.MEDIUM);

			MQConnection				con = MQBuilder.buildConnection(new URI("safmq://localhost:9000"),"admin","");
			int							ec;
			QueueHandle					qhandle = new QueueHandle();
			CursorHandle				cursorID = new CursorHandle();
			UUID						uid = new UUID();
			QueueMessage				msg = new QueueMessage(), resp = new QueueMessage();

			uid.d1 = 42;
			uid.d2 = 63;
			uid.d3 = 74;


			msg.setLabel("Test1");
			msg.setBodyType(Safmq.BT_TEXT);
			(new PrintWriter(msg.getOutputStream())).println("Hello World!\n");
			//msg.setReciptID(uid);

			con.DeleteQueue(queuename);
			ec = con.CreateQueue(queuename);
			ec = con.OpenQueue(queuename,qhandle);
	
	//		con.DeleteQueue(queue2name);
	//		ec = con.CreateQueue(queue2name);
	//		ec = con.OpenQueue(queue2name,q2handle);
	
//			ec = con.BeginTransaction();
			
			ec = con.Enqueue(qhandle,msg);
	//		ec = con.Enqueue(q2handle,msg);
			msg.setLabel("Test2");
			ec = con.Enqueue(qhandle,msg);
	//		ec = con.Enqueue(q2handle,msg);
			msg.setLabel("Test3");
			ec = con.Enqueue(qhandle,msg);
	//		ec = con.Enqueue(q2handle,msg);
			msg.setLabel("Test4");
			ec = con.Enqueue(qhandle,msg);
	//		ec = con.Enqueue(q2handle,msg);
			msg.setLabel("Test5");
			ec = con.Enqueue(qhandle,msg);
	//		ec = con.Enqueue(q2handle,msg);
			msg.setLabel("Test6");
			ec = con.Enqueue(qhandle,msg);
	//		ec = con.Enqueue(q2handle,msg);
			msg.setLabel("Test7");
			ec = con.Enqueue(qhandle,msg);
	//		ec = con.Enqueue(q2handle,msg);
			msg.setLabel("Test8");
			msg.setReciptID(uid);
			ec = con.Enqueue(qhandle,msg);
	//		ec = con.Enqueue(q2handle,msg);
	
//			System.out.println("Enqueue Complete:===================================");
//			
//			while ( (ec=con.Retrieve(qhandle, true, 0, resp)) == Safmq.EC_NOERROR) {
//				System.out.println("Retrieve(): " + EC_Decode(ec) + " label: " + resp.getLabel());
//			}
//			System.out.println("Retrieve 1 Complete:===================================");
//			
//			//ec = con.RollbackTransaction();
//			//System.out.println("con.RollbackTransaction(): " + EC_Decode(ec));
//			//ec = con.CommitTransaction();
//			//System.out.println("con.CommitTransaction(): " + EC_Decode(ec));
//			ec = con.EndTransaction();
//			System.out.println("con.EndTransaction(): " + EC_Decode(ec));
//			
//			while ( (ec=con.Retrieve(qhandle, true, 0, resp)) == Safmq.EC_NOERROR) {
//				System.out.println("Retrieve(): " + EC_Decode(ec) + " label: " + resp.getLabel());
//			}
//			System.out.println("Retrieve 2 Complete:===================================");
	
	
	
	
			ec = con.BeginTransaction();
			System.out.println("con.BeginTransaction(): " + EC_Decode(ec));
	
			ec = con.RetrieveID(qhandle, true, uid, -1, resp);
			System.out.println("RetrieveID(): " + EC_Decode(ec) + " label: " + resp.getLabel() + " should be: Test8");
	
			con.OpenCursor(qhandle,cursorID);
			con.AdvanceCursor(qhandle,cursorID);
			ec = con.RetrieveCursor(qhandle, true, cursorID, resp);
			System.out.println("RetrieveCursor(): " + EC_Decode(ec) + " label: " + resp.getLabel() + " should be: Test2");
			ec = con.RetrieveCursor(qhandle, true, cursorID, resp);
			System.out.println("RetrieveCursor(): " + EC_Decode(ec) + " label: " + resp.getLabel() + " should be: Test3");
	
	
			while ( (ec=con.Retrieve(qhandle, true, 0, resp)) == Safmq.EC_NOERROR) {
				System.out.println("Retrieve(): " + EC_Decode(ec) + " label: " + resp.getLabel());
			}
	
	//		while ( (ec=con.Retrieve(q2handle, true, 0, resp)) == EC_NOERROR) {
	//			System.out.println("q2.Retrieve(): " + EC_Decode(ec) + " label: " + resp.getLabel());
	//		}
	
			ec = con.RollbackTransaction();
			System.out.println("con.RollbackTransaction(): " + EC_Decode(ec));
//			ec = con.CommitTransaction();
//			System.out.println("con.CommitTransaction(): " + EC_Decode(ec));
	
			System.out.println("------------------------------------------");
			while ( (ec=con.Retrieve(qhandle, true, 0, resp)) == Safmq.EC_NOERROR) {
				System.out.println("Retrieve(): " + EC_Decode(ec) + " label: " + resp.getLabel());
			}
	
	//		while ( (ec=con.Retrieve(q2handle, true, 0, resp)) == EC_NOERROR) {
	//			System.out.println("q2.Retrieve(): " + EC_Decode(ec) + " label: " + resp.getLabel());
	//		}

			ec = con.EndTransaction();
			System.out.println("con.EndTransaction(): " + EC_Decode(ec));
	
			con.CloseQueue(qhandle);
			con.DeleteQueue(queuename);

/*
			msg.setLabel("Test");
			msg.setBodyType(Safmq.BT_TEXT);
			(new PrintWriter(msg.getOutputStream())).println("Hello World!\n");
			msg.setReciptID(uid);


			System.out.println("SAFMQ Major Version: " + con.GetServerMajorProtocolVersion());
			System.out.println("SAFMQ Minor Version: " + con.GetServerMinorProtocolVersion());

			ec = con.CreateQueue(queuename);
			System.out.println("con.CreateQueue("+queuename+"): " + EC_Decode(ec));

			ec = con.OpenQueue(queuename, qhandle);
			System.out.println("con.OpenQueue("+queuename+"," + qhandle +"): " + EC_Decode(ec));
		
			ec = con.Enqueue(qhandle, msg);
			System.out.println("con.Enqueue(" + qhandle + ", " + msg + "): " + EC_Decode(ec));

			ec = con.Enqueue(qhandle, msg);
			System.out.println("con.Enqueue(" + qhandle + ", " + msg + "): " + EC_Decode(ec));

			ec = con.Enqueue(qhandle, msg);
			System.out.println("con.Enqueue(" + qhandle + ", " + msg + "): " + EC_Decode(ec));

			ec = con.Enqueue(qhandle, msg);
			System.out.println("con.Enqueue(" + qhandle + ", " + msg + "): " + EC_Decode(ec));

			ec = con.EnqueueWithRelay(new URI("safmq://admin:@localhost/foobar"), msg);
			System.out.println("con.EnqueueWithRelay(safmq://admin:@localhost/foobar, " + msg + "): " + EC_Decode(ec));

			ec = con.OpenCursor(qhandle, cursorID);
			System.out.println("con.OpenCursor("+qhandle+", "+cursorID+"): " + EC_Decode(ec));
			ec = con.SeekID(qhandle, uid, 0, cursorID);
			System.out.println("con.SeekID(" + qhandle + ", " + uid + ", " + 0 + "," +cursorID+"): " + EC_Decode(ec));

			ec = con.PeekFront(qhandle, true, 0, resp);
			System.out.println("con.PeekFront(qhandle, true, 0, resp): " + EC_Decode(ec));
			ec = con.PeekID(qhandle, true, uid, 0, resp);
			System.out.println("con.PeekFront(qhandle, true, 0, resp): " + EC_Decode(ec));

			ec = con.PeekCursor(qhandle, true, cursorID, resp);
			System.out.println("con.PeekCursor(qhandle, true, cursorID, resp): " + EC_Decode(ec));
			ec = con.TestCursor(qhandle, cursorID);
			System.out.println("con.TestCursor("+qhandle+", "+cursorID+"): " + EC_Decode(ec));
			ec = con.AdvanceCursor(qhandle, cursorID);
			System.out.println("con.AdvanceCursor("+qhandle+", "+cursorID+"): " + EC_Decode(ec));

			ec = con.AdvanceCursor(qhandle, cursorID);
			System.out.println("con.AdvanceCursor("+qhandle+", "+cursorID+"): " + EC_Decode(ec));

			ec = con.RetrieveID(qhandle, true, uid, 0, resp);
			System.out.println("con.RetrieveID(qhandle, true, uid, 0, resp): " + EC_Decode(ec));

			ec = con.RetrieveCursor(qhandle, true, cursorID, resp);
			System.out.println("con.RetrieveCursor(qhandle, true, cursorID, resp): " + EC_Decode(ec));

			ec = con.Retrieve(qhandle, true, 0, resp);
			System.out.println("con.Retrieve(qhandle, true, 0, resp): " + EC_Decode(ec));

			ec = con.CloseCursor(qhandle, cursorID);
			System.out.println("con.CloseCursor("+qhandle+", "+cursorID+"): " + EC_Decode(ec));


			con.CloseQueue(qhandle);
			System.out.println("con.CloseQueue("+ qhandle +"): " + EC_Decode(ec));


			Vector qnames = new Vector();
			Vector users= new Vector();
			Vector groups= new Vector();

			ec = con.EnumerateQueues(qnames);
			System.out.println("con.EnumerateQueues(qnames): " + EC_Decode(ec));
			ec = con.EnumerateUsers(users);
			System.out.println("con.EnumerateUsers(users): " + EC_Decode(ec));
			ec = con.EnumerateGroups(groups);
			System.out.println("con.EnumerateGroups(groups): " + EC_Decode(ec));
		
			ec = con.CreateUser(username, "", "Cool Dude");
			System.out.println("con.CreateUser(username, '', 'Cool Dude'): " + EC_Decode(ec));
			ec = con.UserSetPermissions(username, true, true, true);
			System.out.println("con.UserSetPermissions(username, true, true, true): " + EC_Decode(ec));
			ActorPermissions	ap = new ActorPermissions();
			ec = con.UserGetPermissions(username, ap);
			System.out.println("con.UserGetPermissions(username, modifyqueues, modifyusers, modifygroups): " + EC_Decode(ec));
			ec = con.SetPassword(username, "foogoo");
			System.out.println("con.SetPassword(username, 'foogoo'): " + EC_Decode(ec));

			ec = con.CreateGroup(groupname);
			System.out.println("con.CreateGroup(groupname): " + EC_Decode(ec));
			ec = con.GroupSetPermissions(groupname, true, true, true);
			System.out.println("con.GroupSetPermissions(groupname, true, true, true): " + EC_Decode(ec));
			ec = con.GroupGetPermissions(groupname, ap);
			System.out.println("con.GroupGetPermissions(groupname, modifyqueues, modifyusers, modifygroups): " + EC_Decode(ec));

			ec = con.GroupAddUser(groupname, username);
			System.out.println("con.GroupAddUser(groupname, username): " + EC_Decode(ec));

			ec = con.GroupDeleteUser(groupname, username);
			System.out.println("con.GroupDeleteUser(groupname, username): " + EC_Decode(ec));

			Vector group_users = new Vector(); 
			ec = con.GroupGetUsers(groupname, group_users);
			System.out.println("con.GroupGetUsers(groupname, group_users): " + EC_Decode(ec));
			ec = con.UserGetGroups(username, groups);
			System.out.println("con.UserGetGroups(username, groups): " + EC_Decode(ec));


			ec = con.QueueSetUserPermission(queuename, username, true, false, true, false);
			System.out.println("con.QueueSetUserPermission(queuename, username, true, false, true, false): " + EC_Decode(ec));
			ec = con.QueueSetGroupPermission(queuename, groupname, true, false, true, false);
			System.out.println("con.QueueSetGroupPermission(queuename, groupname, true, false, true, false): " + EC_Decode(ec));
			ec = con.QueueDeleteUserPermission(queuename, username);
			System.out.println("con.QueueDeleteUserPermission(queuename, username): " + EC_Decode(ec));
			ec = con.QueueDeleteGroupPermission(queuename, groupname);
			System.out.println("con.QueueDeleteGroupPermission(queuename, groupname): " + EC_Decode(ec));
			Vector perms = new Vector();
			ec = con.QueueEnumeratePermissions(queuename, perms);
			System.out.println("con.QueueEnumeratePermissions(queuename, perms): " + EC_Decode(ec));

			ec = con.DeleteGroup(groupname);
			System.out.println("con.DeleteGroup(groupname): " + EC_Decode(ec));

			ec = con.DeleteUser(username);
			System.out.println("con.DeleteUser(username): " + EC_Decode(ec));

			ec = con.DeleteQueue(queuename);
			System.out.println("con.DeleteQueue("+ queuename +"): " + EC_Decode(ec));
*/
			
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}

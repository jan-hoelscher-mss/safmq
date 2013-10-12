import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.URI;
import java.util.Vector;

import junit.framework.Assert;
import junit.framework.TestCase;

import com.safmq.ActorPermissions;
import com.safmq.CursorHandle;
import com.safmq.MQBuilder;
import com.safmq.MQConnection;
import com.safmq.QueueHandle;
import com.safmq.QueueMessage;
import com.safmq.Safmq;
import com.safmq.UUID;

public class SafmqTest extends TestCase {
	final static String TEST_QUEUE[] = { "test", "foo", "bar", "thequeue", "notthequeue" };
	final static String TEST_USERS[] = { "bob", "mary", "pat", "charlie", "jack", "kris" };
	final static String TEST_PASSWD[] = TEST_USERS;
	final static String TEST_DESC[] = TEST_USERS;
	final static String TEST_GROUPS[] = { "them", "us", "others", "natives", "aliens" };
	final static String SPECIAL_MESSAGE = "special message";
	
	String user = null;
	String password = null;
	URI address;
	URI simpleAddress;

	public SafmqTest() throws Exception {
		address = new URI("safmq://admin:@localhost:40000");
		simpleAddress = new URI("safmq://localhost:40000");
	}

	protected void tearDown() throws Exception {
		// MQConnection con = connectToServer(address,null,null);
		// for(int x=0;x<TEST_QUEUE.length;x++)
		// con.DeleteQueue(TEST_QUEUE[x]);
		//		
		// for(int x=0;x<TEST_USERS.length;x++)
		// con.DeleteUser(TEST_USERS[x]);
		//
		// for(int x=0;x<TEST_GROUPS.length;x++)
		// con.DeleteUser(TEST_GROUPS[x]);
		//		
		super.tearDown();
	}


	MQConnection connectToServer(URI uri, String user, String password) {
		try {
			MQConnection con = MQBuilder.buildConnection(uri, user, password);
			Assert.assertTrue("Successfully Connected", con != null);
			return con;
		} catch (Exception e) {
			Assert.assertTrue(e.toString(), e == null);
		}
		return null;
	}

	void createEmptyQueue(MQConnection con, String queueName) {
		int ec;
		ec = con.DeleteQueue(queueName);
		Assert.assertTrue("Failed to delete queue: " + queueName, (ec == Safmq.EC_NOERROR || ec == Safmq.EC_DOESNOTEXIST));

		ec = con.CreateQueue(queueName);
		Assert.assertTrue("Failed to create queue: " + queueName, ec == Safmq.EC_NOERROR);
	}

	void createNewUser(MQConnection con, String userName, String description, String password) {
		int ec = con.DeleteUser(userName);
		Assert.assertTrue("Failed to remove user prior to create: " + userName, (ec == Safmq.EC_NOERROR || ec == Safmq.EC_DOESNOTEXIST));
		ec = con.CreateUser(userName, password, description);
		Assert.assertTrue("Failed to create user: " + userName, ec == Safmq.EC_NOERROR);
	}

	void createNewGroup(MQConnection con, String groupName) {
		int ec = con.DeleteGroup(groupName);
		Assert.assertTrue("Failed to remove group prior to create: " + groupName, (ec == Safmq.EC_NOERROR || ec == Safmq.EC_DOESNOTEXIST));
		ec = con.CreateGroup(groupName);
		Assert.assertTrue("Failed to create group: " + groupName, ec == Safmq.EC_NOERROR);
	}

	void addAllUsers(MQConnection con) {
		for (int x = 0; x < TEST_USERS.length; x++) {
			createNewUser(con, TEST_USERS[x], TEST_DESC[x], TEST_PASSWD[x]);
		}
	}

	void addAllGroups(MQConnection con) {
		for (int x = 0; x < TEST_GROUPS.length; x++) {
			createNewGroup(con, TEST_GROUPS[x]);
		}
	}

	void addAllQueues(MQConnection con) {
		for (int x = 0; x < TEST_QUEUE.length; x++) {
			createEmptyQueue(con, TEST_QUEUE[x]);
		}
	}

	void sendMessages(MQConnection con, QueueHandle handle, int count, int idxStart) {
		QueueMessage msg;
		int ec;
		
		for(int x=0;x<count;x++) {
			msg = new QueueMessage();
			PrintWriter w = new PrintWriter(new OutputStreamWriter(msg.getOutputStream()));
			
			w.println((x + idxStart));
			w.close();
			
			ec = con.Enqueue(handle, msg);
			Assert.assertTrue("Unable to send message: " + (x + idxStart) + " ec: " + ec, ec == Safmq.EC_NOERROR);
		}
	}
	
	// public int EnqueueWithRelay(URI uri, QueueMessage msg) {

	// public int SetPassword(String username, String password) {


	public void test_CreateQueue() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			createEmptyQueue(con, TEST_QUEUE[0]);
		} finally {
			if (con != null)
				con.Close();
		}
	}

	public void test_CreateTempQueue() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			String[] queueName = new String[1];
			QueueHandle qh = new QueueHandle();
			
			// Try and create a temporary queue
			int err = con.CreateTempQueue(queueName, qh);
			System.out.println("queueName:" + queueName[0]);
			Assert.assertEquals("Creating Queue", Safmq.EC_NOERROR, err);
			
			// Get permissions for that queue
			Vector perms = new Vector();
			con.QueueEnumeratePermissions(queueName[0], perms);
			MQConnection.QueuePermissions perm;
			for(int x = 0; x < perms.size(); ++x) {
				perm = (MQConnection.QueuePermissions)perms.get(x);
				System.out.println(perm.getEntityName() + ":" + perm.getRead() + ":" + perm.getWrite() + ":" + perm.getDestroy() + ":" + perm.getChangeSecurity());
			}
			Assert.assertTrue(perms.size() > 0);

			addAllUsers(con);

			// Try and write to that queue with another user.
			MQConnection con2 = null;
			try {
				con2 = connectToServer(simpleAddress, TEST_USERS[0], TEST_USERS[0]);
				QueueHandle qh2 = new QueueHandle();
				int rc = con2.OpenQueue(queueName[0], qh2);
				Assert.assertTrue("Open Temp Queue rc:" + rc, rc == Safmq.EC_NOERROR);
				QueueMessage msg = new QueueMessage();
				msg.setLabel("Hello World");
				Assert.assertTrue("Enqueue to temp", Safmq.EC_NOERROR == con2.Enqueue(qh2, msg));
				con2.CloseQueue(qh2);
			} finally {
				if (con2 != null)
					con2.Close();
			}
		} finally {
			if (con != null)
				con.Close();
		}
	}
	
	public void test_DeleteQueue() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			createEmptyQueue(con, TEST_QUEUE[0]);
			int ec = con.DeleteQueue(TEST_QUEUE[0]);
			Assert.assertTrue("Failed to delete queue: " + TEST_QUEUE[0], ec == Safmq.EC_NOERROR);
		} finally {
			if (con != null)
				con.Close();
		}
	}

	public void test_CreateUser() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			createNewUser(con, TEST_USERS[0], TEST_DESC[0], TEST_PASSWD[0]);
		} finally {
			if (con != null)
				con.Close();
		}
	}
	
	public void test_SetPassword() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			createNewUser(con, TEST_USERS[0], TEST_DESC[0], TEST_PASSWD[0]);
			
			String newPwd = "new password";
			int ec = con.SetPassword(TEST_USERS[0], newPwd);
			Assert.assertTrue("SetPassword", ec == Safmq.EC_NOERROR);
			
			con.Close();
			con = connectToServer(simpleAddress, TEST_USERS[0], newPwd);
		} finally {
			if (con != null)
				con.Close();
		}
	}
	

	public void test_UserPermissions() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			createNewUser(con, TEST_USERS[0], TEST_DESC[0], TEST_PASSWD[0]);
			int ec;
			for (int x = 7; x >= 0; x--) {
				ec = con.UserSetPermissions(TEST_USERS[0], (x & 0x04) != 0, (x & 0x2) != 0, (x & 0x1) != 0);
				Assert.assertTrue("Failed to set permissions for: " + TEST_USERS[0], ec == Safmq.EC_NOERROR);
				ActorPermissions actorPerms = new ActorPermissions();
				ec = con.UserGetPermissions(TEST_USERS[0], actorPerms);
				Assert.assertTrue("Failed to retrive permissions for: " + TEST_USERS[0], ec == Safmq.EC_NOERROR);
				Assert.assertTrue("Modify queues incorrectly set: " + x, actorPerms.getModifyqueues() == ((x & 0x04) != 0));
				Assert.assertTrue("Modify users incorrectly set: " + x, actorPerms.getModifyusers() == ((x & 0x02) != 0));
				Assert.assertTrue("Modify groups incorrectly set: " + x, actorPerms.getModifygroups() == ((x & 0x01) != 0));
			}
		} finally {
			if (con != null)
				con.Close();
		}
	}

	public void test_DeleteUser() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			createNewUser(con, TEST_USERS[0], TEST_DESC[0], TEST_PASSWD[0]);
			int ec = con.DeleteUser(TEST_USERS[0]);
			Assert.assertTrue("Failed to delete user: " + TEST_USERS[0], ec == Safmq.EC_NOERROR);
		} finally {
			if (con != null)
				con.Close();
		}
	}

	public void test_CreateGroup() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			createNewGroup(con, TEST_GROUPS[0]);
		} finally {
			if (con != null)
				con.Close();
		}
	}

	public void test_GroupPermissions() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			createNewGroup(con, TEST_GROUPS[0]);
			int ec;
			for (int x = 7; x >= 0; x--) {
				ec = con.GroupSetPermissions(TEST_GROUPS[0], (x & 0x04) != 0, (x & 0x2) != 0, (x & 0x1) != 0);
				Assert.assertTrue("Failed to set permissions for: " + TEST_GROUPS[0], ec == Safmq.EC_NOERROR);
				ActorPermissions actorPerms = new ActorPermissions();
				ec = con.GroupGetPermissions(TEST_GROUPS[0], actorPerms);
				Assert.assertTrue("Failed to retrive permissions for: " + TEST_GROUPS[0], ec == Safmq.EC_NOERROR);
				Assert.assertTrue("Modify queues incorrectly set: " + x, actorPerms.getModifyqueues() == ((x & 0x04) != 0));
				Assert.assertTrue("Modify users incorrectly set: " + x, actorPerms.getModifyusers() == ((x & 0x02) != 0));
				Assert.assertTrue("Modify groups incorrectly set: " + x, actorPerms.getModifygroups() == ((x & 0x01) != 0));
			}
		} finally {
			if (con != null)
				con.Close();
		}
	}

	public void test_DeleteGroup() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			createNewGroup(con, TEST_GROUPS[0]);
			int ec = con.DeleteGroup(TEST_GROUPS[0]);
			Assert.assertTrue("Failed to delete group: " + TEST_GROUPS[0], ec == Safmq.EC_NOERROR);
		} finally {
			if (con != null)
				con.Close();
		}
	}

	public void test_EnumerateQueues() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);

			addAllQueues(con);
			Vector qnames = new Vector();
			int ec = con.EnumerateQueues(qnames);
			Assert.assertTrue("Couldn't enumerate queues", ec == Safmq.EC_NOERROR);

			for (int x = 0; x < TEST_QUEUE.length; x++) {
				boolean found = false;
				for (int y = 0; y < qnames.size(); y++) {
					MQConnection.QueueData qd = (MQConnection.QueueData) qnames.get(y);
					if (qd.getName().equals(TEST_QUEUE[x]))
						found = true;
				}
				Assert.assertTrue("Queue not enumerated: " + TEST_QUEUE[0], found);
			}
		} finally {
			if (con != null)
				con.Close();
		}
	}

	public void test_EnumerateUsers() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);

			addAllUsers(con);
			Vector unames = new Vector();
			int ec = con.EnumerateUsers(unames);
			Assert.assertTrue("Couldn't enumerate users", ec == Safmq.EC_NOERROR);

			for (int x = 0; x < TEST_USERS.length; x++) {
				boolean found = false;
				for (int y = 0; y < unames.size(); y++) {
					MQConnection.UserDescription ud = (MQConnection.UserDescription) unames.get(y);
					if (ud.getName().equals(TEST_USERS[x]))
						found = true;
				}
				Assert.assertTrue("User not enumerated: " + TEST_USERS[0], found);
			}
		} finally {
			if (con != null)
				con.Close();
		}
	}

	public void test_EnumerateGroups() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);

			addAllGroups(con);
			Vector gnames = new Vector();
			int ec = con.EnumerateGroups(gnames);
			Assert.assertTrue("Couldn't enumerate groups", ec == Safmq.EC_NOERROR);

			for (int x = 0; x < TEST_GROUPS.length; x++) {
				boolean found = false;
				for (int y = 0; y < gnames.size(); y++) {
					String group = (String) gnames.get(y);
					if (group.equals(TEST_GROUPS[x]))
						found = true;
				}
				Assert.assertTrue("Group not enumerated: " + TEST_GROUPS[0], found);
			}
		} finally {
			if (con != null)
				con.Close();
		}
	}

	public void test_GroupAddUser_GroupDeleteUser_GroupGetUsers() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);

			addAllGroups(con);
			addAllUsers(con);

			int ec;
			for (int x = 0; x < 3; x++) {
				ec = con.GroupAddUser(TEST_GROUPS[0], TEST_USERS[x]);
				Assert.assertTrue("Unable to add user: " + TEST_USERS[x] + " to group: " + TEST_GROUPS[x], ec == Safmq.EC_NOERROR);
			}

			Vector users = new Vector();
			ec = con.GroupGetUsers(TEST_GROUPS[0], users);
			Assert.assertTrue("Unable to get users in group: " + TEST_GROUPS[0], ec == Safmq.EC_NOERROR);

			for (int x = 0; x < 3; x++) {
				Assert.assertTrue("User not found in group: " + TEST_USERS[x], users.contains(TEST_USERS[x]));
			}

			ec = con.GroupDeleteUser(TEST_GROUPS[0], TEST_USERS[1]);
			Assert.assertTrue("Unable to delete user: " + TEST_USERS[1] + " from group: " + TEST_GROUPS[0], ec == Safmq.EC_NOERROR);

			ec = con.GroupGetUsers(TEST_GROUPS[0], users);
			Assert.assertTrue("Unable to get users in group: " + TEST_GROUPS[0], ec == Safmq.EC_NOERROR);
			Assert.assertTrue("User found in group after delete: " + TEST_USERS[1], !users.contains(TEST_USERS[1]));
		} finally {
			if (con != null)
				con.Close();
		}
	}

	public void test_UserGetGroups() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);

			addAllGroups(con);
			addAllUsers(con);

			int ec;
			for (int x = 0; x < 3; x++) {
				ec = con.GroupAddUser(TEST_GROUPS[x], TEST_USERS[0]);
				Assert.assertTrue("Unable to add user: " + TEST_USERS[x] + " to group: " + TEST_GROUPS[x], ec == Safmq.EC_NOERROR);
			}

			Vector groups = new Vector();
			ec = con.UserGetGroups(TEST_USERS[0], groups);
			Assert.assertTrue("Unable to get groups for user: " + TEST_USERS[0], ec == Safmq.EC_NOERROR);

			for (int x = 0; x < 3; x++) {
				Assert.assertTrue("Group not found: " + TEST_GROUPS[x] + " for user: " + TEST_USERS[0], groups.contains(TEST_GROUPS[x]));
			}

			ec = con.GroupDeleteUser(TEST_GROUPS[1], TEST_USERS[0]);
			Assert.assertTrue("Unable to delete user: " + TEST_USERS[1] + " from group: " + TEST_GROUPS[0], ec == Safmq.EC_NOERROR);

			ec = con.UserGetGroups(TEST_GROUPS[0], groups);
			Assert.assertTrue("Unable to get groups for user: " + TEST_USERS[0], ec == Safmq.EC_NOERROR);
			Assert.assertTrue("User found in group after delete: " + TEST_USERS[0], !groups.contains(TEST_GROUPS[1]));
		} finally {
			if (con != null)
				con.Close();
		}
	}
	
	
	public void test_QueueSetUserPermission_QueueDeleteUserPermission_QueueEnumeratePermissions_QueueSetGroupPermission_QueueDeleteGroupPermission() {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			
			addAllGroups(con);
			addAllUsers(con);
			addAllQueues(con);
			
			int ec;
			for(int x=15;x>=0;x--) {
				ec = con.QueueSetUserPermission(TEST_QUEUE[0], TEST_USERS[0], (x&0x08)!=0,(x&0x04)!=0,(x&0x02)!=0,(x&0x01)!=0);
				Assert.assertTrue("Unable to set user permissions: " + x, ec == Safmq.EC_NOERROR);

				ec = con.QueueSetGroupPermission(TEST_QUEUE[0], TEST_GROUPS[0], (x&0x08)!=0,(x&0x04)!=0,(x&0x02)!=0,(x&0x01)!=0);
				Assert.assertTrue("Unable to set group permissions: " + x, ec == Safmq.EC_NOERROR);
				
				Vector perms = new Vector();
				ec = con.QueueEnumeratePermissions(TEST_QUEUE[0], perms);
				Assert.assertTrue("Unable to get permissions: " + TEST_QUEUE[0], ec == Safmq.EC_NOERROR);
				
				for(int y=0;y<perms.size();y++) {
					MQConnection.QueuePermissions perm = (MQConnection.QueuePermissions)perms.get(y);
					if(perm.getIsGroup() && perm.getEntityName().equals(TEST_GROUPS[0])) {
						Assert.assertTrue("Group read perm incorrect", ((x&0x08)!=0) == perm.getRead()); 
						Assert.assertTrue("Group wirte perm incorrect", ((x&0x04)!=0) == perm.getWrite()); 
						Assert.assertTrue("Group destroy perm incorrect", ((x&0x02)!=0) == perm.getDestroy()); 
						Assert.assertTrue("Group change security perm incorrect", ((x&0x01)!=0) == perm.getChangeSecurity()); 
					} else if (!perm.getIsGroup() && perm.getEntityName().equals(TEST_USERS[0])) {
						Assert.assertTrue("user read perm incorrect", ((x&0x08)!=0) == perm.getRead()); 
						Assert.assertTrue("user wirte perm incorrect", ((x&0x04)!=0) == perm.getWrite()); 
						Assert.assertTrue("user destroy perm incorrect", ((x&0x02)!=0) == perm.getDestroy()); 
						Assert.assertTrue("user change security perm incorrect", ((x&0x01)!=0) == perm.getChangeSecurity()); 
					}
				}
			}
			
			ec = con.QueueDeleteGroupPermission(TEST_QUEUE[0], TEST_GROUPS[0]);
			Assert.assertTrue("unable to delete permission for group:" + TEST_GROUPS[0], ec == Safmq.EC_NOERROR);

			ec = con.QueueDeleteGroupPermission(TEST_QUEUE[0], TEST_GROUPS[0]);
			Assert.assertFalse("able to delete permission for group:" + TEST_GROUPS[0], ec == Safmq.EC_NOERROR);
			
			ec = con.QueueDeleteUserPermission(TEST_QUEUE[0], TEST_USERS[0]);
			Assert.assertTrue("unable to delete permission for user:" + TEST_USERS[0], ec == Safmq.EC_NOERROR);

			ec = con.QueueDeleteUserPermission(TEST_QUEUE[0], TEST_USERS[0]);
			Assert.assertFalse("able to delete permission for user:" + TEST_USERS[0], ec == Safmq.EC_NOERROR);
			
		} finally {
			if (con != null)
				con.Close();
		}
	}

	public void test_OpenQueue_Enqueue_Retrieve_CloseQueue() throws Exception {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			
			addAllGroups(con);
			addAllUsers(con);
			addAllQueues(con);

			QueueHandle handle = new QueueHandle();
			int ec = con.OpenQueue(TEST_QUEUE[0], handle);
			Assert.assertTrue("Unable to open queue: " + TEST_QUEUE[0], ec == Safmq.EC_NOERROR);
			
			sendMessages(con, handle, 5, 0);
			
			for(int x=0;x<5;x++) {
				QueueMessage msg = new QueueMessage();
				ec = con.Retrieve(handle, true, 0, msg);
				Assert.assertTrue("Unable to retrieve message: " + x, ec == Safmq.EC_NOERROR);
			
				BufferedReader r = new BufferedReader(new InputStreamReader(msg.getInputStream()));
				String s = r.readLine();
				
				Assert.assertTrue("Incorrect message retrieved: " + x, Integer.parseInt(s) == x);
			}
			
			ec = con.CloseQueue(handle);
			Assert.assertTrue("Unable to close queue", ec == Safmq.EC_NOERROR);
			
		} finally {
			if (con != null)
				con.Close();
		}
	}	
	
	
	public void test_PeekID_PeekFront_OpenCursor_PeekCursor_SeekID_RetrieveCursor_AdvanceCursor_TestCurosr_CloseCursor_RetrieveID() throws Exception {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			
			addAllGroups(con);
			addAllUsers(con);
			addAllQueues(con);

			QueueHandle handle = new QueueHandle();
			int ec = con.OpenQueue(TEST_QUEUE[0], handle);
			Assert.assertTrue("Unable to open queue: " + TEST_QUEUE[0], ec == Safmq.EC_NOERROR);
			
			sendMessages(con, handle, 5, 0);
			QueueMessage msg = new QueueMessage();
			msg.getOutputStream().write(SPECIAL_MESSAGE.getBytes());
			UUID uuid = new UUID();
			uuid.d1 = 123456789;
			uuid.d2 = 4321;
			uuid.d3 = 1234;
			msg.setReciptID((UUID)uuid.clone());
			
			ec = con.Enqueue(handle, msg);
			Assert.assertTrue("Unable to write special message", ec == Safmq.EC_NOERROR);
			
			sendMessages(con, handle, 5, 5);
			
			msg = new QueueMessage();
			ec = con.PeekID(handle, true, uuid, 0, msg);
			Assert.assertTrue("Unable to PeekID", ec == Safmq.EC_NOERROR);
			String txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
			Assert.assertTrue("Incorrect PeekID Message: " + msg.getReciptID(), txt.equals(SPECIAL_MESSAGE) && uuid.equals(msg.getReciptID()));
			
			msg = new QueueMessage();
			ec = con.PeekFront(handle, true, 0, msg);
			Assert.assertTrue("Unable to PeekFront", ec == Safmq.EC_NOERROR);
			txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
			Assert.assertTrue("Incorrect PeekFront Message: body=" + txt, txt.equals("0"));
			
			CursorHandle cur = new CursorHandle();

			ec = con.OpenCursor(handle, cur);
			Assert.assertTrue("Unable to OpenCursor", ec == Safmq.EC_NOERROR);
			
			ec = con.SeekID(handle, uuid, 0, cur);
			Assert.assertTrue("Unable to SeekID", ec == Safmq.EC_NOERROR);
			msg = new QueueMessage();
			ec = con.PeekCursor(handle, true, cur, msg);
			Assert.assertTrue("Unable to PeekCursor", ec == Safmq.EC_NOERROR);
			txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
			Assert.assertTrue("Incorrect PeekCursor Message: " + msg.getReciptID(), txt.equals(SPECIAL_MESSAGE) && uuid.equals(msg.getReciptID()));

			ec = con.AdvanceCursor(handle, cur);
			Assert.assertTrue("Unable to AdvanceCursor", ec == Safmq.EC_NOERROR);
			msg = new QueueMessage();
			ec = con.PeekCursor(handle, true, cur, msg);
			Assert.assertTrue("Unable to PeekCursor (after advance)", ec == Safmq.EC_NOERROR);
			txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
			Assert.assertTrue("Incorrect PeekCursor (after advance) Message: " + msg.getReciptID(), txt.equals("5"));
		
			msg = new QueueMessage();
			ec = con.RetrieveCursor(handle, true, cur, msg);
			Assert.assertTrue("Unable to RetrieveCursor", ec == Safmq.EC_NOERROR);
			txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
			Assert.assertTrue("Incorrect PeekCursor (after RetrieveCursor) Message: " + msg.getReciptID(), txt.equals("5"));
			
			ec = con.TestCursor(handle, cur);
			Assert.assertTrue("Unable to test Cursor, ec: " + ec, ec == Safmq.EC_NOERROR);
			
			ec = con.CloseCursor(handle, cur);
			Assert.assertTrue("Unable to CloseCursor, ec: " + ec, ec == Safmq.EC_NOERROR);
			
			
			msg = new QueueMessage();
			ec = con.RetrieveID(handle, true, uuid, 0, msg);
			Assert.assertTrue("Unable to RetrieveID", ec == Safmq.EC_NOERROR);
			txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
			Assert.assertTrue("Incorrect RetrieveID Message: " + msg.getReciptID(), txt.equals(SPECIAL_MESSAGE) && uuid.equals(msg.getReciptID()));
			
			int bodies[] = {0,1,2,3,4,6,7,8,9}; // 5 was retrieved above.
			for(int x=0;x<bodies.length;x++) {
				msg = new QueueMessage();
				ec = con.Retrieve(handle, true, 0, msg);
				Assert.assertTrue("Unable to retrieve message: " + x, ec == Safmq.EC_NOERROR);
				txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
				Assert.assertTrue("Incorrect message: " + txt, Integer.parseInt(txt) == bodies[x]);
			}
			
			
			ec = con.CloseQueue(handle);
			Assert.assertTrue("Unable to close queue", ec == Safmq.EC_NOERROR);
			
		} finally {
			if (con != null)
				con.Close();
		}
	}	
	
	public void test_Transactions() throws Exception {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			
			addAllGroups(con);
			addAllUsers(con);
			addAllQueues(con);

			QueueHandle handle = new QueueHandle();
			int ec = con.OpenQueue(TEST_QUEUE[0], handle);
			Assert.assertTrue("Unable to open queue: " + TEST_QUEUE[0], ec == Safmq.EC_NOERROR);
			
			sendMessages(con, handle, 10, 0);

			ec = con.BeginTransaction();
			Assert.assertTrue("Unable to BeginTransaction:" + ec, ec == Safmq.EC_NOERROR);
			CursorHandle cur = new CursorHandle();
			
			ec = con.OpenCursor(handle, cur);
			Assert.assertTrue("Unable to OpenCursor:" + ec, ec == Safmq.EC_NOERROR);
			
			ec = con.AdvanceCursor(handle, cur);
			Assert.assertTrue("Unable to AdvanceCursor:" + ec, ec == Safmq.EC_NOERROR);
			ec = con.AdvanceCursor(handle, cur);
			Assert.assertTrue("Unable to AdvanceCursor:" + ec, ec == Safmq.EC_NOERROR);

			QueueMessage msg = new QueueMessage();
			ec = con.RetrieveCursor(handle, true, cur, msg);
			Assert.assertTrue("Unable to RetrieveCursor:" + ec, ec == Safmq.EC_NOERROR);
			String txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
			Assert.assertTrue("Incorrect Message: " + txt + " should be 2", txt.equals("2"));
			
			msg = new QueueMessage();
			ec = con.RetrieveCursor(handle, true, cur, msg);
			Assert.assertTrue("Unable to RetrieveCursor:" + ec, ec == Safmq.EC_NOERROR);
			txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
			Assert.assertTrue("Incorrect Message: " + txt + " should be 3", txt.equals("3"));
		
			msg = new QueueMessage();
			ec = con.Retrieve(handle, true, 0, msg);
			Assert.assertTrue("Unable to RetrieveCursor:" + ec, ec == Safmq.EC_NOERROR);
			txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
			Assert.assertTrue("Incorrect Message: " + txt + " should be 0", txt.equals("0"));
			
			int ids[] = {1,4,5,6,7,8,9};
			for(int x=0;x<ids.length;x++) {
				msg = new QueueMessage();
				ec = con.Retrieve(handle, true, 0, msg);
				Assert.assertTrue("Unable to RetrieveCursor:" + ec, ec == Safmq.EC_NOERROR);
				txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
				Assert.assertTrue("Incorrect Message: " + txt + " should be " + ids[x], Integer.parseInt(txt) == ids[x]);
			}
			
			ec = con.RollbackTransaction();
			Assert.assertTrue("Unable to RollbackTransaction:" + ec, ec == Safmq.EC_NOERROR);
			
			for(int x=0;x<8;x++) {
				msg = new QueueMessage();
				ec = con.Retrieve(handle, true, 0, msg);
				Assert.assertTrue("Unable to RetrieveCursor:" + ec, ec == Safmq.EC_NOERROR);
				txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
				Assert.assertTrue("Incorrect Message: " + txt + " should be " + x, Integer.parseInt(txt) == x);
			}
			
			ec = con.CommitTransaction();
			Assert.assertTrue("Unable to CommitTransaction:" + ec, ec == Safmq.EC_NOERROR);
			
			for(int x=0;x<2;x++) {
				msg = new QueueMessage();
				ec = con.Retrieve(handle, true, 0, msg);
				Assert.assertTrue("Unable to RetrieveCursor:" + ec, ec == Safmq.EC_NOERROR);
				txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
				Assert.assertTrue("Incorrect Message: " + txt+ " should be " + (x+8), Integer.parseInt(txt) == x+8);
			}
			
			ec = con.RollbackTransaction();
			Assert.assertTrue("Unable to RollbackTransaction:" + ec, ec == Safmq.EC_NOERROR);
			
			ec = con.EndTransaction();
			Assert.assertTrue("Unable to RollbackTransaction:" + ec, ec == Safmq.EC_NOERROR);
			
			for(int x=0;x<2;x++) {
				msg = new QueueMessage();
				ec = con.Retrieve(handle, true, 0, msg);
				Assert.assertTrue("Unable to RetrieveCursor:" + ec, ec == Safmq.EC_NOERROR);
				txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
				Assert.assertTrue("Incorrect Message: " + txt+ " should be " +(x+8), Integer.parseInt(txt) == x+8);
			}

			msg = new QueueMessage();
			ec = con.Retrieve(handle, true, 0, msg);
			Assert.assertTrue("Able to RetrieveCursor:" + ec, ec == Safmq.EC_NOMOREMESSAGES);
			
			ec = con.CloseQueue(handle);
			Assert.assertTrue("Unble to CloseQueue:" + ec, ec == Safmq.EC_NOERROR);
		} finally {
			if (con != null)
				con.Close();
		}
	}	

	public void test_EnqueueWithRelay() throws Exception {
		MQConnection con = null;
		try {
			con = connectToServer(address, null, null);
			QueueMessage msg = new QueueMessage();
			msg.getOutputStream().write(SPECIAL_MESSAGE.getBytes());

			addAllQueues(con);
			int ec = con.EnqueueWithRelay(new URI(address + "/" + TEST_QUEUE[1]), msg);
			Assert.assertTrue("Unble to EnqueueWithRelay:" + ec, ec == Safmq.EC_NOERROR);
			
			Thread.sleep(100);
			
			QueueHandle handle = new QueueHandle();
			ec = con.OpenQueue(TEST_QUEUE[1], handle);
			Assert.assertTrue("Unble to OpenQueue:" + ec, ec == Safmq.EC_NOERROR);
			
			msg = new QueueMessage();
			ec = con.Retrieve(handle, true, 0, msg);
			Assert.assertTrue("Unble to Retrieve:" + ec, ec == Safmq.EC_NOERROR);
			String txt = (new BufferedReader(new InputStreamReader(msg.getInputStream()))).readLine();
			Assert.assertTrue("Incorrect Message: " + txt, txt.equals(SPECIAL_MESSAGE));
		
			ec = con.CloseQueue(handle);
			Assert.assertTrue("Unble to CloseQueue:" + ec, ec == Safmq.EC_NOERROR);

		} finally {
			if (con != null)
				con.Close();
		}
	}	
}

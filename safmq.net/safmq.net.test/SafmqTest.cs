using safmq.net;
using MbUnit.Framework;
using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;

namespace safmq.net.test {
    [TestFixture]
    public class SafmqTest{
	    static readonly string[] TEST_QUEUE = { "test", "foo", "bar", "thequeue", "notthequeue" };
        static readonly string[] TEST_USERS = { "bob", "mary", "pat", "charlie", "jack", "kris" };
        static readonly string[] TEST_PASSWD = TEST_USERS;
        static readonly string[] TEST_DESC = TEST_USERS;
        static readonly string[] TEST_GROUPS = { "them", "us", "others", "natives", "aliens" };
	    const string SPECIAL_MESSAGE = "special message";
    	
//	    string user = null;
//	    string password = null;

	    protected Uri address;
	    protected Uri simpleAddress;

        protected string addressString = "safmq://admin:@localhost:40000";

	    public SafmqTest() {
		    address = new Uri(addressString);
            simpleAddress = new Uri("safmq://localhost:40000");
	    }

	    MQConnection connectToServer(Uri uri, string user, string password) {
		    try {
			    MQConnection con = MQBuilder.BuildConnection(uri, user, password);
			    Assert.IsTrue(con != null, "Successfully Connected");
			    return con;
		    } catch (Exception e) {
                Assert.IsTrue(e == null, e.ToString());
		    }
		    return null;
	    }

	    void createEmptyQueue(MQConnection con, string queueName) {
		    ErrorCode ec;
		    ec = con.DeleteQueue(queueName);
            Assert.IsTrue((ec == ErrorCode.EC_NOERROR || ec == ErrorCode.EC_DOESNOTEXIST), "Failed to delete queue: " + queueName);

		    ec = con.CreateQueue(queueName);
            Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Failed to create queue: " + queueName);
	    }

	    void createNewUser(MQConnection con, string userName, string description, string password) {
		    ErrorCode ec = con.DeleteUser(userName);
            Assert.IsTrue((ec == ErrorCode.EC_NOERROR || ec == ErrorCode.EC_DOESNOTEXIST), "Failed to remove user prior to create: " + userName);
		    ec = con.CreateUser(userName, password, description);
            Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Failed to create user: " + userName);
	    }

	    void createNewGroup(MQConnection con, string groupName) {
		    ErrorCode ec = con.DeleteGroup(groupName);
            Assert.IsTrue((ec == ErrorCode.EC_NOERROR || ec == ErrorCode.EC_DOESNOTEXIST), "Failed to remove group prior to create: " + groupName);
		    ec = con.CreateGroup(groupName);
            Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Failed to create group: " + groupName);
	    }

	    void addAllUsers(MQConnection con) {
		    for (int x = 0; x < TEST_USERS.Length; x++) {
			    createNewUser(con, TEST_USERS[x], TEST_DESC[x], TEST_PASSWD[x]);
		    }
	    }

	    void addAllGroups(MQConnection con) {
		    for (int x = 0; x < TEST_GROUPS.Length; x++) {
			    createNewGroup(con, TEST_GROUPS[x]);
		    }
	    }

	    void addAllQueues(MQConnection con) {
		    for (int x = 0; x < TEST_QUEUE.Length; x++) {
			    createEmptyQueue(con, TEST_QUEUE[x]);
		    }
	    }

	    void sendMessages(MQConnection con, QueueHandle handle, int count, int idxStart) {
		    QueueMessage msg;
		    ErrorCode ec;
    		
		    for(int x=0;x<count;x++) {
			    msg = new QueueMessage();
			    StreamWriter w = new StreamWriter(msg.Stream);
    			
			    w.WriteLine((x + idxStart));
			    w.Close();
    			
			    ec = con.Enqueue(handle, msg);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to send message: " + (x + idxStart) + " ec: " + ec);
		    }
	    }
    	
	    // public int EnqueueWithRelay(Uri Uri, QueueMessage msg) {

	    // public int SetPassword(string username, string password) {


        [Test]
        [TestSequence(1)]
        public void test_CreateQueue()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
			    createEmptyQueue(con, TEST_QUEUE[0]);
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

		[Test]
		[TestSequence(50)]
		public void test_CreateTempQueue () {
			using (MQConnection con = connectToServer(address, null, null)) {
				String queueName = String.Empty;
				QueueHandle qh = new QueueHandle();

				// Try and create a temporary queue
				ErrorCode err = con.CreateTempQueue(out queueName, qh);
				Console.WriteLine("queueName:" + queueName);
				Assert.AreEqual(ErrorCode.EC_NOERROR, err, "Creating Queue");

				// Get permissions for that queue
				List<QueuePermissions> perms = new List<QueuePermissions>();
				con.QueueEnumeratePermissions(queueName, perms);
				for (int x = 0; x < perms.Count; ++x) {
					QueuePermissions perm = perms[x];
					Console.WriteLine(perm.EntityName + ":" + perm.Read + ":" + perm.Write + ":" + perm.Destroy + ":" + perm.ChangeSecurity);
				}
				Assert.IsTrue(perms.Count > 0);

				addAllUsers(con);

				// Try and write to that queue with another user.
				using (MQConnection con2 = connectToServer(simpleAddress, TEST_USERS[0], TEST_USERS[0])) {
					QueueHandle qh2 = new QueueHandle();
					ErrorCode rc = con2.OpenQueue(queueName, qh2);
					Assert.IsTrue(rc == ErrorCode.EC_NOERROR, "Open Temp Queue rc:" + rc);
					QueueMessage msg = new QueueMessage();
					msg.Label = "Hello World";
					Assert.IsTrue(ErrorCode.EC_NOERROR == con2.Enqueue(qh2, msg), "Enqueue to temp");
					con2.CloseQueue(qh2);
				}
			}
		}


        [Test]
        [TestSequence(2)]
        public void test_DeleteQueue()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
			    createEmptyQueue(con, TEST_QUEUE[0]);
			    ErrorCode ec = con.DeleteQueue(TEST_QUEUE[0]);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Failed to delete queue: " + TEST_QUEUE[0]);
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(3)]
        public void test_CreateUser()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
			    createNewUser(con, TEST_USERS[0], TEST_DESC[0], TEST_PASSWD[0]);
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(4)]
        public void test_SetPassword()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
			    createNewUser(con, TEST_USERS[0], TEST_DESC[0], TEST_PASSWD[0]);
    			
			    string newPwd = "new password";
			    ErrorCode ec = con.SetPassword(TEST_USERS[0], newPwd);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "SetPassword");
    			
			    con.Close();
			    con = connectToServer(simpleAddress, TEST_USERS[0], newPwd);
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }


        [Test]
        [TestSequence(5)]
        public void test_UserPermissions()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
			    createNewUser(con, TEST_USERS[0], TEST_DESC[0], TEST_PASSWD[0]);
			    ErrorCode ec;
			    for (int x = 7; x >= 0; x--) {
				    ec = con.UserSetPermissions(TEST_USERS[0], (x & 0x04) != 0, (x & 0x2) != 0, (x & 0x1) != 0);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Failed to set permissions for: " + TEST_USERS[0]);
				    ActorPermissions actorPerms = new ActorPermissions();
				    ec = con.UserGetPermissions(TEST_USERS[0], actorPerms);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Failed to retrive permissions for: " + TEST_USERS[0]);
                    Assert.IsTrue(actorPerms.ModifyQueues == ((x & 0x04) != 0), "Modify queues incorrectly set: " + x);
                    Assert.IsTrue(actorPerms.ModifyUsers == ((x & 0x02) != 0), "Modify users incorrectly set: " + x);
                    Assert.IsTrue(actorPerms.ModifyGroups == ((x & 0x01) != 0), "Modify groups incorrectly set: " + x);
			    }
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(6)]
        public void test_DeleteUser()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
			    createNewUser(con, TEST_USERS[0], TEST_DESC[0], TEST_PASSWD[0]);
			    ErrorCode ec = con.DeleteUser(TEST_USERS[0]);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Failed to delete user: " + TEST_USERS[0]);
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(7)]
        public void test_CreateGroup()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
			    createNewGroup(con, TEST_GROUPS[0]);
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(8)]
        public void test_GroupPermissions()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
			    createNewGroup(con, TEST_GROUPS[0]);
			    ErrorCode ec;
			    for (int x = 7; x >= 0; x--) {
				    ec = con.GroupSetPermissions(TEST_GROUPS[0], (x & 0x04) != 0, (x & 0x2) != 0, (x & 0x1) != 0);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Failed to set permissions for: " + TEST_GROUPS[0]);
				    ActorPermissions actorPerms = new ActorPermissions();
				    ec = con.GroupGetPermissions(TEST_GROUPS[0], actorPerms);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Failed to retrive permissions for: " + TEST_GROUPS[0]);
                    Assert.IsTrue(actorPerms.ModifyQueues == ((x & 0x04) != 0), "Modify queues incorrectly set: " + x);
                    Assert.IsTrue(actorPerms.ModifyUsers == ((x & 0x02) != 0), "Modify users incorrectly set: " + x);
                    Assert.IsTrue(actorPerms.ModifyGroups == ((x & 0x01) != 0), "Modify groups incorrectly set: " + x);
			    }
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(9)]
        public void test_DeleteGroup()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
			    createNewGroup(con, TEST_GROUPS[0]);
			    ErrorCode ec = con.DeleteGroup(TEST_GROUPS[0]);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Failed to delete group: " + TEST_GROUPS[0]);
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(10)]
        public void test_EnumerateQueues()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);

			    addAllQueues(con);
                List<QueueData> qnames = new List<QueueData>();
			    ErrorCode ec = con.EnumerateQueues(qnames);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Couldn't enumerate queues");

			    for (int x = 0; x < TEST_QUEUE.Length; x++) {
				    bool found = false;
				    for (int y = 0; y < qnames.Count; y++) {
					    QueueData qd = qnames[y];
					    if (qd.Name.Equals(TEST_QUEUE[x]))
						    found = true;
				    }
                    Assert.IsTrue(found, "Queue not enumerated: " + TEST_QUEUE[0]);
			    }
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(11)]
        public void test_EnumerateUsers()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);

			    addAllUsers(con);
                List<UserDescription> unames = new List<UserDescription>();
			    ErrorCode ec = con.EnumerateUsers(unames);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Couldn't enumerate users");

			    for (int x = 0; x < TEST_USERS.Length; x++) {
				    bool found = false;
				    for (int y = 0; y < unames.Count; y++) {
					    UserDescription ud = unames[y];
					    if (ud.Name.Equals(TEST_USERS[x]))
						    found = true;
				    }
                    Assert.IsTrue(found, "User not enumerated: " + TEST_USERS[0]);
			    }
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(12)]
        public void test_EnumerateGroups()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);

			    addAllGroups(con);
                List<string> gnames = new List<string>();
			    ErrorCode ec = con.EnumerateGroups(gnames);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Couldn't enumerate groups");

			    for (int x = 0; x < TEST_GROUPS.Length; x++) {
				    bool found = false;
				    for (int y = 0; y < gnames.Count; y++) {
					    string group = gnames[y];
					    if (group.Equals(TEST_GROUPS[x]))
						    found = true;
				    }
                    Assert.IsTrue(found, "Group not enumerated: " + TEST_GROUPS[0]);
			    }
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(13)]
        public void test_GroupAddUser_GroupDeleteUser_GroupGetUsers()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);

			    addAllGroups(con);
			    addAllUsers(con);

			    ErrorCode ec;
			    for (int x = 0; x < 3; x++) {
				    ec = con.GroupAddUser(TEST_GROUPS[0], TEST_USERS[x]);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to add user: " + TEST_USERS[x] + " to group: " + TEST_GROUPS[x]);
			    }

                List<string> users = new List<string>();
			    ec = con.GroupGetUsers(TEST_GROUPS[0], users);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to get users in group: " + TEST_GROUPS[0]);

			    for (int x = 0; x < 3; x++) {
                    Assert.IsTrue(users.Contains(TEST_USERS[x]), "User not found in group: " + TEST_USERS[x]);
			    }

			    ec = con.GroupDeleteUser(TEST_GROUPS[0], TEST_USERS[1]);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to delete user: " + TEST_USERS[1] + " from group: " + TEST_GROUPS[0]);

			    ec = con.GroupGetUsers(TEST_GROUPS[0], users);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to get users in group: " + TEST_GROUPS[0]);
                Assert.IsTrue(!users.Contains(TEST_USERS[1]), "User found in group after delete: " + TEST_USERS[1]);
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(14)]
        public void test_UserGetGroups()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);

			    addAllGroups(con);
			    addAllUsers(con);

			    ErrorCode ec;
			    for (int x = 0; x < 3; x++) {
				    ec = con.GroupAddUser(TEST_GROUPS[x], TEST_USERS[0]);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to add user: " + TEST_USERS[x] + " to group: " + TEST_GROUPS[x]);
			    }

                List<string> groups = new List<string>();
			    ec = con.UserGetGroups(TEST_USERS[0], groups);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to get groups for user: " + TEST_USERS[0]);

			    for (int x = 0; x < 3; x++) {
                    Assert.IsTrue(groups.Contains(TEST_GROUPS[x]), "Group not found: " + TEST_GROUPS[x] + " for user: " + TEST_USERS[0]);
			    }

			    ec = con.GroupDeleteUser(TEST_GROUPS[1], TEST_USERS[0]);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to delete user: " + TEST_USERS[1] + " from group: " + TEST_GROUPS[0]);

			    ec = con.UserGetGroups(TEST_GROUPS[0], groups);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to get groups for user: " + TEST_USERS[0]);
			    Assert.IsTrue(!groups.Contains(TEST_GROUPS[1]), "User found in group after delete: " + TEST_USERS[0]);
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }


        [Test]
        [TestSequence(15)]
        public void test_QueueSetUserPermission_QueueDeleteUserPermission_QueueEnumeratePermissions_QueueSetGroupPermission_QueueDeleteGroupPermission()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
    			
			    addAllGroups(con);
			    addAllUsers(con);
			    addAllQueues(con);
    			
			    ErrorCode ec;
			    for(int x=15;x>=0;x--) {
				    ec = con.QueueSetUserPermission(TEST_QUEUE[0], TEST_USERS[0], (x&0x08)!=0,(x&0x04)!=0,(x&0x02)!=0,(x&0x01)!=0);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to set user permissions: " + x);

				    ec = con.QueueSetGroupPermission(TEST_QUEUE[0], TEST_GROUPS[0], (x&0x08)!=0,(x&0x04)!=0,(x&0x02)!=0,(x&0x01)!=0);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to set group permissions: " + x);

                    List<QueuePermissions> perms = new List<QueuePermissions>();
				    ec = con.QueueEnumeratePermissions(TEST_QUEUE[0], perms);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to get permissions: " + TEST_QUEUE[0]);
    				
				    for(int y=0;y<perms.Count;y++) {
					    QueuePermissions perm = perms[y];
					    if(perm.IsGroup && perm.EntityName.Equals(TEST_GROUPS[0])) {
                            Assert.IsTrue(((x & 0x08) != 0) == perm.Read, "Group read perm incorrect");
                            Assert.IsTrue(((x & 0x04) != 0) == perm.Write, "Group wirte perm incorrect"); 
						    Assert.IsTrue(((x&0x02)!=0) == perm.Destroy, "Group destroy perm incorrect"); 
						    Assert.IsTrue(((x&0x01)!=0) == perm.ChangeSecurity, "Group change security perm incorrect"); 
					    } else if (!perm.IsGroup && perm.EntityName.Equals(TEST_USERS[0])) {
						    Assert.IsTrue(((x&0x08)!=0) == perm.Read, "user read perm incorrect"); 
						    Assert.IsTrue(((x&0x04)!=0) == perm.Write, "user wirte perm incorrect"); 
						    Assert.IsTrue(((x&0x02)!=0) == perm.Destroy, "user destroy perm incorrect"); 
						    Assert.IsTrue(((x&0x01)!=0) == perm.ChangeSecurity, "user change security perm incorrect"); 
					    }
				    }
			    }
    			
			    ec = con.QueueDeleteGroupPermission(TEST_QUEUE[0], TEST_GROUPS[0]);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "unable to delete permission for group:" + TEST_GROUPS[0]);

			    ec = con.QueueDeleteGroupPermission(TEST_QUEUE[0], TEST_GROUPS[0]);
			    Assert.IsFalse(ec == ErrorCode.EC_NOERROR, "able to delete permission for group:" + TEST_GROUPS[0]);
    			
			    ec = con.QueueDeleteUserPermission(TEST_QUEUE[0], TEST_USERS[0]);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "unable to delete permission for user:" + TEST_USERS[0]);

			    ec = con.QueueDeleteUserPermission(TEST_QUEUE[0], TEST_USERS[0]);
			    Assert.IsFalse(ec == ErrorCode.EC_NOERROR, "able to delete permission for user:" + TEST_USERS[0]);
    			
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(16)]
        public void test_OpenQueue_Enqueue_Retrieve_CloseQueue()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
    			
			    addAllGroups(con);
			    addAllUsers(con);
			    addAllQueues(con);

			    QueueHandle handle = new QueueHandle();
			    ErrorCode ec = con.OpenQueue(TEST_QUEUE[0], handle);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to open queue: " + TEST_QUEUE[0]);
    			
			    sendMessages(con, handle, 5, 0);
    			
			    for(int x=0;x<5;x++) {
				    QueueMessage msg = new QueueMessage();
				    ec = con.Retrieve(handle, true, 0, msg);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to retrieve message: " + x);
    			
				    StreamReader r = new StreamReader(msg.Stream);
				    string s = r.ReadLine();

                    Assert.IsTrue(int.Parse(s) == x, "Incorrect message retrieved: " + x);
			    }
    			
			    ec = con.CloseQueue(handle);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to Close queue");
    			
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }


        [Test]
        [TestSequence(17)]
        public void test_PeekID_PeekFront_OpenCursor_PeekCursor_SeekID_RetrieveCursor_AdvanceCursor_TestCurosr_CloseCursor_RetrieveID()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
    			
			    addAllGroups(con);
			    addAllUsers(con);
			    addAllQueues(con);

			    QueueHandle handle = new QueueHandle();
			    ErrorCode ec = con.OpenQueue(TEST_QUEUE[0], handle);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to open queue: " + TEST_QUEUE[0]);
    			
			    sendMessages(con, handle, 5, 0);
			    QueueMessage msg = new QueueMessage();
			    StreamWriter sw = new StreamWriter(msg.Stream);
                sw.WriteLine(SPECIAL_MESSAGE);
                sw.Close();

                byte[] b = { 0, 0, 0, 0, 0, 0, 0, 0 };
                UUID uuid = new UUID(new Guid(123456789,4321,1234,b));
                msg.ReciptID = uuid;
			    //msg.ReciptID = ((UUID)uuid.Clone());
    			
			    ec = con.Enqueue(handle, msg);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to write special message");
    			
			    sendMessages(con, handle, 5, 5);
    			
			    msg = new QueueMessage();
			    ec = con.PeekID(handle, true, uuid, 0, msg);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to PeekID:" + ec);

			    string txt = (new StreamReader(msg.Stream)).ReadLine();
                Console.WriteLine("txt:\t\t\t" + txt);
                Console.WriteLine("uuid:\t\t\t" + (Guid)uuid);
                Console.WriteLine("msg.ReceiptID:\t" + (Guid)msg.ReciptID);
                Console.WriteLine("txt.Equals(SPECIAL_MESSAGE): " + txt.Equals(SPECIAL_MESSAGE));
                Console.WriteLine("uuid.Equals(msg.ReceiptID)): " + uuid.Equals(msg.ReciptID));
                Assert.IsTrue(txt.Equals(SPECIAL_MESSAGE) && uuid.Equals(msg.ReciptID), "Incorrect PeekID Message: " + (Guid)msg.ReciptID);
    			
			    msg = new QueueMessage();
			    ec = con.PeekFront(handle, true, 0, msg);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to PeekFront");
			    txt = (new StreamReader(msg.Stream)).ReadLine();
			    Assert.IsTrue(txt.Equals("0"), "Incorrect PeekFront Message: body=" + txt);
    			
			    CursorHandle cur = new CursorHandle();

			    ec = con.OpenCursor(handle, cur);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to OpenCursor");
    			
			    ec = con.SeekID(handle, uuid, 0, cur);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to SeekID");
			    msg = new QueueMessage();
			    ec = con.PeekCursor(handle, true, cur, msg);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to PeekCursor");
			    txt = (new StreamReader(msg.Stream)).ReadLine();
			    Assert.IsTrue(txt.Equals(SPECIAL_MESSAGE) && uuid.Equals(msg.ReciptID), "Incorrect PeekCursor Message: " + msg.ReciptID);

			    ec = con.AdvanceCursor(handle, cur);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to AdvanceCursor");
			    msg = new QueueMessage();
			    ec = con.PeekCursor(handle, true, cur, msg);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to PeekCursor (after advance)");
			    txt = (new StreamReader(msg.Stream)).ReadLine();
			    Assert.IsTrue(txt.Equals("5"), "Incorrect PeekCursor (after advance) Message: " + msg.ReciptID);
    		
			    msg = new QueueMessage();
			    ec = con.RetrieveCursor(handle, true, cur, msg);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to RetrieveCursor");
			    txt = (new StreamReader(msg.Stream)).ReadLine();
			    Assert.IsTrue(txt.Equals("5"), "Incorrect PeekCursor (after RetrieveCursor) Message: " + msg.ReciptID);
    			
			    ec = con.TestCursor(handle, cur);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to test Cursor, ec: " + ec);
    			
			    ec = con.CloseCursor(handle, cur);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to CloseCursor, ec: " + ec);
    			
    			
			    msg = new QueueMessage();
			    ec = con.RetrieveID(handle, true, uuid, 0, msg);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to RetrieveID");
			    txt = (new StreamReader(msg.Stream)).ReadLine();
			    Assert.IsTrue(txt.Equals(SPECIAL_MESSAGE) && uuid.Equals(msg.ReciptID), "Incorrect RetrieveID Message: " + msg.ReciptID);

                int[] bodies = { 0, 1, 2, 3, 4, 6, 7, 8, 9 }; // 5 was retrieved above.
			    for(int x=0; x<bodies.Length; x++) {
				    msg = new QueueMessage();
				    ec = con.Retrieve(handle, true, 0, msg);
				    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to retrieve message: " + x);
				    txt = (new StreamReader(msg.Stream)).ReadLine();
                    Console.WriteLine("txt: " + txt);
				    Assert.IsTrue(int.Parse(txt) == bodies[x], "Incorrect message: " + txt);
			    }
    			
    			
			    ec = con.CloseQueue(handle);
			    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to Close queue");
    			
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(18)]
        public void test_Transactions()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
    			
			    addAllGroups(con);
			    addAllUsers(con);
			    addAllQueues(con);

			    QueueHandle handle = new QueueHandle();
			    ErrorCode ec = con.OpenQueue(TEST_QUEUE[0], handle);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to open queue: " + TEST_QUEUE[0]);
    			
			    sendMessages(con, handle, 10, 0);

			    ec = con.BeginTransaction();
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to BeginTransaction:" + ec);
			    CursorHandle cur = new CursorHandle();
    			
			    ec = con.OpenCursor(handle, cur);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to OpenCursor:" + ec);
    			
			    ec = con.AdvanceCursor(handle, cur);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to AdvanceCursor:" + ec);
			    ec = con.AdvanceCursor(handle, cur);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to AdvanceCursor:" + ec);

			    QueueMessage msg = new QueueMessage();
			    ec = con.RetrieveCursor(handle, true, cur, msg);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to RetrieveCursor:" + ec);
			    string txt = (new StreamReader(msg.Stream)).ReadLine();
                Assert.IsTrue(txt.Equals("2"), "Incorrect Message: " + txt + " should be 2");
    			
			    msg = new QueueMessage();
			    ec = con.RetrieveCursor(handle, true, cur, msg);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to RetrieveCursor:" + ec);
			    txt = (new StreamReader(msg.Stream)).ReadLine();
                Assert.IsTrue(txt.Equals("3"), "Incorrect Message: " + txt + " should be 3");
    		
			    msg = new QueueMessage();
			    ec = con.Retrieve(handle, true, 0, msg);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to RetrieveCursor:" + ec);
			    txt = (new StreamReader(msg.Stream)).ReadLine();
                Assert.IsTrue(txt.Equals("0"), "Incorrect Message: " + txt + " should be 0");

                int[] ids = { 1, 4, 5, 6, 7, 8, 9 };
			    for(int x=0;x<ids.Length;x++) {
				    msg = new QueueMessage();
				    ec = con.Retrieve(handle, true, 0, msg);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to RetrieveCursor:" + ec);
				    txt = (new StreamReader(msg.Stream)).ReadLine();
                    Console.WriteLine("txt: " + txt);
                    Assert.IsTrue(int.Parse(txt) == ids[x], "Incorrect Message: " + txt + " should be " + ids[x]);
			    }
    			
			    ec = con.RollbackTransaction();
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to RollbackTransaction:" + ec);
    			
			    for(int x=0;x<8;x++) {
				    msg = new QueueMessage();
				    ec = con.Retrieve(handle, true, 0, msg);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to RetrieveCursor:" + ec);
				    txt = (new StreamReader(msg.Stream)).ReadLine();
                    Assert.IsTrue(int.Parse(txt) == x, "Incorrect Message: " + txt + " should be " + x);
			    }
    			
			    ec = con.CommitTransaction();
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to CommitTransaction:" + ec);
    			
			    for(int x=0;x<2;x++) {
				    msg = new QueueMessage();
				    ec = con.Retrieve(handle, true, 0, msg);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to RetrieveCursor:" + ec);
				    txt = (new StreamReader(msg.Stream)).ReadLine();
                    Assert.IsTrue(int.Parse(txt) == x + 8, "Incorrect Message: " + txt + " should be " + (x + 8));
			    }
    			
			    ec = con.RollbackTransaction();
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to RollbackTransaction:" + ec);
    			
			    ec = con.EndTransaction();
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to RollbackTransaction:" + ec);
    			
			    for(int x=0;x<2;x++) {
				    msg = new QueueMessage();
				    ec = con.Retrieve(handle, true, 0, msg);
                    Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unable to RetrieveCursor:" + ec);
				    txt = (new StreamReader(msg.Stream)).ReadLine();
                    Assert.IsTrue(int.Parse(txt) == x + 8, "Incorrect Message: " + txt + " should be " + (x + 8));
			    }

			    msg = new QueueMessage();
			    ec = con.Retrieve(handle, true, 0, msg);
                Assert.IsTrue(ec == ErrorCode.EC_NOMOREMESSAGES, "Able to RetrieveCursor:" + ec);
    			
			    ec = con.CloseQueue(handle);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unble to CloseQueue:" + ec);
		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }

        [Test]
        [TestSequence(19)]
        public void test_EnqueueWithRelay()
        {
		    MQConnection con = null;
		    try {
			    con = connectToServer(address, null, null);
			    QueueMessage msg = new QueueMessage();
			    StreamWriter sw = new StreamWriter(msg.Stream);
                sw.WriteLine(SPECIAL_MESSAGE);
                sw.Close();

			    addAllQueues(con);
			    ErrorCode ec = con.EnqueueWithRelay(new Uri(addressString + "/" + TEST_QUEUE[1]), msg);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unble to EnqueueWithRelay:" + ec);
    			
			    Thread.Sleep(100);
    			
			    QueueHandle handle = new QueueHandle();
			    ec = con.OpenQueue(TEST_QUEUE[1], handle);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unble to OpenQueue:" + ec);
    			
			    msg = new QueueMessage();
			    ec = con.Retrieve(handle, true, 0, msg);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unble to Retrieve:" + ec);
			    string txt = (new StreamReader(msg.Stream)).ReadLine();
                Assert.IsTrue(txt.Equals(SPECIAL_MESSAGE), "Incorrect Message: " + txt);
    		
			    ec = con.CloseQueue(handle);
                Assert.IsTrue(ec == ErrorCode.EC_NOERROR, "Unble to CloseQueue:" + ec);

		    } finally {
			    if (con != null)
				    con.Close();
		    }
	    }	
    }
}
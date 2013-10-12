#!/usr/bin/php -d extension_dir=modules/
<?php
dl('safmq.so');
include_once 'Assert.php';
include_once 'TestCase.php';

	$TEST_QUEUE = array("test", "foo", "bar", "thequeue", "notthequeue");
	$TEST_USERS = array("bob", "mary", "pat", "charlie", "jack", "kris");
	$TEST_PASSWD = $TEST_USERS;
	$TEST_DESC = $TEST_USERS;
	$TEST_GROUPS = array("them", "us", "others", "natives", "aliens");
	$SPECIAL_MESSAGE = "special message";

	$address = "safmq://admin:@localhost:40000";
	$oaddress = $address;
	$simpleAddress = "safmq://localhost:40000";
	
	
	//////////////////////////////////////////////////////////////////////
	// Utility Functions
	//////////////////////////////////////////////////////////////////////

	function guid(){
//	    if (function_exists('com_create_guid')){
//	        return com_create_guid();
//	    }else{
	        mt_srand((double)microtime()*10000);//optional for php 4.2.0 and up.
	        $charid = strtoupper(md5(uniqid(rand(), true)));
	        $hyphen = chr(45);// "-"
//	        $uuid = chr(123)// "{"
//	                .substr($charid, 0, 8).$hyphen
//	                .substr($charid, 8, 4).$hyphen
//	                .substr($charid,12, 4).$hyphen
//	                .substr($charid,16, 4).$hyphen
//	                .substr($charid,20,12)
//	                .chr(125);// "}"
	        $uuid = substr($charid, 0, 8).$hyphen
	                .substr($charid, 8, 4).$hyphen
	                .substr($charid,12, 4).$hyphen
	                .substr($charid,16, 4).$hyphen
	                .substr($charid,20,12);
	        return $uuid;
//	    }
	}
	
	
	
	function contains($a, $v) {
		$found = FALSE;
		for($i = 0; !$found && $i < count($a); $i++)
			$found = $a[$i] == $v ? TRUE : FALSE;
		return $found;	
	}
	
	function connectToServer($uri, $user, $password) {
		$con = safmq_open($uri, $user, $password);
		Assert::_True("Failed to Connect: $uri:$user:$password", $con !== FALSE);
		return $con;
	}

	function createEmptyQueue($con, $queueName) {
		global $safmq_ec_message;
		//echo "queueName: ".$queueName."\n";
		//print_r($con);
		$ret = safmq_delete_queue($queueName, $con); // don't care if it's deleted or not, it may not be there.
		Assert::_True("Failed to delete queue prior to recreation: ". $queueName . " ec: " . $safmq_ec_message, $ret === TRUE || $safmq_ec_message == "EC_DOESNOTEXIST" );
		Assert::_True("Failed to create queue: " . $queueName, safmq_create_queue($queueName, $con));
	}

	function createNewUser($con, $userName, $description, $password) {
		global $safmq_ec_message;
		$ret = safmq_delete_user($userName, $con);
		Assert::_True("Failed to delete user prior to recreation: " . $userName . " ec: " . $safmq_ec_message, $ret === TRUE || $safmq_ec_message == "EC_DOESNOTEXIST");
		Assert::_True("Failed to create user: " . $userName, safmq_create_user($userName, $password, $description, $con));
	}

	function createNewGroup($con, $groupName) {
		global $safmq_ec_message;
		$ret = safmq_delete_group($groupName);
		Assert::_True("Failed to delete group prior to recreation" . $groupName . " ec: " . $safmq_ec_message, $ret === TRUE || $safmq_ec_message == "EC_DOESNOTEXIST");
		Assert::_True("Failed to create group: " . $groupName, safmq_create_group($groupName));
	}

	function addAllUsers($con) {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		
		for ($x = 0; $x < count($TEST_USERS); $x++) {
			createNewUser($con, $TEST_USERS[$x], $TEST_DESC[$x], $TEST_PASSWD[$x]);
		}
	}

	function addAllGroups($con) {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		for ($x = 0; $x < count($TEST_GROUPS); $x++) {
			createNewGroup($con, $TEST_GROUPS[$x]);
		}
	}

	function addAllQueues($con) {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		for ($x = 0; $x < count($TEST_QUEUE); $x++) {
			createEmptyQueue($con, $TEST_QUEUE[$x]);
		}
	}

	function sendMessages($con, $handle, $count, $idxStart) {
		for($x = 0; $x < $count; $x++) {
			$msg = safmq_msg();
			$msg->Body = "".($x + $idxStart);  // force this to be text.  a numeric zero becomes a string null
			Assert::_True("Unable to send message: " . ($x + $idxStart), safmq_enqueue($msg, $handle, $con));
		}
	}
	
	//////////////////////////////////////////////////////////////////////
	// Test Cases
	//////////////////////////////////////////////////////////////////////
	function test_CreateQueue() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		$con = connectToServer($address, null, null);
		createEmptyQueue($con, $TEST_QUEUE[0]);
	}
	
	function test_DeleteQueue() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		$con = connectToServer($address, null, null);
		createEmptyQueue($con, $TEST_QUEUE[0]);
		Assert::_True("Failed to delete queue: " . $TEST_QUEUE[0], safmq_delete_queue($TEST_QUEUE[0], $con));
	}

	function test_CreateUser() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		$con = connectToServer($address, null, null);
		createNewUser($con, $TEST_USERS[0], $TEST_DESC[0], $TEST_PASSWD[0]);
	}
	
	function test_SetPassword() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		$con = connectToServer($address, null, null);
		createNewUser($con, $TEST_USERS[0], $TEST_DESC[0], $TEST_PASSWD[0]);
			
		$newPwd = "new password";
		Assert::_True("SetPassword", safmq_Set_password($TEST_USERS[0], $newPwd, $con));
			
		safmq_close($con);
		connectToServer($simpleAddress, $TEST_USERS[0], $newPwd);
	}


	function test_CreateTempQueue() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		
		$con = connectToServer($address, null, null);
		
		// Try and create a temporary queue
		$tmp = safmq_create_temp_queue($con);
 		Assert::_True("Creating Queue", $tmp !== FALSE);
		
		// Get permissions for that queue
		$perms = safmq_queue_enumerate_permissions($tmp->queue_name, $con);
		Assert::_TRUE("Unable to load permissions", $perms !== FALSE);

		addAllUsers($con);

		// Try and write to that queue with another user.
		$con2 = connectToServer($simpleAddress, $TEST_USERS[0], $TEST_PASSWD[0]);
		$qh2 = safmq_open_queue($tmp->queue_name, $con2);
		Assert::_True("Open Temp Queue", $qh2 !== FALSE);

		sendMessages($con2, $qh2, 1, 0);

		safmq_close_queue($qh2);
		safmq_close($con2);
		
	}
	

	function test_UserPermissions() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		
		$con = connectToServer($address, null, null);
		createNewUser($con, $TEST_USERS[0], $TEST_DESC[0], $TEST_PASSWD[0]);

		for ($x = 7; $x >= 0; $x--) {
			$ec = safmq_user_set_permissions($TEST_USERS[0], ($x & 0x04) != 0, ($x & 0x2) != 0, ($x & 0x1) != 0, $con);
			Assert::_True("Failed to set permissions for: " . $TEST_USERS[0], $ec !== FALSE);
			$actorPerms = safmq_user_get_permissions($TEST_USERS[0], $con);
			Assert::_True("Failed to retrive permissions for: " . $TEST_USERS[0], $actorPerms !== FALSE);
			Assert::_True("Modify queues incorrectly set: " . $x, $actorPerms->ModifyQueues == (($x & 0x04) != 0));
			Assert::_True("Modify users incorrectly set: " . $x, $actorPerms->ModifyUsers == (($x & 0x02) != 0));
			Assert::_True("Modify groups incorrectly set: " . $x, $actorPerms->ModifyGroups == (($x & 0x01) != 0));
		}
	}

  	function test_DeleteUser() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
  		
		$con = connectToServer($address, null, null);
		createNewUser($con, $TEST_USERS[0], $TEST_DESC[0], $TEST_PASSWD[0]);
		$ec = safmq_delete_user($TEST_USERS[0]);
		Assert::_True("Failed to delete user: " . $TEST_USERS[0], $ec !== FALSE);
	}
	
	function test_CreateGroup() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		
		$con = connectToServer($address, null, null);
		createNewGroup($con, $TEST_GROUPS[0]);
	}

	function test_GroupPermissions() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;

		$con = connectToServer($address, null, null);
		createNewGroup($con, $TEST_GROUPS[0]);
		for ( $x = 7; $x >= 0; $x--) {
			$ec = safmq_group_set_permissions($TEST_GROUPS[0], ($x & 0x04) != 0, ($x & 0x2) != 0, ($x & 0x1) != 0, $con);
			Assert::_True("Failed to set permissions for: " . $TEST_GROUPS[0], $ec !== FALSE);
			$actorPerms = safmq_group_get_permissions($TEST_GROUPS[0], $con);
			Assert::_True("Failed to retrive permissions for: " . $TEST_GROUPS[0], $actorPerms !== FALSE);
			Assert::_True("Modify queues incorrectly set: " . $x, $actorPerms->ModifyQueues == (($x & 0x04) != 0));
			Assert::_True("Modify users incorrectly set: " . $x, $actorPerms->ModifyUsers == (($x & 0x02) != 0));
			Assert::_True("Modify groups incorrectly set: " . $x, $actorPerms->ModifyGroups == (($x & 0x01) != 0));
		}
	}

	function test_DeleteGroup() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		$con = connectToServer($address, null, null);
		createNewGroup($con, $TEST_GROUPS[0]);
		$ec = safmq_delete_group($TEST_GROUPS[0], $con);
		Assert::_True("Failed to delete group: " . $TEST_GROUPS[0], $ec !== FALSE);
	}

	function test_EnumerateQueues() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		$con = connectToServer($address, null, null);

		addAllQueues($con);
		$qnames = safmq_enumerate_queues($con);
		Assert::_True("Couldn't enumerate queues", $qnames !== FALSE);

		for ($x = 0; $x < count($TEST_QUEUE); $x++) {
			$found = FALSE;
			for ($y = 0; !$found && $y < count($qnames); $y++) {
				if ($qnames[$y] == $TEST_QUEUE[$x])
					$found = TRUE;
			}
			Assert::_True("Queue not enumerated: " . $TEST_QUEUE[$x], $found);
		}
	}

	function test_EnumerateUsers() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		$con = connectToServer($address, null, null);

		addAllUsers($con);
		
		$unames = safmq_enumerate_users($con);
		Assert::_True("Couldn't enumerate users", $unames !== FALSE);

		for ($x = 0; $x < count($TEST_USERS); $x++) {
			$found = false;
			for($y = 0; !$found && $y < count($unames); $y++)
				$found = $unames[$y]->name == $TEST_USERS[$x];
			
			Assert::_True("User not enumerated: " . $TEST_USERS[0], $found);
		}
	}

	function test_EnumerateGroups() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		$con = connectToServer($address, null, null);

		addAllGroups($con);
		$gnames = safmq_enumerate_groups($con);
		Assert::_True("Couldn't enumerate groups", $gnames !== FALSE);

		for ($x = 0; $x < count($TEST_GROUPS); $x++) {
			Assert::_True("Group not enumerated: " . $TEST_GROUPS[0], contains($gnames, $TEST_GROUPS[$x]));
		}
	}

	function test_GroupAddUser_GroupDeleteUser_GroupGetUsers() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		$con = connectToServer($address, null, null);

		addAllGroups($con);
		addAllUsers($con);

		for ($x = 0; $x < 3; $x++) {
			$ec = safmq_group_add_user($TEST_GROUPS[0], $TEST_USERS[$x], $con);
			Assert::_True("Unable to add user: " . $TEST_USERS[$x] . " to group: " . $TEST_GROUPS[$x], $ec !== FALSE);
		}

		$users = safmq_group_enumerate_users($TEST_GROUPS[0], $con);
		Assert::_True("Unable to get users in group: " . $TEST_GROUPS[0], $users !== FALSE);

		for ($x = 0; $x < 3; $x++) {
			Assert::_True("User not found in group: " . $TEST_USERS[$x], contains($users, $TEST_USERS[$x]));
		}

		$ec = safmq_group_delete_user($TEST_GROUPS[0], $TEST_USERS[1], $con);
		Assert::_True("Unable to delete user: " . $TEST_USERS[1] . " from group: " . $TEST_GROUPS[0], $ec !== FALSE);

		$users = safmq_group_enumerate_users($TEST_GROUPS[0], $con);
		Assert::_True("Unable to get users in group: " . $TEST_GROUPS[0], $users !== FALSE);
		
		Assert::_True("User found in group after delete: " . $TEST_USERS[1], !contains($users, $TEST_USERS[1]));
	}

	function test_UserGetGroups() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		$con = connectToServer($address, null, null);

		addAllGroups($con);
		addAllUsers($con);

		for ($x = 0; $x < 3; $x++) {
			$ec = safmq_group_add_user($TEST_GROUPS[$x], $TEST_USERS[0], $con);
			Assert::_True("Unable to add user: " . $TEST_USERS[$x] . " to group: " . $TEST_GROUPS[$x], $ec !== FALSE);
		}

		$groups = safmq_user_enumerate_groups($TEST_USERS[0], $con);
		Assert::_True("Unable to get groups for user: " . $TEST_USERS[0], $groups !== FALSE);

		for ($x = 0; $x < 3; $x++) {
			Assert::_True("Group not found: " . $TEST_GROUPS[$x] . " for user: " . $TEST_USERS[0], contains($groups, $TEST_GROUPS[$x]));
		}

		$ec = safmq_group_delete_user($TEST_GROUPS[1], $TEST_USERS[0], $con);
		Assert::_True("Unable to delete user: " . $TEST_USERS[1] . " from group: " . $TEST_GROUPS[0], $ec !== FALSE);

		$groups = safmq_user_enumerate_groups($TEST_GROUPS[0], $con);
		Assert::_True("Unable to get groups for user: " . $TEST_USERS[0], $groups !== FALSE);
		Assert::_True("User found in group after delete: " . $TEST_USERS[0], !contains($groups, $TEST_GROUPS[1]));
	}
	
	
	function test_QueueSetUserPermission_QueueDeleteUserPermission_QueueEnumeratePermissions_QueueSetGroupPermission_QueueDeleteGroupPermission() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
			$con = connectToServer($address, null, null);
			
			addAllGroups($con);
			addAllUsers($con);
			addAllQueues($con);
			
			for($x = 15; $x >= 0; $x--) {
				$ec = safmq_queue_set_user_permission($TEST_QUEUE[0], $TEST_USERS[0], ($x & 0x08) != 0, ($x & 0x04) != 0, ($x & 0x02) != 0, ($x & 0x01) != 0, $con);
				Assert::_True("Unable to set user permissions: " . $x, $ec !== FALSE);

				$ec = safmq_queue_set_group_permission($TEST_QUEUE[0], $TEST_GROUPS[0], ($x & 0x08) != 0, ($x & 0x04) != 0, ($x & 0x02) != 0, ($x & 0x01) != 0, $con);
				Assert::_True("Unable to set group permissions: " . $x, $ec !== FALSE);
				
				$perms = safmq_queue_enumerate_permissions($TEST_QUEUE[0], $con);
				Assert::_True("Unable to get permissions: " . $TEST_QUEUE[0], $perms !== FALSE);
				
				for($y = 0; $y < count($perms); $y++) {
					
					if($perms[$y]->IsGroup && $perms[$y]->Entity == $TEST_GROUPS[0]) {
						Assert::_True("Group read perm incorrect", (($x & 0x08) != 0) == $perms[$y]->Read); 
						Assert::_True("Group wirte perm incorrect", (($x & 0x04) != 0) == $perms[$y]->Write); 
						Assert::_True("Group destroy perm incorrect", (($x & 0x02) != 0 ) == $perms[$y]->Destroy); 
						Assert::_True("Group change security perm incorrect", (($x & 0x01) != 0) == $perms[$y]->ChangeSecurity); 
					} else if (!$perms[$y]->IsGroup && $perms[$y]->Entity == $TEST_USERS[0]) {
						Assert::_True("user read perm incorrect", (($x & 0x08) != 0) == $perms[$y]->Read); 
						Assert::_True("user wirte perm incorrect", (($x & 0x04) != 0) == $perms[$y]->Write); 
						Assert::_True("user destroy perm incorrect", (($x & 0x02) != 0) == $perms[$y]->Destroy); 
						Assert::_True("user change security perm incorrect", (($x & 0x01) != 0) == $perms[$y]->ChangeSecurity); 
					}
				}
			}
			
			$ec = safmq_queue_delete_group_permission($TEST_QUEUE[0], $TEST_GROUPS[0], $con);
			Assert::_True("unable to delete permission for group:" . $TEST_GROUPS[0], $ec !== FALSE);

			$ec = safmq_queue_delete_group_permission($TEST_QUEUE[0], $TEST_GROUPS[0], $con);
			Assert::_True("able to delete permission for group:" . $TEST_GROUPS[0], $ec === FALSE);
			
			$ec = safmq_queue_delete_user_permission($TEST_QUEUE[0], $TEST_USERS[0], $con);
			Assert::_True("unable to delete permission for user:" . $TEST_USERS[0], $ec !== FALSE);

			$ec = safmq_queue_delete_user_permission($TEST_QUEUE[0], $TEST_USERS[0], $con);
			Assert::_True("able to delete permission for user:" . $TEST_USERS[0], $ec === FALSE);
	}

	
	function test_OpenQueue_Enqueue_Retrieve_CloseQueue() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		$con = connectToServer($address, null, null);
		
		addAllGroups($con);
		addAllUsers($con);
		addAllQueues($con);
		
		$handle = safmq_open_queue($TEST_QUEUE[0], $con);
		Assert::_True("Unable to open queue: " . $TEST_QUEUE[0], $handle !== FALSE);
		
		sendMessages($con, $handle, 5, 0);
		
		for($x = 0; $x < 5; $x++) {
			echo ".";
			$msg = safmq_retrieve(true, 0, $handle, $con);
			Assert::_True("Unable to retrieve message: " . $x, $msg !== FALSE);
			Assert::_True("Incorrect message retrieved: " . $x, $msg->Body == $x);
		}
		
		$ec = safmq_close_queue($handle);
		Assert::_True("Unable to close queue", $ec !== FALSE);
		$ec = safmq_delete_queue($TEST_QUEUE[0]);
		Assert::_True("Unable to delete queue", $ec !== FALSE);
	}	
	

	function test_PeekID_PeekFront_OpenCursor_PeekCursor_SeekID_RetrieveCursor_AdvanceCursor_TestCurosr_CloseCursor_RetrieveID() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;

		$con = connectToServer($address, null, null);
		
		addAllGroups($con);
		addAllUsers($con);
		addAllQueues($con);

		$handle = safmq_open_queue($TEST_QUEUE[0], $con);
		Assert::_True("Unable to open queue: " . $TEST_QUEUE[0], $handle !== FALSE);
		
		sendMessages($con, $handle, 5, 0);
		$msg = safmq_msg();
		$msg->Body = $SPECIAL_MESSAGE;
		$uuid = guid(); 
		$msg->ReciptID = $uuid;
		
		$ec = safmq_enqueue($msg, $handle, $con);
		Assert::_True("Unable to write special message", $ec !== FALSE);
		
		sendMessages($con, $handle, 5, 5);
		
		$msg = safmq_peek_id($uuid, true, 0, $handle, $con);
		Assert::_True("Unable to PeekID", $msg !== FALSE);
		Assert::_True("Incorrect PeekID Message: " . strtoupper($msg->ReciptID). ";" . strtoupper($uuid) . " body: |" . $msg->Body . "|", ($msg->Body == $SPECIAL_MESSAGE) && (strtoupper($uuid) == strtoupper($msg->ReciptID)));
		
		$msg = safmq_peek_front(true, 0, $handle, $con);
		Assert::_True("Unable to PeekFront", $ec !== FALSE);
		Assert::_True("Incorrect PeekFront Message: body=" . $msg->Body, $msg->Body == "0");
		
		$cur = safmq_open_cursor($handle, $con);
		Assert::_True("Unable to OpenCursor", $cur!== FALSE);
		
		$ec = safmq_seek_id($uuid, 0, $cur, $handle, $con);
		Assert::_True("Unable to SeekID", $ec !== FALSE);
		
		$msg = safmq_peek_cursor(true, $cur, $handle, $con);
		Assert::_True("Unable to PeekCursor", $msg !== FALSE);
		Assert::_True("Incorrect PeekCursor Message: " . $msg->ReciptID, $msg->Body == $SPECIAL_MESSAGE && strtoupper($uuid) == strtoupper($msg->ReciptID));

		$ec = safmq_advance_cursor($cur, $handle, $con);
		Assert::_True("Unable to AdvanceCursor", $ec !== FALSE);

		$msg = safmq_peek_cursor(true, $cur, $handle, $con);
		Assert::_True("Unable to PeekCursor (after advance)", $ec !== FALSE);
		Assert::_True("Incorrect PeekCursor (after advance) Message: " . $msg->Body, $msg->Body == "5");
	
		$msg = safmq_retrieve_cursor(true, $cur, $handle, $con);
		Assert::_True("Unable to RetrieveCursor", $msg !== FALSE);
		Assert::_True("Incorrect PeekCursor (after RetrieveCursor) Message: " . $msg->ReciptID, $msg->Body == "5");
		
		$ec = safmq_test_cursor($cur, $handle, $con);
		Assert::_True("Unable to test Cursor, ec: " . $ec, $ec !== FALSE);
		
		$ec = safmq_close_cursor($cur, $handle, $con);
		Assert::_True("Unable to CloseCursor, ec: " . $ec, $ec !== FALSE);
		
		
		$msg = safmq_retrieve_id($uuid, true, 0, $handle, $con);
		Assert::_True("Unable to RetrieveID", $msg !== FALSE);
		Assert::_True("Incorrect RetrieveID Message: " . $msg->ReciptID, $msg->Body == $SPECIAL_MESSAGE && strtoupper($uuid) == strtoupper($msg->ReciptID));
		
		$bodies = array(0,1,2,3,4,6,7,8,9); // 5 was retrieved above.
		for($x = 0; $x < count($bodies); $x++) {
			$msg = safmq_retrieve(true, 0, $handle, $con);
			Assert::_True("Unable to retrieve message: " . $x, $msg !== FALSE);
			Assert::_True("Incorrect message: " . $msg->Body, $msg->Body == $bodies[$x]);
		}
		
		
		$ec = safmq_close_queue($handle, $con);
		Assert::_True("Unable to close queue", $ec !== FALSE);
		
	}	
	
	function test_Transactions() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress;
		$con = connectToServer($address, null, null);
		
		addAllGroups($con);
		addAllUsers($con);
		addAllQueues($con);

		$handle = safmq_open_queue($TEST_QUEUE[0], $con);
		Assert::_True("Unable to open queue: " . $TEST_QUEUE[0], $handle !== FALSE);
		
		sendMessages($con, $handle, 10, 0);
		echo ".";
		
		$ec = safmq_begin_transaction($con);
		Assert::_True("Unable to BeginTransaction:" . $ec, $ec !== FALSE);
		
		$cur = safmq_open_cursor($handle, $con);
		Assert::_True("Unable to OpenCursor:" . $ec, $ec !== FALSE);
		echo ".";
		
		$ec = safmq_advance_cursor($cur, $handle, $con);
		Assert::_True("Unable to AdvanceCursor:" . $ec, $ec !== FALSE);
		echo ".";
		
		$ec = safmq_advance_cursor($cur, $handle, $con);
		Assert::_True("Unable to AdvanceCursor:" . $ec, $ec !== FALSE);
		echo ".";
		
		$msg = safmq_retrieve_cursor(true, $cur, $handle, $con);
		Assert::_True("Unable to RetrieveCursor:", $msg !== FALSE);
		Assert::_True("Incorrect Message: " . $msg->Body. " should be 2", $msg->Body == "2");
		echo ".";
		
		$msg = safmq_retrieve_cursor(true, $cur, $handle, $con);
		Assert::_True("Unable to RetrieveCursor:", $msg !== FALSE);
		Assert::_True("Incorrect Message: " . $msg->Body . " should be 3", $msg->Body == "3");
		echo ".";
		
		$msg = safmq_retrieve(true, 0, $handle, $con);
		Assert::_True("Unable to RetrieveCursor:", $msg !== FALSE);
		Assert::_True("Incorrect Message: " . $msg->Body . " should be 0", $msg->Body == ("0"));
		echo ".+";
		
		$ids= array(1,4,5,6,7,8,9);
		for($x = 0; $x < count($ids); $x++) {
			$msg = safmq_retrieve(true, 0, $handle, $con);
			echo ".";
			Assert::_True("Unable to RetrieveCursor:", $msg !== FALSE);
			Assert::_True("Incorrect Message: " . $msg->Body . " should be " . $ids[$x], $msg->Body == $ids[$x]);
		}
		
		$ec = safmq_rollback_transaction($con);
		Assert::_True("Unable to RollbackTransaction:" . $ec, $ec !== FALSE);
		
		for($x = 0; $x < 8; $x++) {
			$msg = safmq_retrieve(true, 0, $handle, $con);
			echo ".";
			Assert::_True("Unable to RetrieveCursor:", $con !== FALSE);
			Assert::_True("Incorrect Message: " . $msg->Body . " should be " . $x, $msg->Body == $x);
		}
		
		$ec = safmq_commit_transaction($con);
		Assert::_True("Unable to CommitTransaction:" . $ec, $ec !== FALSE);
		
		for($x = 0; $x < 2; $x++) {
			$msg = safmq_retrieve(true, 0, $handle, $con);
			echo ".";
			Assert::_True("Unable to RetrieveCursor:", $msg !== FALSE);
			Assert::_True("Incorrect Message: " . $msg->Body. " should be " . ($x+8), $msg->Body == ($x+8));
		}
		
		$ec = safmq_rollback_transaction($con);
		Assert::_True("Unable to RollbackTransaction:" . $ec, $ec !== FALSE);
		echo ".";
		
		$ec = safmq_end_transaction($con);
		Assert::_True("Unable to RollbackTransaction:" . $ec, $ec !== FALSE);
		echo ".";
		
		for($x = 0; $x < 2; $x++) {
			$msg = safmq_retrieve(true, 0, $handle, $con);
			echo ".";
			Assert::_True("Unable to RetrieveCursor:", $msg !== FALSE);
			Assert::_True("Incorrect Message: " . $msg->Body. " should be " .($x+8), $msg->Body == ($x+8));
		}

		$msg = safmq_retrieve(true, 0, $handle, $con);
		Assert::_True("Able to RetrieveCursor:", $msg === FALSE);
		echo ".";
		
		$ec = safmq_close_queue($handle);
		Assert::_True("Unble to CloseQueue:" . $ec, $ec !== FALSE);
	}	

	function test_EnqueueWithRelay() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress, $oaddress;
		$con = connectToServer($address, null, null);
		$msg = safmq_msg();
		$msg->Body = $SPECIAL_MESSAGE;

		addAllQueues($con);
	

	
		$ec = safmq_enqueue_with_relay($oaddress . "/" . $TEST_QUEUE[1], $msg, $con);
		Assert::_True("Unble to EnqueueWithRelay:" . $ec, $ec !== FALSE);
		
		//for($x = 0; $x < 20; $x++) {
		//	echo ".";
		//	time_nanosleep(0, 100000);
		//}
			
		
		$handle = safmq_open_queue($TEST_QUEUE[1], $con);
		Assert::_True("Unble to OpenQueue:" . $handle, $handle !== FALSE);
		
		$msg = safmq_retrieve(true, 2, $handle, $con);
		Assert::_True("Unble to Retrieve:", $msg !== FALSE);
		Assert::_True("Incorrect Message:" . $msg->Body, $msg->Body == $SPECIAL_MESSAGE);
	
		$ec = safmq_close_queue($handle);
		Assert::_True("Unble to CloseQueue:" . $ec, $ec !== FALSE);
	}	
	
	/// SSL TESTs ---------------------------------------------------------------
	$user="npuser";
	$password="54321";
	$description="passwordless login test user";
	$subject = "OU=Testing, O=Safmq Java Test Client";
	$issuer = "C=US, ST=Nebraska, L=Omaha, O=SAFMQ, OU=Sample X509 Certificate, CN=Sample SAFMQ X509 Certificate";
	
	/**
	 * Creates a user and associates an x.509 certificate with that user. Then
	 * attempts to login without a password for that user.
	 */
	function test_PasswordlessSignon()  {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress, $oaddress;
		global $user, $password, $description, $subject, $issuer;
		global $safmq_ec_message;
		
		$con = connectToServer($address,"","");
		
		$ec = safmq_delete_user($user);
		Assert::_True("Delete User", $ec === TRUE || $safmq_ec_message == "EC_DOESNOTEXIST");
		
		$ec = safmq_create_user($user, $password, $description);
		Assert::_True("Create User", $ec);
		
		$ec = safmq_add_user_identity($subject, $issuer, $user);
		Assert::_True("AddUserIdentity", $ec);
		
		$ids = safmq_enumerate_user_identities($user);
		Assert::_True("EnumerateUserIdentities", $ids !== FALSE);
		Assert::_True("X.509 ID Count", count($ids) > 0);
		// Check the results in the ids
		$idFound = false;
		for($x = 0; !$idFound && $x < count($ids); $x++) 
			$idFound = $ids[$x]->issuerDN == $issuer && $ids[$x]->subjectDN == $subject;
		Assert::_True("Identity found in returned X.509 identities", $idFound);
	}
	
	function setupSSL() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress, $oaddress;
		global $user, $password, $description, $subject, $issuer;
		global $safmq_ec_message;
		
		// Setup the SSL Context using the JKS file created via gen_test_cert in the java/cert directory

		$con = connectToServer($address, "", "");
		$ec = safmq_delete_user($user);
		Assert::_True("Delete User", $ec || $safmq_ec_message == "EC_DOESNOTEXIST");
		
		$ec = safmq_create_user($user, $password, $description);
		Assert::_True("Create User", $ec);
		
		$ec = safmq_add_user_identity($subject, $issuer, $user);
		Assert::_True("AddUserIdentity", $ec);

		safmq_close($con);
	}
	
	function test_EmptyLogin() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress, $oaddress;
		global $user, $password, $description, $subject, $issuer;
		global $safmq_ec_message;
		
		setupSSL();
		$con = safmq_cert_open($simpleAddress, "../java/cert/safmq_java_test.pem", "../java/cert/safmq_java_test.pem");
		Assert::_True("Unable to connect", $con !== FALSE);
	}
	
	function test_EmptyPassword() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress, $oaddress;
		global $user, $password, $description, $subject, $issuer;
		global $safmq_ec_message;
		
		setupSSL();
		$con = safmq_cert_open($simpleAddress, "../java/cert/safmq_java_test.pem", "../java/cert/safmq_java_test.pem", $user);
		Assert::_True("Unable to connect", $con !== FALSE);
	}

	function test_BadPassword() {
		global $TEST_QUEUE, $TEST_USERS, $TEST_PASSWD, $TEST_DESC, $TEST_GROUPS, $SPECIAL_MESSAGE, $address, $simpleAddress, $oaddress;
		global $user, $password, $description, $subject, $issuer;
		global $safmq_ec_message;
		
		$con = safmq_cert_open($simpleAddress, "../java/cert/safmq_java_test.pem", "../java/cert/safmq_java_test.pem", $user, "12345");
		Assert::_True("Able to connect:" . $safmq_ec_message, $con === FALSE);
	}

	

	function RunTests() {
		// Run a test and close the hanging $connection
		TestCase::Test("test_CreateQueue"); safmq_close(); safmq_display_connections(); // close here inscase of an assertion
		TestCase::Test("test_DeleteQueue"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_CreateUser"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_SetPassword"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_CreateTempQueue"); safmq_close(); safmq_display_connections(); 

		TestCase::Test("test_UserPermissions"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_DeleteUser"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_CreateGroup"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_GroupPermissions"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_DeleteGroup"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_EnumerateQueues"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_EnumerateUsers"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_EnumerateGroups"); safmq_close(); safmq_display_connections(); 
		
		TestCase::Test("test_GroupAddUser_GroupDeleteUser_GroupGetUsers"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_UserGetGroups"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_QueueSetUserPermission_QueueDeleteUserPermission_QueueEnumeratePermissions_QueueSetGroupPermission_QueueDeleteGroupPermission"); safmq_close(); safmq_display_connections(); 

		TestCase::Test("test_OpenQueue_Enqueue_Retrieve_CloseQueue"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_PeekID_PeekFront_OpenCursor_PeekCursor_SeekID_RetrieveCursor_AdvanceCursor_TestCurosr_CloseCursor_RetrieveID"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_Transactions"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_EnqueueWithRelay"); safmq_close(); safmq_display_connections(); 
		
	}

	function RunSSLTests() {
		global $address, $simpleAddress;
		
		printf("SSL Testing: =======================================\n");
		$address = "safmqs://admin:@localhost:40001";
		$simpleAddress = "safmqs://localhost:40001";
		RunTests();
		
		TestCase::Test("test_PasswordlessSignon"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_EmptyLogin"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_EmptyPassword"); safmq_close(); safmq_display_connections(); 
		TestCase::Test("test_BadPassword"); safmq_close(); safmq_display_connections(); 
	}
	
	RunTests();
	RunSSLTests();
?>

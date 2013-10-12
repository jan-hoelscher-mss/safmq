#!/bin/bash
	#./safmqc --user=admin --passwd= --enqueue //localhost:9000 QUEUENAME| LONG//localhost:9000 ...
	#./safmqc --user=admin --passwd= --retreive //localhost:9000 QUEUENAME| LONG//localhost:9000 
	#./safmqc --user=admin --passwd= --retreiveid //localhost:9000 QUEUENAME| LONG//localhost:9000 ID 
	#./safmqc --user=admin --passwd= --peekfront //localhost:9000 QUEUENAME| LONG//localhost:9000 
	#./safmqc --user=admin --passwd= --peekid //localhost:9000 //localhost:9000 QUEUENAME| LONG//localhost:9000 ID

	./safmqc --user=admin --passwd= --createuser //localhost:9000 USERNAME PASSWORD DESCRIPTION
	./safmqc --user=admin --passwd= --usersetpermissions //localhost:9000 USERNAME users groups queues
	./safmqc --user=admin --passwd= --usergetpermissions //localhost:9000 USERNAME
	./safmqc --user=admin --passwd= --setpassword //localhost:9000 USERNAME PASSWORD
	./safmqc --user=admin --passwd= --creategroup //localhost:9000 GROUPNAME
	./safmqc --user=admin --passwd= --groupsetpermissions //localhost:9000 GROUPNAME users groups queues
	./safmqc --user=admin --passwd= --groupgetpermissions //localhost:9000 GROUPNAME

	./safmqc --user=admin --passwd= --groupadduser //localhost:9000 GROUPNAME USERNAME
	./safmqc --user=admin --passwd= --groupgetusers //localhost:9000 GROUPNAME
	./safmqc --user=admin --passwd= --usergetgroups //localhost:9000 USERNAME
	./safmqc --user=admin --passwd= --createqueue //localhost:9000 QUEUENAME
	./safmqc --user=admin --passwd= --queuesetuserpermission //localhost:9000 QUEUENAME USERNAME read write destroy change
	./safmqc --user=admin --passwd= --queuesetgrouppermission //localhost:9000 QUEUENAME GROUPNAME read write destroy change
	./safmqc --user=admin --passwd= --queueenumeratepermissions //localhost:9000 QUEUENAME
	./safmqc --user=admin --passwd= --queuedeleteuserpermission //localhost:9000 QUEUENAME USERNAME 
	./safmqc --user=admin --passwd= --queuedeletegrouppermission //localhost:9000 QUEUENAME GROUPNAME 

	./safmqc --user=admin --passwd= --enumeratequeues //localhost:9000 
	./safmqc --user=admin --passwd= --enumerateusers //localhost:9000 
	./safmqc --user=admin --passwd= --enumerategroups //localhost:9000 

	./safmqc --user=admin --passwd= --deletequeue //localhost:9000 QUEUENAME
	./safmqc --user=admin --passwd= --groupdeleteuser //localhost:9000 GROUPNAME USERNAME
	./safmqc --user=admin --passwd= --deletegroup //localhost:9000 GROUPNAME
	./safmqc --user=admin --passwd= --deleteuser //localhost:9000 USERNAME

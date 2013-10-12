MessageIdTypedef=DWORD

SeverityNames=(Info=0x0:STATUS_SEVERITY_INFORMATIONAL)

FacilityNames=(Runtime=0x0:FACILITY_RUNTIME)

LanguageNames=(English=0x409:MSG00409)

MessageId=0x1
Severity=Info
SymbolicName=CATEGORY_STARTUP
Language=English
Startup%0
.

MessageId=0x2
Severity=Info
SymbolicName=CATEGORY_SHUTDOWN
Language=English
Shutdown%0
.

MessageId=0x3
Severity=Info
SymbolicName=CATEGORY_SIGN_ON
Language=English
Sign On%0
.

MessageId=0x4
Severity=Info
SymbolicName=CATEGORY_USER_ACCOUNT
Language=English
Account%0
.

MessageId=0x5
Severity=Info
SymbolicName=CATEGORY_SECURITY_GROUP
Language=English
Group%0
.

MessageId=0x6
Severity=Info
SymbolicName=CATEGORY_QUEUE
Language=English
Queue%0
.

MessageId=0x7
Severity=Info
SymbolicName=CATEGORY_TTL
Language=English
Message TTL%0
.

MessageId=0x8
Severity=Info
SymbolicName=CATEGORY_FORWARD
Language=English
Forwarding%0
.

MessageId=0x9
Severity=Info
SymbolicName=CATEGORY_INFO
Language=English
Information%0
.



MessageId=0x0100
Severity=Info
Facility=Runtime
SymbolicName=STARTUP
Language=English
SAFMQ server started
.

MessageId=0x0108
Severity=Info
Facility=Runtime
SymbolicName=STARTUP_ERROR
Language=English
SAFMQ server started with an error: %1
.

MessageId=0x0200
Severity=Info
Facility=Runtime
SymbolicName=STARTUP
Language=English
SAFMQ server stopped
.

MessageId=0x0208
Severity=Info
Facility=Runtime
SymbolicName=STARTUP_ERROR
Language=English
SAFMQ server stopped with an error: %1
.

MessageId=0x0300
Severity=Info
Facility=Runtime
SymbolicName=SIGNON
Language=English
User Sign On%r
%r
User: %1%r
Addr: %2:%3%r
.

MessageId=0x0308
Severity=Info
Facility=Runtime
SymbolicName=SIGNON_ERROR
Language=English
Invalid Sign On%r
%r
User: %1%r
Addr: %2:%3%r
Error: %4%r
.

MessageId=0x0900
Severity=Info
Facility=Runtime
SymbolicName=INFO
Language=English
%1
.

MessageId=0x0401
Severity=Info
Facility=Runtime
SymbolicName=USER_CREATED
Language=English
New User Account Created%r
%r
Created By:   %1%r
From: %3:%4%r
Account Name: %2%r
Description: %5%r
.

MessageId=0x0402
Severity=Info
Facility=Runtime
SymbolicName=USER_DELETED
Language=English
User Account Deleted%r
%r
Deleted By: %1%r
From: %3:%4%r
Account Name: %2%r
.

MessageId=0x0403
Severity=Info
Facility=Runtime
SymbolicName=USER_ACL
Language=English
User Account Access Control Changed%r
%r
Changed By: %1%r
From: %3:%4%r
Account Name: %2%r
ACL Settings: %5%r
.

MessageId=0x0408
Severity=Info
Facility=Runtime
SymbolicName=USER_ERROR
Language=English
Error Modifying User Account%r
%r
Changed By: %1%r
From: %3:%4%r
Account Name: %2%r
Action: %5%r
Error: %6%r
.

MessageId=0x0501
Severity=Info
Facility=Runtime
SymbolicName=GROUP_CREATED
Language=English
New Security Group Created%r
%r
Created By: %1%r
From: %3:%4%r
Group Name: %2%r
.

MessageId=0x0502
Severity=Info
Facility=Runtime
SymbolicName=GROUP_DELETED
Language=English
Security Group Deleted%r
%r
Deleted By: %1%r
From: %3:%4%r
Group Name: %2%r
.

MessageId=0x0503
Severity=Info
Facility=Runtime
SymbolicName=GROUP_ACL
Language=English
Security Group Access Control Changed%r
%r
Changed By: %1%r
From: %3:%4%r
Group Name: %2%r
ACL Settings: %5%r
.

MessageId=0x0506
Severity=Info
Facility=Runtime
SymbolicName=GROUP_ADD_USER
Language=English
User Account Added to Security Group%r
%r
Changed By: %1%r
From: %3:%4%r
Group Name: %2%r
Account Name: %5%r
.

MessageId=0x0507
Severity=Info
Facility=Runtime
SymbolicName=GROUP_DELETE_USER
Language=English
User Account Removed to Security Group%r
%r
Changed By: %1%r
From: %3:%4%r
Group Name: %2%r
Account Name: %5%r
.

MessageId=0x0508
Severity=Info
Facility=Runtime
SymbolicName=GROUP_ERROR
Language=English
Error Modifying User Account%r
%r
Changed By: %1%r
From: %3:%4%r
Group Name: %2%r
Action: %5%r
Error: %6%r
.

MessageId=0x0601
Severity=Info
Facility=Runtime
SymbolicName=QUEUE_CREATED
Language=English
New Queue Created%r
%r
Created By: %1%r
From: %3:%4%r
Queue Name: %2%r
.

MessageId=0x0602
Severity=Info
Facility=Runtime
SymbolicName=QUEUE_DELETED
Language=English
Queue Deleted%r
%r
Deleted By: %1%r
From: %3:%4%r
Queue Name: %2%r
.

MessageId=0x0603
Severity=Info
Facility=Runtime
SymbolicName=QUEUE_ACL
Language=English
Queue Access Control Changed%r
%r
Changed By: %1%r
From: %3:%4%r
Group Name: %2%r
Actor Type: %5%r
Actor Id: %6%r
ACL Settings: %7%r
.

MessageId=0x0604
Severity=Info
Facility=Runtime
SymbolicName=QUEUE_ACL_CLEAR
Language=English
Access Control Removed%r
%r
Deleted By: %1%r
From: %3:%4%r
Queue Name: %2%r
Actor Type: %5%r
Actor Id: %6%r
.

MessageId=0x0608
Severity=Info
Facility=Runtime
SymbolicName=QUEUE_ERROR
Language=English
Error Modifying Queue%r
%r
Deleted By: %1%r
From: %3:%4%r
Queue Name: %2%r
Action: %5%r
Error Code: %6%r
.

MessageId=0x0705
Severity=Info
Facility=Runtime
SymbolicName=MESSAGE_TTL
Language=English
Message Time To Live Expired%r
%r
Queue: %1%r
Label: %2%r
Message ID: %3%r
.

MessageId=0x0708
Severity=Info
Facility=Runtime
SymbolicName=MESSAGE_ERROR
Language=English
TTL Message Delivery Error%r
%r
Error: %1%r
URL: %2%r
Message ID: %3%r
.

MessageId=0x0808
Severity=Info
Facility=Runtime
SymbolicName=FORWARD_ERROR
Language=English
Message Forwarding Error%r
%r
Error: %1%r
URL: %2%r
.


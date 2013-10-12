//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_RUNTIME                 0x0


//
// Define the severity codes
//
#define STATUS_SEVERITY_INFORMATIONAL    0x0


//
// MessageId: CATEGORY_STARTUP
//
// MessageText:
//
//  Startup%0
//
#define CATEGORY_STARTUP                 ((DWORD)0x00000001L)

//
// MessageId: CATEGORY_SHUTDOWN
//
// MessageText:
//
//  Shutdown%0
//
#define CATEGORY_SHUTDOWN                ((DWORD)0x00000002L)

//
// MessageId: CATEGORY_SIGN_ON
//
// MessageText:
//
//  Sign On%0
//
#define CATEGORY_SIGN_ON                 ((DWORD)0x00000003L)

//
// MessageId: CATEGORY_USER_ACCOUNT
//
// MessageText:
//
//  Account%0
//
#define CATEGORY_USER_ACCOUNT            ((DWORD)0x00000004L)

//
// MessageId: CATEGORY_SECURITY_GROUP
//
// MessageText:
//
//  Group%0
//
#define CATEGORY_SECURITY_GROUP          ((DWORD)0x00000005L)

//
// MessageId: CATEGORY_QUEUE
//
// MessageText:
//
//  Queue%0
//
#define CATEGORY_QUEUE                   ((DWORD)0x00000006L)

//
// MessageId: CATEGORY_TTL
//
// MessageText:
//
//  Message TTL%0
//
#define CATEGORY_TTL                     ((DWORD)0x00000007L)

//
// MessageId: CATEGORY_FORWARD
//
// MessageText:
//
//  Forwarding%0
//
#define CATEGORY_FORWARD                 ((DWORD)0x00000008L)

//
// MessageId: CATEGORY_INFO
//
// MessageText:
//
//  Information%0
//
#define CATEGORY_INFO                    ((DWORD)0x00000009L)

//
// MessageId: STARTUP
//
// MessageText:
//
//  SAFMQ server started
//
#define STARTUP                          ((DWORD)0x00000100L)

//
// MessageId: STARTUP_ERROR
//
// MessageText:
//
//  SAFMQ server started with an error: %1
//
#define STARTUP_ERROR                    ((DWORD)0x00000108L)

//
// MessageId: STARTUP
//
// MessageText:
//
//  SAFMQ server stopped
//
#define STARTUP                          ((DWORD)0x00000200L)

//
// MessageId: STARTUP_ERROR
//
// MessageText:
//
//  SAFMQ server stopped with an error: %1
//
#define STARTUP_ERROR                    ((DWORD)0x00000208L)

//
// MessageId: SIGNON
//
// MessageText:
//
//  User Sign On%r
//  %r
//  User: %1%r
//  Addr: %2:%3%r
//
#define SIGNON                           ((DWORD)0x00000300L)

//
// MessageId: SIGNON_ERROR
//
// MessageText:
//
//  Invalid Sign On%r
//  %r
//  User: %1%r
//  Addr: %2:%3%r
//  Error: %4%r
//
#define SIGNON_ERROR                     ((DWORD)0x00000308L)

//
// MessageId: INFO
//
// MessageText:
//
//  %1
//
#define INFO                             ((DWORD)0x00000900L)

//
// MessageId: USER_CREATED
//
// MessageText:
//
//  New User Account Created%r
//  %r
//  Created By:   %1%r
//  From: %3:%4%r
//  Account Name: %2%r
//  Description: %5%r
//
#define USER_CREATED                     ((DWORD)0x00000401L)

//
// MessageId: USER_DELETED
//
// MessageText:
//
//  User Account Deleted%r
//  %r
//  Deleted By: %1%r
//  From: %3:%4%r
//  Account Name: %2%r
//
#define USER_DELETED                     ((DWORD)0x00000402L)

//
// MessageId: USER_ACL
//
// MessageText:
//
//  User Account Access Control Changed%r
//  %r
//  Changed By: %1%r
//  From: %3:%4%r
//  Account Name: %2%r
//  ACL Settings: %5%r
//
#define USER_ACL                         ((DWORD)0x00000403L)

//
// MessageId: USER_ERROR
//
// MessageText:
//
//  Error Modifying User Account%r
//  %r
//  Changed By: %1%r
//  From: %3:%4%r
//  Account Name: %2%r
//  Action: %5%r
//  Error: %6%r
//
#define USER_ERROR                       ((DWORD)0x00000408L)

//
// MessageId: GROUP_CREATED
//
// MessageText:
//
//  New Security Group Created%r
//  %r
//  Created By: %1%r
//  From: %3:%4%r
//  Group Name: %2%r
//
#define GROUP_CREATED                    ((DWORD)0x00000501L)

//
// MessageId: GROUP_DELETED
//
// MessageText:
//
//  Security Group Deleted%r
//  %r
//  Deleted By: %1%r
//  From: %3:%4%r
//  Group Name: %2%r
//
#define GROUP_DELETED                    ((DWORD)0x00000502L)

//
// MessageId: GROUP_ACL
//
// MessageText:
//
//  Security Group Access Control Changed%r
//  %r
//  Changed By: %1%r
//  From: %3:%4%r
//  Group Name: %2%r
//  ACL Settings: %5%r
//
#define GROUP_ACL                        ((DWORD)0x00000503L)

//
// MessageId: GROUP_ADD_USER
//
// MessageText:
//
//  User Account Added to Security Group%r
//  %r
//  Changed By: %1%r
//  From: %3:%4%r
//  Group Name: %2%r
//  Account Name: %5%r
//
#define GROUP_ADD_USER                   ((DWORD)0x00000506L)

//
// MessageId: GROUP_DELETE_USER
//
// MessageText:
//
//  User Account Removed to Security Group%r
//  %r
//  Changed By: %1%r
//  From: %3:%4%r
//  Group Name: %2%r
//  Account Name: %5%r
//
#define GROUP_DELETE_USER                ((DWORD)0x00000507L)

//
// MessageId: GROUP_ERROR
//
// MessageText:
//
//  Error Modifying User Account%r
//  %r
//  Changed By: %1%r
//  From: %3:%4%r
//  Group Name: %2%r
//  Action: %5%r
//  Error: %6%r
//
#define GROUP_ERROR                      ((DWORD)0x00000508L)

//
// MessageId: QUEUE_CREATED
//
// MessageText:
//
//  New Queue Created%r
//  %r
//  Created By: %1%r
//  From: %3:%4%r
//  Queue Name: %2%r
//
#define QUEUE_CREATED                    ((DWORD)0x00000601L)

//
// MessageId: QUEUE_DELETED
//
// MessageText:
//
//  Queue Deleted%r
//  %r
//  Deleted By: %1%r
//  From: %3:%4%r
//  Queue Name: %2%r
//
#define QUEUE_DELETED                    ((DWORD)0x00000602L)

//
// MessageId: QUEUE_ACL
//
// MessageText:
//
//  Queue Access Control Changed%r
//  %r
//  Changed By: %1%r
//  From: %3:%4%r
//  Group Name: %2%r
//  Actor Type: %5%r
//  Actor Id: %6%r
//  ACL Settings: %7%r
//
#define QUEUE_ACL                        ((DWORD)0x00000603L)

//
// MessageId: QUEUE_ACL_CLEAR
//
// MessageText:
//
//  Access Control Removed%r
//  %r
//  Deleted By: %1%r
//  From: %3:%4%r
//  Queue Name: %2%r
//  Actor Type: %5%r
//  Actor Id: %6%r
//
#define QUEUE_ACL_CLEAR                  ((DWORD)0x00000604L)

//
// MessageId: QUEUE_ERROR
//
// MessageText:
//
//  Error Modifying Queue%r
//  %r
//  Deleted By: %1%r
//  From: %3:%4%r
//  Queue Name: %2%r
//  Action: %5%r
//  Error Code: %6%r
//
#define QUEUE_ERROR                      ((DWORD)0x00000608L)

//
// MessageId: MESSAGE_TTL
//
// MessageText:
//
//  Message Time To Live Expired%r
//  %r
//  Queue: %1%r
//  Label: %2%r
//  Message ID: %3%r
//
#define MESSAGE_TTL                      ((DWORD)0x00000705L)

//
// MessageId: MESSAGE_ERROR
//
// MessageText:
//
//  TTL Message Delivery Error%r
//  %r
//  Error: %1%r
//  URL: %2%r
//  Message ID: %3%r
//
#define MESSAGE_ERROR                    ((DWORD)0x00000708L)

//
// MessageId: FORWARD_ERROR
//
// MessageText:
//
//  Message Forwarding Error%r
//  %r
//  Error: %1%r
//  URL: %2%r
//
#define FORWARD_ERROR                    ((DWORD)0x00000808L)


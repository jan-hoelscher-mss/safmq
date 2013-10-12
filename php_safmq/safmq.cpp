/*
Safmq, the server, library, utilities and documentation are released under
the Apache License Ver. 2.0.

   Copyright 2004-2008 Drew Rash and Matthew J. Battey, Omaha, NE

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

Builds of SAFMQ may include the OpenSSL library to implement SSL communications.
*/
#pragma warning(disable: 4786)

/* include standard header */
#include <stdio.h>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>

#ifdef PHP_WIN32
    #include <iostream>
    #include <math.h>
#endif

#ifdef _WIN32
	#include <winsock2.h>
	#include <windows.h>
#else
	//else
#endif

//Message Queue Files
#include "MessageQueue.h"
#include "MQConnection.h"
#include "MQFactory.h"

#if defined(SAFMQ_SSL)
#include "tcpsocket/sslsocket.h"
#endif

#include <php.h>


using namespace std;
using namespace safmq;

//This structure links to DefaultData as a vector of QHandles which contains a vector of Cursors to Queues.
struct DefaultQHandle{
	DefaultQHandle() {}
	DefaultQHandle(long qid) {
		this->qid = qid;
	}
	DefaultQHandle(const DefaultQHandle& src) {
		*this = src;
	}
	const DefaultQHandle& operator=(const DefaultQHandle& src) {
		this->qid = src.qid;
		this->default_qcursors.clear();
		this->default_qcursors.assign(src.default_qcursors.begin(),src.default_qcursors.end());
		return *this;
	}
	bool operator==(const DefaultQHandle& src) const {
		return qid == src.qid;
	}

	long qid;
	vector<long>	default_qcursors;
};

/*
	Used to hold a thread safe copy of connections, queues, and qcursors
	Connections contain multiple queuehandles, each queue handle can have multiple queuecursors.
	A function for viewing these through php is safmq_display_connections().
*/
struct DefaultData {
	DefaultData() {}
	DefaultData(MQConnection* con, int con_resid) {
		this->con = con;
		this->con_resid = con_resid;//zend resource_number
	}
	DefaultData(const DefaultData& src) {
		*this = src;
	}
	const DefaultData& operator=(const DefaultData& src) {
		this->con = src.con;
		this->con_resid = src.con_resid;
		this->default_qhandle.clear();
		this->default_qhandle.assign(src.default_qhandle.begin(),src.default_qhandle.end());
		return *this;
	}
	bool operator==(const DefaultData& src) const {
		return con == src.con;
	}
	MQConnection*	con;
	int				con_resid;
	vector<DefaultQHandle> default_qhandle;
};

/*Global Variables*/
ZEND_BEGIN_MODULE_GLOBALS(safmq)
	vector<DefaultData>	defaults;	//our thread safe connections/queuehandles/queuecursors
	long num_links;					//increment and decrement when con's established ect.. --not implemented
	long max_links;					//limit --not implemented
ZEND_END_MODULE_GLOBALS(safmq)

ZEND_EXTERN_MODULE_GLOBALS(safmq)
#ifdef ZTS
# define SMQG(v) TSRMG(safmq_globals_id, zend_safmq_globals *, v)
#else
# define SMQG(v) (safmq_globals.v)
#endif
ZEND_DECLARE_MODULE_GLOBALS(safmq)

static int le_queue;				//zend resource list entry for queues
static int le_con;					//zend resource list entry for connections
#define LE_MSGqhandle_RESOURCE_NAME  "PHP Message Queue Connection"
#define LE_MSGQQUEUE_RESOURCE_NAME "PHP Message Queue Queue Connection"

const char* EC_Decode(ErrorCode ec) {//SAFMQ error code decoding
#define ECOUT(v) case v: return #v;
	switch(ec) {
		ECOUT(EC_NOERROR)
		// The operation timed out
		ECOUT(EC_TIMEDOUT)
		// The user is not authorized to take the action
		ECOUT(EC_NOTAUTHORIZED)
		// The resource is already closed
		ECOUT(EC_ALREADYCLOSED)
		// The resource does not exist
		ECOUT(EC_DOESNOTEXIST)
		// The resource has not been opened
		ECOUT(EC_NOTOPEN)
		// A network error occured
		ECOUT(EC_NETWORKERROR)
		// The Time to Live for a message has expired
		ECOUT(EC_TTLEXPIRED)
		// The cursor has been invalidated
		ECOUT(EC_CURSORINVALIDATED)
		// The message contains a duplicate message ID
		ECOUT(EC_DUPLICATEMSGID)
		// The server is unavailable
		ECOUT(EC_SERVERUNAVAIL)
		// No more messages remain
		ECOUT(EC_NOMOREMESSAGES)
		// A file system error has occured
		ECOUT(EC_FILESYSTEMERROR)
		// User cannot change the group
		ECOUT(EC_CANNOTCHANGEGROUP)
		// User cannot change the user
		ECOUT(EC_CANNOTCHANGEUSER)
		// The resource is already defined
		ECOUT(EC_ALREADYDEFINED)
		// The operation is not implemented
		ECOUT(EC_NOTIMPLEMENTED)
		// The queue is currently open by another connection
		ECOUT(EC_QUEUEOPEN)
		// The user is not logged in
		ECOUT(EC_NOTLOGGEDIN)
		// Invalid name for a resource
		ECOUT(EC_INVALIDNAME)
		// System does not allow message forwarding (relaying)
		ECOUT(EC_FORWARDNOTALLOWED)
		// Wronge type for message
		ECOUT(EC_WRONGMESSAGETYPE)
		// Wrong Protocol Version
		ECOUT(EC_UNSUPPORTED_PROTOCOL)
		// Undefined error
		ECOUT(EC_ERROR)
	default: return "UNKNOWN";
	}
#undef ECOUT
}

/*C++ functions*/

void set_errorcode(ErrorCode ec, const string& msg TSRMLS_DC) {
	zval* safmq_ec;
	zval* safmq_ec_message;
	
	MAKE_STD_ZVAL(safmq_ec);
	MAKE_STD_ZVAL(safmq_ec_message);

	ZVAL_LONG(safmq_ec, (long)ec);
	ZVAL_STRING(safmq_ec_message, (char*)msg.c_str(), 1);

	ZEND_SET_SYMBOL(&EG(symbol_table), "safmq_ec", safmq_ec); 
	ZEND_SET_SYMBOL(&EG(symbol_table), "safmq_ec_message", safmq_ec_message); 
}

void set_errorcode(ErrorCode ec TSRMLS_DC) {
	set_errorcode(ec, EC_Decode(ec) TSRMLS_CC);
}


int add_con_to_zend(MQConnection* con TSRMLS_DC){
	zval *return_value=NULL;
	int php_con = zend_register_resource(return_value, con, le_con);//store our pointer as zend_resource

	SMQG(defaults).push_back(DefaultData(con,php_con));//store our own default connection
	return php_con;//return zend_resource int
}

MQConnection* get_con_from_zend(zval* id TSRMLS_DC){
	MQConnection*	con = NULL;
	if( id != NULL ) {//Get Specified ID
		con = (MQConnection*) zend_fetch_resource(&id TSRMLS_CC, -1, LE_MSGqhandle_RESOURCE_NAME, NULL, 1, le_con);
		if( con=NULL )
			php_error(E_ERROR, "Bad Connection Specified, Using Default.");
	}
	if( con == NULL ) {//If ID Fails Get Default Connection (on failure an error message is automatically output)
		vector<DefaultData>::reverse_iterator	it = SMQG(defaults).rbegin();
		if (it != SMQG(defaults).rend())
			con = it->con;
		if( con==NULL ) {
			php_error(E_ERROR, "There Are No Connections To A Queue Server.");
		}
	}

	return con;
}

//remove specific resource from zend
void remove_con_FromZend(zval* id TSRMLS_DC){
	if( id == NULL ) {
		vector<DefaultData>::reverse_iterator	it = SMQG(defaults).rbegin();
		if (it != SMQG(defaults).rend())
			zend_list_delete(it->con_resid);
	} else {
		zend_list_delete(id->value.lval);
	}
}

/*Used By Zend Engine To Free All UnFreed Resources*/
static void _close_safmq_con(zend_rsrc_list_entry *rsrc TSRMLS_DC){
	MQConnection*					con = (MQConnection*)(rsrc->ptr);
	vector<DefaultData>::iterator	it;
	vector<DefaultData>&			connections = SMQG(defaults);

	if( con != NULL ) {
		it = find(connections.begin(),connections.end(),DefaultData(con,-1));
		if (it != connections.end())
				connections.erase(it);
	}
	delete con;//delete pointer
}

void register_default_qhandle(MQConnection* con, long qhandle TSRMLS_DC) {
	if( con != NULL ) {
		vector<DefaultData>::iterator	it;
		vector<DefaultData>&			connections = SMQG(defaults);

		it = find(connections.begin(),connections.end(),DefaultData(con,-1));
		if (it != connections.end()) {
			it->default_qhandle.push_back(qhandle);
		}
	}
}

long get_default_qhandle(MQConnection* con, long queuehandle TSRMLS_DC) {
	/*
	This takes a con and queuehandle and if either is specified then it is checked against the vector table and used
	If either is not specified then the last queue is used for the con or the last con is used with the last queue for that con
  */
	vector<DefaultData>& connections = SMQG(defaults);
	vector<DefaultQHandle>::iterator qhit;
	vector<DefaultData>::iterator it;

	if( con != NULL ) {
		it = find(connections.begin(),connections.end(),DefaultData(con,-1));;
	} else {
		if( connections.end() != connections.begin() ) {
			it =(connections.end()-1);
		} else {
			it = connections.end();
		}
	}

	if (it != connections.end()) {
		if( queuehandle != 0 ) {
			qhit = find(it->default_qhandle.begin(), it->default_qhandle.end(), DefaultQHandle(queuehandle));
		} else {
			if( it->default_qhandle.begin() != it->default_qhandle.end() ) {
				qhit = it->default_qhandle.end()-1;
			} else {
				qhit = it->default_qhandle.end();
			}
		}
		if( qhit != it->default_qhandle.end() ) {
			return qhit->qid;
		}
	}
	return false;
}

void remove_default_qhandle(MQConnection* con, long qhandle TSRMLS_DC) {
	vector<DefaultData>&			connections = SMQG(defaults);
	DefaultData*					pData = NULL;

	if( con != NULL ) {
		vector<DefaultData>::iterator	it;
		it = find(connections.begin(),connections.end(),DefaultData(con,-1));
		if (it != connections.end()) {
			pData = & (*it);
		}
	} else {
		vector<DefaultData>::reverse_iterator	it;
		it = connections.rbegin();
		if (it != connections.rend()) {
			pData = & (*it);
		} else {
			//php_error no open connections..never seen
		}
	}

	if (pData) {
		if (qhandle != 0) {
			vector<DefaultQHandle>::iterator qhit = find(pData->default_qhandle.begin(), pData->default_qhandle.end(), DefaultQHandle(qhandle));
			if (qhit != pData->default_qhandle.end())
				pData->default_qhandle.erase(qhit);
		} else {
			vector<DefaultQHandle>::reverse_iterator	qhit;
			qhit = pData->default_qhandle.rbegin();
			if (qhit != pData->default_qhandle.rend()) {
				pData->default_qhandle.erase(qhit.base());
			} else {
				//php_error no open queues..never seen
			}
		}
	}
}

void register_default_qcursor(MQConnection* con, long qhandle, long cursorID TSRMLS_DC) {//adds a qcursor to the gvector
	vector<DefaultData>::iterator it = find(SMQG(defaults).begin(), SMQG(defaults).end(), DefaultData(con, -1));
	vector<DefaultQHandle>::iterator qhit = find(it->default_qhandle.begin(), it->default_qhandle.end(), DefaultQHandle(qhandle));
	qhit->default_qcursors.push_back(cursorID);
}

long get_default_cursorID(MQConnection* con, long qhandle, long cursorID TSRMLS_DC) {
	vector<long>::reverse_iterator qcit;
	if( con == NULL || qhandle == 0 ) {
		php_error(E_ERROR, "get_default_cursorID called with Null con and qhandle, you must set con and qhandle before calling.");
	}

	vector<DefaultData>::iterator it = find(SMQG(defaults).begin(), SMQG(defaults).end(), DefaultData(con, -1));
	vector<DefaultQHandle>::iterator qhit = find(it->default_qhandle.begin(), it->default_qhandle.end(), DefaultQHandle(qhandle));
	if( cursorID!=-1 ) {
		qcit = find(qhit->default_qcursors.rbegin(), qhit->default_qcursors.rend(), cursorID);
	} else {
		qcit = qhit->default_qcursors.rbegin();
	}
	if( qhit->default_qcursors.rbegin() != qhit->default_qcursors.rend() ) {
		return (*qcit);
	} else {
		php_error(E_ERROR, "No Cursors Available Or The One Sent Was Never Created.");
		return -1;
	}
}

long remove_default_cursorID(MQConnection* con, long qhandle, long cursorID TSRMLS_DC) {
	vector<long>::iterator qcit;
	if( con == NULL || qhandle == 0 ) {
		zend_printf("remove_default_cursorID called with Null con and qhandle, you must set con and qhandle before calling. (should not see this).\n");
	}

	vector<DefaultData>::iterator it = find(SMQG(defaults).begin(), SMQG(defaults).end(), DefaultData(con, -1));
	vector<DefaultQHandle>::iterator qhit = find(it->default_qhandle.begin(), it->default_qhandle.end(), DefaultQHandle(qhandle));
	if( cursorID!=-1 ) {
		qcit = find(qhit->default_qcursors.begin(), qhit->default_qcursors.end(), cursorID);
	} else {
		if( qhit->default_qcursors.end()!=qhit->default_qcursors.begin() ) {
			qcit = qhit->default_qcursors.end()-1;
		} else {
			qcit = qhit->default_qcursors.end();
		}
	}
	if( qcit != qhit->default_qcursors.end() ) {
		cursorID = (long)*qcit;
		qhit->default_qcursors.erase(qcit);
		return cursorID;
	} else {
		php_error(E_ERROR, "No Cursors available to be closed or Cursor specified was invalid.");
		return -1;
	}
}

//handles enumerations
bool enumerate_queues_users_groups(long enumerator, MQConnection* con, zval* queue_array TSRMLS_DC) {
	if( con == NULL ) {
		return false;
	}

	MQConnection::QDATA_VECTOR qdata_vector;
	MQConnection::USER_VECTOR user_vector;
	MQConnection::NAME_VECTOR name_vector;
	ErrorCode ec;
	try {
		switch (enumerator) {
			case 0:
				ec = con->EnumerateQueues(qdata_vector);
				if( ec==EC_NOERROR ) {
					MQConnection::QDATA_VECTOR::iterator it=qdata_vector.begin();
					while( it != qdata_vector.end() ) {
						add_next_index_string(queue_array, (char*)it->queuename.c_str(), 1);
						it++;
					}
					return true;
				}
				break;
			case 1:
				ec = con->EnumerateUsers(user_vector);
				if( ec==EC_NOERROR ) {
					MQConnection::USER_VECTOR::iterator it=user_vector.begin();
					while( it != user_vector.end() ) {
						zval*	user_name;
						MAKE_STD_ZVAL(user_name);
        				object_init(user_name);
					
        				add_property_string(user_name, "description", (char*)it->description.c_str(),1);
        				add_property_string(user_name, "name", (char*)it->name.c_str(),1);

						add_next_index_zval(queue_array, user_name);
						it++;
					}
					return true;
				}
				break;
			case 2:
				ec = con->EnumerateGroups(name_vector);
				if( ec==EC_NOERROR ) {
					MQConnection::NAME_VECTOR::iterator it=name_vector.begin();
					while( it != name_vector.end() ) {
						add_next_index_string(queue_array, (char*)it->c_str(), 1);
						it++;
					}
					return true;
				}
		}
	} catch (ErrorCode ec) {
		php_error(E_ERROR, "'%s encountered while attempting to enumerate.", EC_Decode(ec));
		return false;
	}
	return false;
}

extern "C" {
/* 1st function list that will be exported (available to php) must be declared */
ZEND_FUNCTION(safmq_get_server_major_protocol_version);
ZEND_FUNCTION(safmq_get_server_minor_protocol_version);
ZEND_FUNCTION(safmq_open);
ZEND_FUNCTION(safmq_cert_open);
ZEND_FUNCTION(safmq_retrieve);
ZEND_FUNCTION(safmq_retrieve_id);
ZEND_FUNCTION(safmq_retrieve_cursor);
ZEND_FUNCTION(safmq_create_queue);
ZEND_FUNCTION(safmq_delete_queue);
ZEND_FUNCTION(safmq_queue_set_user_permission);
ZEND_FUNCTION(safmq_queue_delete_user_permission);
ZEND_FUNCTION(safmq_queue_set_group_permission);
ZEND_FUNCTION(safmq_queue_delete_group_permission);
ZEND_FUNCTION(safmq_create_group);
ZEND_FUNCTION(safmq_delete_group);
ZEND_FUNCTION(safmq_create_user);
ZEND_FUNCTION(safmq_delete_user);
ZEND_FUNCTION(safmq_user_set_permissions);
ZEND_FUNCTION(safmq_user_get_permissions);
ZEND_FUNCTION(safmq_set_password);
ZEND_FUNCTION(safmq_group_add_user);
ZEND_FUNCTION(safmq_group_delete_user);
ZEND_FUNCTION(safmq_group_set_permissions);
ZEND_FUNCTION(safmq_group_get_permissions);
ZEND_FUNCTION(safmq_enumerate_queues);
ZEND_FUNCTION(safmq_enumerate_users);
ZEND_FUNCTION(safmq_enumerate_groups);
ZEND_FUNCTION(safmq_group_enumerate_users);
ZEND_FUNCTION(safmq_user_enumerate_groups);
ZEND_FUNCTION(safmq_queue_enumerate_permissions);
ZEND_FUNCTION(safmq_open_queue);
ZEND_FUNCTION(safmq_close_queue);
ZEND_FUNCTION(safmq_msg);
ZEND_FUNCTION(safmq_display_connections);
ZEND_FUNCTION(safmq_enqueue);
ZEND_FUNCTION(safmq_enqueue_with_relay);
ZEND_FUNCTION(safmq_close);
ZEND_FUNCTION(safmq_open_cursor);
ZEND_FUNCTION(safmq_close_cursor);
ZEND_FUNCTION(safmq_advance_cursor);
ZEND_FUNCTION(safmq_peek_cursor);
ZEND_FUNCTION(safmq_seek_id);
ZEND_FUNCTION(safmq_test_cursor);
ZEND_FUNCTION(safmq_peek_front);
ZEND_FUNCTION(safmq_peek_id);
ZEND_FUNCTION(safmq_begin_transaction);
ZEND_FUNCTION(safmq_commit_transaction);
ZEND_FUNCTION(safmq_rollback_transaction);
ZEND_FUNCTION(safmq_end_transaction);
ZEND_FUNCTION(safmq_add_user_identity);
ZEND_FUNCTION(safmq_remove_user_identity);
ZEND_FUNCTION(safmq_enumerate_user_identities);
ZEND_FUNCTION(safmq_get_queue_statistics);
ZEND_FUNCTION(safmq_get_server_statistics);
ZEND_FUNCTION(safmq_get_client_info);
ZEND_FUNCTION(safmq_create_temp_queue);

//zend/php init functions
PHP_MINFO_FUNCTION(safmq);
PHP_RINIT_FUNCTION(safmq);
PHP_RSHUTDOWN_FUNCTION(safmq);
PHP_MINIT_FUNCTION(safmq);
PHP_MSHUTDOWN_FUNCTION(safmq);
//info
void php_info_print_table_start();
void php_info_print_table_row(int x, char str[], char str2[]);
void php_info_print_table_end();

/*2nd function list so that the Zend engine will know what's here */
function_entry safmq_functions[] = {
	PHP_FE(safmq_get_server_major_protocol_version, NULL)
	PHP_FE(safmq_get_server_minor_protocol_version, NULL)
	PHP_FE(safmq_open, NULL)
	PHP_FE(safmq_cert_open, NULL)
	PHP_FE(safmq_retrieve, NULL)
	PHP_FE(safmq_retrieve_id, NULL)
	PHP_FE(safmq_retrieve_cursor, NULL)
	PHP_FE(safmq_create_queue, NULL)
	PHP_FE(safmq_delete_queue, NULL)
	PHP_FE(safmq_queue_set_user_permission, NULL)
	PHP_FE(safmq_queue_delete_user_permission, NULL)
	PHP_FE(safmq_queue_set_group_permission, NULL)
	PHP_FE(safmq_queue_delete_group_permission, NULL)
	PHP_FE(safmq_create_group, NULL)
	PHP_FE(safmq_delete_group, NULL)
	PHP_FE(safmq_create_user, NULL)
	PHP_FE(safmq_delete_user, NULL)
	PHP_FE(safmq_group_add_user, NULL)
	PHP_FE(safmq_group_delete_user, NULL)
	PHP_FE(safmq_user_set_permissions, NULL)
	PHP_FE(safmq_user_get_permissions, NULL)
	PHP_FE(safmq_set_password, NULL)
	PHP_FE(safmq_group_set_permissions, NULL)
	PHP_FE(safmq_group_get_permissions, NULL)
	PHP_FE(safmq_close, NULL)
	PHP_FE(safmq_enumerate_queues, NULL)
	PHP_FE(safmq_enumerate_users, NULL)
	PHP_FE(safmq_enumerate_groups, NULL)
	PHP_FE(safmq_group_enumerate_users, NULL)
	PHP_FE(safmq_user_enumerate_groups, NULL)
	PHP_FE(safmq_queue_enumerate_permissions, NULL)
	PHP_FE(safmq_close_queue, NULL)
	PHP_FE(safmq_open_queue, NULL)
	PHP_FE(safmq_msg, NULL)
	PHP_FE(safmq_display_connections, NULL)
	PHP_FE(safmq_enqueue, NULL)
	PHP_FE(safmq_enqueue_with_relay, NULL)
	PHP_FE(safmq_open_cursor, NULL)
	PHP_FE(safmq_close_cursor, NULL)
	PHP_FE(safmq_advance_cursor, NULL)
	PHP_FE(safmq_peek_cursor, NULL)
	PHP_FE(safmq_seek_id, NULL)
	PHP_FE(safmq_test_cursor, NULL)
	PHP_FE(safmq_peek_front, NULL)
	PHP_FE(safmq_peek_id, NULL)
	PHP_FE(safmq_begin_transaction, NULL)
	PHP_FE(safmq_commit_transaction, NULL)
	PHP_FE(safmq_rollback_transaction, NULL)
	PHP_FE(safmq_end_transaction, NULL)
	PHP_FE(safmq_add_user_identity, NULL)
	PHP_FE(safmq_remove_user_identity, NULL)
	PHP_FE(safmq_enumerate_user_identities, NULL)
	PHP_FE(safmq_get_queue_statistics, NULL)
	PHP_FE(safmq_get_server_statistics, NULL)
	PHP_FE(safmq_get_client_info, NULL)
	PHP_FE(safmq_create_temp_queue, NULL)
	{NULL, NULL, NULL}
};

/* module information */
zend_module_entry safmq_module_entry =
{	STANDARD_MODULE_HEADER,
	"safmq client",
	safmq_functions,
	PHP_MINIT(safmq),
	PHP_MSHUTDOWN(safmq),
	PHP_RINIT(safmq),
	PHP_RSHUTDOWN(safmq),
	PHP_MINFO(safmq),
	NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};

//#if COMPILE_DL_SAFMQ_MOD
ZEND_GET_MODULE(safmq)
//#endif

PHP_RINIT_FUNCTION(safmq){
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(safmq){
	return SUCCESS;
}

/* {{{ php_pgsql_init_globals
 */
static void php_safmq_init_globals(zend_safmq_globals *safmq_globals)
{
	memset(safmq_globals, 0, sizeof(zend_safmq_globals));
	/* Create Zend Global Variable Space */
}
/* }}} */

PHP_MINIT_FUNCTION(safmq){
	ZEND_INIT_MODULE_GLOBALS(safmq, php_safmq_init_globals, NULL);
	le_con = zend_register_list_destructors_ex(_close_safmq_con, NULL, LE_MSGqhandle_RESOURCE_NAME, module_number);
	return SUCCESS;
}

PHP_MINFO_FUNCTION(safmq){
	php_info_print_table_start();
	php_info_print_table_row(2, "PHP Message Queue Extension", "SAFMQ");
	php_info_print_table_end();
}

PHP_MSHUTDOWN_FUNCTION(safmq){
	#ifdef PHP_WIN32
		return SUCCESS;
	#else
//		return (FDFFinalize() == FDFErcOK) ? SUCCESS : FAILURE;
	#endif
}

}//end of extern c

PHP_FUNCTION(safmq_get_server_major_protocol_version){
	zval* php_con=NULL;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|r/", &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct usage: safmq_get_server_major_protocol_version(connection)");
		RETURN_FALSE;
    }

	MQConnection* con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	try {
		long version = con->GetServerMajorProtocolVersion();
		RETURN_LONG( version );
	} catch (ErrorCode e) {
		php_error(E_ERROR, "'%s' Encountered while getting major protocol version", EC_Decode(e));
		RETURN_FALSE;
	}
}

PHP_FUNCTION(safmq_get_server_minor_protocol_version){
	zval* php_con=NULL;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|r/", &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct usage: safmq_get_server_minor_protocol_version(connection)");
		RETURN_FALSE;
    }

	MQConnection* con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	try {
		long version = con->GetServerMinorProtocolVersion();
		RETURN_LONG( version );
	} catch (ErrorCode e) {
		php_error(E_ERROR, "'%s' Encountered while getting minor protocol version", EC_Decode(e));
		RETURN_FALSE;
	}
}

//creates connection and adds a con to a ZTS global vector default_con
PHP_FUNCTION(safmq_open){
	char *url = "";
	int urllen = 0;;
	char *login = "";
	int loginlen = 0;
	char *password = "";
	int passwordlen = 0;

	if ( ZEND_NUM_ARGS()!=1 && ZEND_NUM_ARGS() != 3 ) {
		php_error(E_ERROR, "%s() takes either one string or three strings. (Ex: 'safmqs://user:password@machine:port')", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
    }

	zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|ss", &url, &urllen, &login, &loginlen, &password, &passwordlen);
	if( urllen == 0 && (loginlen == 0 || passwordlen == 0) ) {
		php_error(E_ERROR, "%s() Empty Strings Passed To safmq_open(), (Ex: 'safmqs://user:password@machine:port')", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	try {
		MQConnection*	con = MQFactory::BuildConnection(std::string(url,urllen), std::string(login,loginlen), std::string(password,passwordlen));
		RETURN_RESOURCE( add_con_to_zend(con TSRMLS_CC) );
	} catch (ErrorCode ec) {
		set_errorcode(ec TSRMLS_CC);
	} catch (std::exception& e) {
		set_errorcode((ErrorCode)-1, e.what() TSRMLS_CC);
	}
	RETURN_FALSE;
}

//creates connection and adds a con to a ZTS global vector default_con
PHP_FUNCTION(safmq_cert_open){
	char *url = "";
	int urllen = 0;
	char *login = "";
	int loginlen = 0;
	char *password = "";
	int passwordlen = 0;

	char* keypath = "";
	int keypath_len = 0;
	char* certpath = "";
	int certpath_len = 0;


	if ( ZEND_NUM_ARGS() < 3 && ZEND_NUM_ARGS() > 5 ) {
		php_error(E_ERROR, "%s(): no defaults for %ld arguments; usage -- %s($url, $key_path, $cert_path, $userid, $password) : $userid and $password are optional ", get_active_function_name(TSRMLS_C), ZEND_NUM_ARGS(), get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
    }

	zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "sss|ss", &url, &urllen, &keypath, &keypath_len, &certpath, &certpath_len, &login, &loginlen, &password, &passwordlen);
	if( urllen == 0 && (loginlen == 0 || passwordlen == 0) ) {
		php_error(E_ERROR, "%s() Empty Strings Passed To safmq_open(), (Ex: 'safmqs://user:password@machine:port')", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	try {
		#ifdef SAFMQ_SSL
			tcpsocket::SSLContext		ctx;
			tcpsocket::PrivateKey		pk;
			tcpsocket::X509Certificate	cert;
			if (keypath_len) {
				pk.loadPEMFile(keypath);
				ctx.setPrivateKey(&pk);
			}
			if (certpath_len) {
				cert.loadPEMFile(certpath);
				ctx.setX509Certificate(&cert);
			}

			MQConnection*	con = MQFactory::BuildConnection(std::string(url,urllen), std::string(login,loginlen), std::string(password,passwordlen), &ctx);
			RETURN_RESOURCE( add_con_to_zend(con TSRMLS_CC) );
		#else
		php_error(E_ERROR, "SSL Compatability was not compiled into php_safmq.  Interface unavilable");
		#endif
	} catch (ErrorCode ec) {
		set_errorcode(ec TSRMLS_CC);
	} catch (std::exception& e) {
		set_errorcode((ErrorCode)-1, e.what() TSRMLS_CC);
	}
	RETURN_FALSE;
}


/*usefull information for debugging*/
PHP_FUNCTION(safmq_display_connections){
	vector<DefaultData>::iterator it;
	vector<DefaultData>& connections = SMQG(defaults);
	for( it = connections.begin(); it != connections.end(); it++ ) {
		zend_printf("Connection: %p ResID: %i\n", it->con, it->con_resid);
		vector<DefaultQHandle>::iterator qhit;
		for(qhit=it->default_qhandle.begin(); qhit!=it->default_qhandle.end(); qhit++) {
			zend_printf("\tQHandle: %i\n", qhit->qid);
			vector<long>::iterator qcit;
			for(qcit=qhit->default_qcursors.begin(); qcit!=qhit->default_qcursors.end(); qcit++)
				zend_printf("\t\tQCursor: %i\n", *qcit);
		}
	}
	RETURN_TRUE;
}

PHP_FUNCTION(safmq_close){
	zval *php_id = NULL;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r/", &php_id)==FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_close(connection). (You may have not passed a valid SafMQ connection resource)");
		RETURN_FALSE;
	}
	remove_con_FromZend(php_id TSRMLS_CC);
    RETURN_TRUE;
}

PHP_FUNCTION(safmq_enumerate_queues){
	zval* id=NULL;
	MQConnection* con = NULL;
	zval* queue_array;

	MAKE_STD_ZVAL(queue_array);
	array_init(queue_array);

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|r/", &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_enumerate_queues(connection). (You may have not passed a valid SafMQ connection resource)");
		RETURN_FALSE;
	}

	con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	enumerate_queues_users_groups(0, con, queue_array TSRMLS_CC);

	RETURN_ZVAL(queue_array, true, false);
}

PHP_FUNCTION(safmq_enumerate_users){
	zval* id=NULL;
	MQConnection* con = NULL;
	zval* queue_array;

	MAKE_STD_ZVAL(queue_array);
	array_init(queue_array);

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|r/", &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_enumerate_users(connection). (You may have not passed a valid SafMQ connection resource)");
		RETURN_FALSE;
	}

	con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	enumerate_queues_users_groups(1, con, queue_array TSRMLS_CC);

	RETURN_ZVAL(queue_array, true, false);
}

PHP_FUNCTION(safmq_enumerate_groups){
	zval* id=NULL;
	MQConnection* con = NULL;
	zval* queue_array;

	MAKE_STD_ZVAL(queue_array);
	array_init(queue_array);

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|r/", &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_enumerate_groups(connection). (You may have not passed a valid SafMQ connection resource)");
		RETURN_FALSE;
	}

	con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	enumerate_queues_users_groups(2, con, queue_array TSRMLS_CC);

	RETURN_ZVAL(queue_array, true, false);
}

PHP_FUNCTION(safmq_group_enumerate_users){
	zval* id=NULL;
	MQConnection* con = NULL;
	zval* queue_array;
	char* groupname;
	int groupnamelen=0;

	MAKE_STD_ZVAL(queue_array);
	array_init(queue_array);

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|r/", &groupname, &groupnamelen, &id)==FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_group_enumerate_users(group_name, connection).");
		RETURN_FALSE;
	}

	con = get_con_from_zend(id TSRMLS_CC);
	if( con == NULL ) {
		php_error(E_ERROR, "No connection created or bad connection passed.");
		RETURN_FALSE;
	}

	MQConnection::NAME_VECTOR name_vector;
	ErrorCode ec;

	ec = con->GroupGetUsers(groupname, name_vector);
	if( ec==EC_NOERROR ) {
		MQConnection::NAME_VECTOR::iterator it=name_vector.begin();
		while( it != name_vector.end() ) {
			add_next_index_string(queue_array, (char*)it->c_str(), 1);
			it++;
		}
	}
	RETURN_ZVAL(queue_array, true, false);
}

PHP_FUNCTION(safmq_user_enumerate_groups){
	zval* id=NULL;
	MQConnection* con = NULL;
	zval* queue_array;
	char* username;
	int usernamelen=0;

	MAKE_STD_ZVAL(queue_array);
	array_init(queue_array);

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|r/", &username, &usernamelen, &id)==FAILURE ) {
		php_error(E_ERROR, "Correct usage:safmq_user_enumerate_groups(username, connection).");
		RETURN_FALSE;
	}

	con = get_con_from_zend(id TSRMLS_CC);
	if( con == NULL ) {
		php_error(E_ERROR, "No connection created or bad connection passed.");
		RETURN_FALSE;
	}

	MQConnection::NAME_VECTOR name_vector;
	ErrorCode ec;

	ec = con->UserGetGroups(username, name_vector);
	if( ec==EC_NOERROR ) {
		MQConnection::NAME_VECTOR::iterator it=name_vector.begin();
		while( it != name_vector.end() ) {
			add_next_index_string(queue_array, (char*)it->c_str(), 1);
			it++;
		}
	}
	RETURN_ZVAL(queue_array, true, false);
}

PHP_FUNCTION(safmq_queue_enumerate_permissions){
	zval* id=NULL;
	MQConnection* con = NULL;
	char* qname;
	int qnamelen=0;
	vector<QueuePermissions> qperms;

	zval* queue_array;
	MAKE_STD_ZVAL(queue_array);
	array_init(queue_array);

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|r/", &qname, &qnamelen, &id)==FAILURE ) {
		php_error(E_ERROR, "Correct usage:safmq_queue_enumerate_permissions(queue_name, connection).");
		RETURN_FALSE;
	}

	con = get_con_from_zend(id TSRMLS_CC);
	if( con == NULL ) {
		php_error(E_ERROR, "No connection created or bad connection passed.");
		RETURN_FALSE;
	}

	ErrorCode ec;
	zval* acl_value;

	ec = con->QueueEnumeratePermissions(qname, qperms);
	if( ec==EC_NOERROR ) {
		vector<QueuePermissions>::iterator iter = qperms.begin();
		while( iter != qperms.end() ) {
			MAKE_STD_ZVAL(acl_value);//init with zend that its new
			object_init(acl_value);//init with zend that its new

			add_property_string(acl_value, "Entity", (char*)iter->entity.c_str(), true);
			add_property_bool(acl_value, "IsGroup", iter->isgroup);
			add_property_bool(acl_value, "Read", iter->read);
			add_property_bool(acl_value, "Write", iter->write);
			add_property_bool(acl_value, "Destroy", iter->destroy);
			add_property_bool(acl_value, "ChangeSecurity", iter->change_security);
			add_next_index_zval(queue_array, acl_value);
			iter++;
		}
	} else {
		php_error(E_ERROR, "'%s' encountered while enumerating permissions in a queue.", EC_Decode(ec));
	}
	RETURN_ZVAL(queue_array, true, false);
}

PHP_FUNCTION(safmq_open_queue){
	char*						queuename;
	int							queuenamelen = 0;
	zval*						id=NULL;
	MQConnection::QueueHandle	queuehandle;

	if ( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|r/", &queuename, &queuenamelen, &id)== FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_open_queue(queue_name, connection).", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
    }

	MQConnection* con = get_con_from_zend(id TSRMLS_CC);
	if( con!=NULL ) {
		ErrorCode ec = con->OpenQueue(queuename, queuehandle);
		if( ec == EC_NOERROR ) {
			register_default_qhandle(con,queuehandle TSRMLS_CC);
			RETURN_LONG(queuehandle);
		} else {
			set_errorcode(ec TSRMLS_CC);
			php_error(E_ERROR, "'%s' encountered while opening queue", EC_Decode(ec));
		}
	} else {
		php_error(E_ERROR, "No Connections Available.");
	}
	RETURN_FALSE;
}

//close queues
PHP_FUNCTION(safmq_close_queue){
	long						queuehandle=0;
	zval*						id=NULL;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|lr/", &queuehandle, &id) == FAILURE) {
		php_error(E_ERROR, "Correct usage: safmq_close_queue(queue_handle, connection). (You may have not passed a valid SafMQ connection resource)", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
    }

	MQConnection* con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Unto Which A Queue Can Be Found Open(Queues Are Closed Prior To Connections).");
		RETURN_FALSE;
	}
	if( ZEND_NUM_ARGS() == 0 ) {
		queuehandle=get_default_qhandle(con, queuehandle TSRMLS_CC);
	}

	remove_default_qhandle(con, queuehandle TSRMLS_CC);
	ErrorCode ec = con->CloseQueue((MQConnection::QueueHandle)queuehandle);
	if (ec == EC_NOERROR)
		RETURN_TRUE;

	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_retrieve){
	zval*						id = NULL;
	long						qid = 0;
	QueueMessage				msg;
	zval*						msg_object;
	string						temp;
	bool						get_body=true;
	long						timeout=0;
	MQConnection::QueueHandle queuehandle=0;

	MAKE_STD_ZVAL(msg_object);
	object_init(msg_object);

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|bllr/", &get_body, &timeout, &qid, &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_retrieve(bool get_body, int timeout, queuehandle, connection).");
		RETURN_FALSE;
	}

	MQConnection* con = get_con_from_zend(id TSRMLS_CC);

	if( ZEND_NUM_ARGS() == 0 ) {
		queuehandle = get_default_qhandle(con, qid TSRMLS_CC);
	} else {
		queuehandle = (MQConnection::QueueHandle)qid;
	}

	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ecq = con->Retrieve(queuehandle, get_body, timeout, msg);

	if( ecq == EC_NOERROR ) {
		long			lng;
		short			shrt;
		char			chr;
		stringstream	mp, mid, rid, ttl, ttlew;

		msg.getResponseQueueName(temp);
		add_property_string(msg_object,"ResponseQueueName", (char*)temp.c_str(), true);
		add_property_string(msg_object, "Label", (char*)msg.getLabel().c_str(), true);
		switch ( msg.getBodyType() ) {
			case BT_LONG:
				*msg.getBufferStream() >> lng;
				add_property_long(msg_object,"Body",lng);
				break;
			case BT_SHORT:
				*msg.getBufferStream() >> shrt;
				add_property_long(msg_object,"Body",shrt);
				break;
			case BT_CHAR:
				*msg.getBufferStream() >> chr;
				add_property_long(msg_object,"Body",chr);
				break;
			case BT_WTEXT:
				/// Body type containging binary data
				break;
			case BT_BINARY:
				break;
			case BT_TEXT://it was not getting the body...so this just defaults to something that works
			default:
				temp.assign(msg.getBufferStream()->buffer(),msg.getBufferStream()->length());
				add_property_string(msg_object, "Body", (char*)temp.c_str(), true);
				break;
		}
		mid << msg.getMessageID();
		mid >> temp;
		add_property_string(msg_object,"MessageID", (char*)temp.c_str(), true);

		rid << msg.getReciptID();
		rid >> temp;
		add_property_string(msg_object,"ReciptID", (char*)temp.c_str(), true);

		ttl << msg.getTimeToLiveSeconds();
		ttl >> temp;
		add_property_string(msg_object, "TTL", (char*)temp.c_str(), true);

		ttlew << msg.getTTLErrorWanted();
		ttlew >> temp;
		add_property_string(msg_object, "TTLErrorWanted", (char*)temp.c_str(), true);

		mp << msg.getMessagePriority();
		mp >> temp;
		add_property_string(msg_object,"MessagePriority", (char*)temp.c_str(), true);

		time_t t = msg.getTimeStamp();
		string ts = ctime(&t);//try strftime() with %Y%m%d %H:%M-%S use the help and strftime in the index
		add_property_string(msg_object, "TimeStamp", (char*)ts.c_str(), true);

		RETURN_ZVAL(msg_object, true, false);
	}
	
	if( ecq == EC_NOMOREMESSAGES || ecq == EC_TIMEDOUT) {
		set_errorcode(ecq TSRMLS_CC);
		RETURN_FALSE;
	}
		
	php_error(E_ERROR, "'%s' encountered while retrieving message.", EC_Decode(ecq));
	RETURN_FALSE;
}
//Search by reciptID for a message
PHP_FUNCTION(safmq_retrieve_id){
	zval*						id = NULL;
	long						qid = 0;
	QueueMessage				msg;
	zval*						msg_object;
	string						temp;
	bool						get_body=true;
	long						timeout=0;
	zval*						zrid = NULL;
	MQConnection::QueueHandle queuehandle=0;

	MAKE_STD_ZVAL(msg_object);
	object_init(msg_object);

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "z|bllr/", &zrid, &get_body, &timeout, &qid, &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_retrieve_id(msg->reciptid, bool get_body, int timeout, queuehandle, connection).");
		RETURN_FALSE;
	}
	MQConnection* con = get_con_from_zend(id TSRMLS_CC);
	if( qid==0 ) {
		queuehandle = get_default_qhandle(con, qid TSRMLS_CC);
	} else {
		queuehandle = (MQConnection::QueueHandle)qid;
	}

	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}

	stringstream	s;
	uuid			rcpt_uuid;
	memset(&rcpt_uuid,0,sizeof(rcpt_uuid));
	if (Z_STRVAL(*zrid)) {
		s.write(Z_STRVAL(*zrid),Z_STRLEN(*zrid));
		s >> rcpt_uuid;
	} else {
		php_error(E_ERROR, "Invalid Receipt ID passed.");
	}

	ErrorCode ecq = con->RetrieveID(queuehandle, get_body, rcpt_uuid, timeout, msg);

	if( ecq==EC_NOERROR ) {
		long			lng;
		short			shrt;
		char			chr;
		stringstream	mp, mid, rid, ttl, ttlew;

		msg.getResponseQueueName(temp);
		add_property_string(msg_object,"ResponseQueueName", (char*)temp.c_str(), true);
		add_property_string(msg_object, "Label", (char*)msg.getLabel().c_str(), true);
		switch ( msg.getBodyType() ) {
			case BT_LONG:
				*msg.getBufferStream() >> lng;
				add_property_long(msg_object,"Body",lng);
				break;
			case BT_SHORT:
				*msg.getBufferStream() >> shrt;
				add_property_long(msg_object,"Body",shrt);
				break;
			case BT_CHAR:
				*msg.getBufferStream() >> chr;
				add_property_long(msg_object,"Body",chr);
				break;
			case BT_WTEXT:
				/// Body type containging binary data
				break;
			case BT_BINARY:
				break;
			case BT_TEXT://it was not getting the body...so this just defaults to something that works
			default:
				temp.assign(msg.getBufferStream()->buffer(),msg.getBufferStream()->length());
				add_property_string(msg_object, "Body", (char*)temp.c_str(), true);
				break;
		}
		mid << msg.getMessageID();
		mid >> temp;
		add_property_string(msg_object,"MessageID", (char*)temp.c_str(), true);

		rid << msg.getReciptID();
		rid >> temp;
		add_property_string(msg_object,"ReciptID", (char*)temp.c_str(), true);

		ttl << msg.getTimeToLiveSeconds();
		ttl >> temp;
		add_property_string(msg_object, "TTL", (char*)temp.c_str(), true);

		ttlew << msg.getTTLErrorWanted();
		ttlew >> temp;
		add_property_string(msg_object, "TTLErrorWanted", (char*)temp.c_str(), true);

		mp << msg.getMessagePriority();
		mp >> temp;
		add_property_string(msg_object,"MessagePriority", (char*)temp.c_str(), true);

		time_t t = msg.getTimeStamp();
		string ts = ctime(&t);//try strftime() with %Y%m%d %H:%M-%S use the help and strftime in the index
		add_property_string(msg_object, "TimeStamp", (char*)ts.c_str(), true);

		RETURN_ZVAL(msg_object, true, false);
	}
	if( ecq==EC_NOMOREMESSAGES || ecq == EC_TIMEDOUT) {
		set_errorcode(ecq TSRMLS_CC);
		RETURN_FALSE;
	}

	php_error(E_ERROR, "'%s' encountered while retrieving message.", EC_Decode(ecq));
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_retrieve_cursor){
	zval*						id = NULL;
	long						qid = 0;
	bool						get_body = false;
	long						cursorID=-1;
	QueueMessage				msg;
	zval*						new_msg=NULL;
	MQConnection::QueueHandle	queuehandle=0;

	MAKE_STD_ZVAL(new_msg);
	object_init(new_msg);

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|bllr/", &get_body, &cursorID, &qid, &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_retrieve_cursor(bool get_body, cursor, queuehandle, connection)");
		RETURN_FALSE;
	}

	MQConnection* con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	queuehandle = get_default_qhandle(con, qid TSRMLS_CC);

	if( queuehandle == 0 ) {
		php_error(E_ERROR, "No Open Queues Available Or Bad Queue Handle Passed.");
		RETURN_FALSE;
	}
	if( queuehandle!=0 && con!=NULL ) {
		cursorID = get_default_cursorID(con, queuehandle, cursorID TSRMLS_CC);
	}

	try {
		ErrorCode ec = con->RetrieveCursor(queuehandle, get_body, cursorID, msg);//catches bad cursors
		#if _DEBUG
		printf("RetrieveCursor: %s\n", EC_Decode(ec));
		#endif
		
		if( ec != EC_NOERROR )
			throw ec;
		string			temp;
		long			lng;
		short			shrt;
		char			chr;
		stringstream	mp, mid, rid, ttl, ttlew;

		msg.getResponseQueueName(temp);
		add_property_string(new_msg,"ResponseQueueName", (char*)temp.c_str(), true);
		add_property_string(new_msg, "Label", (char*)msg.getLabel().c_str(), true);
		switch ( msg.getBodyType() ) {
			case BT_LONG:
				*msg.getBufferStream() >> lng;
				add_property_long(new_msg,"Body",lng);
				break;
			case BT_SHORT:
				*msg.getBufferStream() >> shrt;
				add_property_long(new_msg,"Body",shrt);
				break;
			case BT_CHAR:
				*msg.getBufferStream() >> chr;
				add_property_long(new_msg,"Body",chr);
				break;
			case BT_WTEXT:
				/// Body type containging binary data
				break;
			case BT_BINARY:
				break;
			case BT_TEXT://it was not getting the body...so this just defaults to something that works
			default:
				temp.assign(msg.getBufferStream()->buffer(),msg.getBufferStream()->length());
				add_property_string(new_msg, "Body", (char*)temp.c_str(), true);
				break;
		}
		mid << msg.getMessageID();
		mid >> temp;
		add_property_string(new_msg,"MessageID", (char*)temp.c_str(), true);

		rid << msg.getReciptID();
		rid >> temp;
		add_property_string(new_msg,"ReciptID", (char*)temp.c_str(), true);

		ttl << msg.getTimeToLiveSeconds();
		ttl >> temp;
		add_property_string(new_msg, "TTL", (char*)temp.c_str(), true);

		ttlew << msg.getTTLErrorWanted();
		ttlew >> temp;
		add_property_string(new_msg, "TTLErrorWanted", (char*)temp.c_str(), true);

		mp << msg.getMessagePriority();
		mp >> temp;
		add_property_string(new_msg,"MessagePriority", (char*)temp.c_str(), true);

		time_t t = msg.getTimeStamp();
		string ts = ctime(&t);//try strftime() with %Y%m%d %H:%M-%S use the help and strftime in the index
		add_property_string(new_msg, "TimeStamp", (char*)ts.c_str(), true);

		RETURN_ZVAL(new_msg, true, false);
	} catch (ErrorCode ec) {
		if( ec == EC_NOMOREMESSAGES ) {
			RETURN_FALSE;
		}
		php_error(E_ERROR, "'%s' Encountered while retrieving from cursor.", EC_Decode(ec));
	}
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_open_cursor) {
	zval*						id = NULL;
	long						qid = 0;
	MQConnection::QueueHandle	queuehandle=0;
	MQConnection::CursorHandle	cursorID=0;

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|lr/", &qid, &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_open_cursor(queuehandle, connection).");
		RETURN_FALSE;
	}

	MQConnection* con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	queuehandle = get_default_qhandle(con, qid TSRMLS_CC);

	if( queuehandle == 0 ) {
		php_error(E_ERROR, "No Open Queues Available Or Bad Queue Handle Passed.");
		RETURN_FALSE;
	}

	ErrorCode ec = con->OpenCursor(queuehandle, cursorID);
	if (ec == EC_NOERROR) {
		register_default_qcursor(con, queuehandle, cursorID TSRMLS_CC);
		RETURN_LONG(cursorID);
	} 
	php_error(E_ERROR, "'%s' Encountered while opening a cursor", EC_Decode(ec));
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_close_cursor) {
	zval*						id = NULL;
	long						qid = 0;
	bool						get_body = false;
	long						cursorID=-1;
	MQConnection::QueueHandle	queuehandle=0;

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|llr/", &cursorID, &qid, &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_close_cursor(cursorid, queuehandle, connection).");
		RETURN_FALSE;
	}

	MQConnection* con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	queuehandle = get_default_qhandle(con, qid TSRMLS_CC);
	if( queuehandle == 0 ) {
		php_error(E_ERROR, "No Open Queues Available Or Bad Queue Handle Passed.");
		RETURN_FALSE;
	}
	if( queuehandle != 0 && con != NULL ) {
		cursorID = remove_default_cursorID(con, queuehandle, cursorID TSRMLS_CC);
		try {
			ErrorCode ec = con->CloseCursor(queuehandle, cursorID);
		} catch (ErrorCode ec) {
			php_error(E_ERROR, "'%s' Encountered while closing a cursor", EC_Decode(ec));
			RETURN_FALSE;
		}
	} else {
		php_error(E_ERROR, "Invalid queue handle passed.");
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_FUNCTION(safmq_advance_cursor) {
	zval*						id = NULL;
	long						qid = 0;
	long						cursorID=-1;
	MQConnection::QueueHandle	queuehandle=0;

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|llr/", &cursorID, &qid, &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_advance_cursor(cursorid, queuehandle, connection).");
		RETURN_FALSE;
	}

	MQConnection* con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	queuehandle = get_default_qhandle(con, qid TSRMLS_CC);

	if( queuehandle == 0 ) {
		php_error(E_ERROR, "No Open Queues Available Or Bad Queue Handle Passed.");
		RETURN_FALSE;
	}
	if( queuehandle!=0 && con!=NULL ) {
		cursorID = get_default_cursorID(con, queuehandle, cursorID TSRMLS_CC);
	}

	ErrorCode ec = con->AdvanceCursor(queuehandle, cursorID);
	if (ec == EC_NOERROR) {
		RETURN_TRUE;
	} else if (ec == EC_NOMOREMESSAGES) {
		set_errorcode(ec TSRMLS_CC);
		RETURN_FALSE;
	}
	php_error(E_ERROR, "'%s' Encountered while advancing the cursor", EC_Decode(ec));
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_seek_id) {
	zval*						id = NULL;
	long						qid = 0;
	long						cursorID=-1;
	MQConnection::QueueHandle	queuehandle=0;
	long						timeout=0;
	zval*						zrid = NULL;

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "z|lllr/", &zrid, &timeout, &cursorID, &qid, &id) == FAILURE ) {
		php_error(E_ERROR, "Correct use: safmq_seek_id(reciptID, timeout, cursorid, queuehandle, connection)");
		RETURN_FALSE;
	}

	MQConnection* con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	queuehandle = get_default_qhandle(con, qid TSRMLS_CC);

	if( queuehandle == 0 ) {
		php_error(E_ERROR, "No Open Queues Available Or Bad Queue Handle Passed.");
		RETURN_FALSE;
	}
	if( queuehandle!=0 && con!=NULL ) {
		cursorID = get_default_cursorID(con, queuehandle, cursorID TSRMLS_CC);
	}

	stringstream	s;
	uuid			rcpt_uuid;
	memset(&rcpt_uuid,0,sizeof(rcpt_uuid));
	s.write(Z_STRVAL(*zrid),Z_STRLEN(*zrid));
	s >> rcpt_uuid;

	ErrorCode ec = con->SeekID(queuehandle, rcpt_uuid, timeout, cursorID);

	#if _DEBUG
	cout << "SeekID: " << EC_Decode(ec) << endl;
	#endif

	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_test_cursor) {
	zval*						id = NULL;
	long						qid = 0;
	long						cursorID=-1;
	MQConnection::QueueHandle	queuehandle=0;

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|llr/", &cursorID, &qid, &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_test_cursor(cursorid, queuehandle, connection).");
		RETURN_FALSE;
	}

	MQConnection* con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	queuehandle = get_default_qhandle(con, qid TSRMLS_CC);

	if( queuehandle == 0 ) {
		php_error(E_ERROR, "No Open Queues Available Or Bad Queue Handle Passed.");
		RETURN_FALSE;
	}
	if( queuehandle!=0 && con!=NULL ) {
		cursorID = get_default_cursorID(con, queuehandle, cursorID TSRMLS_CC);
	}

	ErrorCode ec;
	try {
		ec = con->TestCursor(queuehandle, cursorID);
		if( ec!=EC_NOERROR )
			throw ec;
	} catch (ErrorCode ec) {
		if( ec==EC_ERROR )
			php_error(E_ERROR, "'%s' encountered while testing the cursor.", EC_Decode(ec));
		set_errorcode(ec TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_FUNCTION(safmq_peek_front) {
	zval*						id = NULL;
	long						qid = 0;
	bool						get_body = false;
	long						cursorID=-1;
	long						timeout=0;
	QueueMessage				msg;
	zval*						new_msg=NULL;
	MQConnection::QueueHandle	queuehandle=0;

	MAKE_STD_ZVAL(new_msg);
	object_init(new_msg);

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|bllr/", &get_body, &timeout, &qid, &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_peek_front(bool getbody, int timeout, queuehandle, connection).");
		RETURN_FALSE;
	}

	MQConnection* con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	queuehandle = get_default_qhandle(con, qid TSRMLS_CC);

	if( queuehandle == 0 ) {
		php_error(E_ERROR, "No Open Queues Available Or Bad Queue Handle Passed.");
		RETURN_FALSE;
	}

	try {
		ErrorCode ec = con->PeekFront(queuehandle, get_body, timeout, msg);//this catches a bad cursor
		if( ec!=EC_NOERROR )
			throw ec;
		string			temp;
		long			lng;
		short			shrt;
		char			chr;
		stringstream	mp, mid, rid, ttl, ttlew;

		msg.getResponseQueueName(temp);
		add_property_string(new_msg,"ResponseQueueName", (char*)temp.c_str(), true);
		add_property_string(new_msg, "Label", (char*)msg.getLabel().c_str(), true);
		switch ( msg.getBodyType() ) {
			case BT_LONG:
				*msg.getBufferStream() >> lng;
				add_property_long(new_msg,"Body",lng);
				break;
			case BT_SHORT:
				*msg.getBufferStream() >> shrt;
				add_property_long(new_msg,"Body",shrt);
				break;
			case BT_CHAR:
				*msg.getBufferStream() >> chr;
				add_property_long(new_msg,"Body",chr);
				break;
			case BT_WTEXT:
				/// Body type containging binary data
				break;
			case BT_BINARY:
				break;
			case BT_TEXT://it was not getting the body...so this just defaults to something that works
			default:
				temp.assign(msg.getBufferStream()->buffer(),msg.getBufferStream()->length());
				add_property_string(new_msg, "Body", (char*)temp.c_str(), true);
				break;
		}
		mid << msg.getMessageID();
		mid >> temp;
		add_property_string(new_msg,"MessageID", (char*)temp.c_str(), true);

		rid << msg.getReciptID();
		rid >> temp;
		add_property_string(new_msg,"ReciptID", (char*)temp.c_str(), true);

		ttl << msg.getTimeToLiveSeconds();
		ttl >> temp;
		add_property_string(new_msg, "TTL", (char*)temp.c_str(), true);

		ttlew << msg.getTTLErrorWanted();
		ttlew >> temp;
		add_property_string(new_msg, "TTLErrorWanted", (char*)temp.c_str(), true);

		mp << msg.getMessagePriority();
		mp >> temp;
		add_property_string(new_msg,"MessagePriority", (char*)temp.c_str(), true);

		time_t t = msg.getTimeStamp();
		string ts = ctime(&t);//try strftime() with %Y%m%d %H:%M-%S use the help and strftime in the index
		add_property_string(new_msg, "TimeStamp", (char*)ts.c_str(), true);

		RETURN_ZVAL(new_msg, true, false);
	} catch (ErrorCode ec) {
		if( ec == EC_NOMOREMESSAGES || ec == EC_TIMEDOUT) {
			set_errorcode(ec TSRMLS_CC);
			RETURN_FALSE;
		}
		php_error(E_ERROR, "'%s' encountered while peeking at cursor.", EC_Decode(ec));
	}
	set_errorcode((ErrorCode)-1, "UNKNOWN" TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_peek_id){
	zval*						id = NULL;
	long						qid = 0;
	QueueMessage				msg;
	zval*						msg_object;
	string						temp;
	bool						get_body=true;
	long						timeout=0;
	zval*						zrid = NULL;
	MQConnection::QueueHandle queuehandle=0;

	MAKE_STD_ZVAL(msg_object);
	object_init(msg_object);

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "z|bllr/", &zrid, &get_body, &timeout, &qid, &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_peek_id(reciptid, bool get_body, timeout, queuehandle, connection).");
		RETURN_FALSE;
	}
	MQConnection* con						= get_con_from_zend(id TSRMLS_CC);
	if( qid==0 ) {
		queuehandle = get_default_qhandle(con, qid TSRMLS_CC);
	} else {
		queuehandle = (MQConnection::QueueHandle)qid;
	}

	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}

	stringstream	s;
	uuid			rcpt_uuid;
	memset(&rcpt_uuid, 0, sizeof(rcpt_uuid));
	
	if (Z_STRVAL(*zrid)) {
		std::string uuid = Z_STRVAL(*zrid);
		if (uuid.length() && uuid[0] == '{')
			uuid = uuid.substr(1);
		if (uuid.length() && uuid[uuid.length()-1] == '}')
			uuid = uuid.substr(0, uuid.length() - 1);
		s << uuid;
		s >> rcpt_uuid;
		
	#if _DEBUG
		printf("\nsafmq_peek_id-guid:%s\n", uuid.c_str());
		cout << "parsed guid: " << rcpt_uuid << endl;
	#endif
	}
	
	ErrorCode ecq = con->PeekID(queuehandle, get_body, rcpt_uuid, timeout, msg);

	if( ecq == EC_NOERROR ) {
		long			lng;
		short			shrt;
		char			chr;
		stringstream	mp, mid, rid, ttl, ttlew;

		msg.getResponseQueueName(temp);
		add_property_string(msg_object,"ResponseQueueName", (char*)temp.c_str(), true);
		add_property_string(msg_object, "Label", (char*)msg.getLabel().c_str(), true);
		switch ( msg.getBodyType() ) {
			case BT_LONG:
				*msg.getBufferStream() >> lng;
				add_property_long(msg_object,"Body",lng);
				break;
			case BT_SHORT:
				*msg.getBufferStream() >> shrt;
				add_property_long(msg_object,"Body",shrt);
				break;
			case BT_CHAR:
				*msg.getBufferStream() >> chr;
				add_property_long(msg_object,"Body",chr);
				break;
			case BT_WTEXT:
				/// Body type containging binary data
				break;
			case BT_BINARY:
				break;
			case BT_TEXT://it was not getting the body...so this just defaults to something that works
			default:
				temp.assign(msg.getBufferStream()->buffer(),msg.getBufferStream()->length());
				add_property_string(msg_object, "Body", (char*)temp.c_str(), true);
				break;
		}
		mid << msg.getMessageID();
		mid >> temp;
		add_property_string(msg_object,"MessageID", (char*)temp.c_str(), true);

		rid << msg.getReciptID();
		rid >> temp;
		add_property_string(msg_object,"ReciptID", (char*)temp.c_str(), true);

		ttl << msg.getTimeToLiveSeconds();
		ttl >> temp;
		add_property_string(msg_object, "TTL", (char*)temp.c_str(), true);

		ttlew << msg.getTTLErrorWanted();
		ttlew >> temp;
		add_property_string(msg_object, "TTLErrorWanted", (char*)temp.c_str(), true);

		mp << msg.getMessagePriority();
		mp >> temp;
		add_property_string(msg_object,"MessagePriority", (char*)temp.c_str(), true);

		time_t t = msg.getTimeStamp();
		string ts = ctime(&t);//try strftime() with %Y%m%d %H:%M-%S use the help and strftime in the index
		add_property_string(msg_object, "TimeStamp", (char*)ts.c_str(), true);

		RETURN_ZVAL(msg_object, true, false);
	}
	if( ecq == EC_NOMOREMESSAGES || ecq == EC_TIMEDOUT ){
		set_errorcode(ecq TSRMLS_CC);
		RETURN_FALSE;
	}

	php_error(E_ERROR, "'%s' encountered while peeking by ID for a message.", EC_Decode(ecq));
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_peek_cursor) {
	zval*						id = NULL;
	long						qid = 0;
	bool						get_body = false;
	long						cursorID=-1;
	QueueMessage				msg;
	zval*						new_msg=NULL;
	MQConnection::QueueHandle	queuehandle=0;

	MAKE_STD_ZVAL(new_msg);
	object_init(new_msg);

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|bllr/", &get_body, &cursorID, &qid, &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_peek_cursor(bool get_body, cursorid, queuehandle, connection).");
		RETURN_FALSE;
	}

	MQConnection* con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	queuehandle = get_default_qhandle(con, qid TSRMLS_CC);

	if( queuehandle == 0 ) {
		php_error(E_ERROR, "No Open Queues Available Or Bad Queue Handle Passed.");
		RETURN_FALSE;
	}
	if( queuehandle!=0 && con!=NULL ) {
		cursorID = get_default_cursorID(con, queuehandle, cursorID TSRMLS_CC);
	}

	try {
		ErrorCode ec = con->PeekCursor(queuehandle, get_body, cursorID, msg);//this catches a bad cursor
		if( ec!=EC_NOERROR )
			throw ec;
		string			temp;
		long			lng;
		short			shrt;
		char			chr;
		stringstream	mp, mid, rid, ttl, ttlew;

		msg.getResponseQueueName(temp);
		add_property_string(new_msg,"ResponseQueueName", (char*)temp.c_str(), true);
		add_property_string(new_msg, "Label", (char*)msg.getLabel().c_str(), true);
		switch ( msg.getBodyType() ) {
			case BT_LONG:
				*msg.getBufferStream() >> lng;
				add_property_long(new_msg,"Body",lng);
				break;
			case BT_SHORT:
				*msg.getBufferStream() >> shrt;
				add_property_long(new_msg,"Body",shrt);
				break;
			case BT_CHAR:
				*msg.getBufferStream() >> chr;
				add_property_long(new_msg,"Body",chr);
				break;
			case BT_WTEXT:
				/// Body type containging binary data
				break;
			case BT_BINARY:
				break;
			case BT_TEXT://it was not getting the body...so this just defaults to something that works
			default:
				temp.assign(msg.getBufferStream()->buffer(),msg.getBufferStream()->length());
				add_property_string(new_msg, "Body", (char*)temp.c_str(), true);
				break;
		}
		mid << msg.getMessageID();
		mid >> temp;
		add_property_string(new_msg,"MessageID", (char*)temp.c_str(), true);

		rid << msg.getReciptID();
		rid >> temp;
		add_property_string(new_msg,"ReciptID", (char*)temp.c_str(), true);

		ttl << msg.getTimeToLiveSeconds();
		ttl >> temp;
		add_property_string(new_msg, "TTL", (char*)temp.c_str(), true);

		ttlew << msg.getTTLErrorWanted();
		ttlew >> temp;
		add_property_string(new_msg, "TTLErrorWanted", (char*)temp.c_str(), true);

		mp << msg.getMessagePriority();
		mp >> temp;
		add_property_string(new_msg,"MessagePriority", (char*)temp.c_str(), true);

		time_t t = msg.getTimeStamp();
		string ts = ctime(&t);//try strftime() with %Y%m%d %H:%M-%S use the help and strftime in the index
		add_property_string(new_msg, "TimeStamp", (char*)ts.c_str(), true);

		RETURN_ZVAL(new_msg, true, false);
	} catch (ErrorCode ec) {
		if( ec==EC_NOMOREMESSAGES || ec == EC_TIMEDOUT ) {
			set_errorcode(ec TSRMLS_CC);
			RETURN_FALSE;
		}
		php_error(E_ERROR, "'%s' Encountered while peeking at cursor.", EC_Decode(ec));
	}
	set_errorcode((ErrorCode)-1, "UNKNOWN" TSRMLS_CC);
	RETURN_FALSE;
}

/*just a predefined object the user can fill out and send back for easy of msg creation ect..*/
PHP_FUNCTION(safmq_msg){
	zval*						msg_object;
	string						temp;

	if( ZEND_NUM_ARGS() > 0 ) {
		php_error(E_ERROR, "Correct usage: safmq_msg().");
	}

	MAKE_STD_ZVAL(msg_object);
	if(object_init(msg_object) == SUCCESS){
		add_property_string(msg_object, "ResponseQueueName", "", true);
		add_property_string(msg_object, "Label", "", true);
		add_property_string(msg_object, "Body", "", true);
		add_property_string(msg_object, "MessageID", "", true);
		add_property_string(msg_object, "ReciptID", "", true);
		add_property_string(msg_object, "TTL", "0", true);
		add_property_string(msg_object, "TTLErrorWanted", "0", true);
		add_property_string(msg_object, "MessagePriority", "1", true);
		add_property_string(msg_object, "TimeStamp", "", true);
	} else {
		php_error(E_ERROR, "Unable ot initialize message object.");
		RETURN_FALSE;
	}

	/*
	zend_hash_update(
		EG(active_symbol_table),
		"safmq_msg",
		strlen("safmq_msg") + 1,
		&msg_object,
		sizeof(zval *),
		NULL
	);
	*/

	RETURN_ZVAL(msg_object, true, false);
}

PHP_FUNCTION(safmq_enqueue) {
/*The user passes in the connection, queue and object*/
	QueueMessage				qmessage;
	zval*						id = NULL;
	zval						*zvRQN, *zvLabel, *zvBody, *zvReciptID, *zvTTL, *zvTTLEW, *zvMsgPriority;
	long 						qid=0;
	char rqn[] =				"ResponseQueueName";
	char label[] =				"Label";
	char body[] =				"Body";
	char mid[] =				"MessageID";
	char reciptid[] =			"ReciptID";
	char ttl[] =				"TTL";
	char ttlew[] =				"TTLErrorWanted";
	char msgpriority[] =		"MessagePriority";
	char timestamp[] =			"TimeStamp";

	zval*						zend_obj=NULL;
	zend_class_entry* scope =	zend_standard_class_def;

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "o/|lr/", &zend_obj, &qid, &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_enqueue(Object msg, queuehandle, connection). (Recommend: $msg=safmq_msg();)");
		RETURN_FALSE;
	}

	zvRQN =			zend_read_property(scope,zend_obj,rqn,			sizeof(rqn)				-1,			0 TSRMLS_CC);//the zero's are bools and are for silent
	zvLabel =		zend_read_property(scope,zend_obj,label,		sizeof(label)			-1,			0 TSRMLS_CC);
	zvBody =		zend_read_property(scope,zend_obj,body,			sizeof(body)			-1,			0 TSRMLS_CC);
	zvReciptID =	zend_read_property(scope,zend_obj,reciptid,		sizeof(reciptid)		-1,			0 TSRMLS_CC);
	zvTTL =			zend_read_property(scope,zend_obj,ttl,			sizeof(ttl)				-1,			0 TSRMLS_CC);
	zvTTLEW =		zend_read_property(scope,zend_obj,ttlew,		sizeof(ttlew)			-1,			0 TSRMLS_CC);
	zvMsgPriority =	zend_read_property(scope,zend_obj,msgpriority,	sizeof(msgpriority)		-1,			0 TSRMLS_CC);

	MQConnection* con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	MQConnection::QueueHandle queuehandle = get_default_qhandle(con, qid TSRMLS_CC);
	if( queuehandle == 0 ) {
		php_error(E_ERROR, "No Open Queues Available Or Bad Queue Handle Passed.");
		RETURN_FALSE;
	}

	qmessage.setResponseQueueName(std::string(Z_STRVAL(*zvRQN), Z_STRLEN(*zvRQN)));
	qmessage.setLabel(std::string(Z_STRVAL(*zvLabel), Z_STRLEN(*zvLabel)));
	
	if (Z_STRVAL(*zvBody) != NULL)
		qmessage.getBufferStream()->write(Z_STRVAL(*zvBody), Z_STRLEN(*zvBody));
		
	if (Z_STRLEN(*zvReciptID)) {
		stringstream	s;
		uuid			rcpt_uuid;
		memset(&rcpt_uuid,0,sizeof(rcpt_uuid));
		s.write(Z_STRVAL(*zvReciptID),Z_STRLEN(*zvReciptID));
		s >> rcpt_uuid;
		qmessage.setReciptID(rcpt_uuid);
	}
	qmessage.setTimeToLiveSeconds(Z_LVAL(*zvTTL));
	qmessage.setTTLErrorWanted(Z_BVAL(*zvTTLEW) != 0);
	switch(Z_LVAL(*zvMsgPriority)) {
		case 0:		qmessage.setMessagePriority(MP_LOW);		break;
		case 1:		qmessage.setMessagePriority(MP_MEDIUMLOW);	break;
		case 2:		qmessage.setMessagePriority(MP_MEDIUM);		break;
		case 3:		qmessage.setMessagePriority(MP_MEDIUMHIGH);	break;
		case 4:		qmessage.setMessagePriority(MP_HIGH);		break;
		case 5:		qmessage.setMessagePriority(MP_HIGHEST);	break;
		default:	qmessage.setMessagePriority(MP_LOW);		break;
	}

	ErrorCode ecq = con->Enqueue(queuehandle, qmessage);
	if( ecq==EC_NOERROR ) {
		stringstream	s;
		time_t			t = qmessage.getTimeStamp();
		string			ts = ctime(&t);
//try strftime() with %Y%m%d %H:%M-%S
		s << qmessage.getMessageID();

		ts = ts.substr(0,ts.length()-1);

		zend_update_property_string(scope, zend_obj, mid, strlen(mid), (char*)s.str().c_str() TSRMLS_CC);
		zend_update_property_string(scope, zend_obj, timestamp, strlen(timestamp), (char*)ts.c_str() TSRMLS_CC);

		RETURN_TRUE;
	}
	php_error(E_ERROR, "'%s' Encountered while enqueueing a message.", (char*)EC_Decode(ecq));
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_enqueue_with_relay) {
	QueueMessage				qmessage;
	char*						url_relay;
	int							url_relay_len;
	zval*						id = NULL;
	zval						*zvRQN, *zvLabel, *zvBody, *zvReciptID, *zvTTL, *zvTTLEW, *zvMsgPriority;
	char rqn[] =				"ResponseQueueName";
	char label[] =				"Label";
	char body[] =				"Body";
	char mid[] =				"MessageID";
	char reciptid[] =			"ReciptID";
	char ttl[] =				"TTL";
	char ttlew[] =				"TTLErrorWanted";
	char msgpriority[] =		"MessagePriority";
	char timestamp[] =			"TimeStamp";

	zval*						zend_obj=NULL;
	zend_class_entry* scope =	zend_standard_class_def;

	if( zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "so/|r/", &url_relay, &url_relay_len, &zend_obj, &id) == FAILURE ) {
		php_error(E_ERROR, "Correct usage: safmq_enqueue_with_relay(url_connection_string, msg, connection). (Recommend: $msg=safmq_msg();)");
		RETURN_FALSE;
	}

	zvRQN =			zend_read_property(scope,zend_obj,rqn,			sizeof(rqn)				-1,			0 TSRMLS_CC);//the zero's are bools and are for silent
	zvLabel =		zend_read_property(scope,zend_obj,label,		sizeof(label)			-1,			0 TSRMLS_CC);
	zvBody =		zend_read_property(scope,zend_obj,body,			sizeof(body)			-1,			0 TSRMLS_CC);
	zvReciptID =	zend_read_property(scope,zend_obj,reciptid,		sizeof(reciptid)		-1,			0 TSRMLS_CC);
	zvTTL =			zend_read_property(scope,zend_obj,ttl,			sizeof(ttl)				-1,			0 TSRMLS_CC);
	zvTTLEW =		zend_read_property(scope,zend_obj,ttlew,		sizeof(ttlew)			-1,			0 TSRMLS_CC);
	zvMsgPriority =	zend_read_property(scope,zend_obj,msgpriority,	sizeof(msgpriority)		-1,			0 TSRMLS_CC);

	MQConnection* con = get_con_from_zend(id TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}

	qmessage.setResponseQueueName(std::string(Z_STRVAL(*zvRQN),Z_STRLEN(*zvRQN)));
	qmessage.setLabel(std::string(Z_STRVAL(*zvLabel),Z_STRLEN(*zvLabel)));
	qmessage.getBufferStream()->write(Z_STRVAL(*zvBody),Z_STRLEN(*zvBody));
	if (Z_STRLEN(*zvReciptID)) {
		stringstream	s;
		uuid			rcpt_uuid;
		memset(&rcpt_uuid,0,sizeof(rcpt_uuid));
		s.write(Z_STRVAL(*zvReciptID),Z_STRLEN(*zvReciptID));
		s >> rcpt_uuid;
		qmessage.setReciptID(rcpt_uuid);
	}
	qmessage.setTimeToLiveSeconds(Z_LVAL(*zvTTL));
	qmessage.setTTLErrorWanted(Z_BVAL(*zvTTLEW) != 0);
	switch(Z_LVAL(*zvMsgPriority)) {
		case 0:		qmessage.setMessagePriority(MP_LOW);		break;
		case 1:		qmessage.setMessagePriority(MP_MEDIUMLOW);	break;
		case 2:		qmessage.setMessagePriority(MP_MEDIUM);		break;
		case 3:		qmessage.setMessagePriority(MP_MEDIUMHIGH);	break;
		case 4:		qmessage.setMessagePriority(MP_HIGH);		break;
		case 5:		qmessage.setMessagePriority(MP_HIGHEST);	break;
		default:	qmessage.setMessagePriority(MP_LOW);		break;
	}

	#if _DEBUG
	cout << "Relay URL:" << url_relay << endl;
	#endif
	ErrorCode ecq = con->EnqueueWithRelay(std::string(url_relay, url_relay_len), qmessage);
	if( ecq == EC_NOERROR ) {
		stringstream	s;
		time_t			t = qmessage.getTimeStamp();
		string			ts = ctime(&t);
		//try strftime() with %Y%m%d %H:%M-%S use the help and strftime in the index
		s << qmessage.getMessageID();

		ts = ts.substr(0,ts.length()-1);

		zend_update_property_string(scope, zend_obj, mid, strlen(mid), (char*)s.str().c_str() TSRMLS_CC);
		zend_update_property_string(scope, zend_obj, timestamp, strlen(timestamp), (char*)ts.c_str() TSRMLS_CC);
		
		RETURN_TRUE;
	}
	php_error(E_ERROR, "'%s' encountered while enqueueing message.", (char*)EC_Decode(ecq));
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_create_queue){
	char *name = "";
	int namelen = 0;
	zval* php_con = 0;
	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|r/", &name, &namelen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct usage: safmq_create_queue(queue_name, connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}

	ErrorCode ec = con->CreateQueue(std::string(name,namelen));
	if (ec == EC_NOERROR)
		RETURN_TRUE;

	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_delete_queue){
	char *name = "";
	int namelen = 0;
	zval* php_con = 0;
	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|r/", &name, &namelen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct usage: safmq_delete_queue(queue_name, connection)");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->DeleteQueue(std::string(name,namelen));
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_queue_set_user_permission){
	char *name = "";
	int namelen = 0;
	char *qname = "";
	int qnamelen = 0;

	bool r = false;
	bool w = false;
	bool destroy = false;
	bool changesecurity = false;
	zval* php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "ss|bbbbr/", &qname, &qnamelen, &name, &namelen, &r, &w, &destroy, &changesecurity, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: %s(queuename, username, bool read, bool write, bool destroy, bool changesecurity, connection).", get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->QueueSetUserPermission(std::string(qname,qnamelen), std::string(name,namelen), r, w, destroy, changesecurity);
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_queue_delete_user_permission){
	char *name = "";
	int namelen = 0;
	char *qname = "";
	int qnamelen = 0;
	zval* php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "ss|r/", &qname, &qnamelen, &name, &namelen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_queue_delete_user_permission(queuename, username, connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->QueueDeleteUserPermission(std::string(qname,qnamelen), std::string(name,namelen));
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_queue_set_group_permission){
	char *qname = "";
	int qnamelen = 0;
	char *gname = "";
	int gnamelen = 0;

	bool r = false;
	bool w = false;
	bool destroy = false;
	bool changesecurity = false;
	zval* php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "ss|bbbbr/", &qname, &qnamelen, &gname, &gnamelen, &r, &w, &destroy, &changesecurity, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_queue_set_group_permission(queuename, groupname, bool read, bool write, bool destroy, bool changesecurity, connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->QueueSetGroupPermission(std::string(qname,qnamelen), std::string(gname,gnamelen), r, w, destroy, changesecurity);
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_queue_delete_group_permission){
	char *gname = "";
	int gnamelen = 0;
	char *qname = "";
	int qnamelen = 0;
	zval* php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "ss|r/", &qname, &qnamelen, &gname, &gnamelen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_queue_delete_group_permission(queuename, groupname, connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->QueueDeleteGroupPermission(std::string(qname,qnamelen), std::string(gname,gnamelen));
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_create_group){
	char *name = "";
	int namelen = 0;
	zval* php_con = 0;
	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|r/", &name, &namelen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct usage: safmq_create_group(groupname, connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->CreateGroup(std::string(name,namelen));
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_delete_group){
	char *name = "";
	int namelen = 0;
	zval* php_con = 0;
	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|r/", &name, &namelen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct usage: safmq_delete_group(group_name, connection)");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->DeleteGroup(std::string(name,namelen));
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_create_user){
	char *name = "";
	int namelen = 0;
	char *password = "";
	int passwordlen = 0;
	char *description = "";
	int descriptionlen = 0;
	zval* php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|ssr/", &name, &namelen, &password, &passwordlen, &description, &descriptionlen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct usage: safmq_create_user(user_name, password, description, connection)");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->CreateUser(std::string(name,namelen), std::string(password,passwordlen), std::string(description,descriptionlen));
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_delete_user){
	char *name = "";
	int namelen = 0;
	zval* php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|r/", &name, &namelen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct usage: safmq_delete_user(user_name, connection)");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->DeleteUser(std::string(name,namelen));
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_user_set_permissions){
	char *name = "";
	int namelen = 0;
	bool modq = false;
	bool modu = false;
	bool modg = false;
	zval* php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|bbbr/", &name, &namelen, &modq, &modu, &modg, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_user_set_permissions(username, bool modifyqueues, bool modifyusers, bool modifygroups).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->UserSetPermissions(std::string(name,namelen), modq, modu, modg);
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_user_get_permissions){
	char*	name = "";
	int		namelen = 0;
	bool	modq = false, modu = false, modg = false;//defaulted to no permissions
	zval*	php_con = 0;
	zval*	user_rights;
	string	temp;

	MAKE_STD_ZVAL(user_rights);
	object_init(user_rights);

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|r/", &name, &namelen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_user_get_permissions(username, connection).");
		RETURN_FALSE;
	}

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	try {

		ErrorCode ec = con->UserGetPermissions(std::string(name,namelen), modq, modu, modg);

		add_property_bool(user_rights, "ModifyQueues", modq);
		add_property_bool(user_rights, "ModifyUsers", modu);
		add_property_bool(user_rights, "ModifyGroups", modg);

		/*
		zend_hash_update(
			EG(active_symbol_table),
			"permissionobj",
			strlen("permissionobj") + 1,
			&user_rights,
			sizeof(zval *),
			NULL
		);
		*/

		RETURN_ZVAL(user_rights, true, false);
	} catch (ErrorCode ec) {
		php_error(E_ERROR, "'%s' encountered while attempting to view user permissions.", EC_Decode(ec));
		RETURN_FALSE;
	}
}

PHP_FUNCTION(safmq_set_password){
	char *name = "";
	int namelen = 0;
	char *password = "";
	int passwordlen = 0;
	zval* php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|sr/", &name, &namelen, &password, &passwordlen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_set_password(username, password, connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->SetPassword(std::string(name,namelen), std::string(password,passwordlen));
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_group_add_user){
	char *groupname = "";
	int groupnamelen = 0;
	char *username = "";
	int usernamelen = 0;
	zval* php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "ss|r/", &groupname, &groupnamelen, &username, &usernamelen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_group_add_user(groupname, username, connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->GroupAddUser(groupname, username);
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_group_delete_user){
	char *groupname = "";
	int groupnamelen = 0;
	char *username = "";
	int usernamelen = 0;
	zval* php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "ss|r/", &groupname, &groupnamelen, &username, &usernamelen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_group_delete_user(groupname, username, connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->GroupDeleteUser(groupname, username);
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_group_set_permissions){
	char *name = "";
	int namelen = 0;
	bool modq = false;
	bool modu = false;
	bool modg = false;
	zval* php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|bbbr/", &name, &namelen, &modq, &modu, &modg, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_group_set_permissions(groupname, bool modifyqueues, bool modifyusers, bool modifygroups).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->GroupSetPermissions(std::string(name,namelen), modq, modu, modg);
	if (ec == EC_NOERROR)
		RETURN_TRUE;
	set_errorcode(ec TSRMLS_CC);
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_group_get_permissions){
	char*	name = "";
	int		namelen = 0;
	bool	modq = false, modu = false, modg = false;//defaulted to no permissions
	zval*	php_con = 0;
	zval*	group_rights;
	string	temp;

	MAKE_STD_ZVAL(group_rights);
	object_init(group_rights);

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|r/", &name, &namelen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_group_get_permissions(groupname, connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	try {

		ErrorCode ec = con->GroupGetPermissions(std::string(name,namelen), modq, modu, modg);
		if (ec != EC_NOERROR)
			throw ec;

		add_property_bool(group_rights, "ModifyQueues", modq);
		add_property_bool(group_rights, "ModifyUsers", modu);
		add_property_bool(group_rights, "ModifyGroups", modg);

		/*
		zend_hash_update(
			EG(active_symbol_table),
			"permissionobj",
			strlen("permissionobj") + 1,
			&group_rights,
			sizeof(zval *),
			NULL
		);
		*/

		RETURN_ZVAL(group_rights, true, false);
	} catch (ErrorCode ec) {
		php_error(E_ERROR, "'%s' encountered while attempting to view group permissions.", EC_Decode(ec));
		RETURN_FALSE;
	}
}

PHP_FUNCTION(safmq_begin_transaction) {
	zval*	php_con = NULL;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|r/", &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_begin_transaction(connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->BeginTransaction();
	if (ec == EC_NOERROR)
		RETURN_TRUE;

	php_error(E_ERROR, "'%s' encountered while attempting to begin transaction.", EC_Decode(ec));
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_commit_transaction) {
	zval*	php_con = NULL;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|r/", &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_commit_transaction(connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->CommitTransaction();
	if (ec == EC_NOERROR)
		RETURN_TRUE;

	php_error(E_ERROR, "'%s' encountered while attempting to commit transaction.", EC_Decode(ec));
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_rollback_transaction) {
	zval*	php_con = NULL;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|r/", &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_rollback_transaction(connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->RollbackTransaction();
	if (ec == EC_NOERROR)
		RETURN_TRUE;

	php_error(E_ERROR, "'%s' encountered while attempting to rollback transaction.", EC_Decode(ec));
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_end_transaction) {
	zval*	php_con = NULL;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|r/", &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_end_transaction(connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	ErrorCode ec = con->EndTransaction();
	if (ec == EC_NOERROR)
		RETURN_TRUE;

	php_error(E_ERROR, "'%s' encountered while attempting to end transaction.", EC_Decode(ec));
	RETURN_FALSE;
}

PHP_FUNCTION(safmq_add_user_identity){
	char*	subjectDN = "";
	int		sublen = 0;
	char*	issuerDN = "";
	int		issuerlen = 0;
	char*	username = "";
	int		namelen = 0;
	
	bool	modq = false, modu = false, modg = false;//defaulted to no permissions
	zval*	php_con = 0;
	string	temp;


	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|s|s|r/", &subjectDN, &sublen, &issuerDN, &issuerlen, &username, &namelen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_add_user_identity(groupname, connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	try {

		ErrorCode ec = con->AddUserIdentity(string(subjectDN,sublen), string(issuerDN, issuerlen), string(username, namelen));
		if (ec != EC_NOERROR)
			throw ec;

		RETURN_TRUE;
	} catch (ErrorCode ec) {
		php_error(E_ERROR, "'%s' encountered while attempting to add a user identity", EC_Decode(ec));
		RETURN_FALSE;
	}
}

PHP_FUNCTION(safmq_remove_user_identity){
	char*	subjectDN = "";
	int		sublen = 0;
	char*	issuerDN = "";
	int		issuerlen = 0;
	
	bool	modq = false, modu = false, modg = false;//defaulted to no permissions
	zval*	php_con = 0;
	string	temp;


	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|s|r/", &subjectDN, &sublen, &issuerDN, &issuerlen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_remove_user_identity(groupname, connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	try {
		ErrorCode ec = con->RemoveUserIdentity(string(subjectDN,sublen), string(issuerDN, issuerlen));
		if (ec != EC_NOERROR)
			throw ec;

		RETURN_TRUE;
	} catch (ErrorCode ec) {
		php_error(E_ERROR, "'%s' encountered while attempting to remove a user identity", EC_Decode(ec));
		RETURN_FALSE;
	}
}


PHP_FUNCTION(safmq_enumerate_user_identities){
	char*	username = "";
	int		usernamelen = 0;
	
	bool	modq = false, modu = false, modg = false;//defaulted to no permissions
	zval*	php_con = 0;
	string	temp;


	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "s|r/", &username, &usernamelen, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_enumerate_user_identites(username, connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con==NULL ) {
		php_error(E_ERROR, "No Connections Available Or Bad Connection.");
		RETURN_FALSE;
	}
	try {
		std::vector<safmq::X509Identity> ids;

		ErrorCode ec = con->EnumerateUserIdentities(string(username, usernamelen), ids);
		if (ec != EC_NOERROR)
			throw ec;

		zval*	id;
		zval* idArray;
		MAKE_STD_ZVAL(idArray);
		array_init(idArray);
		std::vector<safmq::X509Identity>::iterator iter = ids.begin();
		while( iter != ids.end() ) {
			MAKE_STD_ZVAL(id);//init with zend that its new
			object_init(id);//init with zend that its new

			add_property_string(id, "subjectDN", (char*)iter->subjectDN.c_str(), true);
			add_property_string(id, "issuerDN", (char*)iter->issuerDN.c_str(), true);
			add_next_index_zval(idArray, id);
			iter++;
		}
		RETURN_ZVAL(idArray, true, false);
	} catch (ErrorCode ec) {
		php_error(E_ERROR, "'%s' encountered while attempting to remove a user identity", EC_Decode(ec));
		RETURN_FALSE;
	}
}


//virtual ErrorCode GetQueueStatistics(MQConnection::QueueHandle qhandle, bool includeStorageBytes, bool includeMessageBytes, QueueStatistics& stats);
PHP_FUNCTION(safmq_get_queue_statistics) {
	zval*		php_con = 0;
	bool includeStorageBytes = false;
	bool includeMessageBytes = false;
	long qid = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "bb|lr/", &includeStorageBytes, &includeMessageBytes, &qid, &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_get_queue_statistics(connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con!=NULL ) {

		MQConnection::QueueHandle queuehandle = get_default_qhandle(con, qid TSRMLS_CC);
		if( queuehandle == 0 ) {

			QueueStatistics qs;
			ErrorCode ec = con->GetQueueStatistics(queuehandle, includeStorageBytes, includeMessageBytes, qs);
			if ( ec == EC_NOERROR) {
				zval*	zvstats;
				MAKE_STD_ZVAL(zvstats);

				add_property_long(zvstats, "messageCount", qs.messageCount);
				add_property_long(zvstats, "storageBytes", qs.storageBytes);
				add_property_long(zvstats, "messageBytes", qs.messageBytes);
				add_property_long(zvstats, "enqueued10second", qs.enqueued10second);
				add_property_long(zvstats, "enqueued60second", qs.enqueued60second);
				add_property_long(zvstats, "enqueued300second", qs.enqueued300second);
				add_property_long(zvstats, "retrieved10second", qs.retrieved10second);
				add_property_long(zvstats, "retrieved60second", qs.retrieved60second);
				add_property_long(zvstats, "retrieved300second", qs.retrieved300second);
				add_property_long(zvstats, "peeked10second", qs.peeked10second);
				add_property_long(zvstats, "peeked60second", qs.peeked60second);
				add_property_long(zvstats, "peeked300second", qs.peeked300second);

				RETURN_ZVAL(zvstats, true, false);
			} else {
				php_error(E_ERROR, "'%s' encountered while attempting to remove a user identity", EC_Decode(ec));
			}
		} else {
			php_error(E_ERROR, "No Open Queues Available Or Bad Queue Handle Passed.");
		}
	} else {
		php_error(E_ERROR, "Bad Connection");
	}
	RETURN_FALSE;
}

//virtual ErrorCode GetServerStatistics(QueueStatistics& stats);
PHP_FUNCTION(safmq_get_server_statistics) {
	zval*		php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|r/", &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_get_server_statistics(connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con!=NULL ) {
		QueueStatistics qs;
		ErrorCode ec = con->GetServerStatistics(qs);
		if ( ec == EC_NOERROR) {
			zval*	zvstats;
			MAKE_STD_ZVAL(zvstats);

			add_property_long(zvstats, "messageCount", qs.messageCount);
			add_property_long(zvstats, "storageBytes", qs.storageBytes);
			add_property_long(zvstats, "messageBytes", qs.messageBytes);
			add_property_long(zvstats, "enqueued10second", qs.enqueued10second);
			add_property_long(zvstats, "enqueued60second", qs.enqueued60second);
			add_property_long(zvstats, "enqueued300second", qs.enqueued300second);
			add_property_long(zvstats, "retrieved10second", qs.retrieved10second);
			add_property_long(zvstats, "retrieved60second", qs.retrieved60second);
			add_property_long(zvstats, "retrieved300second", qs.retrieved300second);
			add_property_long(zvstats, "peeked10second", qs.peeked10second);
			add_property_long(zvstats, "peeked60second", qs.peeked60second);
			add_property_long(zvstats, "peeked300second", qs.peeked300second);

			RETURN_ZVAL(zvstats, true, false);
		} else {
			php_error(E_ERROR, "'%s' encountered while attempting to remove a user identity", EC_Decode(ec));
		}
	} else {
		php_error(E_ERROR, "Bad Connection");
	}
	RETURN_FALSE;
}

//virtual ErrorCode GetClientInfo(std::list<ClientInfo>& clients);
PHP_FUNCTION(safmq_get_client_info) {
	zval*		php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|r/", &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_get_client_info(connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con!=NULL ) {
		std::list<ClientInfo> clients;

		ErrorCode ec = con->GetClientInfo(clients);
		if (ec == EC_NOERROR) {
			zval*	zvclient;
			zval*	zvclients;
			char	addrbuf[20];

			MAKE_STD_ZVAL(zvclient);
			array_init(zvclient);
			for(std::list<ClientInfo>::iterator i = clients.begin(); i!=clients.end(); ++i) {
				MAKE_STD_ZVAL(zvclient);
				// client				
				//std::string				username;
				//SAFMQ_INT32				address;
				//short					port;

				sprintf(addrbuf, "%d.%d.%d.%d", i->address>>24 & 0x0FF,
					i->address>>16 & 0x0FF,
					i->address>>8 & 0x0FF,
					i->address & 0x0FF);

				add_property_string(zvclient, "username", (char*)i->username.c_str(), true);
				add_property_string(zvclient, "address", addrbuf, true);
				add_property_long(zvclient, "port", i->port);


				add_next_index_zval(zvclients, zvclient);
			}
			RETURN_ZVAL(zvclients, true, false);
		} else {
			php_error(E_ERROR, "'%s' encountered while attempting to remove a user identity", EC_Decode(ec));
		}
	} else {
		php_error(E_ERROR, "Bad Connection");
	}
	RETURN_FALSE;
}

//virtual ErrorCode CreateTempQueue(std::string& tmpQueueName, MQConnection::QueueHandle& tmpQueue);
PHP_FUNCTION(safmq_create_temp_queue) {
	zval*		php_con = 0;

	if (zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC, "|r/", &php_con) == FAILURE) {
		php_error(E_ERROR, "Correct Usage: safmq_create_temp_queue(connection).");
		RETURN_FALSE;
    }

	MQConnection *con = get_con_from_zend(php_con TSRMLS_CC);
	if( con!=NULL ) {
		MQConnection::QueueHandle queuehandle;
		std::string	qname;

		ErrorCode ec = con->CreateTempQueue(qname, queuehandle);
		if (ec == EC_NOERROR) {
			register_default_qhandle(con,queuehandle TSRMLS_CC);

			zval*	tmpq_info;

			MAKE_STD_ZVAL(tmpq_info);
			object_init(tmpq_info);
			add_property_string(tmpq_info, "queue_name", (char*)qname.c_str(), true);
			add_property_long(tmpq_info, "queue_handle", queuehandle);

			RETURN_ZVAL(tmpq_info, true, false);
		} else {
			php_error(E_ERROR, "'%s' while creating temporary queue", EC_Decode(ec));
		}
	}  else {
		php_error(E_ERROR, "Bad Connection");
	}
	RETURN_FALSE;
}

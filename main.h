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
#ifndef _MAIN_H_
#define _MAIN_H_

#pragma warning(disable:4786)

#if !defined(_WIN32)
#define stricmp strcasecmp
#endif

#include "param_config.h"
#include "safmq_defs.h"

#define FORWARD_QUEUE_NAME "_FORWARD_"

#define PORT_PARAM						"port"
#define BIND_ADDRESS					"address"



#define QUEUE_DIR_PARAM					"queue_dir"
#define DEFAULT_QUEUE_DIR				"queues"
#define MAX_FILE_SIZE					"max_file_size"
#define DEFAULT_MAX_FILE_SIZE			(40*1024*1024)
#define FILE_HANDLE_CACHE_SIZE			"file_handle_cache_size"
#define DEFAULT_FILE_HANDLE_CACHE_SIZE	(50)
#define MAX_FILE_HANDLE_CACHE_SIZE		(500)

#define CONFIG_DIR_PARAM				"config_dir"

#define SSL_PORT_PARAM					"ssl_port"
#define SSL_BIND_ADDRESS				"ssl_address"

#define SSL_CERT_PARAM					"ssl_cert"
#define SSL_KEY_PARAM					"ssl_key"
#define SSL_CA_PARAM					"ssl_ca"
#define SSL_CA_DIR_PARAM					"ssl_ca_dir"
#define SSL_REQUIRE_CLIENT_CERT			"ssl_require_cert"

#define SSL_CLIENT_CERT_PARAM			"ssl_client_cert"
#define SSL_CLIENT_KEY_PARAM			"ssl_client_key"

#define ENABLE_FOWARDING_PARAM			"enable_forwarding"
#define ACCEPT_FORWARDS_PARAM			"accept_forwards"


#define STARTUP_LOG		"startup_log"
#define SHUTDOWN_LOG	"shutdown_log"
#define SIGNON_LOG		"signon_log"
#define USER_LOG		"user_log"
#define GROUP_LOG		"group_log"
#define QUEUE_LOG		"queue_log"
#define MESSAGE_LOG		"message_log"
#define FORWARD_LOG		"forward_log"
#define INFO_LOG		"info_log"

#define FORWARD_THREADS		"forward_threads"

extern	Config* pcfg;

extern const std::string system_user;
extern const std::string safmq_all_users;


int startSafmq();
void stopSafmq();
void shutdownSafmq();

#ifdef _WIN32
void createService(const char* path);
void removeService();
void startService();
bool runAsService();
#elif defined(unix)


#endif

#endif

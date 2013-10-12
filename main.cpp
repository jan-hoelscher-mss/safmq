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
#pragma warning(disable: 4786)

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <memory.h>
#include <signal.h>
#include "Log.h"

#ifdef _WIN32
	#include <windows.h>
	#include <direct.h>
	#include "utilities/_getopt_long.c"

#elif unix || (__APPLE__ && __MACH__) || __FreeBSD__ 
	#if !defined(SAFMQ_GETOPT)
		#include <getopt.h>
	#else
		#include "utilities/_getopt_long.c"
	#endif

	#include <unistd.h>
	#include <sys/types.h>
	#define MAXFD 3

#else
	#error Unknown OS
#endif

#ifdef SAFMQ_SSL
#include "SSLServerThread.h"
#endif

#include "ServerThread.h"
#include "QManager.h"
#include "SystemDelivery.h"
#include "ForwardThread.h"
#include "main.h"

using namespace std;


#ifdef SAFMQ_SSL
/// The global instance of the sslserver;
SSLServerThread				sslserver;
#endif

/// The global instance of the system login user
const std::string			system_user("safmq_system");

/// Global group including all users
const std::string			safmq_all_users("safmq_all_users");

/// The global reference configuration data 
Config*						pcfg;
/// The global reference to queue manager thread
QManager*					theQueueManager = NULL;
/// The global instance of the network serevr thread
ServerThread				server;
/// The global instance of the TTL Error Delivery thread
SystemDelivery				sysdeliver;
/// The global instance of the message forwarding thread
ForwardThread*				forwardthds;
int							numForwardThreads = 1;

// TODO: REMOVE std::char_traits<char>	foo; // Need to force linking...

/// The Configuraiton file
static std::string			config_file = "safmq.conf";

#ifdef _WIN32
/// Single character command line options
static const char* option_chars = "Dhc:ir";
#elif unix || (__APPLE__ && __MACH__) || __FreeBSD__
static const char* option_chars = "Dhc:";
#endif


enum {
	/// An enumeration indicating the start in background has been passed
	OPID_daemon = 'D',
	/// An enumeration indicating the program usage should be displayed
	OPID_help = 'h',
	/// An enumeration indicating the config file has been specified in 
	OPID_config = 'c',
	/// An enumeration idicating the winnt service should be installed
	OPID_install = 'i',
	// An enumeration indicating the winnt service should be removed
	OPID_remove = 'r',
};

/**
Options to be passed to getopt_long.  These entries
are read by getopt_long to determine what parameters have been
passed to the application.
*/
static option options[] = {
	{"daemon",0,0,OPID_daemon},
	{"help",0,0,OPID_help},
	{"config",1,0,OPID_config},
#ifdef _WIN32
	{"install",0,0,OPID_install},
	{"remove",0,0,OPID_remove},
#endif
	{0,0,0,0}
};

/******************************************************************************
Handles SEGV signals from the system in the case of a memory reference error.
Calls abort to dump a "core" image on unix systems.

@param sig The signal generating the call to this function
*/
void sigsegv(int sig)
{
	abort();
}

/******************************************************************************
Handles INT interrupt signals from the system, in the case the user has
chosen to interrupt the program and terminate it.

@param sig The signal id this function is responding to.
*/
void sigint(int sig)
{
	Log::getLog()->Info("signal: %ld stopping SAFMQ", sig);
	stopSafmq();
}

void sigerr(int sig)
{
	int* a = NULL;
	*a = 20;
}

/******************************************************************************
Call upon termination of the program to perform cleanup duties
*/
void onExit()
{
	// TODO: Figure out why theQueueManager can't be deleted.
	// delete theQueueManager;
}

/******************************************************************************
Starts the safmq server
*/
int startSafmq()
{
	try {
		pcfg = new Config(config_file);
	} catch (int ) {
		Log::getLog()->Startup(Log::error, "Unable to load configuration file.");
		return -1;
	}

	// setup up the logger
	Log*	plog = Log::getLog();

	plog->SetLogDestination(Log::startup, pcfg->getParam(STARTUP_LOG, Log::STDOUT).c_str());
	plog->SetLogDestination(Log::shutdown, pcfg->getParam(SHUTDOWN_LOG, Log::STDOUT).c_str());
	plog->SetLogDestination(Log::sign_on, pcfg->getParam(SIGNON_LOG, Log::STDOUT).c_str());
	plog->SetLogDestination(Log::user, pcfg->getParam(USER_LOG, Log::STDOUT).c_str());
	plog->SetLogDestination(Log::group, pcfg->getParam(GROUP_LOG, Log::STDOUT).c_str());
	plog->SetLogDestination(Log::queue, pcfg->getParam(QUEUE_LOG, Log::STDOUT).c_str());
	plog->SetLogDestination(Log::message, pcfg->getParam(MESSAGE_LOG, Log::STDOUT).c_str());
	plog->SetLogDestination(Log::forward, pcfg->getParam(FORWARD_LOG, Log::STDOUT).c_str());
	plog->SetLogDestination(Log::info, pcfg->getParam(INFO_LOG, Log::STDOUT).c_str());

	theQueueManager = new QManager;

	atexit(onExit);

	sysdeliver.start();
	theQueueManager->start();


	numForwardThreads = pcfg->getIntParam(FORWARD_THREADS, 1);
	if (numForwardThreads < 1)
		numForwardThreads = 1;
	forwardthds = new ForwardThread[numForwardThreads];
	for(int x = 0; x < numForwardThreads; ++x) {
		forwardthds[x].start();
	}
	server.start();
#ifdef SAFMQ_SSL
	sslserver.start();
#endif

	plog->Startup();

	return 1;
}

/******************************************************************************
Stops the safmq server
*/
void stopSafmq()
{
#ifdef SAFMQ_SSL
	sslserver.stop();
#endif
	server.stop();
	for(int x = 0; x < numForwardThreads; ++x)
		forwardthds[x].stop();
	sysdeliver.stop();
	if (theQueueManager)
		theQueueManager->stop();
}

/******************************************************************************
Shutsdown the safmw server by ensuring started threads have terminated
*/
void shutdownSafmq() 
{
#ifdef SAFMQ_SSL
	sslserver.join();
#endif
	server.join();
	for(int x = 0; x < numForwardThreads; ++x)
		forwardthds[x].join();
	delete [] forwardthds;
	sysdeliver.join();
	theQueueManager->join();

	//Log::getLog()->Info("Shutting down SAFMQ, freeing theQueueManager");
	//delete theQueueManager;

	Log::getLog()->Shutdown();
}

#ifdef unix
/******************************************************************************
Initializes safmq to run in the background on UNIX systems
*/
void daemonInit()
{
	int		i;
	pid_t	pid;

	if ( (pid=fork()) != 0)
		exit(0); // close the parent

	setsid(); // become a session leader
	signal(SIGHUP, SIG_IGN);
	if ( (pid=fork()) != 0)
		exit(0); // close first child

	for(i = 0; i < MAXFD; i++) // close all the inputs and outputs
		close(i);
}
#endif

/******************************************************************************
Displays the usage (command options) for safmq.
*/
void usage()
{
	cerr << "safmq usage:" << endl;
	cerr << "safmq " << endl;
	cerr << "       -h,--help      : Show this information" << endl;
	cerr << "       -c cfg," <<endl;
	cerr << "       --config=cfg   : Use 'cfg' as the configuration file" << endl;
	cerr << "       -D,--daemon    : Start the program in the background" << endl;
#ifdef _WIN32
	cerr << "                        fails if SAFMQ has not been installed as a service" << endl;
	cerr << endl;
	cerr << "       -i,--install   : Installs SAFMQ as a service" << endl;
	cerr << "       -r,--remove    : Uninstalls SAFMQ as a service" << endl;
#endif
	cerr << endl;
	cerr << endl;
}

#ifdef _WIN32
/******************************************************************************
Win32 Service Install

@param argc The number of parameters passed to the program
@param argv The list of parameters passed to the program
*/
void serviceInstall(const char* cmd)
{
	char	path[1024];
	int		len = strlen(cmd);
	char*	pfilename;

	::GetFullPathName(cmd,sizeof(path),path,&pfilename);
	createService(path);
}
#endif

/******************************************************************************
Main function entry point for SAFMQ.

@param argc The number of parameters passed to the program
@param argv The list of parameters passed to the program
*/
int main(int argc, char* argv[])
{
	bool	daemon = false;
	bool	install = false;
	bool	remove = false;
	int		theopt;

	while ((theopt=getopt_long(argc,argv,option_chars,options,NULL)) != -1) {
		switch (theopt) {
			case '?':
				usage();
				return -1;
			case OPID_daemon:
				daemon = true;
				break;
			case OPID_help:
				usage();
				return 0;
			case OPID_config:
				config_file = optarg;
				break;
			case OPID_install:
				install = true;
				break;
			case OPID_remove:
				remove = true;
				break;
		}
	}


#ifdef _WIN32
	if (install) {
		serviceInstall(argv[0]);
		if (!daemon && !remove)
			return 0;
	}

	if (remove) {
		removeService();
		if (!daemon)
			return 0;
	}
#endif

	if (daemon) {
#ifdef unix
		daemonInit();
#elif defined(_WIN32)
		startService();
		return 0;
#endif
	}

	signal(SIGSEGV, sigsegv);
	signal(SIGINT,	sigint);

#ifdef unix
	signal(SIGBUS, sigsegv);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP,	SIG_IGN);
	signal(SIGTERM, sigint);
	signal(SIGQUIT, sigint);
	signal(SIGSTOP, sigint);
#endif

#ifdef WIN32
	if (!runAsService()) {
#endif
		if (startSafmq() > 0) 
			shutdownSafmq();
#ifdef WIN32
	}
#endif

	return 0;
}


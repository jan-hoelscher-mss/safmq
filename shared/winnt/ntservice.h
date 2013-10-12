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


This software implements an NT Service.
*/
#ifndef _NTSERVICE_H_
#define _NTSERVICE_H_

#include <windows.h>

/**
Implements framework for developing applications to operate as an WinNT 
service.  Services are Win32 programs that run in the background.  Services 
are either started automatically by the system upon boot, or are started 
manually using the "Services" controll panel or by another application using 
the Service Control Manager API.<br>

<p>
To implement a service:
<ol>
<li>Derive from NTService
<li>Implement the pure-virtual functions: <code>performService(), getServiceName(), 
		getServiceLongName(),</code> and <code>getServiceDescription()</code>
<li>In <code>main()</code> (or <code>WinMain()</code> or <code>_tmain()</code>,
		 etc.) call <code>dispatchService()</code>.
<li>Optionally provide command line parameters to trigger calls to 
		<code>registerService()</code>, <code>unregisterService()</code>, and 
		<code>startService()</code>.
</ol>

</p>
*/
class NTService
{
public:
	void SetServiceStatus(long);
	/**
	Constructs the instance of the NTService. <b>Note</b>: There should be only
	one isntance of an NTService object.  The constructor will assert if
	more than one instance of NTService is constructed.
	*/
	NTService();
	/**
	Destroys the NTService object.
	*/
	virtual ~NTService();

	/**
	A flag indicating the service should terminate.  Child classes can check this flag
	to determine if they should terminate their operations.

	@return <code>true</code> in the case the service has been shutdown.
	*/
	bool shouldQuit();
	
	/**
	Connects the service with the Service Control Dispatcher.  If this function
	fails the service was either not installed, or was not started by the 
	Service Control Manager.<br>
	
	<p>This function will return <code>false</code> if the program was started in
	foreground.</p>

	@return <code>true</code> if successfully attached to the Service Control 
				Dispatcher, <code>false</code> otherwise.
	*/
	bool dispatchService();

	/**
	[REQUIRED] Implement this function to perform the tasks of the service.  This 
	method is an analog to <code>main()</code>.

	@param argc The number of parameters
	@param argv An array of parameter strings
	*/
	virtual int performService(unsigned long argc, const char** argv) = 0;

	/**
	Installs the service on the WinNT based system.  Call this function to
	register the service with the Service Control Manager.

	@param exename The name of this executable, typically argv[0]
	*/
	void registerService(const char* exename);
	/**
	Removes the service from the WinNT based system.  Call this function
	to un-install the service and de-register it with the Service
	Control Manager.

	*/
	void unregisterService();

	/**
	Starts this service in the background.  Uses the Service Control
	Manager to start the service in the background.

	@param argc The number of parameters
	@param argv An array of parameter strings.
	*/
	void startService(unsigned long argc, const char** argv);

	/**
	[REQUIRED] Implement this function to provide the name of your service.
	@return The name of your service
	*/
	virtual const char* getServiceName() = 0;
	/**
	[REQUIRED] Implement this function to provide the long name of your service, 
	as it will be displayed in the "Services" control pannel.
	@return The long name of your service
	*/
	virtual const char* getServiceLongName() = 0;
	/**
	[REQUIRED] Implement this function to provide the description of your service, 
	as it will be displayed in the "Services" control pannel.
	@return The description  of your service
	*/
	virtual const char* getServiceDescription() = 0;
	/**
	[OPTIONAL] Override this function to provide name of the service load ordering group.
	@return The name of a load ordering group.
	*/
	virtual const char* getLoadOrderGroup();

	/**
	[OPTIONAL] Override this function to a list of services this service is dependant on.
	This buffer contains the names of the services seperated by '\0' characters with two
	'\0' characters at teh end of the list.
	@return A double null terminated list of services this service depends on
	*/
	virtual const char* getDependancies();
	/**
	[OPTIONAL] Override this function to provide login name of the user context this 
	service should operate uner.
	@return The name of a user to run as.
	*/
	virtual const char* getLoginAccount();
	/**
	[OPTIONAL] Override this function to provide login password of the user context this 
	service should operate uner.
	@return The password of a user to run as.
	*/
	virtual const char* getLoginPassword();

	/**
	Called when the service has been notified it should stop.
	*/
	virtual void onStopService();
	/**
	Called when the service has been notified it should shutdown.
	*/
	virtual void onShutdownService();
	/**
	Called when the service has been notified it should pause.
	*/
	virtual void onPauseService();
	/**
	Called when the service has been notified it should continue.
	*/
	virtual void onContinueService();
	/**
	Called when the service has been notified it should report its status.
	*/
	virtual void onInterrogateService();
protected:
	/**
	Called by the Service Control Manager.  This function calls <code>perforService()</code>.

	@param argc Number of paramegers
	@param argv Array of parameter strings.
	*/
	static void WINAPI ServiceMain(unsigned long argc, char** argv);
	/**
	Called by the Service Control Manager to manage the service.
	@param fdwControl The command the service should respond to.
	*/
	static void WINAPI ServiceControlHandler(unsigned long fdwControl);

	/// Flag to quit
	bool					m_shouldQuit;
	/// Service status handle
	SERVICE_STATUS_HANDLE	m_thisServiceStatus;
	/// Service status.
	SERVICE_STATUS			m_status;
};

extern NTService* theService;

#endif

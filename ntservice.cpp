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
#include <windows.h>
#include <iostream>
#include <fstream>

#include "main.h"
#include "winnt/regutil.h"

using namespace std;

const char* serviceName = "safmq";
const char* serviceDesc = "SAFMQ Message Queue Server";
const char* description = "Allows applications to post a message, to a message queue, which is stored until the message is read by the receiving application.";

SERVICE_STATUS_HANDLE	thisServiceStatus = NULL;
SERVICE_STATUS			status;

void createService(const char* path) 
{
	SC_HANDLE manager = OpenSCManager(NULL,NULL,SC_MANAGER_CREATE_SERVICE);

	if (manager) {
		SC_HANDLE schService = CreateService( 
				manager,
				serviceName,
				serviceDesc,
				SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS,
				SERVICE_AUTO_START,
				SERVICE_ERROR_NORMAL,
				path,						// service's binary 
				NULL,						// no load ordering group 
				NULL,						// no tag identifier 
				NULL,						// no dependencies 
				NULL,						// LocalSystem account 
				NULL);						// no password 

		if (schService) {
			try {
				CRegKey	key(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\safmq");
				key.setString("Description",description);
			} catch (CRegKeyException ex) {
				cerr << "While setting description: " << ex.what() << endl;
			}

			cout << serviceName << " installed successfully." << endl;
			CloseServiceHandle(schService);
		} else {
			DWORD error = GetLastError();
			switch (error) {
				case ERROR_ACCESS_DENIED: // The handle to the specified service control manager database does not have SC_MANAGER_CREATE_SERVICE access. 
					cerr << "Error creating service: access denied." << endl; break;
				case ERROR_DUP_NAME: //The display name already exists in the service control manager database either as a service name or as another display name. 
					cerr << "Error creating service: duplicate service name." << endl; break;
				case ERROR_INVALID_SERVICE_ACCOUNT: //The user account name specified in the lpServiceStartName parameter does not exist. 
					cerr << "Error creating service: invalid service account." << endl; break;
				case ERROR_SERVICE_EXISTS: //The specified service already exists in this database. 
					cerr << "Error creating service: service already exists in database." << endl; break;
			}
		}
		CloseServiceHandle(manager);
	} else {
		DWORD error = GetLastError();
		switch (error) {
			case ERROR_ACCESS_DENIED:
				cerr << "Error opening service control manager: Cannot create service, access denied." << endl; break;
			case ERROR_DATABASE_DOES_NOT_EXIST:
				cerr << "Error opening service control manager: Unable to access service database, database does not exist." << endl; break;
			case ERROR_INVALID_PARAMETER:
				cerr << "Error opening service control manager invalid parameter." << endl; break;
		}	
	}
}

void removeService()
{
	SC_HANDLE manager = OpenSCManager(NULL,NULL,SC_MANAGER_CREATE_SERVICE);

	if (manager) {
		SC_HANDLE schService = OpenService(manager,serviceName,SERVICE_ALL_ACCESS);

		if (schService) {
			if (!DeleteService(schService)) {
				DWORD error = GetLastError();
				switch (error) {
					case ERROR_ACCESS_DENIED:
						cerr << "Error removing service: access denied." << endl; break;
					case ERROR_SERVICE_MARKED_FOR_DELETE:
						cerr << "Service already marked for deletion." << endl; break;
					default:
						cerr << "Error removing service." << endl; break;
				}
			} else {
				cout << serviceName << " removed successfully.";
			}
		} else {
			DWORD error = GetLastError();
			switch (error) {
				case ERROR_ACCESS_DENIED: // The handle to the specified service control manager database does not have SC_MANAGER_CREATE_SERVICE access. 
					cerr << "Error removing service: access denied." << endl; break;
				default:
					cerr << "Error removing service while opening service." << endl; break;
			}
		}
		CloseServiceHandle(manager);
	} else {
		DWORD error = GetLastError();
		switch (error) {
			case ERROR_ACCESS_DENIED:
				cerr << "Error opening service control manager: Cannot create service, access denied." << endl; break;
			case ERROR_DATABASE_DOES_NOT_EXIST:
				cerr << "Error opening service control manager: Unable to access service database, database does not exist." << endl; break;
			case ERROR_INVALID_PARAMETER:
				cerr << "Error opening service control manager invalid parameter." << endl; break;
		}	
	}
}


void startService()
{
	SC_HANDLE manager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (manager) {
		SC_HANDLE service = OpenService(manager,serviceName,SERVICE_START);
		if (service) {
			const char* param[] = {"safmq.exe",""};

			cout << "attempting to start service" << endl;
			if (StartService(service,1,param))
				cout << serviceName << " started.";
			else {
				DWORD error = GetLastError();
				switch (error) {
					case ERROR_ACCESS_DENIED: 
						cerr << "The specified handle was not opened with SERVICE_START access." << endl; break;
					case ERROR_INVALID_HANDLE: 
						cerr << "The specified handle is invalid." << endl; break;
					case ERROR_PATH_NOT_FOUND: 
						cerr << "The service binary file could not be found." << endl; break;
					case ERROR_SERVICE_ALREADY_RUNNING: 
						cerr << "An instance of the service is already running." << endl; break;
					case ERROR_SERVICE_DATABASE_LOCKED: 
						cerr << "The database is locked." << endl; break;
					case ERROR_SERVICE_DEPENDENCY_DELETED: 
						cerr << "The service depends on a service that does not exist or has been marked for deletion." << endl; break;
					case ERROR_SERVICE_DEPENDENCY_FAIL: 
						cerr << "The service depends on another service that has failed to start." << endl; break;
					case ERROR_SERVICE_DISABLED: 
						cerr << "The service has been disabled." << endl; break;
					case ERROR_SERVICE_LOGON_FAILED: 
						cerr << "The service could not be logged on. This error occurs if the service was started from an account that does not have the \"Log on as a service\" right." << endl; break;
					case ERROR_SERVICE_MARKED_FOR_DELETE: 
						cerr << "The service has been marked for deletion." << endl; break;
					case ERROR_SERVICE_NO_THREAD: 
						cerr << "A thread could not be created for the service." << endl; break;
					case ERROR_SERVICE_REQUEST_TIMEOUT: 
						cerr << "The process for the service was started, but it did not call StartServiceCtrlDispatcher, or the thread that called StartServiceCtrlDispatcher may be blocked in a control handler function." << endl; break;
					default:
						cerr << "An error occured attempting to start the service." << endl; break;
				}
			}	
			CloseServiceHandle(service);
		} else {
			switch (GetLastError()) {
				case ERROR_ACCESS_DENIED:
					cerr << "Error: Access denied accessing service to start." << endl; break;
				case ERROR_SERVICE_DOES_NOT_EXIST:
					cerr << "Error: Service does not exist, please install first." << endl; break;
				default:
					cerr << "Error occurred accessing service." << endl;
			}
		}
		CloseServiceHandle(manager);
	} else {
		switch (GetLastError()) {
			case ERROR_ACCESS_DENIED:
				cerr << "Error: Access denied accessing service manager." << endl; break;
			default:
				cerr << "Error occurred accessing service." << endl;
		}		
	}
}


void WINAPI ServiceControlHandler(DWORD fdwControl)
{
	switch (fdwControl) {
		default:
		case SERVICE_CONTROL_CONTINUE:		// Notifies a paused service that it should resume.
		case SERVICE_CONTROL_INTERROGATE:	// Notifies a service that it should report its current status information to the service control manager. 
		case SERVICE_CONTROL_PAUSE:			// Notifies a service that it should pause.  
			break;

		case SERVICE_CONTROL_SHUTDOWN:		// Notifies a service that the system is shutting down so the service can perform cleanup tasks. 
		case SERVICE_CONTROL_STOP:// Notifies a service that it should stop.  
			stopSafmq();
			status.dwCurrentState = SERVICE_STOP_PENDING;
			SetServiceStatus(thisServiceStatus, &status);
			shutdownSafmq();

			break;
	}
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
	memset(&status,0,sizeof(status));

	thisServiceStatus = RegisterServiceCtrlHandler(serviceName, ServiceControlHandler);

	status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	status.dwCurrentState = SERVICE_START_PENDING;
	status.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
	SetServiceStatus(thisServiceStatus, &status);

	if (startSafmq() >= 0) {
		status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		status.dwCurrentState = SERVICE_RUNNING;
		status.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
		SetServiceStatus(thisServiceStatus, &status);
		shutdownSafmq();
	}

	status.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(thisServiceStatus, &status);
}

bool runAsService()
{
    SERVICE_TABLE_ENTRY   DispatchTable[] =   { 
								{(char*)serviceName, ServiceMain}, 
								{NULL, NULL}
							}; 
 
    return StartServiceCtrlDispatcher(DispatchTable)!=0;
}
/*
 Copyright 2005 Matthew J. Battey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0



This software implements a platform independent C++ interface to TCP/IP socket
communications.
*/

#include <iostream>
#include <string>
#include "../socstream.h"
#include "../tcpsocket.cpp" // include the source for the socket, typically this compiled seperately

using namespace std;
using namespace tcpsocket;

int main (int argc, char* argv[])
{
	try {
		char* addr;
		string	line;

		// Determine what address to connect to
		if (argc > 1)
			addr = argv[1];
		else
			addr = "www.google.com";

		// construct a socstream, and specify the address to connect to
		socstream	s(addr,80);
		// Send a HTTP 1.0 request for the root resource
		s << "GET / HTTP/1.0\n\n" << flush;
	
		// Read the results until the connection is closed	
		getline(s,line);	
		while (s.good()) {
			// write a line out to the console
			cout << line << endl;
			getline(s,line);
		}	

	} catch (SocketException& e) {
		// Write any errors that might have occured
		cout << e.what() << endl;
	}
	return 0;
}

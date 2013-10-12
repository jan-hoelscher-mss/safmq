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
		// Construct a server socket bound to all interfaces, port 9000/TCP
		ServerSocket	svr(9000);
		// Accept a connection and associate that socket with the socstream
		socstream		s(svr.acceptConnection());

		// Send a message to the client
		s << "Hello World" << endl << flush;
		// terminate the connection with the client
		s.close();

	} catch (SocketException& e) {
		// Output any error that may have occured
		cout << e.what() << endl;
	}
	return 0;
}

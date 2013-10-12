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




This software implements a platform independent C++ interface to TCP/IP socket
communications.
*/
#if !defined(_SOCKET_H_)
#define _SOCKET_H_

#ifdef _WIN32
	#include <winsock.h>
	#pragma warning( disable : 4290 )

	#ifndef in_addr_t
	#define in_addr_t unsigned __int32
	#endif
#else
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

#include <errno.h>
#include <exception>
#include <string>
#include <list>

namespace tcpsocket {

/**
An exception thrown incase of an error occuring in a class derived
from <code>tcpsocket::Socket</code>
*/
class SocketException : public std::exception {
public:
	SocketException() throw() {
		m_err = 0;
	}

	/**
	Constructs the exception from an error code.
	@param err [in] The error code
	*/
	SocketException(int err) throw () {
		m_err = err;
	}

	/**
	Copy constructor.
	@param e [in] The source exception.
	*/
	SocketException(const SocketException& e) throw () {
		m_err = e.m_err;
	}

	/**
	Copy operator
	@param e [in] The source exception.
	@return a reference to this object
	*/
	SocketException& operator =(const SocketException& e) throw () {
		m_err = e.m_err;
		return *this;
	}
	/**
	Destroys the exception
	*/
	virtual ~SocketException() throw () { }
	virtual const char* what();

	/**
	Provides access to the numerical error code
	*/
	virtual int getError() { return m_err; }

protected:
	int		m_err;
};

/**
A class for working with TCP sockets.  This class provides an interface
to open a connection, send data, and receive data.
*/
class Socket  
{
private:
	void init() {
		m_socket = -1;
		m_throwOnClose = false;
	}

private:

public:
	/**
	Default constructor for the socket
	*/
	Socket() { init(); }
	/**
	Constructs the socket from a socket handle.
	@param socket_handle [in] A previously created socket handle
	*/
	Socket(int socket_handle) {
		init();
		m_socket = socket_handle;
	}
	/**
	Copy constructor, copies the socket handle
	@param s [in] The source socket
	*/	
	Socket(const Socket& s) {
		init();
		m_socket = s.m_socket;
	}
	/**
	Copy operator, copies the socket handle
	@param s [in] The source socket
	@return A reference to this object
	*/	
	Socket& operator=(const Socket& s) {
		m_socket = s.m_socket;
		return *this;
	}
	Socket(const char* host, short port) throw (SocketException);
	virtual ~Socket();
	
	virtual size_t send(const char* sendBuffer, size_t length)	throw (SocketException);
	virtual size_t receive(char* readBuffer, size_t length)		throw (SocketException);
	virtual size_t receiveSome(char* readBuffer, size_t length)	throw (SocketException);
	virtual void close() throw (SocketException);
	/**
	Sets a flag to throw a SocketException in the case the socket was closed durring
	a send or receive operation.  Throwing an exception on a socket close can be useful
	when using the insertion and extraction operators associated with the <code>BufferedSocket</code>
	class.

	@param throwOnClose [in] Flags the class to throw an exception on detecting a closed socket
	@see BufferedSocket
	*/
	void setThrowOnClosed(bool throwOnClose)	{ m_throwOnClose = throwOnClose; }
	/**
	Provides the current status of the throw on close flag.
	@return The status of the throw on close flag.
	*/
	bool getThrowOnClosed()						{ return m_throwOnClose; }
	/**
	Provides access to the socket handle
	@return The socket handle
	*/
	virtual int getSocket() { return m_socket; }
	/**
	Clones this Socket object.
	@return A clone of this object.
	*/
	virtual Socket* clone() const { return new Socket(*this); }


	virtual void getPeerName(struct sockaddr_in* addr);

protected:
	void sleep(int milliseconds);
protected:
	/// The Socket handle
	int		m_socket;
	bool	m_throwOnClose;
};

/**
This class provides a buffered interface to TCP socket operations.  It allocates
a application memory buffer to speed communications in both sending and receiving.<br>

<p>Note: Since communications are buffered, flushing the socket buffer is required for
immediate data sends, in the case of synchronous communications.</p>

<p>The buffered socket has been implemented as a facade of the Socket class to 
allow the copy constructor and copy operators to receive a Socket, BufferedSocket,
or SSLSocket.</p>
*/
class BufferedSocket : public Socket
{
protected:

private:
	void init(int bufSize);

public:
	/**
	Constructs the buffered socket.
	@param bufSize [in] Specifies the size of the buffer, -1 indicates the buffer should
						match the system's TCP buffer size
	*/
	explicit BufferedSocket(int bufSize = -1) : Socket(-1), m_allocsocket(true) { 
		m_socket = new Socket();
		init(bufSize);
	}
	/**
	Constructs the buffered socket using an existing socket handle.
	@param socket_handle [in] An existing socket handle to use in communications
	@param bufSize [in] Specifies the size of the buffer, -1 indicates the buffer should
						match the system's TCP buffer size
	*/
	explicit BufferedSocket(int socket_handle, int bufSize = -1) : Socket(-1), m_allocsocket(true) {
		m_socket = new Socket(socket_handle);
		init(bufSize);
	}
	/**
	Constructs the socket and opens a connection to a server.
	@param host [in] The address of the server or DNS name
	@param port [in] The TCP port to connect to
	@param bufSize [in] Specifies the size of the buffer, -1 indicates the buffer should
						match the system's TCP buffer size
	*/
	explicit BufferedSocket(const char* host, short port, int bufSize = -1) throw (SocketException) : Socket(-1), m_allocsocket(true) {
		m_socket = new Socket(host,port);
		init(bufSize);
	}
	/**
	Copy constructor, clones the passed socket and uses it for communications.  
	@param s [in] The source socket
	@param bufSize [in] Specifies the size of the buffer, -1 indicates the buffer should
						match the system's TCP buffer size
	*/	
	explicit BufferedSocket(const Socket& src, int bufSize = -1) : Socket(-1), m_allocsocket(true) {
		m_socket = src.clone();
		init(bufSize);
	}
	/**
	Constructs the BufferedSocket and uses src directly as for communications.
	@param src [in] The socket to use in communications
	@param bufSize [in] Specifies the size of the buffer, -1 indicates the buffer should
						match the system's TCP buffer size
	*/	
	explicit BufferedSocket(Socket* src, int bufSize = -1) : Socket(-1), m_allocsocket(false) {
		m_socket = src;
		init(bufSize);
	}
	/**
	Copy constructor, clones the passed socket and uses it for communications.  
	@param s [in] The source socket
	*/	
	explicit BufferedSocket(const BufferedSocket& s) : Socket(-1), m_allocsocket(true) {
		m_socket = m_socket->clone();
		init((int)s.m_bufSize);
	}
	/**
	Copy operator, clones the passed socket and uses it for communications.  
	@param s [in] The source socket
	@return A reference to this object
	*/	
	BufferedSocket& operator=(const BufferedSocket& s) {
		if (m_allocsocket)
			delete m_socket;
		m_allocsocket = true;
		m_socket = s.m_socket->clone();
		m_inPos = 0;
		m_outPos = 0;
		m_outEnd = 0;
		return *this;
	}

	/**
	Copy operator, clones the passed socket and uses it for communications.  
	@param s [in] The source socket
	@return A reference to this object
	*/	
	BufferedSocket& operator=(const Socket& s) {
		if (m_allocsocket)
			delete m_socket;
		m_allocsocket = true;
		m_socket = s.clone();
		m_inPos = 0;
		m_outPos = 0;
		m_outEnd = 0;
		return *this;
	}


	virtual ~BufferedSocket();
	
	virtual size_t send(const char* sendBuffer, size_t length)	throw (SocketException);
	virtual size_t receive(char* readBuffer, size_t length)		throw (SocketException);
	virtual size_t receiveSome(char* readBuffer, size_t length)	throw (SocketException);

	virtual int get() throw (SocketException);
	virtual int peek() throw (SocketException);
	virtual int putback(int c=-1) throw (SocketException);
	

	virtual void getLine(std::string& line, const std::string& eol = "\n") throw (SocketException);

	virtual int flush() throw (SocketException);
	virtual void close() throw (SocketException);


	BufferedSocket& operator << (const std::string& str) throw (SocketException);
	BufferedSocket& operator << (BufferedSocket& (*sockFunc)(BufferedSocket*) throw (SocketException)) throw (SocketException);

	virtual Socket* clone() { return new BufferedSocket(*this); }

	virtual void getPeerName(struct sockaddr_in* addr) { m_socket->getPeerName(addr); }

	virtual int getSocket() { return m_socket->getSocket(); }

protected:
	virtual void underflow() throw (SocketException);

protected:
	char		*m_inBuffer;
	size_t		m_inPos;

	char		*m_outBuffer;
	size_t		m_outPos;
	size_t		m_outEnd;

	size_t		m_bufSize;
	
	bool		m_allocsocket;
	Socket		*m_socket;
};

/**
Writes a newline character to the buffered socket.
@param s [in] A buffered socket
@return A reference to s
*/
BufferedSocket& endl(BufferedSocket* s) throw (SocketException);
/**
Flushes the buffered socket's buffer.
@param s [in] A buffered socket
@return A reference to s
*/
BufferedSocket& flush(BufferedSocket* s) throw (SocketException);

/**
A class which implements a TCP server socket..
*/
class ServerSocket : protected Socket
{
public:
	ServerSocket(short bindPort, in_addr_t bindAddress = INADDR_ANY, int listenerBacklog = 5) throw (SocketException);
	virtual ~ServerSocket() throw (SocketException);

	Socket acceptConnection() throw (SocketException);

	virtual void close() throw (SocketException) { Socket::close();	}

	virtual int getSocket() { return Socket::getSocket(); };
};

template<class _SOCKET_T>
class Selector
{
public:
	void selectReaders(std::list<_SOCKET_T*>& candidates, int timeoutMS = 500)
	{
		typename std::list<_SOCKET_T*>::iterator i;
		fd_set	rd;
		int		maxfs = 0;
		struct timeval	timeout;

		timeout.tv_sec = 0;
		timeout.tv_usec = timeoutMS * 1000;

		FD_ZERO(&rd);
		for(i = candidates.begin(); i!=candidates.end(); ++i) {
			FD_SET((*i)->getSocket(), &rd);
			maxfs = (maxfs < (*i)->getSocket()) ? (*i)->getSocket() : maxfs;
		}
		
		int rc = select(maxfs+1, &rd, NULL, NULL, &timeout);
		if (rc > 0) {
			for(i = candidates.begin(); i!=candidates.end();) {
				if (!FD_ISSET((*i)->getSocket(), &rd)) {
					i = candidates.erase(i);
				} else {
					++i;
				}
			}
		} else {
			int err = errno;
			candidates.clear();
		}
	}
};




} // end of tcpsocket namespace

#endif

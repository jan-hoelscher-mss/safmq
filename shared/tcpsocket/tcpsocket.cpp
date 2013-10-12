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
#pragma warning(disable: 4786)

#include "tcpsocket.h"
#ifdef _WIN32
	#define EWOULDBLOCK             WSAEWOULDBLOCK
	#define EINPROGRESS             WSAEINPROGRESS
	#define EALREADY                WSAEALREADY
	#define ENOTSOCK                WSAENOTSOCK
	#define EDESTADDRREQ            WSAEDESTADDRREQ
	#define EMSGSIZE                WSAEMSGSIZE
	#define EPROTOTYPE              WSAEPROTOTYPE
	#define ENOPROTOOPT             WSAENOPROTOOPT
	#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
	#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
	#define EOPNOTSUPP              WSAEOPNOTSUPP
	#define EPFNOSUPPORT            WSAEPFNOSUPPORT
	#define EAFNOSUPPORT            WSAEAFNOSUPPORT
	#define EADDRINUSE              WSAEADDRINUSE
	#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
	#define ENETDOWN                WSAENETDOWN
	#define ENETUNREACH             WSAENETUNREACH
	#define ENETRESET               WSAENETRESET
	#define ECONNABORTED            WSAECONNABORTED
	#define ECONNRESET              WSAECONNRESET
	#define ENOBUFS                 WSAENOBUFS
	#define EISCONN                 WSAEISCONN
	#define ENOTCONN                WSAENOTCONN
	#define ESHUTDOWN               WSAESHUTDOWN
	#define ETOOMANYREFS            WSAETOOMANYREFS
	#define ETIMEDOUT               WSAETIMEDOUT
	#define ECONNREFUSED            WSAECONNREFUSED
	#define ELOOP                   WSAELOOP
	#define EHOSTDOWN               WSAEHOSTDOWN
	#define EHOSTUNREACH            WSAEHOSTUNREACH
	#define EPROCLIM                WSAEPROCLIM
	#define EUSERS                  WSAEUSERS
	#define EDQUOT                  WSAEDQUOT
	#define ESTALE                  WSAESTALE
	#define EREMOTE                 WSAEREMOTE
	typedef int socklen_t;
	#define SOCERRNO				h_errno

	struct _WSAINIT {
		_WSAINIT() {
			WSADATA data;
			::WSAStartup(MAKEWORD( 2, 0 ), & data);
		}
		~_WSAINIT() {
			::WSACleanup();
		}
	} _INITTER;

	#ifdef min
		#undef min
		#define min _cpp_min
	#endif
	#ifdef max
		#undef max
		#define max _cpp_max
	#endif


#else
	#include <netdb.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/time.h>
	#include <fcntl.h>
	#define SOCERRNO				errno
#endif
#include <new>
#include <memory.h>
#include <iostream>
using namespace std;

namespace tcpsocket {

static void sleep(int milliseconds)
{
#ifdef _WIN32
	::Sleep(milliseconds);
#else
	timespec ts = { 0, milliseconds*1000*1000 };
	::nanosleep(&ts,NULL);
#endif
}

// /////////////////////////////////////////////////////////////////////////////
// SocketException Implementation
// /////////////////////////////////////////////////////////////////////////////
/**
Provides the error message from the exception
@return A string depicting the error.
*/
const char* SocketException::what()
{ 
#define RETS(s,m) case s: return #s": "m;
#ifdef _WIN32
#define SRETS(s,m) case s+WSABASEERR: return #s": "m;
#else
#define SRETS(s,m) RETS(s,m)
#endif

	switch (m_err) {
		RETS(EWOULDBLOCK,"Resource temporarily unavailable")
		RETS(EINPROGRESS,"Operation now in progress")
		RETS(EALREADY,"Operation already in progress")
		RETS(ENOTSOCK,"Socket operation on nonsocket")
		RETS(EDESTADDRREQ,"Destination address required")
		RETS(EMSGSIZE,"Message too long")
		RETS(EPROTOTYPE,"Protocol wrong type for socket")
		RETS(ENOPROTOOPT,"Bad protocol option")
		RETS(EPROTONOSUPPORT,"Protocol not supported")
		RETS(ESOCKTNOSUPPORT,"Socket type not supported")
		RETS(EOPNOTSUPP,"Operation not supported")
		RETS(EPFNOSUPPORT,"Protocol family not supported")
		RETS(EAFNOSUPPORT,"Address family not supported by protocol family")
		RETS(EADDRINUSE,"Address already in use")
		RETS(EADDRNOTAVAIL,"Cannot assign requested address")
		RETS(ENETDOWN,"Network is down")
		RETS(ENETUNREACH,"Network is unreachable")
		RETS(ENETRESET,"Network dropped connection on reset")
		RETS(ECONNABORTED,"Software caused connection abort")
		RETS(ECONNRESET,"Connection reset by peer")
		RETS(ENOBUFS,"No buffer space available")
		RETS(EISCONN,"Socket is already connected")
		RETS(ENOTCONN,"Socket is not connected")
		RETS(ESHUTDOWN,"Cannot send after socket shutdown")
		RETS(ETIMEDOUT,"Connection timed out")
		RETS(ECONNREFUSED,"Connection refused")
		RETS(EHOSTDOWN,"Host is down")
		RETS(EHOSTUNREACH,"No route to host")
		//RETS(EPROCLIM,"Too many processes")
		SRETS(EINTR,"Operation interrupted")
		SRETS(EBADF,"Bad file number")
		SRETS(EACCES,"Permission denied")
		SRETS(EFAULT,"Bad address")
		SRETS(EINVAL,"Invalid argument")
		SRETS(EMFILE,"Too many files opened")
		//RETS(ETOOMANYREFS,"")
		//RETS(ELOOP,"")
		//RETS(EUSERS,"")
		//RETS(EDQUOT,"")
		//RETS(ESTALE,"")
		//RETS(EREMOTE,"")
	}
#undef RETS
#undef SRETS
	return "Unknown Error";
}

// ////////////////////////////////////////////////////////////////////
// Socket Implementation
// ////////////////////////////////////////////////////////////////////
/**
Constructs the socket and opens a connection to a server.
@param host [in] The address of the server or DNS name
@param port [in] The TCP port to connect to
*/
Socket::Socket(const char* host, short port) throw (SocketException)
{
	struct sockaddr_in	dest;
	init();

	m_socket = (int)::socket(AF_INET, SOCK_STREAM, 0);

	if (m_socket == -1)
		throw SocketException(SOCERRNO);

	unsigned long addr;
	if ( (addr = ::inet_addr(host)) == (INADDR_NONE))  {
		struct hostent*	hp = ::gethostbyname(host);
		if (hp)
			addr = *(unsigned long*)hp->h_addr;
		else
			throw SocketException(ENOENT);
	}

	::memset(&dest, 0, sizeof(dest));
	dest.sin_addr	= *(struct in_addr*)&addr;
	dest.sin_family	= AF_INET;
	dest.sin_port	= htons(port);

	if (::connect(m_socket, (struct sockaddr*)&dest, sizeof(dest)) != 0) {
		int err = SOCERRNO;
		close();
		throw SocketException(err);
	}
}

/**
Destroys the socket.  Note: does not close the socket handle
*/
Socket::~Socket()
{

}

/**
Sends data over the socket.  Sends exactly <code>length</code> bytes.

@param sendBuffer [in] A buffer containing the data to be sent
@param length [in] The number of bytes in the buffer
@return The number of bytes sent, -1 on error, and 0 if the connection was closed
@exception SocketException if the connection was closed and the throw on close flag was set.
*/
size_t Socket::send(const char* sendBuffer, size_t length)	throw (SocketException)
{
	int	sent = 0;
	int	isent;

	while ( sent < (int)length ) {
		isent = ::send(m_socket,sendBuffer+sent,(int)length-sent,0);
		if (isent == 0) {
			if (m_throwOnClose)
				throw SocketException(ECONNRESET);
			break;
		} else if (isent < 0) {
			int err = SOCERRNO;
			if (err == EWOULDBLOCK) {
				sleep(200);
			} else
				throw SocketException(err);
		} else {
			sent += isent;
		}
	}
	return sent;
}

/**
Receives data from the socket.  Receives exactly <code>length</code> bytes or
less if the socket is closed early.

@param sendBuffer [out] A buffer receiving the data 
@param length [in] The maximum number of bytes in the buffer
@return The number of bytes received, -1 on error, and 0 if the connection was closed
@exception SocketException if the connection was closed and the throw on close flag was set.
*/
size_t Socket::receive(char* readBuffer, size_t length)		throw (SocketException)
{
	int	nread = 0;
	int	iread;

	while ( nread < (int)length ) {
		iread = ::recv(m_socket,readBuffer+nread,(int)length-nread,0);
		if (iread == 0) {
			if (m_throwOnClose)
				throw SocketException(ECONNRESET);
			break;
		} else if (iread < 0) {
			int err = SOCERRNO;
			if (err == EWOULDBLOCK) {
				sleep(10);
			} else
				throw SocketException(err);
		} else {
			nread += iread;
		}
	}
	return nread;

}

/**
Receives data from the socket.  Receives upto <code>length</code> bytes.

@param sendBuffer [out] A buffer receiving the data 
@param length [in] The maximum number of bytes in the buffer
@return The number of bytes received, -1 on error, and 0 if the connection was closed
@exception SocketException if the connection was closed and the throw on close flag was set.
*/
size_t Socket::receiveSome(char* readBuffer, size_t length)	throw (SocketException)
{
	int	nread = 0;
	int	iread;
	
	while ( nread == 0 ) {
		iread = ::recv(m_socket,readBuffer+nread,(int)length-nread,0);
		if (iread == 0) {
			int err = SOCERRNO;
			if (m_throwOnClose)
				throw SocketException(ECONNRESET);
			break;
		} else if (iread < 0) {
			int err = SOCERRNO;
			if (err == EWOULDBLOCK) {
				sleep(10);
			} else
				throw SocketException(err);
		} else {
			nread += iread;
		}
	}
	return nread;
}

/**
Closes the socket handle.
*/
void Socket::close() throw (SocketException)
{
	if (m_socket == -1)
		return;
#ifdef _WIN32
	::closesocket(m_socket);
#else
	::shutdown(m_socket,SHUT_RDWR);
	::close(m_socket);
#endif
}

void Socket::sleep(int milliseconds)
{
	tcpsocket::sleep(milliseconds);
}


void Socket::getPeerName(struct sockaddr_in* addr)
{
	socklen_t	len = sizeof(struct sockaddr_in);
	::getpeername(m_socket, (struct sockaddr*)addr, &len);
}

// /////////////////////////////////////////////////////////////////////////////
// BufferedSocket Implementation
// /////////////////////////////////////////////////////////////////////////////

void BufferedSocket::init(int bufSize)
{
	if (bufSize > 0) {
		m_bufSize = bufSize;
		m_inBuffer = new char[bufSize];
		m_outBuffer = new char[bufSize];
	} else if (bufSize < 0) {
		if (m_socket != 0) {
			int	rcvs;
			int snds;
			socklen_t	ilen;
			ilen = sizeof(rcvs);
			::getsockopt(m_socket->getSocket(),SOL_SOCKET,SO_RCVBUF,(char*)&rcvs,&ilen);
			ilen = sizeof(snds);
			::getsockopt(m_socket->getSocket(),SOL_SOCKET,SO_SNDBUF,(char*)&snds,&ilen);


			m_bufSize = std::min(rcvs,snds);
			m_inBuffer = new char[m_bufSize];
			m_outBuffer = new char[m_bufSize];
		}
	} else {
		throw std::bad_alloc();
	}
	m_inPos = 0;
	
	m_outPos = m_outEnd = 0xFFFFFFFF;
}

/**
Destroys the buffered socket object.
*/
BufferedSocket::~BufferedSocket()
{
	delete [] m_inBuffer;
	delete [] m_outBuffer;
	if (m_allocsocket)
		delete m_socket;
}

/**
Sends data over the socket.  Sends exactly <code>length</code> bytes.

@param sendBuffer [in] A buffer containing the data to be sent
@param length [in] The number of bytes in the buffer
@return The number of bytes sent, -1 on error, and 0 if the connection was closed
@exception SocketException if the connection was closed and the throw on close flag was set.
*/
size_t BufferedSocket::send(const char* sendBuffer, size_t length)	throw (SocketException)
{
	int	olength = (int)length;
	while (length > 0) {
		if (m_inPos+length <= m_bufSize) {
			::memcpy(m_inBuffer+m_inPos,sendBuffer,length);
			m_inPos += length;
			length = 0;
			if (m_inPos == m_bufSize) // don't delay the send if it's not necessary
				flush();
		} else { 
			::memcpy(m_inBuffer+m_inPos,sendBuffer,m_bufSize-m_inPos);
			sendBuffer += m_bufSize-m_inPos;
			length -= m_bufSize-m_inPos;
			m_inPos = m_bufSize;
			flush();
		}
	}
	return olength;
}

/**
Receives data from the socket.  Receives exactly <code>length</code> bytes or
less if the socket is closed early.

@param sendBuffer [out] A buffer receiving the data 
@param length [in] The maximum number of bytes in the buffer
@return The number of bytes received, -1 on error, and 0 if the connection was closed
@exception SocketException if the connection was closed and the throw on close flag was set.
*/
size_t BufferedSocket::receive(char* readBuffer, size_t length)		throw (SocketException)
{
	int	olength = (int)length;
	
	while (m_outEnd && length > 0) {
		if (length <= m_outEnd-m_outPos) {
			::memcpy(readBuffer, m_outBuffer+m_outPos, length);
			m_outPos += length;
			length = 0;
		} else {
			if (m_outEnd != m_outPos)
				::memcpy(readBuffer, m_outBuffer+m_outPos, m_outEnd-m_outPos);
			length -= m_outEnd-m_outPos;
			readBuffer += m_outEnd-m_outPos;
			underflow();
		}
	}
	if (length)
		return olength - length;

	return olength;
}

/**
Receives data from the socket.  Receives upto <code>length</code> bytes.

@param sendBuffer [out] A buffer receiving the data 
@param length [in] The maximum number of bytes in the buffer
@return The number of bytes received, -1 on error, and 0 if the connection was closed
@exception SocketException if the connection was closed and the throw on close flag was set.
*/
size_t BufferedSocket::receiveSome(char* readBuffer, size_t length)	throw (SocketException)
{
	int	olength = (int)length;
	while (m_outEnd && length == olength) {
		if (length <= m_outEnd-m_outPos) {
			::memcpy(readBuffer, m_outBuffer+m_outPos, length);
			m_outPos += length;
			length = 0;
		} else {
			if (m_outEnd != m_outPos)
				::memcpy(readBuffer, m_outBuffer+m_outPos, m_outEnd-m_outPos);
			length -= m_outEnd-m_outPos;
			underflow();
		}
	}
	return olength - length;
}

/**
Reads a single character from the stream
@return -1 in case the stream is closed
@throw SocketException
*/
int BufferedSocket::get() throw (SocketException)
{
	unsigned char	c;
	size_t	i = receive((char*)&c,sizeof(c));
	if (i != sizeof(c))
		return -1;
	return (int)c;
}

/**
Looks at the buffer and returns the first byte in the buffer.
@return The first byte in teh buffer or -1 on error
*/
int BufferedSocket::peek() throw (SocketException)
{
	if (m_outPos == m_outEnd) underflow();
	if (m_outPos < m_outEnd)
		return (int)(((unsigned char*)m_outBuffer)[m_outPos]);
	return -1;
}
/**
Places a character back into the buffer
@return -1 on error
*/
int BufferedSocket::putback(int c) throw (SocketException)
{
	if (m_outPos > 0) {
		if (c == -1) {
			m_outPos--;
		} else {
			m_outPos--;
			((unsigned char*)m_outBuffer)[m_outPos] = (unsigned char)(c & 0x00FF);
		}
		return (int)(((unsigned char*)m_outBuffer)[m_outPos]);
	}
	return -1;
}

/**
Reads until the end of the line has been read
@throw SocketException
*/
void BufferedSocket::getLine(std::string& line, const std::string& eol) throw (SocketException)
{
	int	c;
	line.erase(line.begin(),line.end());
	while ( line.length() < eol.length() || line.substr(line.length()-eol.length()) != eol) {
		try {
			c = get();
		
			if (c >= 0)
				line.append(1,(char)c);
			else if (m_throwOnClose) {
				throw SocketException(ECONNRESET);
			} else
				break;
		} catch (SocketException e) {
			if (line.length() == 0)
				throw;
			break;
		}
	}
	if ( line.length() >= eol.length() && line.substr(line.length()-eol.length()) == eol)
		line.erase(line.length()-eol.length());
}

/**
Flushes the write buffer over the socket connection.
@return the number of bytes sent, 0 if the socket was closed, or -1 on an error
@exception Throws SocketException if the socket was closed and the throw on close flag was set.
*/
int BufferedSocket::flush() throw (SocketException)
{
	int sent = (int)m_socket->send(m_inBuffer,m_inPos);
	if (sent > 0)
		m_inPos = 0;
	else {
		int err = SOCERRNO;
		return sent;
	}
	return sent;
}

/**
Writes a string to the buffered socket.
@param str The string to write to the buffered socket
@return A reference to this buffered socket
*/
BufferedSocket& BufferedSocket::operator << (const std::string& str) throw (SocketException)
{
	send(str.c_str(),str.length());
	return *this;
}

/**
Calls a stream format function like endl or flush to operate against this socket.
@param str The string to write to the buffered socket
@return A reference to this buffered socket
*/
BufferedSocket& BufferedSocket::operator << (BufferedSocket& (*sockFunc)(BufferedSocket*) throw (SocketException)) throw (SocketException)
{
	return sockFunc(this);
}

void BufferedSocket::underflow() throw (SocketException)
{
	m_outPos = 0;
	m_outEnd = 0; // set just incase Socket::receiveSome throws an exception
	m_outEnd = m_socket->receiveSome(m_outBuffer,m_bufSize);
}

/**
Flushes and closes the socket.
*/
void BufferedSocket::close() throw (SocketException)
{
	flush();
	m_socket->close();
}

BufferedSocket& endl(BufferedSocket* s) throw (SocketException)
{
	s->send("\n",1);
	s->flush();
	return *s;
}

BufferedSocket& flush(BufferedSocket* s) throw (SocketException)
{
	s->flush();
	return *s;	
}

// /////////////////////////////////////////////////////////////////////////////
// ServerSocket Implementation
// /////////////////////////////////////////////////////////////////////////////
/**
Constructs the server socket binding to the port and optionally using the bind address and listener backlog
@param bindPort [in] The TCP port to bind to
@param bindAddress [in,optional] Optional address to bind to, if not specified all local address
									will be bound.
@param listenerBacklog [in,optional] Optional depth of the listener queue.
@throw SocketException on an error
*/
ServerSocket::ServerSocket(short bindPort, in_addr_t bindAddress, int listenerBacklog) throw (SocketException)
{
	struct sockaddr_in	saddr;

	m_socket = (int)::socket(AF_INET, SOCK_STREAM, 0);

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(bindPort);
	saddr.sin_addr.s_addr = bindAddress;

	int	iReuse = 1;
	::setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &iReuse, sizeof(int));

	if (::bind(m_socket, (struct sockaddr*)&saddr, sizeof(saddr)) != 0) {
		int err = SOCERRNO;
		close();
		throw SocketException(err);
	}

	if (::listen(m_socket, listenerBacklog) != 0) {
		int err = SOCERRNO;
		close();
		throw SocketException(err);
	}
}

/**
Releases the server socket
@throw SocketException on an error
*/
ServerSocket::~ServerSocket() throw (SocketException)
{
	close();
}

/**
Accepts a connection from the server socket.
@return A socket connected to a client
@throw SocketException on an error
*/
Socket ServerSocket::acceptConnection() throw (SocketException)
{
	int	soc = (int)::accept(m_socket,NULL,NULL);
	if (soc < 0)
		throw SocketException(SOCERRNO);
	return Socket(soc);
}



} // end of namespace tcpsocket

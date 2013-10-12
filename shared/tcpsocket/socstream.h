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
#ifndef _SOCKSTREAM_H_
#define _SOCKSTREAM_H_

#include <ios>
#include <string>
#include <stdlib.h>
#include "tcpsocket.h"
#include <iostream>

/**
A namespace containing classes to work with TCP socket communications.
*/
namespace tcpsocket {

/**
Drop in replacement for basic_streambuf.  This template class derives from
<code>std::basic_streambuf</code> to provide a stream buffer which reads to and
from a <code>tcpsocket::BufferedSocket</code>.
@param _E integer type
@param _Tr Traits for _E
*/
template<class _E, class _Tr = std::char_traits<_E> >
class basic_socstreambuf : public std::basic_streambuf<_E, _Tr> {
public:
	/// basic_socstreambuf integer type
	typedef typename std::basic_streambuf<_E,_Tr>::int_type int_type;
	/// basic_socstreambuf stream position type
	typedef typename std::basic_streambuf<_E,_Tr>::pos_type pos_type;
	/// basic_socstreambuf stream offset type
	typedef typename std::basic_streambuf<_E,_Tr>::off_type off_type;

	/**
	Constructs a basic_socstreambuf specifying the buffer size and buffer mode.

	@param bufSize [in,optional] The size of the buffer to be used, default uses a buffer the same size as a TCP socket buffer
	@param _W [in,optional] Specifies the mode of the buffer, default provides for read, write, in binary mode.
	*/
	explicit basic_socstreambuf(int bufSize=-1, std::ios_base::openmode _W = std::ios::in | std::ios::out | std::ios::binary)  throw (SocketException) : soc(bufSize)
	{
		init(_W);
		opened = false;
	}
	/**
	Constructs a basic_socstreambuf from an already opened socket.

	@param s [in] An already opened socket
	@param bufSize [in,optional] The size of the buffer to be used, default uses a buffer the same size as a TCP socket buffer
	@param _W [in,optional] Specifies the mode of the buffer, default provides for read, write, in binary mode.
	@exception SocketException thrown if an error occurs
	*/
	explicit basic_socstreambuf(const Socket& s, int bufSize=-1, std::ios_base::openmode _W = std::ios_base::in | std::ios_base::out) throw (SocketException)
		: soc(s,bufSize)
	{
		init(_W);
		opened = false;
	}
	/**
	Constructs a basic_socstreambuf from an address and port.

	@param addr [in] can be a TCP/IP address or DSN name
	@param port [in] The TCP/IP port to connect to
	@param bufSize [in,optional] The size of the buffer to be used, default uses a buffer the same size as a TCP socket buffer
	@param _W [in,optional] Specifies the mode of the buffer, default provides for read, write, in binary mode.
	@exception SocketException thrown if an error occurs connecting.
	*/
	explicit basic_socstreambuf(const char* addr, short port, int bufSize=-1, std::ios_base::openmode _W = std::ios_base::in | std::ios_base::out) throw (SocketException)
		: soc(addr, port, bufSize)
	{
		init(_W);
		opened = true;
	}
	/**
	Destroys the basic_socstreambuf, closes the socket if the socket was opened by
	the stream buffer
	*/
	virtual ~basic_socstreambuf()
	{
		if (opened)
			close();
	}
	/**
	Opens a TCP/IP connection to the endpoint.
	@param addr [in] can be a TCP/IP address or DSN name
	@param port [in] The TCP/IP port to connect to
	@exception SocketException thrown if an error occurs connecting.
	*/
	void open(const char *addr, short port) throw (SocketException)
	{
		soc = BufferedSocket(addr, port);
	}
	/**
	Closes the TCP/IP socket connection
	@exception SocketException thrown if an error occurs
	*/
	void close() throw (SocketException)
	{
		soc.close();
		opened = false;
	}
	/**
	Provides the most recent socket error.
	*/
	int getsocerror() {
		return socerror;
	}

	/**
	Obtains the address of the peer
	*/
	void getpeername(struct sockaddr_in* addr) { soc.getPeerName(addr); }
protected:
	virtual void imbue(const std::locale &loc)																									{	}
	virtual std::basic_streambuf<_E, _Tr> *setbuf(_E *s, std::streamsize n)																		{ return this; }
	virtual pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)	{ return pos_type(off_type(-1)); }
	virtual pos_type seekpos(pos_type sp, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out)								{  return pos_type(off_type(-1)); }

	virtual int sync() { 
		try {
			return soc.flush();
		} catch (SocketException e) {
			socerror = e.getError();
			return -1;
		}
	}
	virtual std::streamsize showmanyc()		{ return 0; }
	virtual std::streamsize xsgetn(_E *s, std::streamsize n) {  
		try {
			return soc.receive((char*)s, n * sizeof(typename _Tr::char_type)) / sizeof(typename _Tr::char_type);
		} catch (SocketException e) {
			socerror = e.getError();
			return std::streamsize(-1);
		}
	}
	virtual int_type underflow() {
		try {
			int_type r = soc.peek();
			if ( r < 0 ) return _Tr::eof();
			return r;
		} catch (SocketException e) {
			socerror = e.getError();
			return _Tr::eof();
		}
	}
	virtual int_type uflow() { 
		try {
			int_type r = soc.get(); 
			if (r < 0) return _Tr::eof();
			return r;
		} catch (SocketException e) {
			socerror = e.getError();
			return _Tr::eof();
		}
	}
	virtual int_type pbackfail(int_type c = _Tr::eof())	{
		try {
			return (int_type)soc.putback((int)c);
		} catch (SocketException e) {
			socerror = e.getError();
			return _Tr::eof();
		}
		return _Tr::eof();
	}
	virtual std::streamsize xsputn(const _E *s, std::streamsize n)	{
		try {
			return soc.send((const char*)s, n * sizeof(typename _Tr::char_type)) / sizeof(typename _Tr::char_type);
		} catch (SocketException e) {
			socerror = e.getError();
			return _Tr::eof();
		}
	}
	virtual int_type overflow(int_type c = _Tr::eof()) {
		try {
			if (_Tr::eq_int_type(c, _Tr::eof()))
				return _Tr::eof();
			else {
				_E	C = _Tr::to_char_type(c);

				int send = soc.send((const char*)&C, sizeof(C));
				if (send != sizeof(C))
					return _Tr::eof();
				return _Tr::not_eof(c);
			}
			return _Tr::eof();
		} catch (SocketException e) {
			socerror = e.getError();
			return _Tr::eof();
		}
	}
protected:
	void init(std::ios::openmode mode) {
		socerror = 0;
#ifdef GNU_STREAM_SET_M_MODE
		_M_mode = mode;
#endif
	}
private:
	BufferedSocket	soc;
	bool			opened;
	int				socerror;
};

/**
Drop in replacement for a basic_iostream.  This class implements it's stream buffer as
a basic_socstreambuf to allow for TCP/IP stream communications.
@param _E integer type
@param _Tr Traits for _E
*/
template<class _E, class _Tr = std::char_traits<_E> >
	class basic_socstream : public std::basic_iostream<_E, _Tr> {
public:

	/**
	Constructs a basic_socstream specifying the buffer size and buffer mode.

	@param bufSize [in,optional] The size of the buffer to be used, default uses a buffer the same size as a TCP socket buffer
	@param _W [in,optional] Specifies the mode of the buffer, default provides for read, write, in binary mode.
	*/
	explicit basic_socstream(int bufSize=-1, std::ios_base::openmode _W = std::ios::in | std::ios::out | std::ios::binary) : std::basic_iostream<_E, _Tr>(&socbuf), socbuf(bufSize,_W) {}
	/**
	Constructs a basic_socstream from an address and port.

	@param addr [in] can be a TCP/IP address or DSN name
	@param port [in] The TCP/IP port to connect to
	@param bufSize [in,optional] The size of the buffer to be used, default uses a buffer the same size as a TCP socket buffer
	@param _W [in,optional] Specifies the mode of the buffer, default provides for read, write, in binary mode.
	@exception SocketException thrown if an error occurs connecting.
	*/
	explicit basic_socstream(const char* addr, short port, int bufSize=-1, std::ios_base::openmode _W = std::ios::in | std::ios::out)  throw (SocketException) : std::basic_iostream<_E, _Tr>(&socbuf), socbuf(addr,port,bufSize,_W) {}
	/**
	Constructs a basic_socstream from an already opened socket.

	@param s [in] An already opened socket
	@param bufSize [in,optional] The size of the buffer to be used, default uses a buffer the same size as a TCP socket buffer
	@param _W [in,optional] Specifies the mode of the buffer, default provides for read, write, in binary mode.
	@exception SocketException thrown if an error occurs
	*/
	explicit basic_socstream(const Socket& soc, int bufSize=-1, std::ios_base::openmode _W = std::ios::in | std::ios::out)  throw (SocketException) : std::basic_iostream<_E, _Tr>(&socbuf), socbuf(soc,bufSize,_W) 	{ }
	/**
	Destroys the basic_socstream
	*/
	virtual ~basic_socstream() { }
	/**
	Opens a TCP/IP connection to the endpoint.
	@param addr [in] can be a TCP/IP address or DSN name
	@param port [in] The TCP/IP port to connect to
	@exception SocketException thrown if an error occurs connecting.
	*/
	void open(const char *addr, short port) throw (SocketException)	{ socbuf.open(addr,port); }
 	/**
	Closes the TCP/IP socket connection
	@exception SocketException thrown if an error occurs
	*/
	void close() throw (SocketException)							{ socbuf.close(); }
	/**
	Provides the most recent socket error.
	*/
	int getsocerror()												{ return socbuf.getsocerror();}
	/**
	Provides access to the stream buffer
	*/
	basic_socstreambuf<_E, _Tr> *rdbuf() const						{ return ((basic_socstreambuf<_E, _Tr> *)&socbuf);  }

	/**
	Provides the address of the peer
	*/
	void getpeername(struct sockaddr_in* addr)						{ socbuf.getpeername(addr); }
private:
	basic_socstreambuf<_E, _Tr> socbuf;
};

typedef basic_socstream<char> socstream;

} // end of namespace tcpsocket

#endif

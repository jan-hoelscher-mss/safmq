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




This software implements a drop in replacemet for a std::iostream and stores
data in a memory buffer.
*/

#ifndef _BUFSTREAM_H_
#define _BUFSTREAM_H_

#ifdef _WIN32
	#undef min
	#define min _cpp_min
	#undef max
	#define max _cpp_max
#endif


#include <ios>
#include <iostream>
#include <stdlib.h>

template<class _E, class _Tr = std::char_traits<_E> >
class buf_streambuf : public std::basic_streambuf<_E, _Tr> {
public:
	typedef typename std::basic_streambuf<_E,_Tr>::int_type int_type;
	typedef typename std::basic_streambuf<_E,_Tr>::pos_type pos_type;
	typedef typename std::basic_streambuf<_E,_Tr>::off_type off_type;

	explicit buf_streambuf(int bufSize=-1, std::ios_base::openmode _W = std::ios::in | std::ios::out | std::ios::binary)
	{
		init(_W, bufSize);
	}

	virtual ~buf_streambuf()
	{
		free(buf);
	}

	_E* buffer()				{ return buf+gpos; }
	const _E* buffer() const	{ return buf+gpos; }
	size_t length() const		{ return ppos-gpos; }
	void reserve(off_type size) {
		if (size > buflen)
			realloc(size);
	}
protected:
    virtual void imbue(const std::locale &loc) {	}
    virtual std::basic_streambuf<_E, _Tr> *setbuf(_E *s, std::streamsize n)	{ return this; }
	virtual pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) {
		off_type gcur = gpos, pcur = ppos;
		
		if (which & std::ios_base::in) {
			if (way == std::ios_base::beg) {
				gcur =  0;
			} else if (way == std::ios_base::end) {
				gcur = ppos;
			} else if (way != std::ios_base::cur)
				return pos_type(off_type(-1));

			if ( off < 0 && -off > gcur)	return pos_type(off_type(-1)); // the offset is to far reversing for success
			gcur = gcur + off;
		}
		if (which & std::ios_base::out) {
			if (way == std::ios_base::beg)
				pcur =  0;

			if ( off < 0 && -off > pcur) return pos_type(off_type(-1)); // negative wrap around
			pcur = pcur + off;
		}
		if (pcur > buflen)  return pos_type(off_type(-1)); // position past the end of the stream
		if (gcur > pcur) return pos_type(off_type(-1)); // output position positioned past end of input position

		gpos = gcur;
		ppos = pcur;
		
		// Give a precidence to the input position as it is most likely reset.
		if (which & std::ios_base::in)		
			return pos_type(off_type(gpos));
		if (which & std::ios_base::out)
			return pos_type(off_type(ppos));

		return pos_type(off_type(-1));
	}
	virtual pos_type seekpos(pos_type sp, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) {
		off_type gcur = gpos, pcur = ppos;

		if ( which & std::ios_base::in ) {
			if ( (off_type)sp < 0 )	return pos_type(off_type(-1)); // negative wrap
			gcur = (off_type)sp;
		}
		if ( which & std::ios_base::out ) {
			if ( (off_type)sp < 0)	return pos_type(off_type(-1)); // negative wrap
			pcur = (off_type)sp;
		}
		if (pcur > buflen) return pos_type(off_type(-1));
		if (gcur > pcur) return pos_type(off_type(-1));

		ppos = pcur;
		gpos = gcur;

		if (which & std::ios_base::in)		
			return pos_type(off_type(gpos));
		if (which & std::ios_base::out)
			return pos_type(off_type(ppos));

		return pos_type(off_type(-1));
	}
	virtual int sync()			{ return 0; }
	virtual std::streamsize showmanyc()		{ return 0; }

	virtual std::streamsize xsgetn(_E *s, std::streamsize n) {
		int ln = std::min(n, (std::streamsize)(ppos-gpos));

		memcpy(s,buf+gpos, bytesize(ln));
		gpos += ln;
		return ln;
	}
	virtual int_type underflow() {
		if ( gpos < ppos ) return _Tr::to_int_type(buf[gpos]);
		return _Tr::eof();
	}
	virtual int_type uflow() { 
		if ( gpos < ppos ) 
			return _Tr::to_int_type(buf[gpos++]);
		return _Tr::eof();
	}
	virtual int_type pbackfail(int_type c = _Tr::eof())	{
		if (gpos > 0) {
			if (_Tr::eq_int_type(_Tr::eof(), c)) {
				gpos--;
				return _Tr::to_int_type(buf[gpos]);
			} else {
				gpos--;
				buf[gpos] = _Tr::to_char_type(c);
				return c;
			}
		}
		return _Tr::eof();
	}
	virtual std::streamsize xsputn(const _E *s, std::streamsize n) {
		if (ppos+n > buflen)
			realloc(buflen+n);
		memcpy(buf+ppos,s,bytesize(n));
		ppos+=n;
		return n;
	}
	virtual int_type overflow(int_type c = _Tr::eof()) {
		if (_Tr::eq_int_type(c, _Tr::eof()))
			return _Tr::eof();
		else {
			_E	C = _Tr::to_char_type(c);
			xsputn(&C,1);
			return _Tr::not_eof(c);
		}
		return _Tr::eof();
	}

private:
	void init(std::ios::openmode mode, int len) {
#ifdef GNU_STREAM_SET_M_MODE
		_M_mode = mode;
#endif
		if (len < 0) 
			buflen = 1024;
		else
			buflen = len;
		buf = (_E*)::malloc(bytesize(buflen));
		gpos = ppos = 0;
	}

	void realloc(int len) {
		buflen = len;
		buf = (_E*)::realloc(buf, bytesize(buflen));
	}

	size_t bytesize(size_t len) {
		return sizeof(_E) * len;
	}

	_E*			buf;
	off_type	buflen;		// buffer length
	off_type	gpos;	// location read from
	off_type	ppos;	// location written to
public: 
	static int cdcnt;
};

template<class _E, class _Tr = std::char_traits<_E> >
	class basic_bufstream : public std::basic_iostream<_E, _Tr> {
public:

	explicit basic_bufstream(int bufSize=-1, std::ios_base::openmode _W = std::ios::in | std::ios::out | std::ios::binary) : std::basic_iostream<_E, _Tr>(&buf), buf(bufSize,_W) {}
	//explicit basic_bufstream(const _E* pbuf, size_t bufSize, std::ios_base::openmode _W = in|out) : std::basic_iostream<_E,_Tr>(&buf), buf(pbuf,bufSize,_W) {}

	virtual ~basic_bufstream() { }
	buf_streambuf<_E, _Tr> *rdbuf() const							{ return ((buf_streambuf<_E, _Tr> *)&buf);  }

	_E*			buffer()			{ return buf.buffer(); }
	const _E*	buffer() const		{ return buf.buffer(); }
	size_t		length() const		{ return buf.length(); }
	void		reserve(size_t size)	{ buf.reserve(size); }

private:
	buf_streambuf<_E, _Tr> buf;
};

typedef basic_bufstream<char> bufstream;

#endif

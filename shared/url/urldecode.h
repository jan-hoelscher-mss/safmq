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




This software implements a platform independent C++ interface URL/URI decoding.
*/
#ifndef _URLDECODE_H_
#define _URLDECODE_H_

#pragma warning(disable:4786)
#pragma warning(disable:4290)

#include <map>
#include <string>
#include <exception>

namespace URL {

/**
Exception thrown if there is a problem decoding the URL string.
This class extends the standard library's <code>exception</code> 
class.
*/
class urlexception : public std::exception {
public:
    urlexception() throw() {}
 	urlexception(const char* _what) throw() {
		this->_what = _what;
	}
	urlexception(const exception& rhs) throw() {	
		_what = rhs.what();
	}
    exception& operator=(const exception& rhs) throw() {
		_what = rhs.what();
		return *this;
	}
    virtual ~urlexception() throw() {}
    virtual const char *what() const throw() {
		return _what.c_str();
	}
protected:
	std::string _what;
};

/**
This class deocdes urls to determine the following characteristics:

protocol
destination
resource
user identification
user authentication (password)

Encoded into the following format

	protocol://user:password@destination:port/resource?param1=value1&param2=value2

Required items:
	//
	destination

Optional items
	protocol
	user
	password
	port
	resource
	paramN=valueN
*/
class urldecode
{
public:
	typedef std::multimap<std::string,std::string>	ParameterMultimap;

	urldecode(std::string url) throw (urlexception) : pass_specified(false) {
		const std::string		destleader("//");
		std::string::size_type	b,e;


		b = url.find(destleader);
		if (b == std::string::npos) {
			resource = url;
			//throw urlexception("no destination leader");
		} else {
			if (b > 0) {
				if (b > 2 && url[b-1] == ':') {
					protocol = url.substr(0,b-1);
				} else {
					throw urlexception("syntax error: invalid protocol specification");
				}
			}

			if (b+2 < url.length() && url[b+2] != '/') {
				e = url.find("/",b+2);
				if (e != std::string::npos) {
					if (e+1 <= url.length())
						resource = url.substr(e);
					destination = url.substr(b+2, e-(b+2));
				} else {
					destination = url.substr(b+2);
				}

				b = resource.find("?");
				if (b != std::string::npos) {
					std::string::size_type	tmp_b = b;
					if (b+1 < resource.length()) {
						std::string				params = resource.substr(b+1);
						std::string::size_type	eq;
						e = (std::string::size_type)-1;
						while ( (e=params.find("&",b=e+1)) != std::string::npos) {
							eq = params.find("=",b);
							if (eq <= e)
								parameters.insert(ParameterMultimap::value_type(params.substr(b,eq-b),params.substr(eq+1,e-eq-1)));
						}
						if ( b < params.length()) {
							eq = params.find("=",b);
							e = params.length();
							if (eq <= e)
								parameters.insert(ParameterMultimap::value_type(params.substr(b,eq-b),params.substr(eq+1,e-eq-1)));
						}
					}
					resource = resource.substr(0,tmp_b);
				}
				
				b = destination.find("@");
				if (b != std::string::npos) {
					if (b+1 < destination.length()) {
						user = destination.substr(0,b);
						destination = destination.substr(b+1);
						
						b = user.find(":");
						if (b != std::string::npos) {
							pass_specified = true;
							if (b+1 < user.length())
								password = user.substr(b+1);
							user = user.substr(0,b);
						}
					} else {
						throw urlexception("syntax: invalid user/destination specification");
					}
				}

				b = destination.find(":");
				if (b != std::string::npos) {
					if (b+1 < destination.length())
						port = destination.substr(b+1);
					else
						throw urlexception("syntax error: invalid port specification");
					destination = destination.substr(0,b);
				}

			} else {
				throw urlexception("syntax error: invalid destination specificaiton");
			}
		}
	}

	const std::string&	getProtocol() const 		{ return protocol; }
	const std::string&	getUser() const 			{ return user; }
	bool				isPasswordSpecified() const { return pass_specified; }
	const std::string&	getPassword() const 		{ return password; }
	const std::string&	getDestination()	const	{ return destination; }
	const std::string&	getPort() const 			{ return port; }
	const std::string&	getResource() const 		{ return resource; }

	const ParameterMultimap&	getParameters() const		{ return parameters; }
protected:

	std::string protocol;
	std::string user;
	bool		pass_specified;
	std::string password;
	std::string destination;
	std::string port;
	std::string resource;
	ParameterMultimap	parameters;
};

}

#endif

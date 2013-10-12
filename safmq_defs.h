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
#ifndef _SAFMQ_DEFS_H_
#define _SAFMQ_DEFS_H_

/// Maximum length of a SAFMQ mesasge label
#define SAFMQ_MSGLBL_LENGTH			256
/// Maximum length of a SAFMQ message queue path name
#define SAFMQ_QNAME_LENGTH			1024
/// Maximum length of a SAFMQ user group name
#define SAFMQ_GROUP_NAME_LENGTH		20
/// Maximum length of a SAFMQ user name
#define SAFMQ_USER_NAME_LENGTH		20
/// Maximum length of a SAFMQ password
#define SAFMQ_PASSWORD_LENGTH		20
/// Maximum length of a safmq user description
#define SAFMQ_DESCRIPTION_LENGTH	100

// Type definition for a safmq wide character -- Note: currently a single byte..
//typedef char								SAFMQ_WCHAR_T;

// Type definition for a safmq wide character c++ string
//typedef std::string							SAFMQ_WSTRING;
//typedef std::basic_string<SAFMQ_WCHAR_T>	SAFMQ_WSTRING;

// Type definition for a single-byte character safmq string
//typedef std::string							SAFMQ_STRING;

// Define for labeling a string as a wide character string
#define _WS(s)			s


// NOTE: On systems other than WIN32 these types will be set by the configure program
#ifdef _WIN32
/// Defines the type for a 4 byte integer
#define SAFMQ_INT32		long
/// Defines the type for a 4 byte unsigned integer
#define SAFMQ_UINT32	unsigned long
#else

// For backward compatability with safmq clinets.
// These can be overridden with -D compiler options.
#ifdef SAFMQ_USE_STDINT
#include <stdint.h>
#endif
#ifdef SAFMQ_USE_INTTYPES
#include <inttypes.h>
#endif

#ifndef SAFMQ_INT32
#define SAFMQ_INT32		int32_t
#endif

#ifndef SAFMQ_UINT32
#define SAFMQ_UINT32	uint32_t
#endif

#endif


#define SAFMQ_FILE_ID	unsigned long
//#define UUID_UINT32		SAFMQ_UINT32


// NOTE: if SAFMQ_WCHAR_T is typedef'ed as wchar_t then _WS(s) sould evaluate to L##s

namespace safmq {

/**
The union of an X509 digital certificate subject distinguished name and issuer distinguished name.
*/
struct X509Identity {
	/**
	Default constructor
	*/
	X509Identity() { }
	/**
	Constructs from a subject name and issuer
	@param subject The Subject Name
	@param issuer The issuer name
	*/
	X509Identity(const std::string& subject, const std::string& issuer) : subjectDN(subject), issuerDN(issuer) { }
	/**
	Copy constructor
	@param src The source object
	*/
	X509Identity(const X509Identity& src) {
		*this = src;
	}
	/**
	Copy operator
	@param src The source object
	@returns A reference to this object
	*/
	const X509Identity& operator=(const X509Identity& src) {
		subjectDN = src.subjectDN;
		issuerDN = src.issuerDN;
		return *this;
	}
	/**
	Comparison operator, sorts by subject name then issuer
	@param rhs The object to compare to this
	@returns true if this object preceeds <code>rhs</code>
	*/
	bool operator<(const X509Identity& rhs) const {
		int	cmp;
		return (cmp=subjectDN.compare(rhs.subjectDN)) < 0 || (cmp == 0 && issuerDN < rhs.issuerDN);
	}

	std::string	subjectDN;
	std::string	issuerDN;
};

}

#endif

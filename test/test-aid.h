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

#ifndef _TEST_AID_H_
#define _TEST_AID_H_

#include <string>
#include <sstream>
#include <algorithm>
#include <memory.h>

/**
Classes and functions to simplify test case writing, when converting
tests from Java or C#.
*/
namespace test_aid {

/**
An auto pointer class.  Implements the "acquisition-is-initialization"
pattern.  In this case construction or assignment acquires the pointer resource.
The pointer is deleted if the object goes out of scope or if the object is
reassigned without a call to <code>relese()</code>.<br>

<p>Use the object with the same pointer syntax of a standard pointer.  A cast operator
is provided for auto-casting.  Example: </p>

<pre><code>
//
// Function using a dynamically allocated auto buffer
// buffer automatically released when it is out of scope
//
void foo()
{
	_&lt;char*> a = new char[20];
	time_t	t = time(NULL);

	// note sizeof(a) doesn't work in this case (wouldn't any way with a new...)
	strftime(a, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
	printf("%s", (char*)a);
}
</code></pre>

@param MyType The pointer type to be managed.
*/
template<class MyType>
class _ {
public:
	_()          { p = NULL; }	
	_(MyType* p) { this->p = p; }	
	virtual ~_() { if (p) { delete p; } }
	MyType* operator=(MyType* p) {
		if (this->p) delete this->p;
		this->p = p;
		return this->p;
	}
	/// Audo cast operator to provide access to the raw pointer
	operator MyType*() { return p; }
	const MyType& operator*() const { return *p; }
	MyType& operator*() { return *p; }
	const MyType* operator->() const { return p; }
	MyType* operator->() { return p; }
	/// Releases the pointer and removes it from object control.
	MyType* release() { MyType* t = p; p = NULL; return t; }

protected:
	MyType*	p;
};

/**
Generic function which determines if a vlaue is found in a container.
Uses the generic function <code>std::find<I,T>()</code> to find the
value. <b>Note</b>: Not intended for high performance use!

@param T_CONTAINER [typename] The container type, must support <code>begin()</code> and <code>end()</code> methods.
@param T_PARAM [typename] The type of the parameter.

@param container [in] The container to be searched
@param value [in] The value to look for.
@return [true] if the value was found in the container's sequence, [false] otherwise.
*/
template<class T_CONTAINER, class T_PARAM>
bool contains(T_CONTAINER& container, T_PARAM& value) {
	return find(container.begin(), container.end(), value) != container.end();
}

/**
Parses an <code>int</code> from a std::string.
@param s The source std::string
@return The <code>int</code> found
*/
int parseInt(const std::string s) {
	std::stringstream	i(s);
	int	num;

	i >> num;
	return num;
}

/**
Compares two <code>uuid</code>'s for equality.
@param l The left hand <code>uuid</code>
@param r The right hand <code>uuid</code>
@return [true] if they are equal, [false] otherwise
*/
bool operator==(const uuid& l, const uuid& r) {
	return memcmp(&l, &r, sizeof(uuid)) == 0;
}


/**
Appends a numeric value to a std::string, allows all numeric types
to be upcast to <code>double</code> for appending
@param l The left hand porition of the new std::string
@param r The right hand porition of the new std::string
@return A new std::string consiting of the left hand and right hand poritions
*/
std::string operator+(const std::string& l, double r) {
	std::stringstream o;
	o << l << r;
	return o.str();
}

/**
Appends a <code>uuid</code> value to a std::string.
@param l The left hand porition of the new std::string
@param r The right hand porition of the new std::string
@return A new std::string consiting of the left hand and right hand poritions
*/
std::string operator+(const std::string& l, const uuid& r) {
	std::stringstream o;
	o << l << r;
	return o.str();
}

/**
Decodes an error code to a string value
@param ec The error code to decode.
@return A c-string with the description of the error code.
*/
const char* EC_Decode(safmq::ErrorCode ec) {
#define ECOUT(v) case v: return #v;
    switch(ec) {
        ECOUT(safmq::EC_NOERROR)
        ECOUT(safmq::EC_TIMEDOUT)
        ECOUT(safmq::EC_NOTAUTHORIZED)
        ECOUT(safmq::EC_ALREADYCLOSED)
        ECOUT(safmq::EC_DOESNOTEXIST)
        ECOUT(safmq::EC_NOTOPEN)
        ECOUT(safmq::EC_NETWORKERROR)
        ECOUT(safmq::EC_TTLEXPIRED)
        ECOUT(safmq::EC_CURSORINVALIDATED)
        ECOUT(safmq::EC_DUPLICATEMSGID)
        ECOUT(safmq::EC_SERVERUNAVAIL)
        ECOUT(safmq::EC_NOMOREMESSAGES)
        ECOUT(safmq::EC_FILESYSTEMERROR)
        ECOUT(safmq::EC_CANNOTCHANGEGROUP)
        ECOUT(safmq::EC_CANNOTCHANGEUSER)
        ECOUT(safmq::EC_ALREADYDEFINED)
        ECOUT(safmq::EC_NOTIMPLEMENTED)
        ECOUT(safmq::EC_QUEUEOPEN)
        ECOUT(safmq::EC_NOTLOGGEDIN)
        ECOUT(safmq::EC_ERROR)
        default: return "UNKNOWN";
    }
#undef ECOUT
}

/**
Concat a <code>string</code> and an <code>ErrorCode</code>.
@param l The string
@param r The error code
@return The string concatenated with the ErrorCode.
*/
std::string operator+(const std::string& l, safmq::ErrorCode r) {
	std::stringstream o;
	o << l << EC_Decode(r);
	return o.str();
}

}
#endif


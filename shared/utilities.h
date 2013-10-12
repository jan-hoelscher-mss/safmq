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




This software implements utility classes
*/
#ifndef PROC_UTILITIES_H
#define PROC_UTILITIES_H

#pragma warning(disable: 4786) // Disable warning about long DBG symbols
#pragma warning(disable:4290) // Disable warning about exception specifications


#include <string>
#include <exception>
#include <vector>
#include <stdio.h>

#define SAFESTRCPY(dest,src) (strncpy(dest,src,sizeof(dest)-1), dest[sizeof(dest)-1]='\0')

/**
A collection of utility functions
*/
class CUtilities {
public:
	static int atoi_SubString(const char* sz, size_t len);
	static double Round_Double(double d, int decimal_digits);
	static std::string trim(const std::string& src);
	static std::string tolower(const std::string& src);
	static std::string toupper(const std::string& src);
};

/**
Represents an integer.
*/
class CInteger {
private:
	int		m_val;
public:
	CInteger(int i=0) : m_val(i) {}

	operator int() const { return m_val; }

	std::string ToString() const { 
		return ToString(m_val);
	}

	static std::string ToString(int val) {
		char	buf[32];
		sprintf(buf,"%ld", val);
		return buf;
	}
};

/**
Represents an integer.
*/
class CDouble {
private:
	double		m_val;
public:
	CDouble(double i=0) : m_val(i) {}

	operator double() const { return m_val; }

	std::string ToString() const { 
		return ToString(m_val);
	}

	static std::string ToString(double val, int precision = 4) {
		char	buf[64];
		sprintf(buf,"%.*lf", precision, val);
		return buf;
	}
};


class CStringHelper {
public:
	static std::string trim(const std::string& src) {
		int	b = src.find_first_not_of("\t\r\n ");
		int e = src.find_last_not_of("\t\r\n ");
		
		if (b != std::string::npos)
			return src.substr(b,e-b+1);
		return "";
	}
//	static std::wstring trim(const std::wstring& src) {
//		int	b = src.find_first_not_of(L"\t\r\n ");
//		int e = src.find_last_not_of(L"\t\r\n ");
//
//		if (b != std::wstring::npos)
//			return src.substr(b,e-b+1);
//		return L"";
//	}

	static void tokenize(const std::string& src, const std::string& tokenSep, std::vector<std::string>& tokens) {
		int	e = -1, b;
		
		tokens.clear();
		while ( (e=src.find(tokenSep, b=e+1)) != std::string::npos) {
			tokens.push_back(src.substr(b,e-b));
		}
		if (b < (int)src.length())
			tokens.push_back(src.substr(b));
	}

//	static void tokenize(const std::wstring& src, const std::wstring& token, std::vector<std::wstring>& tokens) {
//		int	e = -1, b;
//		tokens.clear();
//		while ( (e=src.find(token, b=e+1)) != std::wstring::npos) {
//			tokens.push_back(src.substr(b,e-b));
//		}
//		if (b < (int)src.length())
//			tokens.push_back(src.substr(b));
//	}

	static void tokenizeEsc(const std::string& src, char tokenSep, char escChr, std::vector<std::string>& tokens) {
		std::string	item;

		tokens.clear();
		for(std::string::const_iterator i = src.begin(); i != src.end(); ++i) {
			if (*i == tokenSep) {
				tokens.push_back(item);
				item = "";
			} else {
				if (*i == escChr)
					++i;
				item.append(1, *i);
			}
		}
		if (tokens.size() || item.length())
			tokens.push_back(item);
	}


	static std::string& escape(const std::string &src, const std::string& tokens, const char escChr, std::string& buf) {
		buf = "";

		for(std::string::const_iterator i = src.begin(); i != src.end(); ++i) {
			if (tokens.find(*i) != std::string::npos || *i == escChr) {
				buf.append(1, escChr);
			}
			buf.append(1, *i);
		}

		return buf;
	}
};


/**
The CTimeSpan class is used to identify the amount of time
between two dates.
*/
class CTimeSpan
{
protected:
	double	m_span;
public:
	static const CTimeSpan	DAY;
	static const CTimeSpan	MONTH;
	static const CTimeSpan	YEAR;
	static const CTimeSpan	HOUR;
	static const CTimeSpan	MINUTE;
	static const CTimeSpan	SECOND;
	static const CTimeSpan	MILLISECOND;

	CTimeSpan(double spanInMilliseconds) { m_span = spanInMilliseconds; }

	double GetSpanInYears() const;
	double GetSpanInMonths() const;
	double GetSpanInDays() const;
	double GetSpanInHours() const;
	double GetSpanInMinutes() const;
	double GetSpanInSeconds() const;
	double GetSpanInMilliseconds() const;

	int GetSpanYears() const;
	int GetSpanMonths() const;
	int GetSpanDays() const;
	int GetSpanHours() const;
	int GetSpanMinutes() const;
	int GetSpanSeconds() const;
	int GetSpanMilliseconds() const;

	bool operator <(const CTimeSpan& ts) const;
	CTimeSpan operator *(double multiplier) const;
	CTimeSpan operator +(const CTimeSpan& rhs) const;
	CTimeSpan operator -() const;

	std::string ToString() const;
};

inline CTimeSpan operator * (double multiplier, const CTimeSpan& ts) {
	return CTimeSpan(multiplier * ts.GetSpanInMilliseconds());
}	

class CDateTimeException : public std::exception
{
public:
    CDateTimeException() throw()									{ }
	CDateTimeException(const char* sz) throw()						{ m_message = sz; }
    CDateTimeException(const CDateTimeException& rhs) throw()		{ m_message = rhs.m_message; }
    CDateTimeException& operator=(const CDateTimeException& rhs) throw() { m_message = rhs.m_message; return *this; }
    virtual ~CDateTimeException() throw()							{ }

    virtual const char *what() const throw()						{ return m_message.c_str(); };
protected:
	std::string	m_message;
};


/**
A class to represent a date and time.
*/
class CDateTime  
{
protected:
	int m_year;
	int m_month;
	int m_day;
	int m_hour;
	int m_minute;
	int m_second;
	int m_millisecond;

private:
	void init();
	void initialize(int year, int month, int day, int hour, int minute, int second, int millisecond) throw (CDateTimeException);
public:
	enum DateFormat { SQL_FORMAT, YYYYMMDD, YYMMDD, DDMMYY, MMDDYY, MMDDYYYY};

	CDateTime(time_t t);
	CDateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0, int millisecond = 0) throw (CDateTimeException);
	CDateTime(const char* szDate, DateFormat format) throw (CDateTimeException);
	CDateTime();

	static CDateTime GetCurrentDate();

	CTimeSpan operator -(const CDateTime& dt) const;
	CDateTime operator +(const CTimeSpan& span) const;

	bool operator <(const CDateTime& dt) const;

	std::string ToSQLString() const;
	std::string ToDateString() const;
	operator time_t() const;

	int GetMilliSecond() const;
	int GetSecond() const;
	int GetMinute() const;
	int GetHour() const;
	int GetDay() const;
	int GetMonth() const;
	int GetYear() const;
};

#endif

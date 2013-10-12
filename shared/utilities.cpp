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




This software implements a C++ utility classes.
*/
#pragma warning(disable: 4786)

#include "dbg_alloc.h"
#ifdef	__USE_DBG_ALLOC
#define new DEBUG_NEW
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "utilities.h"

#ifdef _WIN32
tm* localtime_r(time_t* t, tm* buf)
{
	*buf = *localtime(t);
	return buf;
}
#endif

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// CUtilities Class
//////////////////////////////////////////////////////////////////////

int CUtilities::atoi_SubString(const char* sz, size_t len)
{
	char tmp[16];
	::memset(tmp,0,sizeof(tmp));
	::strncpy(tmp, sz, (len<sizeof(tmp)-1)?len:sizeof(tmp)-1);
	return ::atoi(tmp);
}

double CUtilities::Round_Double(double d, int decimal_digits)
{
	double m = 1;
	for (int x=0; x<decimal_digits; x++) m *= 10.0;
	return ::floor((d*m)+0.5)/m;
}

std::string CUtilities::trim(const std::string& src) {
	std::string::size_type b = src.find_first_not_of(" \t\r\n");
	std::string::size_type e = src.find_last_not_of(" \t\r\n");
	if (b < e && b != std::string::npos)
		return src.substr(b, e-b+1);
	return "";
}

std::string CUtilities::tolower(const std::string& src)
{
	std::string	ret;
	for(std::string::const_iterator i=src.begin(); i!=src.end(); i++)
		if ( *i >= 'A' && *i <= 'Z')
			ret += (char)('a' + (*i - 'A'));
		else
			ret += (char)*i;
	return ret;
}

std::string CUtilities::toupper(const std::string& src)
{
	std::string	ret;
	for(std::string::const_iterator i=src.begin(); i!=src.end(); i++)
		if ( *i >= 'a' && *i <= 'z')
			ret += (char)('A' + (*i - 'a'));
		else
			ret += (char)*i;
	return ret;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// CTimeSpan Class
//////////////////////////////////////////////////////////////////////
const CTimeSpan	CTimeSpan::DAY(24.0*60.0*60.0*1000.0);
const CTimeSpan	CTimeSpan::MONTH(365.25/12.0*24.0*60.0*60.0*1000.0);
const CTimeSpan	CTimeSpan::YEAR(365.25*24.0*60.0*60.0*1000.0);
const CTimeSpan	CTimeSpan::HOUR(60.0*60.0*1000.0);
const CTimeSpan	CTimeSpan::MINUTE(60.0*1000.0);
const CTimeSpan	CTimeSpan::SECOND(1000.0);
const CTimeSpan	CTimeSpan::MILLISECOND(1.0);


double CTimeSpan::GetSpanInYears () const
{
	return m_span / (365.25*24.0*60.0*60.0*1000.0);
}

double CTimeSpan::GetSpanInMonths () const
{
	return m_span / (356.25/12*24.0*60.0*60.0*1000.0);
}

double CTimeSpan::GetSpanInDays () const
{
	return m_span / (24.0*60.0*60.0*1000.0);
}

double CTimeSpan::GetSpanInHours () const
{
	return m_span / (60.0*60.0*1000.0);
}

double CTimeSpan::GetSpanInMinutes () const
{
	return m_span / (60.0*1000.0);
}

double CTimeSpan::GetSpanInSeconds () const
{
	return m_span / (1000.0);
}

double CTimeSpan::GetSpanInMilliseconds () const
{
	return m_span;
}

int CTimeSpan::GetSpanYears () const
{
	return (int) (m_span / (356.25*24.0*60.0*60.0*1000.0));
}

/**
Note: a Span Month = 365.25/12.0 == 30.4375 days long
*/
int CTimeSpan::GetSpanMonths () const
{
	double months = m_span - GetSpanYears()*(356.25*24.0*60.0*60.0*1000.0);
	months /= (356.25/12*24.0*60.0*60.0*1000.0);
	return (int)months;
}

int CTimeSpan::GetSpanDays () const
{
	double days = m_span - GetSpanYears()*(356.25*24.0*60.0*60.0*1000.0);
	days  -= GetSpanMonths()*(356.25/12*24.0*60.0*60.0*1000.0);
	days /= (24.0*60.0*60.0*1000.0);
	return (int)days;
}

int CTimeSpan::GetSpanHours () const
{
	double hours = m_span - GetSpanYears()*(356.25*24.0*60.0*60.0*1000.0);
	hours  -= GetSpanMonths()*(356.25/12*24.0*60.0*60.0*1000.0);
	hours -= GetSpanDays()*(24.0*60.0*60.0*1000.0);
	hours /= (60.0*60.0*1000.0);
	return (int)hours;
}

int CTimeSpan::GetSpanMinutes () const
{
	double minutes = m_span - GetSpanYears()*(356.25*24.0*60.0*60.0*1000.0);
	minutes  -= GetSpanMonths()*(356.25/12*24.0*60.0*60.0*1000.0);
	minutes -= GetSpanDays()*(24.0*60.0*60.0*1000.0);
	minutes -= GetSpanHours()*(60.0*60.0*1000.0);
	minutes /= (60.0*1000.0);
	return (int)minutes;
}

int CTimeSpan::GetSpanSeconds () const
{
	double seconds = m_span - GetSpanYears()*(356.25*24.0*60.0*60.0*1000.0);
	seconds  -= GetSpanMonths()*(356.25/12*24.0*60.0*60.0*1000.0);
	seconds -= GetSpanDays()*(24.0*60.0*60.0*1000.0);
	seconds -= GetSpanHours()*(60.0*60.0*1000.0);
	seconds -= GetSpanMinutes()*(60.0*1000.0);
	seconds /= (1000.0);
	return (int)seconds;
}

int CTimeSpan::GetSpanMilliseconds () const
{
	double millisec = m_span - GetSpanYears()*(356.25*24.0*60.0*60.0*1000.0);
	millisec  -= GetSpanMonths()*(356.25/12*24.0*60.0*60.0*1000.0);
	millisec -= GetSpanDays()*(24.0*60.0*60.0*1000.0);
	millisec -= GetSpanHours()*(60.0*60.0*1000.0);
	millisec -= GetSpanMinutes()*(60.0*1000.0);
	millisec -= GetSpanSeconds()*(1000.0);
	return (int)millisec;
}

bool CTimeSpan::operator <(const CTimeSpan& ts) const
{
	return m_span < ts.m_span;
}

CTimeSpan CTimeSpan::operator *(double multiplier) const
{
	return CTimeSpan(m_span * multiplier);
}

CTimeSpan CTimeSpan::operator +(const CTimeSpan& rhs) const
{
	return CTimeSpan(m_span + rhs.m_span);
}

CTimeSpan CTimeSpan::operator -() const
{
	return CTimeSpan(-m_span);
}

std::string CTimeSpan::ToString () const
{
	char	tmp[128];
	sprintf(tmp,"y:%d, m:%d, d:%d, h:%d, M:%d, S:%d, U:%d", GetSpanYears(), GetSpanMonths(), GetSpanDays(), 
		GetSpanHours(), GetSpanMinutes(), GetSpanSeconds(), GetSpanMilliseconds());
	return tmp;
}

//////////////////////////////////////////////////////////////////////
// CDateTime Class
//////////////////////////////////////////////////////////////////////

void CDateTime::init ()
{
	m_year = m_month = m_day = m_hour = m_minute = m_second = m_millisecond = -1;
}

CDateTime::CDateTime ()
{
	init();
}

CDateTime::CDateTime (time_t t)
{
	struct tm tmp_tm = *localtime(&t);
	
	init();
	m_year = tmp_tm.tm_year + 1900;
	m_month = tmp_tm.tm_mon + 1;
	m_day = tmp_tm.tm_mday;
	m_hour = tmp_tm.tm_hour;
	m_minute = tmp_tm.tm_min;
	m_second = tmp_tm.tm_sec;
	m_millisecond = 0;
}

/**
Parses the string based on the parameter <code>format</code> and
initilizes the date string.
*/
CDateTime::CDateTime (const char* szDate, CDateTime::DateFormat format) throw (CDateTimeException)
{
	init();
	int len = strlen(szDate);
	int year, month, day, hour, minute, second, millisecond;

	year = month = day = hour = minute = second = millisecond = 0;

	switch (format) {
		case SQL_FORMAT:
			sscanf(szDate, "%d-%d-%d %d:%d:%d.%d", &year, &month, &day, &hour, &minute, &second, &millisecond);
			if (year >= 10 && year < 100) 		year += 1900;
			else if (year < 10)					year += 2000;
			break;
		case YYYYMMDD:
			if (len >= 8) {
				sscanf(szDate,"%4d%2d%2d", &year, &month, &day);
			}
			break;
		case YYMMDD:
			if (len >= 8) {
				sscanf(szDate,"%2d%2d%2d", &year, &month, &day);
				if (year > 10) 	year += 1900;
				else			year += 2000;
			}
			break;
		case DDMMYY:
			if (len >= 8) {
				sscanf(szDate,"%2d%2d%2d", &day, &month, &year);
				if (year > 10) 	year += 1900;
				else			year += 2000;
			}
			break;
		case MMDDYY:
			if (len >= 8) {
				sscanf(szDate,"%2d%2d%2d", &month, &day, &year);
				if (year > 10) 	year += 1900;
				else			year += 2000;
			}
			break;
		case MMDDYYYY:
			if (len >= 8) {
				sscanf(szDate,"%2d%2d%4d", &month, &day, &year);
			}
			break;
	}

	initialize(year, month, day, hour, minute, second, millisecond);
}

CDateTime::CDateTime (int year, int month, int day, int hour, int minute, int second, int millisecond) throw (CDateTimeException)
{
	initialize(year, month, day, hour, minute, second, millisecond);
}

void CDateTime::initialize(int year, int month, int day, int hour, int minute, int second, int millisecond) throw (CDateTimeException)
{
	m_year = year;
	m_month = month;
	m_day = day;
	m_hour = hour;
	m_minute = minute;
	m_second = second;
	m_millisecond = millisecond;

	if (m_year < 0)
		throw CDateTimeException("Invalid Year");

	if (m_month < 1 || m_month > 12)
		throw CDateTimeException("Invalid Month");

	if (m_day < 1 
			|| ( (m_month==1 || m_month==3 || m_month==5 || m_month==7 || m_month==8 || m_month==10 || m_month==12) && m_day > 31)
			|| ( (m_month==4 || m_month==6 || m_month==9 || m_month==11) && m_day > 30)
			|| ( (m_month==2) && (((m_year%4==0)&& m_day>29) || ((m_year%4!=0)&&m_day>28)) ) )
		throw CDateTimeException(("Invalid Day (" +CInteger::ToString(m_year)+"-"+CInteger::ToString(m_month)+"-"+CInteger::ToString(m_day)+")").c_str());

	if (m_hour < 0 || m_hour > 23)
		throw CDateTimeException("Invalid Hour");

	if (m_minute < 0 || m_minute > 59)
		throw CDateTimeException("Invalid Minute");

	if (m_second < 0 || m_second > 59)
		throw CDateTimeException("Invalid Second");

	if (m_millisecond < 0 || m_millisecond > 999)
		throw CDateTimeException("Invalid Millisecond");
}


CDateTime CDateTime::GetCurrentDate ()
{
	time_t	t = time(NULL);
	struct tm	now;

	localtime_r(&t,&now);

	return CDateTime(now.tm_year+1900, now.tm_mon+1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
}


CTimeSpan CDateTime::operator -(const CDateTime& dt) const
{
	double rhs = (double)m_millisecond/1000.0 + (time_t)*this;
	double lhs = (double)dt.m_millisecond/1000.0 + (time_t)dt;
	return CTimeSpan((rhs-lhs)*1000);
}

CDateTime CDateTime::operator +(const CTimeSpan& span) const
{
	int		milliseconds = span.GetSpanMilliseconds() + m_millisecond;
	time_t	seconds = (time_t)span.GetSpanInSeconds() + (time_t)*this;

	if (milliseconds > 1000) {
		seconds += 1;
		milliseconds %= 1000;
	} else if (milliseconds < 0) {
		seconds -= 1;
		milliseconds *= -1;
		milliseconds %= 100;
	}
	CDateTime	t((time_t)seconds);
	t.m_millisecond = milliseconds;
	return t;
}

bool CDateTime::operator <(const CDateTime& dt) const
{
	double rhs = (double)m_millisecond/1000.0 + (time_t)*this;
	double lhs = (double)dt.m_millisecond/1000.0 + (time_t)dt;
	return rhs < lhs;
}

std::string CDateTime::ToDateString () const
{
	char tmp[32];
	
	sprintf(tmp, "%04d-%02d-%02d", m_year, m_month, m_day);

	return tmp;
}

/**
Converts the data an SQL formatted string
*/
std::string CDateTime::ToSQLString () const
{
	char tmp[32];
	
	sprintf(tmp, "%04d-%02d-%02d %02d:%02d:%02d.%03d", m_year, m_month, m_day, m_hour, m_minute, m_second, m_millisecond);

	return tmp;
}

CDateTime::operator time_t() const
{
	struct tm tmp_tm;
	memset(&tmp_tm, 0, sizeof(tmp_tm));

    tmp_tm.tm_sec = m_second;     /* seconds after the minute - [0,59] */
    tmp_tm.tm_min = m_minute;     /* minutes after the hour - [0,59] */
    tmp_tm.tm_hour = m_hour;    /* hours since midnight - [0,23] */
    tmp_tm.tm_mday = m_day;    /* day of the month - [1,31] */
    tmp_tm.tm_mon = m_month - 1;     /* months since January - [0,11] */
    tmp_tm.tm_year = m_year - 1900;    /* years since 1900 */
    tmp_tm.tm_isdst = -1;   /* daylight savings time flag */
	return mktime(&tmp_tm);
}

int CDateTime::GetYear () const 
{
	return m_year;
}

int CDateTime::GetMonth () const 
{
	return m_month;
}

int CDateTime::GetDay () const 
{
	return m_day;
}

int CDateTime::GetHour () const 
{
	return m_hour;
}

int CDateTime::GetMinute () const 
{
	return m_minute;
}

int CDateTime::GetSecond () const 
{
	return m_second;
}

int CDateTime::GetMilliSecond () const 
{
	return m_millisecond;
}

#ifndef _CERTUS_UTIL_IO__H_
#define _CERTUS_UTIL_IO__H_

#include <iostream>
#include <boost/lexical_cast.hpp>
#include "pystring.h"
#include "sys.h"


/*
 * Various type printing utility functions
 */

namespace certus { namespace util {

	const unsigned int g_max_chars = 80;


	/*
	 * Return the string representation of an object
	 */
	template<typename T>
	std::string to_str(const T& t)
	{
		return boost::lexical_cast<std::string>(t);
	}


	/*
	 * Print a sequence
	 */
	template<typename T>
	struct seq_short_printer
	{
		seq_short_printer(const T& t):m_t(t){}
		const T& m_t;
	};


//////////////// impl

template<typename T>
std::ostream& operator<<(std::ostream& os, const seq_short_printer<T>& p)
{
	std::vector<std::string> strs;
	unsigned int nchars = 0;

	for(typename T::const_iterator it=p.m_t.begin(); it!=p.m_t.end(); ++it)
	{
		std::string s("'");
		s += boost::lexical_cast<std::string>(*it);
		s += "'";
		strs.insert(strs.end(), s);
		nchars += s.size() + 2;
	}

	while((strs.size() > 2) && (nchars > g_max_chars))
	{
		std::vector<std::string>::iterator it = strs.end(); --it; --it;
		nchars -= (it->size() + 2);
		if((strs.size() == 3) || (nchars <= g_max_chars))
			*it = "...";
		else
			strs.erase(it);
	}

	std::string s = pystring::join(", ", strs);
	os << '[' << s << ']';

	return os;
}

} }

#endif






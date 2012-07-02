#ifndef _CERTUS_VER_TOKEN_UNSIGNED__H_
#define _CERTUS_VER_TOKEN_UNSIGNED__H_

#include <iostream>
#include <sstream>
#include <string>
#include <limits>
#include <boost/operators.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include "exceptions.h"


namespace certus { namespace ver {
    
    // fwd decls
    template<typename T> class ver_token_unsigned;
    
    template<typename T>
    std::ostream& operator<<(std::ostream&, const ver_token_unsigned<T>&);
    
	
	/*
	 * Convenience class which represents version number tokens as an unsigned number. Only basic
	 * numbers are supported (eg 1, 12), no scientific notation or zero-padded (eg 01).
	 */
	template<typename T>
	class ver_token_unsigned
	:	boost::less_than_comparable<ver_token_unsigned<T>,
	 	boost::equality_comparable<ver_token_unsigned<T>
		> >
	{
	public:
		ver_token_unsigned(T n):m_n(n){}
		ver_token_unsigned(const std::string& s);
		
		bool operator<(const ver_token_unsigned& rhs) const		{ return (m_n<rhs.m_n); }
		bool operator==(const ver_token_unsigned& rhs) const	{ return (m_n==rhs.m_n); }
		
		T value() const											{ return m_n; }
		ver_token_unsigned get_next() const						{ return ver_token_unsigned(m_n+T(1)); }
		static ver_token_unsigned get_min()						{ return ver_token_unsigned(T(0)); }
		static ver_token_unsigned get_max()						{ return ver_token_unsigned(std::numeric_limits<T>::max()); }
		
		friend std::ostream& operator<< <T>(std::ostream& s, const ver_token_unsigned<T>& v);
	
	protected:
		T m_n;
	};

	
//////////////// impl

template<typename T>
ver_token_unsigned<T>::ver_token_unsigned(const std::string& s)
{
	{
		std::istringstream strm(s);
		strm >> m_n;
		if(strm.fail())
			throw invalid_version_error(s);
	}

	{
		std::ostringstream strm;
		strm << m_n;
		if(strm.str() != s)
			throw invalid_version_error(s);
	}
}

	
template<typename T>
std::ostream& operator<<(std::ostream& s, const ver_token_unsigned<T>& v)
{
	s << v.m_n;
	return s;
}
	
		
} }

#endif


















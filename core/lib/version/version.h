#ifndef _CERTUS_VER_VERSION__H_
#define _CERTUS_VER_VERSION__H_

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <cassert>
#include <boost/operators.hpp>
#include "exceptions.h"
#include "pystring.h"

namespace certus { namespace ver {
    
    // fwd decls
    template<typename Token> class version;
    template<typename Token> class version_range;
    
    template<typename Token>
    std::ostream& operator<<(std::ostream&, const version<Token>&);
    
	
	/*
	 * A set of one or more tokens, delimited by dots.
	 * Eg: '1', '1.5'. The null version is denoted as '.'.
	 */
	template<typename Token>
	class version
	:	boost::less_than_comparable<version<Token>,
	 	boost::equality_comparable<version<Token>
		> >
	{
	public:
		version(){}
		version(const std::string& s) { set(s); }
		version(const Token& t);
		
		void set(const std::string& s);
		inline void set_none() 			{ m_tokens.clear(); }
		
		inline bool is_none() const 	{ return m_tokens.empty(); }
        
        std::size_t rank() const 						{ return m_tokens.size(); }
        const Token& operator[](std::size_t i) const 	{ assert(i<rank()); return m_tokens[i]; }
        Token& operator[](std::size_t i) 				{ assert(i<rank()); return m_tokens[i]; }
		bool operator<(const version& rhs) const 		{ return (m_tokens < rhs.m_tokens); }
		bool operator==(const version& rhs) const		{ return (m_tokens == rhs.m_tokens); }
		
		version get_next() const;
		version get_nearest() const;
        
		friend std::ostream& operator<< <Token>(std::ostream& s, const version& v);
		friend class version_range<Token>;
		
	protected:
		std::vector<Token> m_tokens;
	};
	
	
//////////////// impl

template<typename Token>
version<Token>::version(const Token& t)
:	m_tokens(1, t)
{
}


template<typename Token>
void version<Token>::set(const std::string& s)
{
	m_tokens.clear();
	
	if(s.empty()) // special case - the 'none' version
		return;

	std::vector<std::string> toks;
	::certus::pystring::split(s, toks, ".");
    std::size_t i = 0;
	
    try
    {
        for(; i<toks.size(); ++i)
            m_tokens.push_back(Token(toks[i]));
    }
    catch(const invalid_version_error& e)
	{
		std::ostringstream strm;
		strm << "\'" << s << "\' (token #" << i << " invalid: \'"
			<< e.what() << "\')";
        throw invalid_version_error(strm.str());
    }
}

	
template<typename Token>
version<Token> version<Token>::get_next() const
{
	if(is_none())
		return get_nearest();
	
	version v(*this);
	Token& tok = v.m_tokens.back();
	tok = tok.get_next();
	return v;
}


template<typename Token>
version<Token> version<Token>::get_nearest() const
{
	version v(*this);
	v.m_tokens.push_back(Token::get_min());
	return v;
}
	

template<typename Token>
std::ostream& operator<<(std::ostream& s, const version<Token>& v)
{
	if(!v.m_tokens.empty())
	{
		typename std::vector<Token>::const_iterator it = v.m_tokens.begin();
		s << *it;
		for(++it; it!=v.m_tokens.end(); ++it)
			s << '.' << *it;
	}
	
	return s;
}

} }

#endif
























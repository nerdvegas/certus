#ifndef _CERTUS_VER_TOKEN_ALPHANUMERIC__H_
#define _CERTUS_VER_TOKEN_ALPHANUMERIC__H_

#include <iostream>
#include <sstream>
#include <string>
#include <limits>
#include "exceptions.h"
#include "ver_token_unsigned.h"


namespace certus { namespace ver {
    
    // fwd decls
    template<typename T> class ver_token_alphanumeric;
    
    template<typename T>
    std::ostream& operator<<(std::ostream&, const ver_token_alphanumeric<T>&);
    
	
	/*
	 * Convenience class which represents version number tokens as alphanumerics. Tokens can 
	 * contain any combination of numbers, and letters a-z,_. If you consider a token as a
	 * list of numbers and words, then tokens are sorted lexicographically by this list, with 
	 * the following values in ascending order: 
	 * '_', 'a', ..., 'zzzzz...z', 0, 1, ..., max(T)
	 */
	template<typename T>
	class ver_token_alphanumeric
	{
	protected:
	
		struct subtok
		{
			subtok(T n):m_num(n){}
			subtok(const std::string& s):m_num(0),m_str(s){}
			bool is_num() const { return m_str.empty(); }
			bool is_str() const { return !is_num(); }
			bool operator<(const subtok& rhs) const;
			bool operator==(const subtok& rhs) const {return (m_num==rhs.m_num)&&(m_str==rhs.m_str); }
			void write(std::ostream& s) const { if(is_num()) s<<m_num; else s<<m_str; }
			
			T m_num;
			std::string m_str;
		};
	
	public:
		ver_token_alphanumeric(const std::string& s);
		
		bool operator<(const ver_token_alphanumeric& rhs) const		{ return (m_subtoks<rhs.m_subtoks); }
		bool operator==(const ver_token_alphanumeric& rhs) const	{ return (m_subtoks==rhs.m_subtoks); }
		bool operator!=(const ver_token_alphanumeric& rhs) const	{ return (m_subtoks!=rhs.m_subtoks); }		
		
		ver_token_alphanumeric get_next() const;
		static ver_token_alphanumeric get_min()			{ return ver_token_alphanumeric(subtok("_")); }
		static ver_token_alphanumeric get_max()			{ return ver_token_alphanumeric(subtok(std::numeric_limits<T>::max())); }
		
		friend std::ostream& operator<< <T>(std::ostream& s, const ver_token_alphanumeric<T>& v);
	
	protected:
		ver_token_alphanumeric(){}
		ver_token_alphanumeric(const subtok& t):m_subtoks(1, t){}
		
	protected:
		std::vector<subtok> m_subtoks;
	};

	
//////////////// impl

template<typename T>
bool ver_token_alphanumeric<T>::subtok::operator<(const subtok& rhs) const
{
	if(is_str())
		return rhs.is_num() || (m_str < rhs.m_str);
	else
		return rhs.is_num() && (m_num < rhs.m_num);
}


template<typename T>
ver_token_alphanumeric<T>::ver_token_alphanumeric(const std::string& s)
{
	if(s.empty())
		throw invalid_version_error(s);
	
	std::string str_num, str_str;
	for(std::size_t i=0; i<s.length(); ++i)
	{
		char ch = s[i];
		if(((ch>='a') && (ch<='z')) || (ch=='_'))
		{
			if(!str_num.empty())
			{
				ver_token_unsigned<T> ntok(str_num);
				m_subtoks.push_back(ntok.value());
				str_num.clear();
			}
			str_str.push_back(ch);
		}
		else if((ch>='0') && (ch<='9'))
		{
			if(!str_str.empty())
			{
				m_subtoks.push_back(str_str);
				str_str.clear();
			}
			str_num.push_back(ch);
		}
		else
			throw invalid_version_error(s);
	}
	
	if(!str_num.empty())
	{
		ver_token_unsigned<T> ntok(str_num);
		m_subtoks.push_back(ntok.value());
	}
	else if(!str_str.empty())
		m_subtoks.push_back(str_str);
}


template<typename T>
ver_token_alphanumeric<T> ver_token_alphanumeric<T>::get_next() const
{
	ver_token_alphanumeric v;
	v.m_subtoks = m_subtoks;
	subtok& t = v.m_subtoks.back();
	if(t.is_num())
		t.m_num++;
	else
		t.m_str.push_back('_');
	return v;
}

	
template<typename T>
std::ostream& operator<<(std::ostream& s, const ver_token_alphanumeric<T>& v)
{
	typedef typename ver_token_alphanumeric<T>::subtok _subtok;
	for(typename std::vector<_subtok>::const_iterator it=v.m_subtoks.begin(); it!=v.m_subtoks.end(); ++it)
		it->write(s);
	return s;
}
	
		
} }

#endif


















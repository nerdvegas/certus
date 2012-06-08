#ifndef _CERTUS_VER_VERSIONRANGE__H_
#define _CERTUS_VER_VERSIONRANGE__H_

#include "version.h"


namespace certus { namespace ver {

	// fwd decls
	template<typename Token> class version_range;
	
    template<typename Token>
    std::ostream& operator<<(std::ostream&, const version_range<Token>&);
	
    
    /*
	 * A continuous range of version numbers.
	 * Eg: '', '1.5+<2', '7.0+', '1.2.'
	 */
	template<typename Token>
	class version_range
	{
	public:
		typedef version<Token> ver_type;
		
		version_range() { set_none(); }
		version_range(const std::string& s) { set(s); }
		version_range(const ver_type& v, bool exact=true);
		version_range(const ver_type& ge, const ver_type& lt);

		void set(const std::string& s);
		void set_any();
		void set_none();
        
		bool operator<(const version_range& rhs) const;
		bool operator==(const version_range& rhs) const		{ return (m_ge==rhs.m_ge)&&(m_lt==rhs.m_lt); }
		bool operator!=(const version_range& rhs) const		{ return (m_ge!=rhs.m_ge)||(m_lt!=rhs.m_lt); }
		
		inline bool is_any() const			{ return (m_ge.is_none() && (m_lt[0]==Token::get_max())); }
		inline bool is_none() const			{ return (m_ge.is_none() && (m_lt.rank()==1) && (m_lt[0]==Token::get_min())); }
		inline const ver_type& ge() const	{ return m_ge; }
		inline const ver_type& lt() const	{ return m_lt; }
		
		bool touches(const version_range& rhs) const;
		bool union_with(const version_range& rhs);
		
		bool intersects(const version_range& rhs) const;
		bool intersect_with(const version_range& rhs);
		
		friend std::ostream& operator<< <Token>(std::ostream& s, const version_range& v);
		
	protected:
		ver_type m_ge;
		ver_type m_lt;
	};


//////////////// impl

template<typename Token>
version_range<Token>::version_range(const version<Token>& v, bool exact)
:	m_ge(v),
    m_lt(exact? v.get_nearest() : v.get_next())
{
}


template<typename Token>
version_range<Token>::version_range(const version<Token>& ge, const version<Token>& lt)
:	m_ge(ge),
	m_lt(lt)
{
	if(m_ge >= m_lt)
	{
		std::ostringstream strm;
		strm << "\'" << m_ge << "+<" << m_lt << "\' (leading version >= trailing version)";
		throw invalid_version_error(strm.str());
	}
}


template<typename Token>
void version_range<Token>::set(const std::string& s)
{
    if(s.empty())
		set_any();
	else if(s == ".")
		set_none();
	else
	{
		std::vector<std::string> vers;
		::certus::pystring::split(s, vers, "+", 1);
		
		std::size_t nch = vers[0].size();
		if((vers.size()==1) && (nch>0) && (vers[0][nch-1]=='.'))
		{
			// 'exact' range, eg '1.2.'
			std::ostringstream os;
			os << '<' << vers[0] << Token::get_min();
			vers[0].resize(nch-1);
			vers.push_back(os.str());
		}
		
		try {
			m_ge.set(vers[0]);
		}
		catch(const invalid_version_error& e)
		{
			std::ostringstream strm;
			strm << "\'" << s << "\' (invalid leading version: "
				<< e.what() << ")";
			throw invalid_version_error(strm.str());
		}
		
		if(vers.size()==1)
			m_lt = m_ge.get_next();
		else
		{
			if(vers[1].empty())
				m_lt = ver_type(Token::get_max());
			else
			{
				if(vers[1][0] != '<')
					throw invalid_version_error(s);
				else
				{
					try {
						m_lt.set(vers[1].c_str()+1);
					}
					catch (const invalid_version_error& e)
					{
						std::ostringstream strm;
						strm << "\'" << s << "\' (invalid trailing version: "
							<< e.what() << ")";
						throw invalid_version_error(strm.str());
					}
					
					if(!(m_lt > m_ge))
					{
						std::ostringstream strm;
						strm << "\'" << s << "\' (leading version >= trailing version)";
						throw invalid_version_error(strm.str());
					}
				}
			}
		}
	}
}


template<typename Token>
void version_range<Token>::set_none()
{
	m_ge.set_none();
	m_lt = m_ge.get_nearest();
}


template<typename Token>
void version_range<Token>::set_any()
{
	m_ge.set_none();
	m_lt = ver_type(Token::get_max());
}

					   
template<typename Token>
bool version_range<Token>::operator<(const version_range& rhs) const
{
	return (m_ge < rhs.m_ge) ||
		((m_ge == rhs.m_ge) && (m_lt < rhs.m_lt));
}


template<typename Token>
bool version_range<Token>::intersects(const version_range& rhs) const
{
	return (*this == rhs) || ((rhs.m_ge < m_lt) && (rhs.m_lt > m_ge));
}


template<typename Token>
bool version_range<Token>::touches(const version_range& rhs) const
{
	return (rhs.m_ge <= m_lt) && (rhs.m_lt >= m_ge);
}


template<typename Token>
bool version_range<Token>::union_with(const version_range& rhs)
{
	if(!touches(rhs))
		return false;
	
	if(m_ge > rhs.m_ge)
		m_ge = rhs.m_ge;
	if(m_lt < rhs.m_lt)
		m_lt = rhs.m_lt;
	return true;
}


template<typename Token>
bool version_range<Token>::intersect_with(const version_range& rhs)
{
	if(!intersects(rhs))
		return false;
	
	if(m_ge < rhs.m_ge)
		m_ge = rhs.m_ge;
	if(m_lt > rhs.m_lt)
		m_lt = rhs.m_lt;
	return true;
}


template<typename Token>
std::ostream& operator<<(std::ostream& s, const version_range<Token>& v)
{
    if(!v.is_any())
	{
		s << v.m_ge;
		
		if(v.m_lt == v.m_ge.get_nearest())
			s << '.';
		else if(v.m_lt != v.m_ge.get_next())
		{
			s << '+';
			if(v.m_lt[0] != Token::get_max())
				s << '<' << v.m_lt;
		}
	}
	
	return s;
}

} }

#endif
























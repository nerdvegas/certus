#ifndef _CERTUS_VER_MULTIVERSIONRANGE__H_
#define _CERTUS_VER_MULTIVERSIONRANGE__H_

#include "version_range.h"
#include <set>
#include <cassert>


namespace certus { namespace ver {

	// fwd decls
	template<typename Token> class multi_version_range;
	
    template<typename Token>
    std::ostream& operator<<(std::ostream&, const multi_version_range<Token>&);
	
    
    /*
	 * Zero or more non-overlapping ranges of version numbers.
	 * Eg: '1|5', '1.5+<2|5.8'
	 * The 'Any' range intersects with all versions.
	 * The 'None' range intersects with only the 'None' version.
	 * The 'Empty' range intersects with no versions (do not confuse the None and Empty ranges).
	 */
	template<typename Token>
	class multi_version_range
	{
	public:
		typedef version<Token> 												ver_type;
		typedef version_range<Token> 										ver_range_type;
		typedef typename std::set<ver_range_type>::iterator					iterator;
		typedef typename std::set<ver_range_type>::const_iterator			const_iterator;
		typedef typename std::set<ver_range_type>::reverse_iterator			reverse_iterator;
		typedef typename std::set<ver_range_type>::const_reverse_iterator	const_reverse_iterator;
		
		multi_version_range(){} // constructs the 'Empty' range
		explicit multi_version_range(const std::string& s) 	{ set(s); }
		explicit multi_version_range(const ver_range_type& v);
		
		bool operator==(const multi_version_range& rhs) const	{ return (m_ranges==rhs.m_ranges); }
		bool operator!=(const multi_version_range& rhs) const	{ return (m_ranges!=rhs.m_ranges); }

		void set(const std::string& s);
		void set_any();
		void set_none();
		void set_empty()					{ m_ranges.clear(); }
		
		std::size_t num_ranges() const		{ return m_ranges.size(); }
		bool is_empty() const				{ return m_ranges.empty(); }
		bool is_any() const					{ return (m_ranges.size()==1) && m_ranges.begin()->is_any(); }
		bool is_none() const				{ return (m_ranges.size()==1) && m_ranges.begin()->is_none(); }
		inline bool is_exact() const		{ return (m_ranges.size()==1) && m_ranges.begin()->is_exact(); }

		iterator begin() 						{ return m_ranges.begin(); }
		iterator end()							{ return m_ranges.end(); }
		reverse_iterator rbegin() 				{ return m_ranges.rbegin(); }
		reverse_iterator rend()					{ return m_ranges.rend(); }

		const_iterator begin() const 			{ return m_ranges.begin(); }
		const_iterator end() const				{ return m_ranges.end(); }
		const_reverse_iterator rbegin() const 	{ return m_ranges.rbegin(); }
		const_reverse_iterator rend() const		{ return m_ranges.rend(); }

		void erase(iterator it)				{ m_ranges.erase(it); }
		bool get_exact_version(ver_type& v) const;

		bool is_superset(const ver_range_type& v) const;
		bool is_superset(const multi_version_range& v) const;

		template<typename T>
		bool is_subset(const T& v) const;

		void union_of(const ver_range_type& v, multi_version_range& result) const;
		void union_of(const multi_version_range& v, multi_version_range& result) const;

		void union_with(const ver_range_type& v);
		void union_with(const multi_version_range& v);
		
		bool intersects(const ver_range_type& v) const;
		bool intersects(const multi_version_range& v) const;
		
		bool intersection(const ver_range_type& v, multi_version_range& result) const;
		bool intersection(const multi_version_range& v, multi_version_range& result) const;

		bool discrete_intersects(const ver_range_type& v) const;
		bool discrete_intersects(const multi_version_range& v) const;

		bool discrete_intersection(const ver_range_type& v, multi_version_range& result) const;
		bool discrete_intersection(const multi_version_range& v, multi_version_range& result) const;

		bool intersect(const ver_range_type& v);
		bool intersect(const multi_version_range& v);

		void inverse(multi_version_range& result) const;
		void invert();
		        
		friend std::ostream& operator<< <Token>(std::ostream& s, const multi_version_range& v);
		
	protected:
	
		template<typename Iterator>
		bool get_overlap(const ver_range_type& v, bool include_touching,
			Iterator& it_begin, Iterator& it_end);
		
	public:
		static const std::string s_empty_str;
		
	protected:
		std::set<ver_range_type> m_ranges;
		
		typedef typename std::set<ver_range_type>::iterator 		it_type;
		typedef typename std::set<ver_range_type>::const_iterator 	c_it_type;
	};


//////////////// impl

template<typename Token>
const std::string multi_version_range<Token>::s_empty_str("(empty)");


template<typename Token>
multi_version_range<Token>::multi_version_range(const ver_range_type& v)
{
	m_ranges.insert(v);
}


template<typename Token>
void multi_version_range<Token>::set(const std::string& s)
{
	m_ranges.clear();
	if(s == s_empty_str)
		return;
	
	if(s.empty())
	{
		set_any();
		return;
	}
	
	std::vector<std::string> range_strs;
	::certus::pystring::split(s, range_strs, "|");
    std::size_t i = 0;

    try
    {
        for(; i<range_strs.size(); ++i)
		{
			ver_range_type vr(range_strs[i]);
			union_with(vr);
		}
    }
    catch(const invalid_version_error& e)
	{
		std::ostringstream strm;
		strm << "\'" << s << "\' (version_range #" << i << " invalid: \'"
			<< e.what() << "\')";
        throw invalid_version_error(strm.str());
    }
}


template<typename Token>
void multi_version_range<Token>::set_any()
{
	ver_range_type v;
	v.set_any();
	m_ranges.clear();
	m_ranges.insert(v);
}


template<typename Token>
void multi_version_range<Token>::set_none()
{
	ver_range_type v;
	v.set_none();
	m_ranges.clear();
	m_ranges.insert(v);
}


template<typename Token>
bool multi_version_range<Token>::get_exact_version(version<Token>& v) const
{
	if(!is_exact())
		return false;
	return m_ranges[0].get_exact_version(v);
}


template<typename Token>
bool multi_version_range<Token>::is_superset(const multi_version_range<Token>& v) const
{
	multi_version_range result;
	if(!intersection(v, result))
		return false;
	return (result == v);
}


template<typename Token>
bool multi_version_range<Token>::is_superset(const version_range<Token>& v) const
{
	multi_version_range result;
	if(!intersection(v, result))
		return false;
	return ((result.m_ranges.size()==1) && (*(result.m_ranges.begin()) == v));
}


template<typename Token>
template<typename T>
bool multi_version_range<Token>::is_subset(const T& v) const
{
	multi_version_range result;
	if(!intersection(v, result))
		return false;
	return (result == *this);
}


template<typename Token>
template<typename Iterator>
bool multi_version_range<Token>::get_overlap(const version_range<Token>& v, 
	bool include_touching, Iterator& it_begin, Iterator& it_end)
{
	if(m_ranges.empty())
		return false;
	
	Iterator ibegin = m_ranges.begin();
	Iterator iend = m_ranges.end();
	it_begin = iend; --it_begin;
	
	if(v.ge() < it_begin->ge())
	{
		it_begin = m_ranges.lower_bound(v);
		assert(it_begin != iend);

		// find first range past end of v, if any
		for(it_end=it_begin; (it_end != iend) && 
			((it_end->ge() < v.lt()) || (include_touching && (it_end->ge() == v.lt()))); ++it_end){}
		
		// find earliest overlapping range, if any
		if(it_end != ibegin)
		{
			if(it_begin == it_end)
			{
				--it_begin;
				if((v.ge() < it_begin->lt()) || (include_touching && (v.ge() == it_begin->lt())))
					return true;
			}
			else if(it_begin == ibegin)
				return true;
			else
			{
				Iterator it = it_begin; --it;
				if((v.ge() < it->lt()) || (include_touching && (v.ge() == it->lt())))
					it_begin = it;
				return true;
			}
		}
	}
	else
	{
		// v can only be overlapping with last range
		if((v.ge() < it_begin->lt()) || (include_touching && (v.ge() == it_begin->lt())))
		{
			it_end = iend;
			return true;
		}
	}
	
	return false;
}
			

template<typename Token>
void multi_version_range<Token>::union_with(const version_range<Token>& v)
{
	it_type it_begin, it_end;
	if(get_overlap(v, true, it_begin, it_end))
	{
		ver_range_type v_(v);
		v_.union_with(*it_begin);
		it_type it = it_end; --it;
		v_.union_with(*it);
		m_ranges.erase(it_begin, it_end);
		m_ranges.insert(v_);
	}
	else
		m_ranges.insert(v);
}


template<typename Token>
void multi_version_range<Token>::union_with(const multi_version_range<Token>& v)
{
	for(c_it_type it=v.m_ranges.begin(); it!=v.m_ranges.end(); ++it)
		union_with(*it);
}


template<typename Token>
void multi_version_range<Token>::union_of(const version_range<Token>& v,
	multi_version_range<Token>& result) const
{
	result = *this;
	result.union_with(v);
}


template<typename Token>
void multi_version_range<Token>::union_of(const multi_version_range<Token>& v,
	multi_version_range<Token>& result) const
{
	result = *this;
	result.union_with(v);
}


template<typename Token>
bool multi_version_range<Token>::intersects(const version_range<Token>& v) const
{
	c_it_type it, it_end;
	return const_cast<multi_version_range*>(this)->get_overlap(v, false, it, it_end);
}


template<typename Token>
bool multi_version_range<Token>::intersects(const multi_version_range<Token>& v) const
{
	for(c_it_type it=v.m_ranges.begin(); it!=v.m_ranges.end(); ++it)
	{
		if(intersects(*it))
			return true;
	}
	return false;
}


template<typename Token>
bool multi_version_range<Token>::intersection(const version_range<Token>& v,
	multi_version_range<Token>& result) const
{
	result.m_ranges.clear();
	
	c_it_type it, it_end;
	if(const_cast<multi_version_range*>(this)->get_overlap(v, false, it, it_end))
	{
		for(; it!=it_end; ++it)
		{
			ver_range_type v_(*it);
			v_.intersect(v);
			result.m_ranges.insert(result.m_ranges.end(), v_);
		}
		return true;
	}
	return false;
}


template<typename Token>
bool multi_version_range<Token>::intersection(const multi_version_range<Token>& v,
	multi_version_range<Token>& result) const
{
	if(v.num_ranges() > num_ranges())
	{
		// intersection is more optimal when v has less ranges than us
		return v.intersection(*this, result);
	}
	
	result.m_ranges.clear();
	bool b = false;
	
	for(c_it_type it=v.m_ranges.begin(); it!=v.m_ranges.end(); ++it)
	{
		multi_version_range r;
		b |= intersection(*it, r);
		result.m_ranges.insert(r.m_ranges.begin(), r.m_ranges.end());
	}
	
	return b;
}


template<typename Token>
bool multi_version_range<Token>::discrete_intersects(const ver_range_type& v) const
{
	c_it_type it, it_end;
	if(const_cast<multi_version_range*>(this)->get_overlap(v, false, it, it_end))
	{
		for(; it!=it_end; ++it)
		{
			if(it->is_subset(v))
				return true;
		}
	}
	return false;
}


template<typename Token>
bool multi_version_range<Token>::discrete_intersects(const multi_version_range& v) const
{
	for(c_it_type it=v.m_ranges.begin(); it!=v.m_ranges.end(); ++it)
	{
		if(discrete_intersects(*it))
			return true;
	}
	return false;
}


template<typename Token>
bool multi_version_range<Token>::discrete_intersection(const ver_range_type& v,
	multi_version_range& result) const
{
	result.m_ranges.clear();

	c_it_type it, it_end;
	if(const_cast<multi_version_range*>(this)->get_overlap(v, false, it, it_end))
	{
		for(; it!=it_end; ++it)
		{
			if(it->is_subset(v))
				result.m_ranges.insert(result.m_ranges.end(), *it);
		}
	}
	return !result.is_empty();
}


template<typename Token>
bool multi_version_range<Token>::discrete_intersection(const multi_version_range& v,
	multi_version_range& result) const
{
	result.m_ranges.clear();
	bool b = false;

	for(c_it_type it=v.m_ranges.begin(); it!=v.m_ranges.end(); ++it)
	{
		multi_version_range r;
		b |= discrete_intersection(*it, r);
		result.m_ranges.insert(r.m_ranges.begin(), r.m_ranges.end());
	}

	return b;
}


template<typename Token>
bool multi_version_range<Token>::intersect(const version_range<Token>& v)
{
	multi_version_range<Token> result;
	this->intersection(v, result);
	*this = result;
}


template<typename Token>
bool multi_version_range<Token>::intersect(const multi_version_range<Token>& v)
{
	multi_version_range<Token> result;
	this->intersection(v, result);
	*this = result;
}


template<typename Token>
void multi_version_range<Token>::inverse(multi_version_range<Token>& result) const
{
	if(is_empty())
	{
		result.set_any();
		return;
	}

	result.set_empty();
	if(is_any())
		return;
	
	c_it_type it = m_ranges.begin();
	if(!it->ge().is_none())
		result.m_ranges.insert(ver_range_type(ver_type(), it->ge()));
	
	while(it != m_ranges.end())
	{
		c_it_type it2=it; ++it2;
		if(it2 == m_ranges.end())
		{
			ver_type vmax(Token::get_max());
			if(it->lt() != vmax)
				result.m_ranges.insert(result.m_ranges.end(), ver_range_type(it->lt(), vmax));
		}
		else
			result.m_ranges.insert(result.m_ranges.end(), ver_range_type(it->lt(), it2->ge()));
		
		it = it2;
	}
}


template<typename Token>
void multi_version_range<Token>::invert()
{
	multi_version_range<Token> mvr;
	this->inverse(mvr);
	*this = mvr;
}


template<typename Token>
std::ostream& operator<<(std::ostream& s, const multi_version_range<Token>& v)
{
	if(v.m_ranges.empty())
	{
		s << multi_version_range<Token>::s_empty_str;
		return s;
	}

	typename std::set<version_range<Token> >::const_iterator it = v.m_ranges.begin();
	s << *it;
	for(++it; it!=v.m_ranges.end(); ++it)
		s << '|' << *it;
	
	return s;
}

} }

#endif
















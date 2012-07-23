#include "request_list.h"
#include "pystring.h"


namespace certus { namespace req {


request_list::request_list(const std::string& s)
{
	std::vector<std::string> req_strs;
	pystring::split(s, req_strs);

	for(unsigned int i=0; i<req_strs.size(); ++i)
		add(request(req_strs[i]));
}


request_list::const_iterator request_list::find(const std::string& name) const
{
	request_map::const_iterator it = m_requests_lookup.find(name);
	return (it == m_requests_lookup.end())?
		m_requests.end() : it->second;
}


void request_list::append(const request& r)
{
	requests_iterator it2 = m_requests.insert(m_requests.end(), r);
	m_requests_lookup.insert(request_map::value_type(r.name(), it2));
}


void request_list::add(const request& r, bool replace)
{
	if(replace)
	{
		request_map::iterator it = m_requests_lookup.find(r.name());
		if(it == m_requests_lookup.end())
			append(r);
		else
			*(it->second) = r;
	}
	else
	{
		request_conflict conf;
		if(!add(r, conf))
			throw request_conflict_error(conf);
	}
}


bool request_list::add(const request& r, request_conflict& conf, bool test_only)
{
	request_map::iterator it = m_requests_lookup.find(r.name());
	if(it == m_requests_lookup.end())
	{
		if(test_only)
			return true;
		append(r);
	}
	else
	{
		request& r_exist = *(it->second);
		bool a = r_exist.is_anti();
		bool b = r.is_anti();

		if(a && b)
		{
			r_exist.m_mvr.union_with(r.m_mvr);
			return true;
		}

		multi_ver_range_type result;
		if(!a && !b)
		{
			r_exist.m_mvr.intersection(r.m_mvr, result);
		}
		else if(a)
		{
			r_exist.m_mvr.inverse(result);
			result.intersect(r.m_mvr);
		}
		else
		{
			r.m_mvr.inverse(result);
			result.intersect(r_exist.m_mvr);
		}

		if(result.is_empty())
		{
			conf.set(r_exist, r);
			return false;
		}
		else if(!test_only)
			r_exist.m_mvr = result;
	}

	return true;
}


bool request_list::add(const request_list& rl, request_conflict& conf, bool test_only)
{
	for(const_iterator it=rl.begin(); it!=rl.end(); ++it)
	{
		if(!add(*it, conf, test_only))
			return false;
	}
	return true;
}


bool request_list::remove(const std::string& name)
{
	request_map::iterator it = m_requests_lookup.find(name);
	if(it == m_requests_lookup.end())
		return false;

	m_requests.erase(it->second);
	m_requests_lookup.erase(it);
	return true;
}


request_list& request_list::operator=(const request_list& rhs)
{
	clear();

	for(const_iterator it=rhs.begin(); it!=rhs.end(); ++it)
		add(*it, true);
	return *this;
}


void request_list::clear()
{
	m_requests.clear();
	m_requests_lookup.clear();
}


std::ostream& operator<<(std::ostream& os, const request_list& rl)
{
	const request_list::request_list_type& requests = rl.requests();
	for(request_list::request_list_type::const_iterator it=requests.begin(); it!=requests.end(); ++it)
	{
		if(it != requests.begin())
			os << ' ';
		os << *it;
	}

	return os;
}


} }














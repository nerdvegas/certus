#include "request.h"
#include "exceptions.h"
#include "pystring.h"
#include "util/io_utils.h"


namespace certus { namespace req {


const std::string _g_empty_name_str 	= "Empty name is invalid.";
const std::string _g_empty_mvr_str 		= "Cannot specify the Empty range.";


request::request(const std::string& s)
{
	set(s);
}


request::request(const std::string& name, const multi_ver_range_type& mvr, bool anti)
{
	set_name(name);
	set_range(mvr);
	set_anti(anti);
}


void request::set(const std::string& s)
{
	m_mvr.set_any();
	m_is_anti = false;

	std::vector<std::string> parts;
	pystring::split(s, parts, "-", 1);
	if(parts.size() > 1)
	{
		m_mvr.set(parts[1]);
		if(m_mvr.is_empty())
			throw invalid_request_error(_g_empty_mvr_str);
	}

	std::string name = parts[0];
	if(name.empty())
		throw invalid_request_error(_g_empty_name_str);

	bool del_op = false;
	if(name[0] == '!')
	{
		m_is_anti = true;
		del_op = true;
	}
	else if(name[0] == '~')
	{
		if(m_mvr.is_any())
			throw invalid_request_error("The weak operator cannot be applied to the Any version.");

		m_mvr.invert();
		m_is_anti = true;
		del_op = true;
	}

	if(del_op)
		name = pystring::slice(name, 1);
	set_name(name);
}


void request::set_name(const std::string& name)
{
	if(name.empty())
		throw invalid_request_error(_g_empty_name_str);

	for(unsigned int i=0; i<name.size(); ++i)
		if(!((name[i]>='a' && name[i]<='z') || (name[i]=='_')))
			CERTUS_THROW(invalid_request_error, "Name must contain only a-z,_ - '" << name << "'");

	m_name = name;
}


void request::set_range(const multi_ver_range_type& mvr)
{
	if(mvr.is_empty())
		throw invalid_request_error(_g_empty_mvr_str);
	m_mvr = mvr;
}


bool request::operator==(const request& rhs) const
{
	return ((m_name == rhs.m_name)
		&& (m_is_anti == rhs.m_is_anti)
		&& (m_mvr == rhs.m_mvr));
}


bool request::operator!=(const request& rhs) const
{
	return ((m_name != rhs.m_name)
		|| (m_is_anti != rhs.m_is_anti)
		|| (m_mvr != rhs.m_mvr));
}


// note this is here purely so requests can be used as keys
bool request::operator<(const request& rhs) const
{
	return (util::to_str(*this) < util::to_str(rhs));
}


std::ostream& operator<<(std::ostream& s, const request& r)
{
	if(r.is_anti())
		s << '!';

	s << r.m_name;
	if(!r.m_mvr.is_any())
		s << '-' << r.m_mvr;

	return s;
}


} }














#include <sstream>
#include <boost/lexical_cast.hpp>
#include "object.h"
#include "util/ptree_utils.h"


namespace certus { namespace obj {


std::string object::get_qualified_name(const std::string& name, const version_type& v,
	int variant_index, bool show_empty_variant)
{
	std::ostringstream strm;
	strm << name;
	if(!v.is_none())
		strm << '-' << v;
	if(variant_index >= 0)
		strm << '[' << variant_index << ']';
	else if(show_empty_variant)
		strm << "[]";

	return strm.str();
}


object::object(const std::string& name, const version_type& v, ptree_ptr metadata)
:	m_name(name),
 	m_version(v),
 	m_variant_index(-1),
 	m_metadata(metadata)
{
}


std::string object::qualified_name(bool show_empty_variant) const
{
	return get_qualified_name(m_name, m_version, m_variant_index, show_empty_variant);
}


std::ostream& operator<<(std::ostream& s, const object& o)
{
	s << "Object:   " << o.qualified_name();

	if(!o.m_requires.empty())
		s << "\nRequires: " << o.m_requires;

	if(!o.m_metadata->empty())
	{
		s << "\nMetadata: \n";
		util::print_ptree(s, *(o.m_metadata));
	}

	return s;
}


} }

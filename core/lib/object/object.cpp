#include <sstream>
#include <boost/lexical_cast.hpp>
#include "object.h"
#include "util/ptree_utils.h"


namespace certus { namespace obj {


object::object(const std::string& name, const version_type& v, ptree_ptr metadata)
:	m_name(name),
 	m_version(v),
 	m_variant_index(-1),
 	m_metadata(metadata)
{
}


std::string object::qualified_name() const
{
	std::ostringstream strm;
	strm << m_name;
	if(!m_version.is_none())
		strm << '-' << m_version;
	if(m_variant_index >= 0)
		strm << ':' << m_variant_index;

	return strm.str();
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

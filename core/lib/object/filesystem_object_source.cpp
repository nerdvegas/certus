#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "filesystem_object_source.h"
#include "version/exceptions.h"


namespace bf = boost::filesystem;


namespace certus { namespace obj {


void filesystem_object_source::load_json(const std::string& path, boost::property_tree::ptree& pt)
{
	boost::property_tree::json_parser::read_json(path, pt);
}


void filesystem_object_source::load_xml(const std::string& path, boost::property_tree::ptree& pt)
{
	boost::property_tree::xml_parser::read_xml(path, pt);
}


filesystem_object_source::filesystem_object_source(std::vector<std::string>& paths,
	const std::string& filename, fn_load_object loader, const std::string& none_version_name)
:	m_paths(paths),
 	m_loader(loader),
 	m_filename(filename),
 	m_none_str(none_version_name)
{
}


void filesystem_object_source::get_object_handles(const std::string& object_name,
	std::set<object_handle>& result) const
{
	result.clear();
	object_handle h;
	h.m_name = object_name;
	std::set<std::string> ver_names;

	for(unsigned int i=0; i<m_paths.size(); ++i)
	{
		bf::path p(m_paths[i]);
		if(!bf::is_directory(p))
			continue;

		bf::path p2 = p / bf::path(object_name);
		if(!bf::is_directory(p2))
			continue;

		bf::directory_iterator it(p2);
		for(; it!=bf::directory_iterator(); ++it)
		{
			if(!bf::is_directory(it->path()))
				continue;

			bf::path p3 = it->path() / m_filename;
			if(!bf::is_regular_file(p3))
				continue;

			std::string ver_name = it->path().filename().string();
			if(ver_names.find(ver_name) != ver_names.end())
				continue;

			if(ver_name == m_none_str)
				h.m_version.set_none();
			else
			{
				try {
					h.m_version.set(ver_name);
				}
				catch(const ver::invalid_version_error&) {
					continue;
				}
			}

			h.m_blind = i;
			result.insert(result.end(), h);
			ver_names.insert(ver_names.end(), ver_name);
		}
	}
}


void filesystem_object_source::get_object_metadata(const object_handle& h,
	boost::property_tree::ptree& metadata) const
{
	metadata.clear();
	unsigned int i = boost::any_cast<unsigned int>(h.m_blind);

	std::string ver_name;
	if(h.m_version.is_none())
		ver_name = m_none_str;
	else
		ver_name = boost::lexical_cast<std::string>(h.m_version);

	bf::path p(m_paths[i]);
	p /= h.m_name;
	p /= ver_name;
	p /= m_filename;

	m_loader(p.string(), metadata);
}


} }




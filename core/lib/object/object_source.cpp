#include <sstream>
#include <boost/optional.hpp>
#include "object_source.h"
#include "exceptions.h"
#include "util/ptree_utils.h"
#include "sys.h"

namespace bp = boost::property_tree;


namespace certus { namespace obj {


object_source::settings::settings()
:	m_append_variant_requires(true),
 	m_variant_merge_key_index(0)
{
	m_requires_keys.push_back(g_default_requires_key);
}


object_source::object_source()
{
}


void object_source::set_settings(const settings& s)
{
	m_settings = s;
}


void object_source::extract_objects(const object_source::object_handle& h,
	std::vector<object_ptr>& objects) const
{
	typedef bp::ptree::assoc_iterator 	a_it;
	typedef bp::ptree::iterator 		_it;

	ptree_ptr metadata(new bp::ptree());
	bp::ptree& meta = *metadata;
	get_object_metadata(h, meta);

	a_it it = meta.find(g_default_variants_key);
	if(it == meta.not_found())
	{
		object_ptr obj = extract_object(h, metadata);
		objects.push_back(obj);
	}
	else
	{
		// extract variants
		bp::ptree& pt_variants = it->second;
		if(!util::is_list(pt_variants))
		{
			std::string s("Invalid 'variants' construct:\n");
			s += util::as_string(pt_variants);
			throw invalid_object_error(s);
		}

		// detect simple form - list of request lists
		bool simple_variant_construct = true;
		std::vector<std::vector<std::string> > variant_strs;
		for(_it it2=pt_variants.begin(); it2!=pt_variants.end(); ++it2)
		{
			std::vector<std::string> req_strs;
			if(util::get_string_list(it2->second, req_strs))
				variant_strs.push_back(req_strs);
			else
			{
				simple_variant_construct = false;
				break;
			}
		}

		if(simple_variant_construct)
		{
			// iterate over variants
			meta.erase(g_default_variants_key);
			unsigned int num_variants = variant_strs.size();

			for(unsigned int i=0; i<num_variants; ++i)
			{
				ptree_ptr metadata2(new bp::ptree(meta));
				bp::ptree& meta2 = *metadata2;

				// merge requires
				const std::vector<std::string>& req_strs = variant_strs[i];
				const std::string& reqkey = m_settings.m_requires_keys[m_settings.m_variant_merge_key_index];
				util::add_key_string_list(meta2, reqkey, req_strs, m_settings.m_append_variant_requires);

				// inject internal metadata
				util::add_key_string_list(meta2, g_default_variant_identifier, req_strs);

				// create object, append version with variant token
				object_ptr obj = extract_object(h, metadata2);
				obj->m_variant_index = static_cast<int>(i);

				objects.push_back(obj);
			}
		}
		else
		{
			// expect variants to be list of dicts, which means they are expressing more than
			// just extra dependencies (ie there is extra variant-specific metadata present)
			throw certus_error("NOT YET IMPLEMENTED");
		}
	}
}


object_ptr object_source::extract_object(const object_handle& h, ptree_ptr metadata) const
{
	object_ptr obj = object_ptr(new object(h.m_name, h.m_version, metadata));
	bp::ptree& meta = *metadata;

	extract_requires(meta, obj->m_requires);

	// remove redundant data from metadata
	meta.erase(g_default_name_key);
	meta.erase(g_default_version_key);
	for(unsigned int i=0; i<m_settings.m_requires_keys.size(); ++i)
		meta.erase(m_settings.m_requires_keys[i]);

	return obj;
}


void object_source::extract_string_list(const bp::ptree& meta, const std::string& key,
	std::vector<std::string>& strs) const
{
	if(!util::get_key_string_list(meta, key, strs))
	{
		bp::ptree pt;
		util::filter(meta, key, pt);

		std::string s("Invalid '");
		s += key;
		s += "' construct:\n";
		s += util::as_string(pt);
		throw invalid_object_error(s);
	}
}


void object_source::extract_requires(const bp::ptree& meta, req::request_list& requires) const
{
	std::vector<std::string> req_strs;
	for(unsigned int i=0; i<m_settings.m_requires_keys.size(); ++i)
	{
		const std::string& reqkey = m_settings.m_requires_keys[i];
		extract_string_list(meta, reqkey, req_strs);
	}

	for(unsigned int i=0; i<req_strs.size(); ++i)
	{
		req::request r(req_strs[i]);
		requires.add(r);
	}
}


} }









#ifndef _CERTUS_OBJ_OBJECT_SOURCE__H_
#define _CERTUS_OBJ_OBJECT_SOURCE__H_

#include <set>
#include <vector>
#include <boost/any.hpp>
#include <boost/property_tree/ptree.hpp>
#include "object.h"


namespace certus { namespace obj {

	/*
	 * An 'object source' is a place where objects are found. For example, you may have software
	 * packages which are found on the filesystem, or assets that are found in a database. Derive
	 * from this class to implement your own object source.
	 * TODO caching support.
	 */
	class object_source
	{
	public:

		struct object_handle
		{
			std::string m_name;
			version_type m_version;
			boost::any m_blind;

			bool operator<(const object_handle& rhs) const {
				return (m_name == rhs.m_name)?
					(m_version < rhs.m_version) : m_name < rhs.m_name;
			}
		};

		struct settings
		{
			settings();

			// controls which keys in the metadata lists dependencies.
			std::vector<std::string> m_requires_keys;

			// if true, variant dependencies are appended to main dependencies; prepended otherwise
			bool m_append_variant_requires;

			// determines which requires key variant dependencies are merged into, when variants
			// are described using the short-form notation.
			unsigned int m_variant_merge_key_index;
		};

		object_source();
		virtual ~object_source(){}

		void set_settings(const settings& s);

		void extract_objects(const object_handle& h, std::vector<object_ptr>& objects) const;

		virtual void get_object_handles(const std::string& object_name,
			std::set<object_handle>& result) const = 0;

	protected:

		virtual void get_object_metadata(const object_handle& h,
			boost::property_tree::ptree& metadata) const = 0;

		object_ptr extract_object(const object_handle& h, ptree_ptr metadata) const;

		void extract_string_list(const boost::property_tree::ptree& meta, const std::string& key,
			std::vector<std::string>& strs) const;

		void extract_requires(const boost::property_tree::ptree& meta,
			req::request_list& requires) const;

	protected:

		settings m_settings;
	};

} }

#endif



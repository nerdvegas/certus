#ifndef _CERTUS_OBJ_IMPL_FILESYS_OBJECT_SOURCE__H_
#define _CERTUS_OBJ_IMPL_FILESYS_OBJECT_SOURCE__H_

#include <vector>
#include "object_source.h"


namespace certus { namespace obj {

	/*
	 * A reference object source implementation.
	 *
	 * Objects are stored in one or more directories on a filesystem. Each top-level directory
	 * contains subdirectories that match the name of each object. These in turn contain
	 * subdirectories that match the available versions of the object. Each of these leaf
	 * directories contain a file that contains object meta-data, for example - name, version,
	 * description, dependencies etc.
	 *
	 * Different "loader"s can be registered with the object source. The loader is responsible for
	 * reading the metadata file and converting it to a ptree. Some standard loaders are provided.
	 */
	class filesystem_object_source : public object_source
	{
	public:

		typedef void(*fn_load_object)(const std::string&, boost::property_tree::ptree&);

		static void load_json(const std::string& path, boost::property_tree::ptree& pt);
		static void load_xml(const std::string& path, boost::property_tree::ptree& pt);

		filesystem_object_source(std::vector<std::string>& paths, const std::string& filename,
			fn_load_object loader = filesystem_object_source::load_json,
			const std::string& none_version_name = "__no_version__");

		virtual ~filesystem_object_source(){}

		virtual void get_object_handles(const std::string& object_name,
			std::set<object_handle>& result) const;

	protected:

		virtual void get_object_metadata(const object_handle& h,
			boost::property_tree::ptree& metadata) const;

	protected:

		std::vector<std::string> m_paths;
		fn_load_object m_loader;
		std::string m_filename;
		std::string m_none_str;
	};

} }

#endif

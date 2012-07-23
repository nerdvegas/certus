#ifndef _CERTUS_OBJ_OBJECT__H_
#define _CERTUS_OBJ_OBJECT__H_

#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include "version/version.h"
#include "request/request_list.h"
#include "version/types.h"


namespace certus {

	// fwd decl
	namespace obj {
		class object;
	}

	typedef boost::shared_ptr<obj::object> 							object_ptr;
	typedef boost::shared_ptr<boost::property_tree::ptree>			ptree_ptr;
	typedef boost::shared_ptr<const boost::property_tree::ptree>	ptree_cptr;

	namespace obj {

    std::ostream& operator<<(std::ostream&, const object&);


	/*
	 * Objects are the things that Certus deals with. They might be software projects in a
	 * software management system, or assets in an asset management system for eg, depending on how
	 * Certus is being used. The object class only describes an object's metadata, not what the
	 * object represents - for example, it would describe the version of some software, and its
	 * dependencies on other pieces of software, but would not contain the software's code.
	 */
	struct object
	{
		object(const std::string& name, const version_type& v, ptree_ptr metadata);
		~object(){}
		std::string qualified_name(bool show_empty_variant = false) const;

		static std::string get_qualified_name(const std::string& name, const version_type& v,
			int variant_index = -1, bool show_empty_variant = false);

		std::string m_name;
		version_type m_version;
		int m_variant_index;
		ptree_ptr m_metadata;
		req::request_list m_requires;
	};

} }

#endif



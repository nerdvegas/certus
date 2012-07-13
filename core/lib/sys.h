#ifndef _CERTUS_SYSTEM__H_
#define _CERTUS_SYSTEM__H_

#include <stdexcept>
#include <string>


/*
 * Library-wide definitions and types.
 */

// platform detection
#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)
#define CERTUS_PLATFORM_LINUX
#endif

// useful throw macro
#define CERTUS_THROW(ExcType, Msg) { 	\
        std::stringstream strm;         \
        strm << Msg;              		\
        throw ExcType(strm.str());		\
}


namespace certus {

	/*
	 * Base class for all Certus exceptions.
	 */
	class certus_error : public std::exception
	{
	public:
		certus_error(){}
		virtual ~certus_error() throw(){}

		explicit certus_error(const std::string& s): m_what(s){}
		virtual const char* what() const throw() { return m_what.c_str(); }

	protected:
		std::string m_what;
	};

	/*
	 * General unexpected error. These may be caused by things such as objects being removed/added
	 * to a store while a resolve is in progress.
	 */
	class certus_internal_error : public certus_error
	{
	public:
		explicit certus_internal_error(const std::string& s) : certus_error(s){}
	};


	// Numeric type that version class, version py bindings are instantiated on.
	typedef unsigned int number_type;

	// Default object metadata keys
	const std::string g_default_name_key				("name");
	const std::string g_default_version_key				("version");
	const std::string g_default_requires_key			("requires");
	const std::string g_default_variants_key			("variants");

	const std::string g_default_variant_identifier		("_variant_identifier");

}

#endif




#ifndef _CERTUS_SYSTEM__H_
#define _CERTUS_SYSTEM__H_

#include <stdexcept>
#include <string>

/*
 * Library-wide definitions and types.
 */


namespace certus {

	/*
	 * Base class for all Certus exceptions.
	 */
	class certus_error : public std::runtime_error
	{
	public:
		certus_error(const std::string& s) : std::runtime_error(s){}
	};

	/*
	 * Numeric type that version class, version py bindings are instantiated on.
	 */
	typedef unsigned int number_type;

}

#endif




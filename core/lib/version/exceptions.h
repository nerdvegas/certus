#ifndef _CERTUS_VER_EXCEPTIONS__H_
#define _CERTUS_VER_EXCEPTIONS__H_

#include <stdexcept>


namespace certus { namespace ver {
	
	/*
	 * Invalid version error.
	 */
	class invalid_version_error : public std::runtime_error
	{
	public:
		invalid_version_error(const std::string& s) : std::runtime_error(s){}
	};

} }

#endif

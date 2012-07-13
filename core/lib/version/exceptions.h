#ifndef _CERTUS_VER_EXCEPTIONS__H_
#define _CERTUS_VER_EXCEPTIONS__H_

#include "sys.h"


namespace certus { namespace ver {

	/*
	 * Invalid version error.
	 */
	class invalid_version_error : public certus_error
	{
	public:
		explicit invalid_version_error(const std::string& s) : certus_error(s){}
	};

	/*
	 * Inexact version error.
	 */
	class inexact_version_error : public certus_error
	{
	public:
		explicit inexact_version_error(const std::string& s) : certus_error(s){}
	};

} }

#endif




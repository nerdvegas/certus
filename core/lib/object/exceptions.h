#ifndef _CERTUS_OBJ_EXCEPTIONS__H_
#define _CERTUS_OBJ_EXCEPTIONS__H_

#include "sys.h"


namespace certus { namespace obj {

	/*
	 * Invalid object error.
	 */
	class invalid_object_error : public certus_error
	{
	public:
		explicit invalid_object_error(const std::string& s) : certus_error(s){}
	};

} }

#endif


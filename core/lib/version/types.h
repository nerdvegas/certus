#ifndef _CERTUS_VERSION_TYPES__H_
#define _CERTUS_VERSION_TYPES__H_

#include "sys.h"
#include "version.h"
#include "version_range.h"
#include "multi_version_range.h"
#include "ver_token_alphanumeric.h"


namespace certus {

	typedef ver::ver_token_alphanumeric<number_type>	ver_token_type;
	typedef ver::version<ver_token_type>				version_type;
	typedef ver::version_range<ver_token_type>			ver_range_type;
	typedef ver::multi_version_range<ver_token_type>	multi_ver_range_type;

}

#endif

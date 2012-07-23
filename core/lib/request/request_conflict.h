#ifndef _CERTUS_REQ_REQUEST_CONFLICT__H_
#define _CERTUS_REQ_REQUEST_CONFLICT__H_

#include "request.h"


namespace certus { namespace req {

	/*
	 * A request conflict is a pair of requests for the same object, that are incompatible.
	 */
	struct request_conflict
	{
		request_conflict(){}
		request_conflict(const request& a, const request& b)	{ set(a,b); }
		void set(const request& a, const request& b)			{ m_a=a; m_b=b; }
		request m_a; // the request that was already there
		request m_b; // the request that was added
	};

	std::ostream& operator<<(std::ostream&, const request_conflict&);

} }

#endif

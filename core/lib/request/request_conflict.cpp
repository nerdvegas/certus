#include "request_conflict.h"


namespace certus { namespace req {


std::ostream& operator<<(std::ostream& os, const request_conflict& conf)
{
	// reverse order intentional - it reads more naturally to see the new (offending) request
	// first, followed by the existing request.
	os << "{\'" << conf.m_b << "\' <--!--> \'" << conf.m_a << "\'}";
	return os;
}


} }

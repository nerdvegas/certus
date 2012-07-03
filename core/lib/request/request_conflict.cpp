#include "request_conflict.h"


namespace certus { namespace req {


std::ostream& operator<<(std::ostream& os, const request_conflict& conf)
{
	os << conf.m_a << " <--!--> " << conf.m_b;
	return os;
}


} }

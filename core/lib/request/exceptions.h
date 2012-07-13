#ifndef _CERTUS_REQ_EXCEPTIONS__H_
#define _CERTUS_REQ_EXCEPTIONS__H_

#include <boost/lexical_cast.hpp>
#include "sys.h"
#include "request_conflict.h"


namespace certus { namespace req {

	/*
	 * Invalid request error.
	 */
	class invalid_request_error : public certus_error
	{
	public:
		explicit invalid_request_error(const std::string& s) : certus_error(s){}
	};


	/*
	 * Request conflict error.
	 */
	class request_conflict_error : public certus_error
	{
	public:
		explicit request_conflict_error(const request_conflict& conf)
		:	certus_error(boost::lexical_cast<std::string>(conf)),
			m_conf(conf){}

		const request_conflict& get_conflict() const { return m_conf; }

	protected:
		request_conflict m_conf;
	};

} }

#endif





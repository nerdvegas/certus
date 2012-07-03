#ifndef _CERTUS_REQ_EXCEPTIONS__H_
#define _CERTUS_REQ_EXCEPTIONS__H_

#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include "version/exceptions.h"
#include "request_conflict.h"


namespace certus { namespace req {

	/*
	 * Invalid request error.
	 */
	class invalid_request_error : public certus_error
	{
	public:
		invalid_request_error(const std::string& s) : certus_error(s){}
	};


	/*
	 * Request conflict error.
	 */
	class request_conflict_error : public certus_error
	{
	public:
		request_conflict_error(const request_conflict& conf)
		:	certus_error(boost::lexical_cast<std::string>(conf)),
			m_conf(conf){}

		const request_conflict& get_conflict() const { return m_conf; }

	protected:
		request_conflict m_conf;
	};

} }

#endif





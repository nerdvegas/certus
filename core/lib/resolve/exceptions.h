#ifndef _CERTUS_RES_EXCEPTIONS__H_
#define _CERTUS_RES_EXCEPTIONS__H_

#include "sys.h"
#include "request/request.h"


namespace certus { namespace res {

	/*
	 * Nonexistent object error.
	 * For example, a request for "foo-2.5" might fail because there is no object named "foo".
	 */
	class nonexistent_object_error : public certus_error
	{
	public:
		explicit nonexistent_object_error(const std::string& object_name)
		:	m_object_name(object_name)
		{
			m_what = "There is no object named '";
			m_what += object_name;
			m_what += "'";
		}

		virtual ~nonexistent_object_error() throw(){}

	protected:
		std::string m_object_name;
	};

	/*
	 * Nonexistent object version error.
	 * For example, a request for "foo-2.5+" might fail because those versions of "foo" don't exist.
	 */
	class nonexistent_object_version_error : public certus_error
	{
	public:
		explicit nonexistent_object_version_error(const req::request& r)
		: 	m_request(r)
		{
			m_what += "The request '";
			m_what += boost::lexical_cast<std::string>(r);
			m_what += "' does not match any version of the object.";
		}

		virtual ~nonexistent_object_version_error() throw(){}

	protected:
		req::request m_request;
	};

} }

#endif


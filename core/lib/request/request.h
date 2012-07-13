#ifndef _CERTUS_REQ_REQUEST__H_
#define _CERTUS_REQ_REQUEST__H_

#include <string>
#include "version/types.h"


namespace certus { namespace req {

    // fwd decls
    class request;
    class request_list;
    class request_conflict;
    std::ostream& operator<<(std::ostream&, const request&);

	/*
	 * A 'request' is a range of versions of a named object. It represents a request for an object
	 * whos version lies within this range. For example, "plugin-1.2" might represent a request for
	 * a piece of software called "plugin", whos version intersects with the range "1.2". Examples
	 * that would match this request are "plugin-1.2", "plugin-1.2.5", "plugin-1.2.0.3".
	 *
	 * The name of an object can contain only characters a-z,_. Certain 'operator' characters can
	 * precede the name, these are as follows:
	 *
	 * The 'conflict' operator '!': A request for these versions of the object to NOT be present.
	 * This is also known as an "anti"-request.
	 *
	 * The 'weak' operator '~': The object does not need to be present but if it is, its version
	 * must be within the given range. This is a convenience operator - '~V' is equivalent to
	 * !(inverse(V)).
	 *
	 * Example requests:
	 * 'foo'		# Any version of foo
	 * 'foo-1.3'	# foo-1.3[.X.X.X...]
	 * 'foo-5.'		# The exact version foo-5
	 * '!foo'		# No version of foo
	 * '~foo-3'		# Same as !foo-+<3|4+
	 */
	class request
	{
	public:
		request(const std::string& s);
		request(const std::string& name, const multi_ver_range_type& mvr, bool anti=false);

		void set(const std::string& s);
		void set_name(const std::string& name);
		void set_range(const multi_ver_range_type& mvr);
		void set_anti(bool anti)					{ m_is_anti=anti; }

		const std::string& name() const 			{ return m_name; }
		const multi_ver_range_type& range() const 	{ return m_mvr; }
		bool is_anti() const						{ return m_is_anti; }

	protected:
    	request(){}

	protected:
		std::string m_name;
		multi_ver_range_type m_mvr;
		bool m_is_anti;

		friend class request_list;
		friend class request_conflict;
		friend std::ostream& operator<<(std::ostream& s, const request& r);
	};

} }

#endif







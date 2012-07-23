#ifndef _CERTUS_REQ_REQUEST_LIST__H_
#define _CERTUS_REQ_REQUEST_LIST__H_

#include <list>
#include <map>
#include "request.h"
#include "request_conflict.h"
#include "exceptions.h"


namespace certus { namespace req {

	// fwd decl
	class request_list;
	std::ostream& operator<<(std::ostream&, const request_list&);

	/*
	 * A 'request list' is a list of requests that a user would like to resolve. Objects can be
	 * added to the list, and if an object is already there, the new request's range is intersected
	 * with the existing, or an error occurs if there is no intersection. The order of requests
	 * is maintained, since this can affect the resolve.
	 */
	class request_list
	{
	public:
		typedef std::list<request>					request_list_type;
		typedef request_list_type::const_iterator	const_iterator;

		request_list(){}
		request_list(const std::string& s);

		template<typename Iterator>
		request_list(Iterator begin, Iterator end) { set(begin, end); }

		template<typename Iterator>
		void set(Iterator begin, Iterator end);

		bool operator==(const request_list& rhs) const { return (m_requests == rhs.m_requests); }
		bool operator!=(const request_list& rhs) const { return (m_requests != rhs.m_requests); }
		request_list& operator=(const request_list& rhs);

		void add(const request& r, bool replace = false);
		bool add(const request& r, request_conflict& conf, bool test_only = false);
		bool add(const request_list& rl, request_conflict& conf, bool test_only = false);
		bool remove(const std::string& name);
		void clear();

		bool empty() const 							{ return m_requests.empty(); }
		const request_list_type& requests() const	{ return m_requests; }

		const_iterator begin() const	{ return m_requests.begin(); }
		const_iterator end() const 		{ return m_requests.end(); }
		const_iterator find(const std::string& name) const;

	protected:
		void append(const request& r);

	protected:
		typedef request_list_type::iterator 				requests_iterator;
		typedef std::map<std::string, requests_iterator>	request_map;

		request_list_type m_requests;
		request_map m_requests_lookup;

		friend std::ostream& operator<<(std::ostream& s, const request_list& rl);
	};


//////////////// impl

template<typename Iterator>
void request_list::set(Iterator begin, Iterator end)
{
	clear();
	for(; begin!=end; ++begin)
		this->add(*begin);
}


} }

#endif




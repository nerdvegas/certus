#ifndef _CERTUS_REQ_REQUEST_LIST__H_
#define _CERTUS_REQ_REQUEST_LIST__H_

#include <list>
#include <map>
#include "request.h"
#include "request_conflict.h"
#include "exceptions.h"


namespace certus { namespace req {


	// fwd decls
	class request_list;
	std::ostream& operator<<(std::ostream&, const request_list&);

	/*
	 * A 'request list' is a list of requests that a user would like to satisfy. Objects can be
	 * added to the list, and if an object is already there, the new request's range is intersected
	 * with the existing, or an error occurs if there is no intersection. The order of requests
	 * is maintained, since this can affect the resolve.
	 */
	class request_list
	{
	public:
		typedef std::list<request> request_list_type;

		request_list(){}
		request_list(const std::string& s);

		template<typename Iterator>
		request_list(Iterator begin, Iterator end) { set(begin, end); }

		template<typename Iterator>
		void set(Iterator begin, Iterator end);

		template<typename Iterator>
		bool set(Iterator begin, Iterator end, request_conflict& conf);

		void add(const request& r);
		bool add(const request& r, request_conflict& conf);
		bool remove(const std::string& name);
		void clear();

		const request_list_type& requests() const { return m_requests; }

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
	request_conflict conf;
	clear();

	for(; begin!=end; ++begin)
	{
		if(!this->add(*begin, conf))
			throw request_conflict_error(conf);
	}
}


template<typename Iterator>
bool request_list::set(Iterator begin, Iterator end, request_conflict& conf)
{
	clear();

	for(; begin!=end; ++begin)
	{
		if(!this->add(*begin, conf))
			return false;
	}

	return true;
}

} }

#endif




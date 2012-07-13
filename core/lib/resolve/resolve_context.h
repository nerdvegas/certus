#ifndef _CERTUS_RES_RESOLVE_CONTEXT__H_
#define _CERTUS_RES_RESOLVE_CONTEXT__H_

#include <ctime>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "object/object_source.h"
#include "request/request_list.h"


namespace certus { namespace res {

	typedef boost::shared_ptr<obj::object_source> object_source_ptr;


	/*
	 * A resolve context finds objects from the given object source(s), caches resolving-related
	 * data, and performs the actual dependency resolve.
	 */
	class resolve_context
	{
	public:

		struct settings : public obj::object_source::settings
		{
			settings();

			// print warnings to stderr
			bool m_print_warnings;

			// 0: Nothing is printed.
			// 1: Failed resolves are printed.
			// 2: Every resolve is printed.
			// 3: Resolve sub-sections are printed.
			// 4: Caching information is printed.
			unsigned int m_verbosity;

			// color certus's output so it's easier to read
			bool m_colored_output;

			// number of resolve attempts until the resolve aborts, no limit if zero
			unsigned int m_max_attempts;

			// number of seconds until resolve aborts, no limit if zero
			unsigned int m_timeout_secs;

			// silently discard requests for nonexistent objects
			bool m_discard_nonexistent_objects;

			// throw an exception on requests for nonexistent objects (disregarded when
			// m_discard_nonexistent_objects is true).
			bool m_throw_on_nonexistent_objects;

			// silently discard anti-requests for nonexistent objects
			bool m_discard_nonexistent_anti_objects;

			// throw an exception on anti-requests for nonexistent objects (disregarded when
			// m_discard_nonexistent_anti_objects is true).
			bool m_throw_on_nonexistent_anti_objects;

			// throw an exception on requests for object versions that do not exist.
			bool m_throw_on_nonexistent_object_versions;
		};

		struct resolve_result
		{
			std::vector<object_ptr> m_objects;
			unsigned int m_num_attempts;
			std::string m_graph;
		};

		enum resolve_status
		{
			// the resolve was successful
			RESOLVE_OK = 0,

			// the resolve is not possible
			RESOLVE_FAIL,

			// the max attempts limit was reached before a resolve could be found
			RESOLVE_MAX_ATTEMPTS,

			// the time limit was reached before a resolve could be found
			RESOLVE_TIMEOUT
		};

		// testing, just here for py binding atm
		resolve_context(){}

		resolve_context(object_source_ptr obj_src, const settings& s = settings());
		~resolve_context(){}

		resolve_status resolve(const req::request_list& rl, resolve_result& result);

		void test(const req::request_list& rl);

	protected:

		enum error_type
		{
			ERROR_OK = 0,
			ERROR_NONEXISTENT_OBJECT,
			ERROR_NONEXISTENT_OBJECT_VERSION,
			ERROR_CONFLICT,
			ERROR_MAX_ATTEMPTS,
			ERROR_TIMEOUT
		};

		enum edge_type
		{
			EDGE_NORMALISE = 0,
			EDGE_REQUIRES,
			EDGE_RESOLVE,
			EDGE_CONFLICT,
			EDGE_NONEXISTENT_OBJECT,
			EDGE_NONEXISTENT_OBJECT_VERSION
		};

		struct graph_edge
		{
			graph_edge(edge_type etype, const std::string& object_name)
			: m_type(etype), m_object_name(object_name){}

			edge_type m_type;
			std::string m_object_name;
		};

		typedef std::vector<graph_edge> graph_edge_vector;

		struct variant_key : public std::pair<version_type,int>
		{
			variant_key(const version_type& v, int variant_index=-1)
			: std::pair<version_type,int>(v, variant_index){}
			const version_type& version() const { return this->first; }
			const int variant_index() const { return this->second; }
		};

		typedef std::map<variant_key, object_ptr>		object_map;
		typedef std::map<version_type, boost::any> 		object_handle_blind_map;

		/*
		 * Cached data for all the versions of a named object
		 */
		struct object_cache
		{
			std::string m_object_name;
			multi_ver_range_type m_all_versions;
			object_map m_objects;
			object_handle_blind_map m_object_blind_data;
		};

		typedef std::map<std::string, object_cache>	object_cache_map;

		/*
		 * Internal data for tracking the resolve of a single request.
		 */
		struct request_working_data
		{
			object_map m_objects;
		};

		typedef std::map<std::string, request_working_data> request_data_map;

		/*
		 * Internal data shared across recursive resolve calls
		 */
		struct shared_working_data
		{
			unsigned int m_num_attempts;
			std::clock_t m_start_time;
		};

		/*
		 * Internal data chained into recursive resolve calls
		 */
		struct local_working_data;
		typedef boost::shared_ptr<local_working_data>			lwd_ptr;
		typedef boost::shared_ptr<const local_working_data>		lwd_cptr;

		struct local_working_data
		{
			local_working_data(lwd_cptr parent=lwd_cptr()): m_parent(parent){}

			lwd_cptr m_parent;
			req::request_list m_rl;
			request_data_map m_request_data;
			graph_edge_vector m_edges;
		};

		typedef std::pair<lwd_cptr, error_type> recurse_resolve_result;

	protected:

		/*
		 * Internal recursive resolve method.
		 */
		recurse_resolve_result _resolve(shared_working_data& swd, lwd_cptr lwd_parent);

		error_type filter_nonexistent_objects(local_working_data& lwd);

		error_type normalise_requests(local_working_data& lwd);

		void create_graph(const req::request_list& rl_top, lwd_cptr lwd, resolve_result& result);

		/*
		 * Get the object map for the given object name. Returns false if an object by that name
		 * does not exist.
		 */
		object_cache* get_object_cache(const std::string& object_name, bool expected=false);

		inline bool print_failed_resolves() const 			{ return (m_settings.m_verbosity >= 1); }
		inline bool print_resolves() const 					{ return (m_settings.m_verbosity >= 2); }
		inline bool print_resolve_subs() const 				{ return (m_settings.m_verbosity >= 3); }
		inline bool print_caching() const 					{ return (m_settings.m_verbosity >= 4); }

	protected:

		settings m_settings;
		object_source_ptr m_obj_src;
		const char** m_colors;

		object_cache_map m_objects_cache;
	};

} }

#endif





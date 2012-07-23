#ifndef _CERTUS_RES_RESOLVE_CONTEXT__H_
#define _CERTUS_RES_RESOLVE_CONTEXT__H_

#include <ctime>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "object/object_source.h"
#include "request/request_list.h"
#include "resolve_settings.h"
#include "resolve_graph.h"


namespace certus { namespace res {

	typedef boost::shared_ptr<obj::object_source> object_source_ptr;


	/*
	 * A resolve context finds objects from the given object source(s), caches resolving-related
	 * data, and performs the actual dependency resolve.
	 */
	class resolve_context
	{
	public:

		typedef std::pair<unsigned int, std::string> annotation_type;

		struct resolve_result
		{
			std::vector<object_ptr> m_objects;
			unsigned int m_num_attempts;
			std::string m_graph;
			std::vector<annotation_type> m_annotations;
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

		resolve_context(object_source_ptr obj_src, const resolve_settings& s = resolve_settings());
		~resolve_context(){}

		resolve_status resolve(const req::request_list& rl, resolve_result& result);

		void test(const req::request_list& rl);

	protected:

		enum error_type
		{
			ERROR_OK = 0,
			ERROR_NONEXISTENT_OBJECT,
			ERROR_NONEXISTENT_ANTI_OBJECT,
			ERROR_NONEXISTENT_OBJECT_VERSION,
			ERROR_CONFLICT,
			ERROR_MAX_ATTEMPTS,
			ERROR_TIMEOUT
		};


		struct variant_key : public std::pair<version_type,int>
		{
			variant_key(const version_type& v, int variant_index=-1)
			: std::pair<version_type,int>(v, variant_index){}
			const version_type& version() const { return this->first; }
			const int variant_index() const { return this->second; }
		};

		typedef std::vector<object_ptr>					object_vector;
		typedef std::map<variant_key, object_ptr>		object_map;
		typedef object_map::iterator					object_map_it;
		typedef std::pair<object_map_it,object_map_it>	object_map_range;
		typedef std::map<version_type, boost::any> 		object_handle_blind_map;

		/*
		 * Cached data for all the versions of a named object
		 */
		struct object_cache
		{
			std::string m_object_name;
			object_map m_objects;
			object_handle_blind_map m_object_blind_data;
		};

		typedef std::map<std::string, object_cache>	object_cache_map;

		/*
		 * Internal data for tracking the resolve of a single request.
		 */
		struct request_working_data
		{
			request_working_data():m_common_requires_added(false){}

			multi_ver_range_type m_mvr;
			std::vector<version_type> m_versions;
			object_vector m_objects;
			object_ptr m_resolved_object;
			bool m_common_requires_added;
		};

		typedef boost::shared_ptr<request_working_data> rwd_ptr;
		typedef std::map<std::string, rwd_ptr> request_data_map;

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
			local_working_data(lwd_cptr parent = lwd_cptr());

			lwd_cptr m_parent;
			req::request_list m_rl;
			request_data_map m_request_data;
			resolve_graph_ptr m_graph;
			unsigned int m_annotation_index;
			std::vector<annotation_type> m_annotations;
		};

		typedef std::pair<lwd_cptr, error_type> recurse_resolve_result;

	protected:

		/*
		 * Internal resolve methods.
		 */
		recurse_resolve_result resolve_recurse(shared_working_data& swd, lwd_ptr lwdp);

		error_type resolve_without_selection(local_working_data& lwd);

		bool is_fully_resolved(local_working_data& lwd);

		error_type remove_nonexistent_objects(local_working_data& lwd);

		error_type normalise_requests(local_working_data& lwd);

		error_type remove_conflicting_requests(local_working_data& lwd);

		void get_normalised_lite(const req::request_list& rl, req::request_list& result);

		error_type add_common_dependencies(local_working_data& lwd);

		bool add_request(local_working_data& lwd, const req::request& req_parent,
			const req::request& req_child);

		void process_result(const req::request_list& rl_top, lwd_cptr lwd, resolve_result& result);

		/*
		 * Get the object map for the given object name. Returns false if an object by that name
		 * does not exist.
		 */
		object_cache* get_object_cache(const std::string& object_name);

		/*
		 * Return the objects that match the given version range.
		 */
		enum request_normalise_clipping_mode
		{
			RNCM_NONE = 0,		// give the most readable result
			RNCM_CLIP_TO_RANGE,	// readable, but also guaranteed to be clipped within 'mvr'
			RNCM_CLIP_TIGHT		// less readable but boundaries of ranges are exact
		};

		bool get_object_versions(object_cache* ocache, const multi_ver_range_type& mvr,
			multi_ver_range_type* normalised_mvr, std::vector<version_type>* versions);
			//request_normalise_clipping_mode clip_mode = RNCM_CLIP_TIGHT);

		/*
		 * Return the list of objects that match the given version.
		 */
		object_map_range get_objects(object_cache* ocache, const version_type& v);

		inline bool print_failed_resolves() const 			{ return (m_settings.m_verbosity >= 1); }
		inline bool print_resolves() const 					{ return (m_settings.m_verbosity >= 2); }
		inline bool print_resolve_subs() const 				{ return (m_settings.m_verbosity >= 3); }
		inline bool print_caching() const 					{ return (m_settings.m_verbosity >= 4); }

	protected:

		resolve_settings m_settings;
		object_source_ptr m_obj_src;
		const char** m_colors;

		object_cache_map m_objects_cache;
		std::set<std::string> m_nonexistent_objects;
	};

} }

#endif





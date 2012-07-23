#include "resolve_context.h"
#include "exceptions.h"
#include "util/io_utils.h"
#include "util/algorithm.h"
#include <algorithm>

// testing
#include <iostream>
#include <fstream>


namespace certus {

using namespace req;
using namespace obj;

namespace res {

// todo move to sys
const char* g_cols_none[] = {
	"",
	"","","","",
	"","","",""
};

#ifdef CERTUS_PLATFORM_LINUX
const char* g_cols[] = {
	"\033[0m", // reset
	"\033[;31m", "\033[;32m", "\033[;33m", "\033[;36m", // r,g,y,c
	"\033[1;31m", "\033[1;32m", "\033[1;33m", "\033[1;36m", // r,g,y,c (bold)
};
#else
const char* g_cols[] = g_cols_none;
#endif
#define _Z 0
#define _R 1
#define _G 2
#define _Y 3
#define _C 4
#define _RB 5
#define _GB 6
#define _YB 7
#define _CB 8


#define _ANNOTATE(lwd, Msg) { 		\
        std::stringstream strm;		\
        strm << Msg;              	\
        lwd.m_annotations.push_back(annotation_type(lwd.m_annotation_index, strm.str())); \
}


resolve_context::local_working_data::local_working_data(resolve_context::lwd_cptr parent)
:	m_parent(parent),
 	m_annotation_index(parent? parent->m_annotation_index : 1)
{
	resolve_graph_ptr parent_graph = (parent)? parent->m_graph : resolve_graph_ptr();
	m_graph.reset(new resolve_graph(parent_graph));
}


resolve_context::resolve_context(object_source_ptr obj_src, const resolve_settings& s)
:	m_settings(s),
 	m_obj_src(obj_src),
 	m_colors(g_cols_none)
{
	if(m_settings.m_colored_output)
		m_colors = g_cols;

	if(!m_settings.m_create_graph)
		m_settings.m_create_annotations = false;
}


resolve_context::resolve_status resolve_context::resolve(const request_list& rl,
	resolve_context::resolve_result& result)
{
	// init working data
	shared_working_data swd;
	swd.m_num_attempts = 0;
	swd.m_start_time = std::clock();

	lwd_ptr lwdp(new local_working_data());
	lwdp->m_rl = rl;

	// pass settings onto object source
	m_obj_src->set_settings(m_settings);

	// do the recursive resolve
	recurse_resolve_result r = resolve_recurse(swd, lwdp);
	lwd_cptr result_lwd = r.first;
	error_type& result_error = r.second;

	// process the result - create the graph, annotation file etc
	process_result(rl, result_lwd, result);

	switch(result_error)
	{
	case ERROR_OK:				return RESOLVE_OK; break;
	case ERROR_MAX_ATTEMPTS:	return RESOLVE_MAX_ATTEMPTS; break;
	case ERROR_TIMEOUT:			return RESOLVE_TIMEOUT; break;
	default:					return RESOLVE_FAIL; break;
	}
}


resolve_context::recurse_resolve_result resolve_context::resolve_recurse(
	resolve_context::shared_working_data& swd, resolve_context::lwd_ptr lwdp)
{
	error_type ret = ERROR_OK;
	local_working_data& lwd = *lwdp;

	// iteratively run the resolve process until the request list stablises. Once this happens,
	// either all requests have been resolved, or we need to do a selection and recursively solve.
	request_list prev_rl;
	do
	{
		prev_rl = lwd.m_rl;
		error_type ret = resolve_without_selection(lwd);
		if(ret != ERROR_OK)
			return recurse_resolve_result(lwdp, ret);

		if(is_fully_resolved(lwd))
			return recurse_resolve_result(lwdp, ERROR_OK);
	}
	while(lwd.m_rl != prev_rl);

	// here we enter the 'selection' process. We nominate some request, and split its range
	// into subranges. For each subrange, we attempt to resolve a copy of the current request
	// list, with only this sub-range in the nominated request.
	//return resolve_select(swd, lwdp);

	return recurse_resolve_result(lwdp, ERROR_OK);
}


bool resolve_context::is_fully_resolved(resolve_context::local_working_data& lwd)
{
	for(request_list::const_iterator it=lwd.m_rl.begin(); it!=lwd.m_rl.end(); ++it)
	{
		const request& req = *it;
		if(req.is_anti())
			continue;

		request_data_map::const_iterator it_req = lwd.m_request_data.find(req.name());
		if(it_req != lwd.m_request_data.end())
		{
			const request_working_data& rwd = *(it_req->second);
			if(!rwd.m_resolved_object)
				return false;
		}
	}

	return true;
}


resolve_context::error_type resolve_context::resolve_without_selection(
	resolve_context::local_working_data& lwd)
{
	if(print_resolves())
	{
		if(print_resolve_subs())
			std::cout << '\n';
		std::cout << m_colors[_CB] << "Attempting to resolve: ["
			<< lwd.m_rl << "]..." << m_colors[_Z] << std::endl;
	}

	error_type ret;

	// Check for nonexistent objects. This may discard some objects from the request, throw an
	// exception or result in a failed resolve, depending on settings
	ret = remove_nonexistent_objects(lwd);
	if(ret !=ERROR_OK)
		return ret;

	// Normalise requests. 'Normalising' means taking the request version range, and intersecting
	// it with the actual versions of the object that exist. This may throw or result in a failed
	// resolve, if requesting a version(s) of an object that do not exist.
	ret = normalise_requests(lwd);
	if(ret !=ERROR_OK)
		return ret;

	// Remove requests for objects whos dependencies would introduce a conflict into the current
	// request list. This may result in a failed resolve, if all possible requests for a given
	// object introduces conflicts. This may also fully-resolve some requests, if there is just
	// a single version/variant left.
	ret = remove_conflicting_requests(lwd);
	if(ret !=ERROR_OK)
		return ret;

	// Find dependencies that are common to all objects in a request, and add these as new requests
	// into the current request list.
	ret = add_common_dependencies(lwd);
	if(ret !=ERROR_OK)
		return ret;

	return ERROR_OK;
}


resolve_context::error_type resolve_context::remove_nonexistent_objects(
	resolve_context::local_working_data& lwd)
{
	if(print_resolve_subs())
		std::cout << m_colors[_C] << "Finding nonexistent objects..." << m_colors[_Z] << std::endl;

	std::vector<std::string> del_reqs;

	for(request_list::const_iterator it=lwd.m_rl.begin(); it!=lwd.m_rl.end(); ++it)
	{
		const request& req = *it;

		// new object, attempt to load it
		object_cache* ocache = get_object_cache(req.name());
		if(!ocache)
		{
			bool discard = false;
			edge_type et;

			if(req.is_anti())
			{
				if(m_settings.m_discard_nonexistent_anti_objects)
					discard = true;
				else if(m_settings.m_throw_on_nonexistent_anti_objects)
					throw nonexistent_object_error(req.name());
				et = EDGE_NONEXISTENT_ANTI_OBJECT;
			}
			else
			{
				if(m_settings.m_discard_nonexistent_objects)
					discard = true;
				else if(m_settings.m_throw_on_nonexistent_objects)
					throw nonexistent_object_error(req.name());
				et = EDGE_NONEXISTENT_OBJECT;
			}

			if(m_settings.m_create_graph)
			{
				graph_edge e(et, util::to_str(req));
				lwd.m_graph->add_edge(e);
			}

			if(discard)
			{
				del_reqs.push_back(req.name());
				if(m_settings.m_print_warnings)
				{
					std::cerr << m_colors[_Y] << "Request '" << req << "' references nonexistent object '"
						<< req.name() << "' and was discarded." << m_colors[_Z] << std::endl;
				}
				continue;
			}
			else
			{
				if(m_settings.m_print_warnings)
				{
					const char* col = req.is_anti()?
						(m_settings.m_abort_on_nonexistent_anti_objects? m_colors[_RB] : m_colors[_YB]) :
						(m_settings.m_abort_on_nonexistent_objects? m_colors[_RB] : m_colors[_YB]);

					std::cerr << col << "Request '" << req << "' references nonexistent object '"
						<< req.name() << "'" << m_colors[_Z] << std::endl;
				}

				return req.is_anti()? ERROR_NONEXISTENT_ANTI_OBJECT : ERROR_NONEXISTENT_OBJECT;
			}
		}
	}

	for(unsigned int i=0; i<del_reqs.size(); ++i)
		lwd.m_rl.remove(del_reqs[i]);

	return ERROR_OK;
}


resolve_context::error_type resolve_context::normalise_requests(
	resolve_context::local_working_data& lwd)
{
	if(print_resolve_subs())
		std::cout << m_colors[_C] << "Normalising requests..." << m_colors[_Z] << std::endl;

	std::vector<std::string> del_anti_reqs;

	// iterate over requests, normalise each against the versions in the cache
	for(request_list::const_iterator it=lwd.m_rl.begin(); it!=lwd.m_rl.end(); ++it)
	{
		const request& req = *it;

		if(lwd.m_request_data.find(req.name()) != lwd.m_request_data.end())
			continue; // this request has already been normalised

		object_cache* ocache = get_object_cache(req.name());
		if(!ocache)
			continue;

		std::string req_str = util::to_str(req);
		rwd_ptr rwdp(new request_working_data());
		request_working_data& rwd = *rwdp;

		if(get_object_versions(ocache, req.range(), &(rwd.m_mvr), &(rwd.m_versions)))
		{
			// normalise the request
			request normalised_req(req.name(), rwd.m_mvr, req.is_anti());
			std::string normalised_req_str = util::to_str(normalised_req);

			if(print_resolve_subs())
				std::cout << "'" << req << "' -> '" << normalised_req_str << "'" << std::endl;

			if(m_settings.m_create_graph && (normalised_req.range() != req.range()))
			{
				graph_edge e(EDGE_NORMALISE, req_str, normalised_req_str);
				e.m_annotation_index = lwd.m_annotation_index;
				lwd.m_graph->add_edge(e);

				if(m_settings.m_create_annotations)
				{
					_ANNOTATE(lwd, "'" << req << "' was normalised to '"
						<< normalised_req << "' and contains the versions:");
					for(unsigned int i=0; i<rwd.m_versions.size(); ++i)
					{
						std::string s = object::get_qualified_name(req.name(), rwd.m_versions[i]);
						_ANNOTATE(lwd, '\'' << s << '\'');
					}
					++lwd.m_annotation_index;
				}
			}

			lwd.m_rl.add(normalised_req, true);
			lwd.m_request_data.insert(request_data_map::value_type(req.name(), rwdp));
		}
		else
		{
			// request does not contain existing object version(s)
			if(m_settings.m_create_graph)
			{
				edge_type et = (req.is_anti())?
					EDGE_NONEXISTENT_ANTI_OBJECT_VERSION : EDGE_NONEXISTENT_OBJECT_VERSION;
				graph_edge e(et, req_str);
				lwd.m_graph->add_edge(e);
			}

			if(req.is_anti())
			{
				// antis can just disappear - user is asking for NOT something that is NOT there
				del_anti_reqs.push_back(req.name());
			}
			else
			{
				if(m_settings.m_throw_on_nonexistent_object_versions)
					throw nonexistent_object_version_error(req);
				else
				{
					if(m_settings.m_print_warnings)
					{
						const char* col =
							(m_settings.m_abort_on_nonexistent_object_versions? m_colors[_RB] : m_colors[_YB]);

						std::cerr << col << "Request '" << req
							<< "' references nonexistent object version(s)'" << m_colors[_Z] << std::endl;
					}

					return ERROR_NONEXISTENT_OBJECT_VERSION;
				}
			}
		}
	}

	for(unsigned int i=0; i<del_anti_reqs.size(); ++i)
		lwd.m_rl.remove(del_anti_reqs[i]);

	return ERROR_OK;
}


void resolve_context::get_normalised_lite(const request_list& rl, request_list& result)
{
	result.clear();
	for(request_list::const_iterator it=rl.begin(); it!=rl.end(); ++it)
	{
		request req = *it;

		object_cache* ocache = get_object_cache(req.name());
		if(ocache)
		{
			multi_ver_range_type mvr;
			if(get_object_versions(ocache, req.range(), &mvr, 0))
				req.set_range(mvr);
		}

		result.add(req);
	}
}


struct _conflict_entry
{
	_conflict_entry():num_obj(0),num_ver(0){}
	std::string object_name;
	multi_ver_range_type mvr;
	unsigned int num_obj;
	unsigned int num_ver;
	std::vector<std::pair<const object*,request_conflict> > object_conflicts;
};

resolve_context::error_type resolve_context::remove_conflicting_requests(
	resolve_context::local_working_data& lwd)
{
	typedef std::map<std::string, _conflict_entry> conflict_map;

	if(print_resolve_subs())
		std::cout << m_colors[_C] << "Removing conflicting requests..." << m_colors[_Z] << std::endl;

	for(request_list::const_iterator it=lwd.m_rl.begin(); it!=lwd.m_rl.end(); ++it)
	{
		const request& req = *it;
		if(req.is_anti())
			continue;

		request_data_map::iterator it_req = lwd.m_request_data.find(req.name());
		if(it_req == lwd.m_request_data.end())
			CERTUS_THROW(certus_internal_error, req << " missing rwd in remove_conflicting_requests");

		request_working_data& rwd = *(it_req->second);
		if(!rwd.m_objects.empty())
			continue; // conflicting requests have already been removed

		if(rwd.m_resolved_object)
			continue; // this request has already been fully resolved

		if(rwd.m_versions.empty())
			CERTUS_THROW(certus_internal_error, req << " has no versions in remove_conflicting_requests");

		object_cache* ocache = get_object_cache(req.name());
		if(!ocache)
			CERTUS_THROW(certus_internal_error, "No object cache in remove_conflicting_requests on " << req);

		multi_ver_range_type conf_mvr;
		conflict_map conflicts;
		object_vector& req_objects = rwd.m_objects;
		req_objects.clear();

		unsigned int num_discarded_objects = 0;
		unsigned int num_discarded_versions = 0;

		// iterate over object versions and find those that cannot be added to the request list
		for(std::vector<version_type>::const_iterator it2=rwd.m_versions.begin();
			it2!=rwd.m_versions.end(); ++it2)
		{
			object_map_range p = get_objects(ocache, *it2);
			bool all_variants_conflict = true;
			std::set<std::string> visited_conflicts;

			for(; p.first != p.second; ++p.first)
			{
				const object& o = *(p.first->second);
				request_conflict conf;

				// must normalise requires first
				request_list normalised_rl;
				get_normalised_lite(o.m_requires, normalised_rl);

				if(lwd.m_rl.add(normalised_rl, conf, true))
				{
					req_objects.push_back(p.first->second);
					all_variants_conflict = false;
				}
				else
				{
					std::string conf_req_str = util::to_str(conf.m_a);
					_conflict_entry& conflict = conflicts[conf_req_str];
					conflict.object_name = conf.m_a.name();
					conflict.mvr.union_with(ver_range_type(*it2, true));
					++conflict.num_obj;

					if(m_settings.m_create_annotations)
					{
						std::pair<const object*,request_conflict> p(&o, conf);
						conflict.object_conflicts.push_back(p);
					}

					visited_conflicts.insert(visited_conflicts.end(), conf_req_str);
					if(print_resolve_subs())
					{
						std::cout << "Removed " << o.qualified_name() << ' '
							<< conf << std::endl;
					}
					++num_discarded_objects;
				}
			}

			for(std::set<std::string>::const_iterator it3=visited_conflicts.begin();
				it3!=visited_conflicts.end(); ++it3)
			{
				++(conflicts.find(*it3)->second.num_ver);
			}

			if(all_variants_conflict)
			{
				conf_mvr.union_with(ver_range_type(*it2, true));
				++num_discarded_versions;
			}
		}

		// we don't need these any more
		rwd.m_versions.clear();

		// remove those versions that clash entirely. If there's nothing left, then the current
		// resolve fails.
		bool all_versions_conflict = false;
		std::string filtered_req_str;

		if(!conf_mvr.is_empty())
		{
			multi_ver_range_type conf_inv_mvr, filtered_mvr, normalised_filtered_mvr;
			conf_mvr.inverse(conf_inv_mvr);
			req.range().intersection(conf_inv_mvr, filtered_mvr);

			if(get_object_versions(ocache, filtered_mvr, &normalised_filtered_mvr, 0))
			{
				assert(req.range() != normalised_filtered_mvr);
				request filtered_req(req.name(), normalised_filtered_mvr);
				filtered_req_str = util::to_str(filtered_req);

				if(m_settings.m_create_graph)
				{
					graph_edge e(EDGE_DISCARD, util::to_str(req), filtered_req_str);
					e.m_child_field = filtered_req_str;
					e.m_num_discarded_objects = num_discarded_objects;
					e.m_num_discarded_versions = num_discarded_versions;
					if(!m_settings.m_graph_show_conflict_removal)
						e.m_annotation_index = lwd.m_annotation_index;
					lwd.m_graph->add_edge(e);

					if(!m_settings.m_graph_show_conflict_removal && (m_settings.m_create_annotations))
					{
						for(conflict_map::const_iterator it=conflicts.begin(); it!=conflicts.end(); ++it)
						{
							const _conflict_entry& ce = it->second;
							for(unsigned int i=0; i<ce.object_conflicts.size(); ++i)
							{
								const std::pair<const object*,request_conflict>& p = ce.object_conflicts[i];
								_ANNOTATE(lwd, '\'' << p.first->qualified_name() <<
									"\' introduces conflict " << p.second << " and was discarded");
							}
						}
						++lwd.m_annotation_index;
					}
				}

				lwd.m_rl.add(filtered_req, true);
			}
			else
				all_versions_conflict = true;
		}

		assert(all_versions_conflict == req_objects.empty());

		if(all_versions_conflict || m_settings.m_graph_show_conflict_removal)
		{
			if(m_settings.m_create_graph)
			{
				std::string gname("All versions conflict: ");
				gname += req.name();
				graph_group full_conf_group(GROUP_FULL_CONFLICT, gname);

				if(all_versions_conflict)
				{
					filtered_req_str = "FULL_CONFLICT";
					full_conf_group.m_nodes.push_back(filtered_req_str);
					std::string req_str = util::to_str(req);

					graph_edge e(EDGE_DISCARD, req_str, filtered_req_str);
					e.m_num_discarded_objects = num_discarded_objects;
					e.m_num_discarded_versions = num_discarded_versions;
					lwd.m_graph->add_edge(e);
				}

				edge_type et = (all_versions_conflict)? EDGE_CONFLICT : EDGE_PASSIVE_CONFLICT;

				for(conflict_map::const_iterator it=conflicts.begin(); it!=conflicts.end(); ++it)
				{
					full_conf_group.m_nodes.push_back(it->first);
					const _conflict_entry& ce = it->second;

					multi_ver_range_type normalised_mvr;
					get_object_versions(ocache, ce.mvr, &normalised_mvr, 0);
					assert(!normalised_mvr.is_empty());

					if(all_versions_conflict)
					{
						// show more info in this case
						multi_ver_range_type mvr;
						for(unsigned int i=0; i<ce.object_conflicts.size(); ++i)
							mvr.union_with(ce.object_conflicts[i].second.m_b.range());

						request r(ce.object_name, mvr);
						std::string r_str = util::to_str(r);

						graph_edge e(EDGE_REQUIRES, filtered_req_str, r_str);
						e.m_parent_field = util::to_str(normalised_mvr);
						lwd.m_graph->add_edge(e);
						full_conf_group.m_nodes.push_back(r_str);

						graph_edge e2(et, r_str, it->first);
						e2.m_num_discarded_objects = ce.num_obj;
						e2.m_num_discarded_versions = ce.num_ver;
						e2.m_annotation_index = lwd.m_annotation_index;
						lwd.m_graph->add_edge(e2);
					}
					else
					{
						graph_edge e(et, filtered_req_str, it->first);
						e.m_parent_field = util::to_str(normalised_mvr);
						e.m_num_discarded_objects = ce.num_obj;
						e.m_num_discarded_versions = ce.num_ver;
						e.m_annotation_index = lwd.m_annotation_index;
						lwd.m_graph->add_edge(e);
					}

					if(m_settings.m_create_annotations)
					{
						for(unsigned int i=0; i<ce.object_conflicts.size(); ++i)
						{
							const std::pair<const object*,request_conflict>& p = ce.object_conflicts[i];
							_ANNOTATE(lwd, '\'' << p.first->qualified_name() <<
								"\' introduces conflict " << p.second << " and was discarded");
						}
						++lwd.m_annotation_index;
					}
				}

				if(all_versions_conflict)
					lwd.m_graph->add_group(full_conf_group);
			}

			if(all_versions_conflict)
				return ERROR_CONFLICT;
		}

		if(req_objects.size() == 1)
		{
			// resolve this object! We know that this is the exact version/variant we need
			rwd.m_resolved_object = req_objects[0];
			const object& o = *(rwd.m_resolved_object);

			if(print_resolve_subs())
			{
				std::cout << m_colors[_GB] << "Resolved '" << req << "' -> '"
					<< o.qualified_name() << "'" << m_colors[_Z] << std::endl;
			}

			if(m_settings.m_create_graph)
			{
				graph_edge e(EDGE_RESOLVE, util::to_str(req), o.qualified_name(true));
				lwd.m_graph->add_edge(e);
			}
		}
	}

	return ERROR_OK;
}


resolve_context::error_type resolve_context::add_common_dependencies(
	resolve_context::local_working_data& lwd)
{
	if(print_resolve_subs())
		std::cout << m_colors[_C] << "Adding common dependencies..." << m_colors[_Z] << std::endl;

	for(request_list::const_iterator it=lwd.m_rl.begin(); it!=lwd.m_rl.end(); ++it)
	{
		const request& req = *it;
		if(req.is_anti())
			continue;

		request_data_map::iterator it_req = lwd.m_request_data.find(req.name());
		if(it_req == lwd.m_request_data.end())
			continue;

		request_working_data& rwd = *(it_req->second);
		if(rwd.m_objects.empty())
			continue;

		if(rwd.m_common_requires_added)
			continue; // common dependencies for this request have already been added

		// find common dependencies
		typedef std::map<std::string, multi_ver_range_type> dependencies_map;
		dependencies_map dependencies;

		{
			object_vector::const_iterator it2 = rwd.m_objects.begin();
			const request_list& rl = (*it2)->m_requires;
			for(request_list::const_iterator it3=rl.begin(); it3!=rl.end(); ++it3)
			{
				if(!it3->is_anti())
					dependencies.insert(dependencies_map::value_type(it3->name(), it3->range()));
			}

			for(++it2; ((it2!=rwd.m_objects.end()) && !dependencies.empty()); ++it2)
			{
				const request_list& rl = (*it2)->m_requires;
				dependencies_map::iterator it3_;
				for(dependencies_map::iterator it3=dependencies.begin(); it3!=dependencies.end(); it3=it3_)
				{
					it3_ = it3; ++it3_;
					request_list::const_iterator it4 = rl.find(it3->first);
					if(it4 == rl.end())
						dependencies.erase(it3);
					else
						it3->second.union_with(it4->range());
				}
			}
		}

		for(dependencies_map::const_iterator it2=dependencies.begin(); it2!=dependencies.end(); ++it2)
		{
			request req2(it2->first, it2->second);

			// this may fail, but that's ok! Two different objects may introduce new objects that
			// conflict, if that happens then we just let the next iteration deal with the
			// problem cleanly (ie add to the graph properly etc).
			if(add_request(lwd, req, req2))
				rwd.m_common_requires_added = true;
		}
	}

	return ERROR_OK;
}


bool resolve_context::add_request(resolve_context::local_working_data& lwd,
	const request& req_parent, const request& req_child)
{
	request_conflict conf;
	if(!lwd.m_rl.add(req_child, conf, true))
		return false; // cannot add, introduces a conflict

	std::string req_parent_str = util::to_str(req_parent);
	request_data_map::const_iterator it = lwd.m_request_data.find(req_parent.name());
	if((it != lwd.m_request_data.end()) && it->second->m_resolved_object)
		req_parent_str = it->second->m_resolved_object->qualified_name(true);

	std::string req_child_str = util::to_str(req_child);

	bool exact_child_exists = lwd.m_graph->exists(req_child_str);
	graph_edge e(EDGE_REQUIRES, req_parent_str, req_child_str);
	lwd.m_graph->add_edge(e);

	// intersect with existing request if necessary
	if(!exact_child_exists)
	{
		request_list::const_iterator it = lwd.m_rl.find(req_child.name());
		if(it == lwd.m_rl.end())
		{
			lwd.m_rl.add(req_child);
			if(print_resolve_subs())
			{
				std::cout << m_colors[_G] << "Added '" << req_child
					<< "' to request list" << m_colors[_Z] << std::endl;
			}
		}
		else
		{
			std::string r_str = util::to_str(*it);
			if(req_child.range().is_superset(it->range()))
			{
				graph_edge e(EDGE_REDUCE, req_child_str, r_str);
				lwd.m_graph->add_edge(e);
			}
			else
			{
				if(it->range().is_superset(req_child.range()))
				{
					graph_edge e(EDGE_REDUCE, r_str, req_child_str);
					lwd.m_graph->add_edge(e);
				}
				else
				{
					multi_ver_range_type mvr_int;
					req_child.range().intersection(it->range(), mvr_int);
					request req_int(req_child.name(), mvr_int);
					std::string ri_str = util::to_str(req_int);

					graph_edge e(EDGE_REDUCE, r_str, ri_str);
					lwd.m_graph->add_edge(e);
					graph_edge e2(EDGE_REDUCE, req_child_str, ri_str);
					lwd.m_graph->add_edge(e2);
				}

				request_data_map::iterator it2 = lwd.m_request_data.find(req_child.name());
				if(it2 != lwd.m_request_data.end())
					lwd.m_request_data.erase(it2);
				lwd.m_rl.add(req_child);
			}
		}
	}

	return true;
}


resolve_context::object_cache* resolve_context::get_object_cache(const std::string& object_name)
{
	typedef std::set<object_source::object_handle> object_handle_set;

	// see if cache already loaded
	object_cache_map::iterator it = m_objects_cache.find(object_name);
	if(it != m_objects_cache.end())
		return &(it->second);

	// see if object already known not to exist
	if(m_nonexistent_objects.find(object_name) != m_nonexistent_objects.end())
		return 0;

	if(print_caching())
		std::cout << "[cache] Loading object handles for '" << object_name << "'..." << std::endl;

	// load object handles
	object_handle_set obj_handles;
	m_obj_src->get_object_handles(object_name, obj_handles);
	if(obj_handles.empty())
	{
		m_nonexistent_objects.insert(m_nonexistent_objects.end(), object_name);
		if(print_caching())
		{
			std::cout << m_colors[_Y] << "[cache] Object '" << object_name << "' not found"
				<< m_colors[_Z] <<  std::endl;
		}
		return 0;
	}

	if(print_caching())
	{
		const version_type v_ge = obj_handles.begin()->m_version;
		version_type v_lt = obj_handles.rbegin()->m_version.get_next();
		ver_range_type vr(v_ge, v_lt);
		std::cout << "[cache] Found " << obj_handles.size()
			<< " versions within version range '" << vr << "'" << std::endl;
	}

	it = m_objects_cache.insert(object_cache_map::value_type(object_name, object_cache())).first;
	object_cache& ocache = it->second;
	ocache.m_object_name = object_name;

	// store handle data into cache, but not actual objects (they load lazily)
	for(object_handle_set::iterator it2=obj_handles.begin(); it2!=obj_handles.end(); ++it2)
	{
		ocache.m_object_blind_data.insert(
			object_handle_blind_map::value_type(it2->m_version, it2->m_blind));
	}

	return &(it->second);
}


bool resolve_context::get_object_versions(resolve_context::object_cache* ocache,
	const multi_ver_range_type& mvr, multi_ver_range_type* normalised_mvr,
	std::vector<version_type>* versions) //, resolve_context::request_normalise_clipping_mode clip_mode)
{
	assert(ocache);
	assert(normalised_mvr || versions);

	if(normalised_mvr)	normalised_mvr->set_empty();
	if(versions)		versions->clear();
	if(mvr.is_empty())	return false;

	version_type v_ge = mvr.begin()->ge();
	version_type v_lt = mvr.rbegin()->lt();
	object_handle_blind_map::const_iterator it_end = ocache->m_object_blind_data.end();
	object_handle_blind_map::const_iterator it = ocache->m_object_blind_data.lower_bound(v_ge);
	unsigned int nvers = 0;

	for(; (it!=it_end) && (it->first<v_lt); ++it)
	{
		const version_type& ver = it->first;
		ver_range_type v(ver);

		if(mvr.is_superset(v))
		{
			if(versions)
				versions->push_back(ver);
			if(normalised_mvr)
				normalised_mvr->union_with(v);
			++nvers;
		}
	}

	return (nvers > 0);

	/*
	version_type v_ge = mvr.begin()->ge();
	version_type v_lt = mvr.rbegin()->lt();
	version_type last_ver;
	unsigned int nvers = 0;
	bool found = false;
	bool in_range = false;

	object_handle_blind_map::const_iterator it_end = ocache->m_object_blind_data.end();
	object_handle_blind_map::const_iterator it = ocache->m_object_blind_data.lower_bound(v_ge);

	for(; (it!=it_end) && (it->first<v_lt); ++it)
	{
		const version_type& ver = it->first;
		ver_range_type v(ver);

		if(mvr.is_superset(v))
		{
			if(versions)
				versions->push_back(ver);
			if(normalised_mvr && !in_range)
			{
				v_ge = ver;
				in_range = true;
			}

			++nvers;
			found = true;
			last_ver = ver;
		}
		else if(normalised_mvr && in_range)
		{
			version_type v_next = ((clip_mode == RNCM_CLIP_TIGHT) || (nvers == 1))?
				last_ver.get_nearest() : last_ver.get_next();

			if(ver < v_next)
				v_next = ver;

			ver_range_type v2(v_ge, v_next);
			normalised_mvr->union_with(v2);
			in_range = false;
			nvers = 0;
		}
	}

	if(normalised_mvr && in_range)
	{
		version_type v_next = ((clip_mode == RNCM_CLIP_TIGHT) || (nvers == 1))?
			last_ver.get_nearest() : last_ver.get_next();

		if((it!=it_end) && (it->first < v_next))
			v_next = it->first;

		if((clip_mode == RNCM_CLIP_TO_RANGE) && (v_lt < v_next))
			v_next = v_lt;

		ver_range_type v2(v_ge, v_next);
		normalised_mvr->union_with(v2);
	}

	return found;
	*/
}


resolve_context::object_map_range resolve_context::get_objects(
	resolve_context::object_cache* ocache, const version_type& v)
{
	assert(ocache);
	assert(ocache->m_object_blind_data.find(v) != ocache->m_object_blind_data.end());

	variant_key vkey(v);
	object_map_it it = ocache->m_objects.lower_bound(vkey);
	if(it == ocache->m_objects.end())
	{
		object_source::object_handle h;
		h.m_name = ocache->m_object_name;
		h.m_version = v;
		h.m_blind = ocache->m_object_blind_data.find(v)->second;

		std::vector<object_ptr> objects;
		m_obj_src->extract_objects(h, objects);
		if(objects.empty())
		{
			CERTUS_THROW(certus_internal_error, "Failed to find objects at '" <<
				object::get_qualified_name(ocache->m_object_name, v) << "'");
		}

		for(unsigned int i=0; i<objects.size(); ++i)
		{
			variant_key vkey2(v, objects[i]->m_variant_index);
			ocache->m_objects.insert(object_map::value_type(vkey2, objects[i]));
		}

		it = ocache->m_objects.lower_bound(vkey);

		if(print_caching())
		{
			std::cout << "[cache] Loaded " << objects.size() << " objects at '" <<
				object::get_qualified_name(ocache->m_object_name, v) << "'" << std::endl;
		}
	}

	object_map_it it2 = it;
	for(; (it2!=ocache->m_objects.end()) && (it2->first.version()==v); ++it2){}
	return object_map_range(it, it2);
}


void resolve_context::process_result(const request_list& rl_top, resolve_context::lwd_cptr lwd,
	resolve_context::resolve_result& result)
{
	// graph
	if(m_settings.m_create_graph)
	{
		std::ostringstream strm;
		lwd->m_graph->render(strm, m_settings, rl_top);
		result.m_graph = strm.str();
	}

	// annotations
	if(m_settings.m_create_annotations)
	{
		result.m_annotations.clear();

		for(lwd_cptr lwd_=lwd; lwd_; lwd_=lwd_->m_parent)
		{
			result.m_annotations.insert(result.m_annotations.begin(),
				lwd_->m_annotations.begin(), lwd_->m_annotations.end());
		}
	}
}




// TESTING
void resolve_context::test(const req::request_list& rl)
{
	resolve_result result;
	resolve_status ret = resolve(rl, result);

	std::cout << "\n\nresult_code = " << ret << std::endl;
	std::cout << "result_graph = \n" << result.m_graph << std::endl;

	{
		std::ofstream f;
		f.open("./out.dot", std::ios::trunc);
		f << result.m_graph;
		f.close();
	}

	{
		std::ofstream f;
		f.open("./out.annotate", std::ios::trunc);
		unsigned int last_i = 1;
		for(std::vector<annotation_type>::const_iterator it=result.m_annotations.begin();
			it!=result.m_annotations.end(); ++it)
		{
			if(it->first != last_i)
				f << '\n';
			f << '#' << it->first << ":\t" << it->second << '\n';
			last_i = it->first;
		}
		f.close();
	}
}


} }





#include "resolve_context.h"
#include "exceptions.h"

// testing
#include <iostream>
#include <fstream>


namespace certus {

using namespace req;
using namespace obj;

namespace res {

const char* g_cols_none[] = {
	"",
	"","","",
	"","","",
};

#ifdef CERTUS_PLATFORM_LINUX
const char* g_cols[] = {
	"\033[0m", // reset
	"\033[;31m", "\033[;32m", "\033[;33m", // r,b,y
	"\033[1;31m", "\033[1;32m", "\033[1;33m", // r,b,y (bold)
};
#else
const char* g_cols[] = g_cols_none;
#endif
#define _Z 0
#define _R 1
#define _G 2
#define _Y 3
#define _RB 4
#define _GB 5
#define _YB 6


resolve_context::settings::settings()
:	m_print_warnings(true),
 	m_verbosity(100),
 	m_colored_output(true),
	m_max_attempts(0),
 	m_timeout_secs(0),
 	m_discard_nonexistent_objects(false),
 	m_throw_on_nonexistent_objects(true),
 	m_discard_nonexistent_anti_objects(true),
 	m_throw_on_nonexistent_anti_objects(false),
 	m_throw_on_nonexistent_object_versions(true)
{
}


resolve_context::resolve_context(object_source_ptr obj_src, const settings& s)
:	m_settings(s),
 	m_obj_src(obj_src),
 	m_colors(g_cols_none)
{
	if(m_settings.m_colored_output)
		m_colors = g_cols;
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
	recurse_resolve_result r = _resolve(swd, lwdp);
	lwd_cptr result_lwd = r.first;
	error_type& result_error = r.second;

	// process the result
	create_graph(rl, result_lwd, result);

	switch(result_error)
	{
	case ERROR_OK:				return RESOLVE_OK; break;
	case ERROR_MAX_ATTEMPTS:	return RESOLVE_MAX_ATTEMPTS; break;
	case ERROR_TIMEOUT:			return RESOLVE_TIMEOUT; break;
	default:					return RESOLVE_FAIL; break;
	}
}


resolve_context::recurse_resolve_result resolve_context::_resolve(
	resolve_context::shared_working_data& swd, resolve_context::lwd_cptr lwd_parent)
{
	error_type ret = ERROR_OK;
	lwd_ptr lwdp(new local_working_data(lwd_parent));
	local_working_data& lwd = *lwdp;

	if(print_resolves())
	{
		if(print_resolve_subs())
			std::cout << '\n';
		std::cout << m_colors[_GB] << "Attempting to resolve: ["
			<< lwd_parent->m_rl << "]..." << m_colors[_Z] << std::endl;
	}

	// Check for nonexistent objects.
	if(ret == ERROR_OK)
	{
		ret = filter_nonexistent_objects(lwd);
	}

	// Normalise requests. 'Normalising' means taking the request version range, and intersecting
	// it with the actual versions of the object that exist.
	if(ret == ERROR_OK)
	{
		ret = normalise_requests(lwd);
	}

	return recurse_resolve_result(lwdp, ret);
}


resolve_context::error_type resolve_context::filter_nonexistent_objects(
	resolve_context::local_working_data& lwd)
{
	if(print_resolve_subs())
		std::cout << m_colors[_G] << "Filtering nonexistent objects..." << m_colors[_Z] << std::endl;

	const local_working_data& lwd_parent(*(lwd.m_parent));
	for(request_list::const_iterator it=lwd_parent.m_rl.begin(); it!=lwd_parent.m_rl.end(); ++it)
	{
		const request& req = *it;

		request_data_map::const_iterator it2 = lwd_parent.m_request_data.find(req.name());
		if(it2 == lwd_parent.m_request_data.end())
		{
			// new object, attempt to load it
			object_cache* ocache = get_object_cache(req.name());
			if(!ocache)
			{
				bool discard = false;
				if(req.is_anti())
				{
					if(m_settings.m_discard_nonexistent_anti_objects)
						discard = true;
					else if(m_settings.m_throw_on_nonexistent_anti_objects)
						throw nonexistent_object_error(req.name());
				}
				else
				{
					if(m_settings.m_discard_nonexistent_objects)
						discard = true;
					else if(m_settings.m_throw_on_nonexistent_objects)
						throw nonexistent_object_error(req.name());
				}

				graph_edge e(EDGE_NONEXISTENT_OBJECT, req.name());
				lwd.m_edges.push_back(e);

				if(discard)
				{
					if(m_settings.m_print_warnings)
					{
						std::cerr
							<< m_colors[_Y] << "Request '" << req << "' references nonexistent object '"
							<< req.name() << "' and was discarded." << m_colors[_Z] << std::endl;
					}
					continue;
				}
				else
					return ERROR_NONEXISTENT_OBJECT;
			}
		}

		// forward existing request
		lwd.m_rl.add(*it);
	}

	return ERROR_OK;
}


resolve_context::error_type resolve_context::normalise_requests(
	resolve_context::local_working_data& lwd)
{
	if(print_resolve_subs())
		std::cout << m_colors[_G] << "Normalising requests..." << m_colors[_Z] << std::endl;

	std::vector<std::string> del_anti_reqs;

	// iterate over requests, normalise each against the versions in the cache
	for(request_list::const_iterator it=lwd.m_rl.begin(); it!=lwd.m_rl.end(); ++it)
	{
		const request& req = *it;
		object_cache* ocache = get_object_cache(req.name(), true);
		multi_ver_range_type normalised_mvr;

		if(ocache->m_all_versions.discrete_intersection(req.range(), normalised_mvr))
		{
			// normalise the request
			request normalised_req(req.name(), normalised_mvr);
			lwd.m_rl.add(normalised_req, true);

			graph_edge e(EDGE_NORMALISE, req.name());
			lwd.m_edges.push_back(e);

			if(print_resolve_subs())
				std::cout << "'" << req << "' -> '" << normalised_req << "'" << std::endl;
		}
		else
		{
			// request does not contain existing object version(s)
			graph_edge e(EDGE_NONEXISTENT_OBJECT_VERSION, req.name());
			lwd.m_edges.push_back(e);

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
					return ERROR_NONEXISTENT_OBJECT_VERSION;
			}
		}
	}

	for(unsigned int i=0; i<del_anti_reqs.size(); ++i)
		lwd.m_rl.remove(del_anti_reqs[i]);

	return ERROR_OK;
}


resolve_context::object_cache* resolve_context::get_object_cache(const std::string& object_name,
	bool expected)
{
	typedef std::set<object_source::object_handle> object_handle_set;

	// see if cache already loaded
	object_cache_map::iterator it = m_objects_cache.find(object_name);
	if(it != m_objects_cache.end())
		return &(it->second);

	if(expected)
		CERTUS_THROW(certus_internal_error, "Expected object cache: " << object_name);

	if(print_caching())
		std::cout << "Loading object handles for '" << object_name << "'..." << std::endl;

	// load object handles
	object_handle_set obj_handles;
	m_obj_src->get_object_handles(object_name, obj_handles);
	if(obj_handles.empty())
		return 0;

	if(print_caching())
	{
		const version_type v_ge = obj_handles.begin()->m_version;
		version_type v_lt = obj_handles.rbegin()->m_version.get_nearest();
		ver_range_type vr(v_ge, v_lt);
		std::cout << "Found " << obj_handles.size()
			<< " versions within version range '" << vr << "'" << std::endl;
	}

	it = m_objects_cache.insert(object_cache_map::value_type(object_name, object_cache())).first;
	object_cache& ocache = it->second;
	ocache.m_object_name = object_name;

	// store handle data into cache, but not actual objects (they load lazily)
	for(object_handle_set::iterator it2=obj_handles.begin(); it2!=obj_handles.end(); ++it2)
	{
		ocache.m_all_versions.union_with(ver_range_type(it2->m_version, true));

		ocache.m_object_blind_data.insert(
			object_handle_blind_map::value_type(it2->m_version, it2->m_blind));
	}

	return &(it->second);
}


void resolve_context::create_graph(const request_list& rl_top, resolve_context::lwd_cptr lwd,
	resolve_context::resolve_result& result)
{
	typedef std::pair<request,request> edge;
	typedef std::map<edge, edge_type> edge_map;

	std::ostringstream strm;
	strm << "digraph g {\n";

	// insert topmost requests
	for(request_list::const_iterator it=rl_top.begin(); it!=rl_top.end(); ++it)
		strm << "  \"" << *it << "\" [shape=box];\n";
	strm << '\n';

	// iterate lwds from bottom up, gathering graph edges
	edge_map g;
	for(lwd_cptr lwd_=lwd; lwd->m_parent; lwd=lwd->m_parent)
	{
		const local_working_data& lwd_this = *(lwd_);
		const local_working_data& lwd_parent = *(lwd_this.m_parent);
		const graph_edge_vector& edges = lwd_this.m_edges;

		for(graph_edge_vector::const_iterator it=edges.begin(); it!=edges.end(); ++it)
		{
			const request& r = *(lwd_parent.m_rl.find(it->m_object_name));

			if(	(it->m_type == EDGE_NONEXISTENT_OBJECT) ||
				(it->m_type == EDGE_NONEXISTENT_OBJECT_VERSION) )
			{
				std::string child_node;
				if(it->m_type == EDGE_NONEXISTENT_OBJECT)
					child_node = "NONEXISTENT_OBJECT";
				else if(it->m_type == EDGE_NONEXISTENT_OBJECT_VERSION)
					child_node = "NONEXISTENT_OBJECT_VERSION";

				strm << "  \"" << r << "\" -> \"" << child_node << "\" ;\n";
			}
			else
			{
				std::string edge_label;
				if(it->m_type == EDGE_NORMALISE)
					edge_label = "norm";

				const request& r_child = *(lwd_this.m_rl.find(it->m_object_name));
				strm << "  \"" << r << "\" -> \"" << r_child << "\" [label=\"" << edge_label << "\"];\n";
			}
		}
	}

	strm << '}';
	result.m_graph = strm.str();
}


/*
object_ptr resolve_context::get_object(object_cache& ocache, const variant_key& k)
{

}

resolve_context::object_variants* resolve_context::get_object_variants(
	resolve_context::object_cache& ocache, const version_type& v)
{
	object_handle_blind_map::iterator it = ocache.m_object_blind_data.find(v);
	if(it == ocache.m_object_blind_data.end())
	{
		throw certus_internal_error("Nonexistent version asked for from object cache");
		return 0;
	}

	object_map::iterator it2 = ocache.m_objects.find(v);
	if(it2 == ocache.m_objects.end())
	{
		object_source::object_handle h;
		h.m_name = ocache.m_object_name;
		h.m_version = v;
		h.m_blind = it->second;

		std::vector<object_ptr> objects;
		m_obj_src->extract_objects(h, objects);
		if(objects.empty())
		{
			throw certus_internal_error("Failed to find objects for an object handle");
			return 0;
		}
		else
		{
			it2 = ocache.m_objects.insert(object_map::value_type(v, object_variants())).first;
			object_variants& objv = it2->second;
			for(unsigned int i=0; i<objects.size(); ++i)
			{
				objv.m_objects.insert(std::map<int, object_ptr>::value_type(
					objects[i]->m_variant_index, objects[i]));
			}
		}
	}

	return &(it2->second);
}


resolve_context::error_type resolve_context::normalise_request(request& r)
{
	object_map* pm = get_object_map(r.name());
	if(!pm)
		return ERROR_NONEXISTENT_OBJECT;

	return ERROR_OK;
}
*/


// TESTING
void resolve_context::test(const req::request_list& rl)
{
	resolve_result result;
	resolve_status ret = resolve(rl, result);

	std::cout << "\n\nresult_code = " << ret << std::endl;
	std::cout << "result_graph = \n" << result.m_graph << std::endl;

	std::ofstream f;
	f.open("./out.dot", std::ios::trunc);
	f << result.m_graph;
	f.close();


	/*
	typedef object_source::object_handle handle_type;
	typedef std::set<handle_type> handles_set;
	typedef boost::property_tree::ptree ptree;

	std::cout << "\nTESTING RESOLVE" << std::endl;

	std::vector<std::string> paths;
	paths.push_back("/home/allanjohns/install/certus/pkgs");
	filesystem_object_source obj_src(paths, "package.json");

	handles_set vhandles;
	obj_src.get_object_handles("foo", vhandles);
	std::cout << "Versions: ";
	for(handles_set::iterator it=vhandles.begin(); it!=vhandles.end(); ++it)
		std::cout << it->m_version << ' ';
	std::cout << std::endl << std::endl;

	const handle_type& h = *(vhandles.begin());

	std::vector<object_ptr> objs;
	obj_src.extract_objects(h, objs);
	unsigned int i=1;
	for(std::list<object_ptr>::iterator it=objs.begin(); it!=objs.end(); ++it, ++i)
	{
		std::cout << "\nOBJECT #" << i << ":\n";
		std::cout << *(*it) << std::endl;
	}
	*/
}


} }





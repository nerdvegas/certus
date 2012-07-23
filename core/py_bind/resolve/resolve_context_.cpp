#include "util/util.h"
#include "resolve/resolve_context.h"
#include "boost_python/raw_constructor.hpp"

// temp
#include "object/filesystem_object_source.h"

using namespace certus;
using namespace certus::obj;
using namespace certus::res;


template<typename T>
void _get_karg(bp::dict& kargs, T& val, const char* key)
{
	if(kargs.has_key(key))
		val = bp::extract<T>(kargs[key]);
}


resolve_context* rawInit(bp::tuple args, bp::dict kargs)
{
	// temp, until we bind filesystem_object_source
	std::vector<std::string> paths;
	paths.push_back("/home/allanjohns/install/certus/pkgs");
	boost::shared_ptr<filesystem_object_source> obj_src(
		new filesystem_object_source(paths, "package.json"));

	resolve_settings s;
	_get_karg(kargs, s.m_print_warnings,						"print_warnings");
	_get_karg(kargs, s.m_verbosity, 							"verbosity");
	_get_karg(kargs, s.m_colored_output, 						"colored_output");
	_get_karg(kargs, s.m_max_attempts, 							"max_attempts");
	_get_karg(kargs, s.m_timeout_secs, 							"timeout_secs");

	_get_karg(kargs, s.m_discard_nonexistent_objects, 			"discard_nonexistent_objects");
	_get_karg(kargs, s.m_throw_on_nonexistent_objects, 			"throw_on_nonexistent_objects");
	_get_karg(kargs, s.m_abort_on_nonexistent_objects, 			"abort_on_nonexistent_objects");

	_get_karg(kargs, s.m_discard_nonexistent_anti_objects, 		"discard_nonexistent_anti_objects");
	_get_karg(kargs, s.m_throw_on_nonexistent_anti_objects, 	"throw_on_nonexistent_anti_objects");
	_get_karg(kargs, s.m_abort_on_nonexistent_anti_objects, 	"abort_on_nonexistent_anti_objects");

	_get_karg(kargs, s.m_throw_on_nonexistent_object_versions, 	"throw_on_nonexistent_object_versions");
	_get_karg(kargs, s.m_abort_on_nonexistent_object_versions, 	"abort_on_nonexistent_object_versions");

	_get_karg(kargs, s.m_create_graph, 							"create_graph");
	_get_karg(kargs, s.m_create_annotations, 					"create_annotations");
	_get_karg(kargs, s.m_graph_group_requests, 					"graph_group_requests");
	_get_karg(kargs, s.m_graph_show_conflict_removal, 			"graph_show_conflict_removal");

	resolve_context* prc = new resolve_context(obj_src, s);
	return prc;
}


void _export_resolve_context()
{
    bp::class_<resolve_context> cl("resolve_context");
    cl
    //.def(bp::init<>())
    .def("__init__", bp::raw_constructor(rawInit, 0))
    .def("test", &resolve_context::test)
    ;
}

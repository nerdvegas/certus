#include "resolve_settings.h"

namespace certus { namespace res {


resolve_settings::resolve_settings()
:	m_print_warnings(true),
 	m_verbosity(100),
 	m_colored_output(true),

	m_max_attempts(0),
 	m_timeout_secs(0),

 	m_discard_nonexistent_objects(false),
 	m_throw_on_nonexistent_objects(false),
 	m_abort_on_nonexistent_objects(true),

 	m_discard_nonexistent_anti_objects(true),
 	m_throw_on_nonexistent_anti_objects(false),
	m_abort_on_nonexistent_anti_objects(false),

 	m_throw_on_nonexistent_object_versions(false),
 	m_abort_on_nonexistent_object_versions(true),

 	m_create_graph(true),
 	m_create_annotations(true),
 	m_graph_group_requests(false),
 	m_graph_show_conflict_removal(true)
{
}


} }


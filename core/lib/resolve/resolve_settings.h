#ifndef _CERTUS_RES_RESOLVE_SETTINGS__H_
#define _CERTUS_RES_RESOLVE_SETTINGS__H_

#include "object/object_source.h"


namespace certus { namespace res {

	struct resolve_settings : public obj::object_source::settings
	{
		resolve_settings();

		//######################################################################################
		// Settings that affect what gets printed out during a resolve
		//######################################################################################

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

		//######################################################################################
		// Settings that affect the resolve process itself
		//######################################################################################

		// number of resolve attempts until the resolve aborts, no limit if zero
		unsigned int m_max_attempts;

		// number of seconds until resolve aborts, no limit if zero
		unsigned int m_timeout_secs;

		// How to handle requests for nonexistent objects. If discard is true, the request is
		// silently removed; if throw is true, an exception is thrown immediately; if abort is
		// true, the resolve fails. If none are true then the algorithm will continue to search
		// for a valid resolve, but the attempt involving the nonexistent object is discarded.
		bool m_discard_nonexistent_objects;
		bool m_throw_on_nonexistent_objects; // ignored if discard is true
		bool m_abort_on_nonexistent_objects; // ignored if throw is true

		bool m_discard_nonexistent_anti_objects;
		bool m_throw_on_nonexistent_anti_objects; // ignored if discard is true
		bool m_abort_on_nonexistent_anti_objects; // ignored if throw is true

		// How to handle requests for nonexistent object versions. If throw is true, an exception
		// is thrown immediately; if abort is true, the resolve fails. If neither are true then
		// the algorithm will continue to search for a valid resolve, but the attempt involving
		// the nonexistent object version is discarded.
		bool m_throw_on_nonexistent_object_versions;
		bool m_abort_on_nonexistent_object_versions; // ignored if throw is true

		//######################################################################################
		// Settings that affect the information returned in the result (graph etc)
		//######################################################################################

		// if true, resolve graph is generated
		bool m_create_graph;

		// if true, annotations are generated
		bool m_create_annotations;

		// group the request nodes together in the graph
		bool m_graph_group_requests;

		// include detailed info in the graph showing which objects caused others to be
		// removed due to causing a request conflict.
		bool m_graph_show_conflict_removal;
	};

} }

#endif

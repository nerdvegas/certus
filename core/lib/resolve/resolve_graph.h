#ifndef _CERTUS_RES_RESOLVE_GRAPH__H_
#define _CERTUS_RES_RESOLVE_GRAPH__H_

#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>
#include <iostream>
#include "resolve_settings.h"
#include "request/request_list.h"


namespace certus { namespace res {


	/*
	 * types of edges in the resolve graph.
	 */
	enum edge_type
	{
		EDGE_NORMALISE = 0,
		EDGE_DISCARD,
		EDGE_REQUIRES,
		EDGE_RESOLVE,
		EDGE_REDUCE,
		EDGE_CONFLICT,
		EDGE_PASSIVE_CONFLICT,
		EDGE_NONEXISTENT_OBJECT,
		EDGE_NONEXISTENT_ANTI_OBJECT,
		EDGE_NONEXISTENT_OBJECT_VERSION,
		EDGE_NONEXISTENT_ANTI_OBJECT_VERSION
	};

	/*
	 * Types of groups (subgraphs) in the resolve graph.
	 */
	enum group_type
	{
		GROUP_FULL_CONFLICT = 0
	};

	/*
	 * An edge in the resolve graph
	 */
	struct graph_edge
	{
		graph_edge(edge_type etype, const std::string& parent, const std::string& child = "")
		: 	m_type(etype),
			m_parent(parent),
			m_child(child),
			m_annotation_index(0),
			m_num_discarded_objects(0),
			m_num_discarded_versions(0){}

		edge_type m_type;
		std::string m_parent, m_child;
		std::string m_parent_field, m_child_field;
		unsigned int m_annotation_index;

		// custom edgetype-specific data
		unsigned int m_num_discarded_objects;
		unsigned int m_num_discarded_versions;
	};

	/*
	 * A group in the resolve graph
	 */
	struct graph_group
	{
		graph_group(group_type gtype, const std::string& label)
		: m_type(gtype), m_label(label){}

		group_type m_type;
		std::string m_label;
		std::vector<std::string> m_nodes;
	};

	// fwd decl
	class resolve_graph;
	typedef boost::shared_ptr<resolve_graph> resolve_graph_ptr;


	/*
	 * The resolve graph
	 */
	class resolve_graph
	{
	public:
		resolve_graph(resolve_graph_ptr parent = resolve_graph_ptr())
		: m_parent(parent){}

		void add_edge(const graph_edge& e);

		void add_group(const graph_group& g);

		bool exists(const std::string& node) const;

		void render(std::ostream& s, const resolve_settings& rset,
			const req::request_list& top_nodes) const;

	protected:

		typedef std::vector<graph_edge> 	graph_edge_vector;
		typedef std::vector<graph_group> 	graph_group_vector;

		resolve_graph_ptr m_parent;
		graph_edge_vector m_edges;
		graph_group_vector m_groups;
	};

} }

#endif





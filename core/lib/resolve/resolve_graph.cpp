#include <map>
#include <set>
#include "resolve_graph.h"
#include "util/algorithm.h"
#include "util/io_utils.h"

using namespace certus::req;


namespace certus { namespace res {


void resolve_graph::add_edge(const graph_edge& e)
{
	m_edges.push_back(e);
}


void resolve_graph::add_group(const graph_group& g)
{
	m_groups.push_back(g);
}


bool resolve_graph::exists(const std::string& node) const
{
	if(m_parent && (m_parent->exists(node)))
		return true;

	for(graph_edge_vector::const_iterator it=m_edges.begin(); it!=m_edges.end(); ++it)
	{
		if((it->m_parent == node) || (it->m_child == node))
			return true;
	}
	return false;
}


struct _error_graph_node_settings
{
	_error_graph_node_settings(){}
	_error_graph_node_settings(const std::string& name_, const std::string& label_, bool err_)
	: node_name(name_), label(label_), is_error(err_), visited(false){}

	std::string node_name;
	std::string label;
	bool is_error;
	bool visited;
};


template<typename T>
std::string _escape_dot_str(const T& t)
{
	static const char* verpipe_uchar = "&#8214;";

	std::string escaped_txt = util::to_str(t);
	escaped_txt = pystring::replace(escaped_txt, "<", "\\<");
	escaped_txt = pystring::replace(escaped_txt, "|", verpipe_uchar);
	return escaped_txt;
}

void resolve_graph::render(std::ostream& s, const resolve_settings& rset,
	const request_list& top_nodes) const
{
	typedef std::map<std::string, std::vector<std::string> > 	node_field_map;
	typedef std::map<edge_type, _error_graph_node_settings> 	error_node_settings_map;
	typedef _error_graph_node_settings							_egns;

	static const char* request_color				= "darkslategray1";
	static const char* request_bgcolor				= "lightcyan";
	static const char* warning_color				= "yellow";
	static const char* error_color					= "orangered";
	static const char* node_color					= "gray93";
	static const char* resolved_node_color			= "green1";
	static const char* group_full_conflict_color 	= "tomato";
	static const char* annotate_uchar				= "&#8224;";
	static unsigned int edge_font_size				= 10;
	static unsigned int conflict_font_size			= 12;

	error_node_settings_map error_settings;
	error_settings[EDGE_NONEXISTENT_OBJECT] 				= _egns("unknown_object",					"unknown\\nobject",						rset.m_abort_on_nonexistent_objects);
	error_settings[EDGE_NONEXISTENT_ANTI_OBJECT] 			= _egns("unknown_anti_object",				"unknown\\nobject",						rset.m_abort_on_nonexistent_anti_objects);
	error_settings[EDGE_NONEXISTENT_OBJECT_VERSION] 		= _egns("nonexistent_object_version",		"nonexistent\\nobject version(s)",		rset.m_abort_on_nonexistent_object_versions);
	error_settings[EDGE_NONEXISTENT_ANTI_OBJECT_VERSION]	= _egns("nonexistent_anti_object_version",	"nonexistent\\nobject version(s)",		false);

	if(rset.m_abort_on_nonexistent_objects == rset.m_abort_on_nonexistent_anti_objects)
		error_settings[EDGE_NONEXISTENT_ANTI_OBJECT] = error_settings[EDGE_NONEXISTENT_OBJECT];

	if(!rset.m_abort_on_nonexistent_object_versions)
		error_settings[EDGE_NONEXISTENT_OBJECT_VERSION] = error_settings[EDGE_NONEXISTENT_ANTI_OBJECT_VERSION];

	s << "digraph g {\n";

	// insert topmost request nodes
	if(rset.m_graph_group_requests)
	{
		s << "  subgraph cluster_request {\n"
			<< "  style=\"rounded,filled\";\n"
			<< "  fillcolor=" << request_bgcolor << '\n';
	}

	for(request_list::const_iterator it=top_nodes.begin(); it!=top_nodes.end(); ++it)
	{
		std::string label = _escape_dot_str(*it);
		s << "  \"" << *it << "\" [label=\"" << label
			<< "\" shape=box style=\"rounded,filled\" "
			<< "fillcolor=" << request_color << "];\n";
	}

	if(rset.m_graph_group_requests)
		s << "  }\n";
	s << '\n';

	// insert intermediary nodes; collect annotations
	node_field_map node_fields;
	std::set<std::string> nodes;
	std::set<std::string> resolved_nodes;

	for(const resolve_graph* pg=this; pg; pg=(m_parent)? m_parent.get() : 0)
	{
		const graph_edge_vector& edges = pg->m_edges;
		for(graph_edge_vector::const_reverse_iterator it=edges.rbegin(); it!=edges.rend(); ++it)
		{
			const std::string* req_str[2] = { &(it->m_child), &(it->m_parent) };
			const std::string* field[2] = { &(it->m_child_field), &(it->m_parent_field) };
			for(unsigned int i=0; i<2; ++i)
			{
				const std::string& r_str = *(req_str[i]);
				if(!r_str.empty())
				{
					nodes.insert(nodes.end(), r_str);
					if(!field[i]->empty())
					{
						std::vector<std::string>& fields = node_fields[r_str];
						fields.push_back(*(field[i]));
					}
				}
			}

			if(it->m_type == EDGE_RESOLVE)
				resolved_nodes.insert(resolved_nodes.end(), it->m_child);
		}
	}

	// record nodes
	std::vector<std::string> single_field_nodes;
	for(node_field_map::iterator it=node_fields.begin(); it!=node_fields.end(); ++it)
	{
		if(it->second.size() == 1)
		{
			// one field, demote to normal node
			std::string label = _escape_dot_str(*(it->second.begin()));
			s << "  \"" << it->first << "\" ["
				<< " label=\"" << label << '\"'
				<< " style=filled fillcolor=" << node_color << "];\n";

			single_field_nodes.push_back(it->first);
			continue;
		}

		// ensure record fields are listed in the order they were defined
		std::reverse(it->second.begin(), it->second.end());
		it->second = util::make_unique(it->second);

		std::vector<std::string> field_texts;
		unsigned int j = 0;

		for(std::vector<std::string>::const_iterator it2=it->second.begin();
			it2!=it->second.end(); ++it2, ++j)
		{
			std::ostringstream strm;
			std::string escaped_field_text = _escape_dot_str(*it2);
			strm << "<f" << j << '>' << escaped_field_text;
			field_texts.push_back(strm.str());
		}

		std::string flabel = pystring::join("|", field_texts);
		s << "  \"" << it->first << "\" ["
			<< " label=\"" << flabel << "\""
			<< " shape=record style=\"rounded,filled\""
			<< " fillcolor=" << node_color << "];\n";
	}

	// leaf resolved nodes
	for(std::set<std::string>::const_iterator it=resolved_nodes.begin(); it!=resolved_nodes.end(); ++it)
	{
		nodes.erase(*it);
		std::string label = _escape_dot_str(*it);
		s << "  \"" << *it << "\" ["
			<< " label=\"" << label << '\"'
			<< " shape=box style=\"rounded,filled\" "
			<< "fillcolor=" << resolved_node_color << "];\n";
	}

	// normal nodes
	for(request_list::const_iterator it=top_nodes.begin(); it!=top_nodes.end(); ++it)
	{
		std::string req_str = util::to_str(*it);
		nodes.erase(req_str);
	}

	for(std::set<std::string>::const_iterator it=nodes.begin(); it!=nodes.end(); ++it)
	{
		if(node_fields.find(*it) != node_fields.end())
			continue;
		std::string label = _escape_dot_str(*it);
		s << "  \"" << *it << "\" ["
			<< " label=\"" << label << '\"'
			<< " style=filled fillcolor=" << node_color << "];\n";
	}

	for(unsigned int i=0; i<single_field_nodes.size(); ++i)
		node_fields.erase(single_field_nodes[i]);

	// iterate lwds from bottom up, gathering graph edges
	for(const resolve_graph* pg=this; pg; pg=(m_parent)? m_parent.get() : 0)
	{
		const graph_edge_vector& edges = pg->m_edges;
		for(graph_edge_vector::const_iterator it=edges.begin(); it!=edges.end(); ++it)
		{
			error_node_settings_map::iterator it2 = error_settings.find(it->m_type);
			if(it2 != error_settings.end())
			{
				_egns& e = it2->second;
				if(!e.visited)
				{
					std::string col = (e.is_error)? error_color : warning_color;
					s << "  \"" << e.node_name << "\" ["
						<< " label=\"" << e.label << '\"'
						<< " shape=box"
						<< " style=filled"
						<< " fillcolor=" << col
						<< " ];\n";
					e.visited = true;
				}

				s << "  \"" << it->m_parent << "\" -> \"" << e.node_name << "\";\n";
			}
			else
			{
				std::string edge_label;
				std::string edge_color = "black";
				std::string edge_style = "solid";
				std::string arrowhead = "normal";
				unsigned int edge_font_sz = edge_font_size;

				switch(it->m_type)
				{
				case EDGE_NORMALISE:
					edge_label = "norm";
					break;
				case EDGE_CONFLICT:
					edge_label = "CONFLICT";
					edge_style = "bold";
					edge_font_sz = conflict_font_size;
					break;
				case EDGE_PASSIVE_CONFLICT:
					edge_label = "conflict";
					edge_style = "dashed";
					arrowhead = "empty";
					break;
				case EDGE_DISCARD:
					edge_label = "discard";
					break;
				case EDGE_REQUIRES:
					edge_label = "requires";
					break;
				case EDGE_REDUCE:
					edge_label = "reduce";
					edge_style = "dashed";
					break;
				case EDGE_RESOLVE:
					edge_label = "resolve";
				default:
					break;
				}

				const std::string* node[2] = { &(it->m_parent), &(it->m_child) };
				const std::string* field[2] = { &(it->m_parent_field), &(it->m_child_field) };
				std::string field_ref[2];

				for(unsigned int i=0; i<2; ++i)
				{
					node_field_map::const_iterator it3=node_fields.find(*(node[i]));
					if(it3 != node_fields.end())
					{
						if(field[i]->empty())
							field_ref[i] = ":<f0>";
						else
						{
							unsigned int j = std::find(it3->second.begin(),
								it3->second.end(), *(field[i])) - it3->second.begin();
							std::ostringstream strm;
							strm << ":<f" << j << '>';
							field_ref[i] = strm.str();
						}
					}
				}

				if(it->m_num_discarded_versions > 0)
				{
					std::ostringstream strm;
					strm << edge_label << '(' << it->m_num_discarded_versions;
					if(it->m_num_discarded_versions != it->m_num_discarded_objects)
						strm << '[' << it->m_num_discarded_objects << ']';
					strm << ')';
					edge_label = strm.str();
				}

				if(rset.m_create_annotations && (it->m_annotation_index > 0))
				{
					std::ostringstream strm;
					strm << edge_label << "\\n" << annotate_uchar << it->m_annotation_index;
					edge_label = strm.str();
				}

				s << "  "
					<< '\"' << *(node[0]) << '\"' << field_ref[0] << " -> "
					<< '\"' << *(node[1]) << '\"' << field_ref[1] << " ["
					<< " label=\"" << edge_label << '\"'
					<< " style=" << edge_style
					<< " color=" << edge_color
					<< " fontcolor=" << edge_color
					<< " arrowhead=" << arrowhead
					<< " fontsize=" << edge_font_sz
					<< " ];\n";
			}
		}
	}

	// groups
	unsigned int i = 0;
	for(graph_group_vector::const_iterator it=m_groups.begin(); it!=m_groups.end(); ++it, ++i)
	{
		std::string col = group_full_conflict_color;

		s << "  subgraph cluster_" << i << " {\n"
			<< "    label=\"" << it->m_label << "\";\n"
			<< "    style=\"filled,rounded\";\n"
			<< "    fillcolor=" << col << ";\n";

		for(unsigned int j=0; j<it->m_nodes.size(); ++j)
			s << "    \"" << it->m_nodes[j] << "\";\n";

		s << "  }\n";
	}

	s << '}';
}


} }









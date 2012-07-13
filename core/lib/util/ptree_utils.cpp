#include <sstream>
#include "ptree_utils.h"
#include "pystring.h"


namespace certus { namespace util {


bp::ptree::assoc_iterator find_(bp::ptree& pt, const std::string& key, bool get_first)
{
	typedef bp::ptree::assoc_iterator a_it;
	std::pair<a_it,a_it> p = pt.equal_range(key);

	if(get_first || (p.first == pt.not_found()))
		return p.first;
	else
		return --p.second;
}


bp::ptree::const_assoc_iterator c_find_(const bp::ptree& pt, const std::string& key, bool get_first)
{
	return find_(const_cast<bp::ptree&>(pt), key, get_first);
}


bool is_list(const boost::property_tree::ptree& pt)
{
	if(!pt.data().empty())
		return false;

	for(bp::ptree::const_iterator it=pt.begin(); it!=pt.end(); ++it)
	{
		if(!it->first.empty())
			return false;
	}
	return true;
}


void filter(const bp::ptree& pt, const std::string& key, bp::ptree& result)
{
	typedef bp::ptree::const_assoc_iterator ca_it;

	result.clear();
	result.data() = pt.data();

	std::pair<ca_it,ca_it> p = pt.equal_range(key);
	for(; p.first!=p.second; ++p.first)
		result.push_back(*(p.first));
}


bool get_string_list(const boost::property_tree::ptree& pt, std::vector<std::string>& seq)
{
	typedef bp::ptree::const_iterator c_it;
	std::vector<std::string> seq2;

	for(c_it it=pt.begin(); it!=pt.end(); ++it)
	{
		if(!it->first.empty() || !it->second.empty())
			return false;
		std::string stripped = pystring::strip(it->second.data());
		seq2.push_back(stripped);
	}

	seq.insert(seq.end(), seq2.begin(), seq2.end());
	return true;
}


bool get_key_string_list(const bp::ptree& pt, const std::string& key,
	std::vector<std::string>& seq)
{
	typedef bp::ptree::const_assoc_iterator ca_it;

	std::vector<std::string> seq2;
	std::pair<ca_it,ca_it> p = pt.equal_range(key);

	for(; p.first!=p.second; ++p.first)
	{
		const bp::ptree& ptchild = (p.first)->second;
		if(!get_string_list(ptchild, seq2))
			return false;
	}

	seq.insert(seq.end(), seq2.begin(), seq2.end());
	return true;
}


void add_string_list(bp::ptree& pt, const std::vector<std::string>& seq, bool append)
{
	typedef bp::ptree::iterator 	_it;
	typedef bp::ptree::value_type	pt_value_type;

	_it it = (append)? pt.end() : pt.begin();
	for(unsigned int i=0; i<seq.size(); ++i)
		pt.insert(it, pt_value_type("", bp::ptree(seq[i])));
}


void add_key_string_list(bp::ptree& pt, const std::string& key,
	const std::vector<std::string>& seq, bool append)
{
	typedef bp::ptree::assoc_iterator 	a_it;
	typedef bp::ptree::value_type		pt_value_type;

	a_it it = find_(pt, key, !append);
	if(it == pt.not_found())
	{
		pt_value_type v(key, bp::ptree());
		add_string_list(v.second, seq, true);
		pt.push_back(v);
	}
	else
		add_string_list(it->second, seq, append);
}


void print_ptree(std::ostream& os, const boost::property_tree::ptree& pt, int depth)
{
	typedef bp::ptree::const_iterator c_it;

	if(pt.empty())
		os << "'" << pt.data() << "'\n";
	else
	{
		std::string pad("");
		pad.assign(depth*4,' ');
		++depth;
		std::string pad2 = pad + "    ";

		if(is_list(pt))
		{
			os << "[\n";
			for(c_it it=pt.begin(); it!=pt.end(); ++it)
			{
				os << pad2;
				print_ptree(os, it->second, depth);
			}
			os << pad << "]\n";
		}
		else
		{
			os << "{\n";
			for(c_it it=pt.begin(); it!=pt.end(); ++it)
			{
				os << pad2 << "'" << it->first << "': ";
				print_ptree(os, it->second, depth);
			}
			os << pad << "}\n";
		}
	}
}


std::ostream& operator<<(std::ostream& os, const boost::property_tree::ptree& pt)
{
	print_ptree(os, pt, 0);
	return os;
}


std::string as_string(const boost::property_tree::ptree& pt)
{
	std::ostringstream strm;
	strm << pt;
	return strm.str();
}


} }










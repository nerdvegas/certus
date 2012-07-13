#ifndef _CERTUS_UTIL_PTREE__H_
#define _CERTUS_UTIL_PTREE__H_

#include <string>
#include <vector>
#include <iostream>
#include <boost/property_tree/ptree.hpp>


/*
 * Various boost.ptree utility functions
 */

namespace certus { namespace util {

	namespace bp = boost::property_tree;

	/*
	 * Like ptree::find but with order guarantees when there are duplicate keys.
	 */
	bp::ptree::assoc_iterator find_(bp::ptree& pt, const std::string& key, bool get_first = true);
	bp::ptree::const_assoc_iterator c_find_(const bp::ptree& pt, const std::string& key, bool get_first = true);

	/*
	 * Return true if this subtree represents a list.
	 * @note Seq is appended.
	 */
	bool is_list(const bp::ptree& pt);

	/*
	 * Create a second ptree containing only the matching keys from the first
	 */
	void filter(const bp::ptree& pt, const std::string& key, bp::ptree& result);

	/*
	 * If this subtree represents a list, extract a list of strings and return true. If any element
	 * in the list is not a string, false is returned.
	 * @note Seq is appended.
	 */
	bool get_string_list(const bp::ptree& pt, std::vector<std::string>& seq);

	/*
	 * Find all values under 'key' and create a list of values. Values of identical keys are
	 * appended. Returns false if values could not be represented as a string sequence.
	 * @param split If true, strings will be split into separate elements using sep.
	 * @note Seq is appended.
	 * @note True is still returned even if the key is not found.
	 */
	bool get_key_string_list(const bp::ptree& pt, const std::string& key,
		std::vector<std::string>& seq);

	/*
	 * Add the given strings to the ptree, as new list elements.
	 */
	void add_string_list(bp::ptree& pt, const std::vector<std::string>& seq, bool append = true);

	/*
	 * Find 'key' and add the given values to the list.
	 */
	void add_key_string_list(bp::ptree& pt, const std::string& key,
		const std::vector<std::string>& seq, bool append = true);

	/*
	 * printing
	 */
	std::string as_string(const bp::ptree& pt);
	void print_ptree(std::ostream& os, const bp::ptree& pt, int depth = 0);
	std::ostream& operator<<(std::ostream& os, const bp::ptree& pt);

} }

#endif

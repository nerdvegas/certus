#ifndef _CERTUS_UTIL_ALGORITHM__H_
#define _CERTUS_UTIL_ALGORITHM__H_

#include <set>
#include <iterator>


/*
 * Std-like algorithms.
 */

namespace certus { namespace util {


	/*
	 * Return a copy of the container with duplicates removed.
	 */
	template<typename T>
	T make_unique(const T& t);

	/*
	 * In-place set intersection.
	 */
	template<typename T>
	void intersect(std::set<T>& s, const std::set<T>& t);


//////////////// impl

template<typename T>
T make_unique(const T& t)
{
	typedef typename T::value_type V;

	T u;
	std::set<V> seen;

	for(typename T::const_iterator it=t.begin(); it!=t.end(); ++it)
	{
		if(seen.find(*it) == seen.end())
		{
			std::back_inserter(u) = *it;
			seen.insert(seen.end(), *it);
		}
	}
	return u;
}


template<typename T>
void intersect(std::set<T>& s, const std::set<T>& t)
{
	typedef typename std::set<T>::iterator _it;

	_it it2;
	for(_it it=s.begin(); it!=s.end(); it=it2)
	{
		it2 = it; ++it2;
		if(t.find(*it) == t.end())
			s.erase(it);
	}
}


} }

#endif






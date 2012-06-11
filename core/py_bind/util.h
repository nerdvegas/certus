#ifndef _CERTUS_PY_UTIL__H_
#define _CERTUS_PY_UTIL__H_

#include <sstream>
#include <boost/python.hpp>


namespace bp = boost::python;

#define CERTUS_THROW(ExcType, Msg) { \
        std::stringstream strm;                        	\
        strm << Msg;                               		\
        PyErr_SetString(ExcType, strm.str().c_str());   \
        boost::python::throw_error_already_set();      	\
}

namespace certus {

	typedef bp::return_value_policy<bp::return_by_value> bp_ret_val;

	template<typename T>
    struct _str_pickle : boost::python::pickle_suite
    {
		static bp::tuple getinitargs(const T& t) {
			return bp::make_tuple(boost::lexical_cast<std::string>(t));
		}
    };

}

#endif

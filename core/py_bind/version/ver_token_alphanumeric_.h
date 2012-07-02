#ifndef _CERTUS_VER_TOKEN_ALPHANUMERIC_BINDINGS__H_
#define _CERTUS_VER_TOKEN_ALPHANUMERIC_BINDINGS__H_

#include <boost/python.hpp>
#include <boost/lexical_cast.hpp>
#include "version/version.h"
#include "version/ver_token_alphanumeric.h"
#include "util.h"


namespace certus { namespace ver {

    template<typename T>
    struct ver_token_alphanumeric_bind
    {
        typedef ver_token_alphanumeric<T> token_type;

        ver_token_alphanumeric_bind(const char *name)
        {
            boost::python::class_<token_type> cl(name,
            	boost::python::init<const std::string &>());
            cl
            .def_pickle(_str_pickle<token_type>())
            .def("__repr__",boost::lexical_cast<std::string,token_type>)
            .def("__int__",toInt)
            .def("get_next",&token_type::get_next)
            .def("get_min",&token_type::get_min)
            .def("get_max",&token_type::get_max)
            .def(boost::python::self < boost::python::self)
            .def(boost::python::self > boost::python::self)
            .def(boost::python::self <= boost::python::self)
            .def(boost::python::self >= boost::python::self)
            .def(boost::python::self == boost::python::self)
            .def(boost::python::self != boost::python::self)
            ;
        }

        static int toInt(const token_type& self)
        {
        	int result = -1;
        	if(!self.as_int(result))
        		CERTUS_THROW(PyExc_TypeError, "Version cannot be represented as an integer.");
        	return result;
        }
    };
}}

#endif

#ifndef _CERTUS_VER_VERSION_BINDINGS__H_
#define _CERTUS_VER_VERSION_BINDINGS__H_

#include "util/util.h"
#include <boost/lexical_cast.hpp>
#include "version/version.h"


namespace certus { namespace ver {

    template<typename Token>
    struct version_bind
    {
        typedef version<Token> version_type;

        version_bind(const char *name)
        {
            bp::class_<version_type> cl(name);
            cl
            .def(bp::init<const std::string&>())
            .def(bp::init<const version_type&>())
            .def_pickle(_str_pickle<version_type>())
            .def("__repr__", boost::lexical_cast<std::string,version_type>)
            .def("set", &version_type::set)
            .def("set_none", &version_type::set_none)
            .def("is_none", &version_type::is_none)
            .def("rank", &version_type::rank)
            .def("get_next",&version_type::get_next)
            .def("get_nearest",&version_type::get_nearest)
            .def("__getitem__",getItem)
            .def(bp::self < bp::self)
            .def(bp::self > bp::self)
            .def(bp::self <= bp::self)
            .def(bp::self >= bp::self)
            .def(bp::self == bp::self)
            .def(bp::self != bp::self)
            ;
        }

        static Token getItem(const version_type& self, int i)
        {
        	if((i<0) || (i>=self.rank()))
        		CERTUS_THROW(PyExc_IndexError, "Version index out of range.");
        	return self[i];
        }
    };
}}

#endif

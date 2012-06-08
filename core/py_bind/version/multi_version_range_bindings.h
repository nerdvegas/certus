#ifndef _CERTUS_VER_MULTI_VERSION_RANGE_BINDINGS__H_
#define _CERTUS_VER_MULTI_VERSION_RANGE_BINDINGS__H_

#include <boost/python.hpp>

#include <version.h>
#include <version_range.h>
#include <multi_version_range.h>

namespace certus { namespace ver {

    template<typename Token>
    struct multi_version_range_bind
    {
        typedef version<Token>              version_type;
        typedef version_range<Token>        version_range_type;
        typedef multi_version_range<Token>  multi_version_range_type;

        multi_version_range_bind(const char *name)
        {
            boost::python::class_<multi_version_range_type> cl(name);
            cl
            .def(boost::python::init<const std::string&>())
            .def("set",&multi_version_range_type::set)
            .def("set_any",&multi_version_range_type::set_any)
            .def("set_none",&multi_version_range_type::set_none)
            .def("set_empty",&multi_version_range_type::set_empty)

#ifdef TODO
            //.def("intersect",&multi_version_range_type::intersect)
            //.def("inverse",&multi_version_range_type::inverse)
#endif
            ;
        }
    };
}}

#endif

#ifndef _CERTUS_VER_VERSION_RANGE_BINDINGS__H_
#define _CERTUS_VER_VERSION_RANGE_BINDINGS__H_

#include <boost/python.hpp>

#include <version.h>
#include <version_range.h>

namespace certus { namespace ver {

    template<typename Token>
    struct version_range_bind
    {
        typedef version<Token>        version_type;
        typedef version_range<Token>  version_range_type;

        version_range_bind(const char *name)
        {
            boost::python::class_<version_range_type> cl(name);
            cl
            .def(boost::python::init<const std::string&>())
            .def("__init__",boost::python::make_constructor(defaultInit))
            .def("__init__",boost::python::make_constructor(defaultInitWithBool))
            .def(boost::python::init<const version_type&, const version_type&>())
            .def("set",&version_range_type::set)
            .def("set_any",&version_range_type::set_any)
            .def("set_none",&version_range_type::set_none)
            .def(boost::python::self < boost::python::self)
            .def(boost::python::self == boost::python::self)
            .def(boost::python::self != boost::python::self)
            .def("is_any",&version_range_type::is_any)
            .def("is_none",&version_range_type::is_none)
#ifdef TODO
            //.add_property("ge",&version_range_type::ge)
            //.add_property("lt",&version_range_type::lt)
#endif
            .def("touches",&version_range_type::touches)
            .def("union_with",&version_range_type::union_with)
            .def("intersects",&version_range_type::intersects)
            .def("intersect_with",&version_range_type::intersect_with)
            ;
        }

        static version_range_type *defaultInit(const version_type &v)
        {
            return new version_range_type(v);
        }
        static version_range_type *defaultInitWithBool(const version_type &v, bool exact)
        {
            return new version_range_type(v,exact);
        }
    };
}}

#endif

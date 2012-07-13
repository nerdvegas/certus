#ifndef _CERTUS_VER_VERSION_RANGE_BINDINGS__H_
#define _CERTUS_VER_VERSION_RANGE_BINDINGS__H_

#include "util/util.h"
#include "version/version.h"
#include "version/version_range.h"


namespace certus { namespace ver {

    template<typename Token>
    struct version_range_bind
    {
        typedef version<Token>        version_type;
        typedef version_range<Token>  version_range_type;

        version_range_bind(const char *name)
        {
            bp::class_<version_range_type> cl(name);
            cl
            .def(bp::init<const std::string&>())
            .def("__init__",bp::make_constructor(defaultInit))
            .def("__init__",bp::make_constructor(defaultInitWithBool))
            .def(bp::init<const version_type&, const version_type&>())
            .def_pickle(_str_pickle<version_range_type>())
            .add_property("lower_bound", bp::make_function(&version_range_type::ge, bp_ret_val()))
            .add_property("upper_bound", bp::make_function(&version_range_type::lt, bp_ret_val()))
            .def("__repr__", boost::lexical_cast<std::string,version_range_type>)
            .def("set",&version_range_type::set)
            .def("set_any",&version_range_type::set_any)
            .def("set_none",&version_range_type::set_none)
            .def("is_any",&version_range_type::is_any)
            .def("is_none",&version_range_type::is_none)
            .def("is_superset",&version_range_type::is_superset)
            .def("is_subset",&version_range_type::is_subset)
            .def("intersects",&version_range_type::intersects)
            .def("intersect",&version_range_type::intersect)
            .def(bp::self < bp::self)
            .def(bp::self == bp::self)
            .def(bp::self != bp::self)
            ;
        }

        static version_range_type *defaultInit(const version_type &v) {
            return new version_range_type(v);
        }

        static version_range_type *defaultInitWithBool(const version_type &v, bool exact) {
            return new version_range_type(v,exact);
        }
    };
}}

#endif

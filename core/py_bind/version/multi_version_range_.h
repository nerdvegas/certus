#ifndef _CERTUS_VER_MULTI_VERSION_RANGE_BINDINGS__H_
#define _CERTUS_VER_MULTI_VERSION_RANGE_BINDINGS__H_

#include "util/util.h"
#include "version/version.h"
#include "version/version_range.h"
#include "version/multi_version_range.h"


namespace certus { namespace ver {

    template<typename Token>
    struct multi_version_range_bind
    {
        typedef version<Token>              version_type;
        typedef version_range<Token>        version_range_type;
        typedef multi_version_range<Token>  multi_version_range_type;

        static multi_version_range_type _inverse(const multi_version_range_type& v)
        {
        	multi_version_range_type result;
        	v.inverse(result);
        	return result;
        }

        template<typename T>
        static multi_version_range_type _union_of(const multi_version_range_type& v, const T& t)
        {
        	multi_version_range_type result(v);
        	result.union_with(t);
        	return result;
        }

        template<typename T>
        static multi_version_range_type _intersection(const multi_version_range_type& v, const T& t)
        {
        	multi_version_range_type result;
        	v.intersection(t, result);
        	return result;
        }

        multi_version_range_bind(const char *name)
        {
            void(multi_version_range_type::*fn_union_with_mvr)(const multi_version_range_type&) =
            	&multi_version_range_type::union_with;
            void(multi_version_range_type::*fn_union_with_vr)(const version_range_type&) =
            	&multi_version_range_type::union_with;

            bool(multi_version_range_type::*fn_intersects_mvr)(const multi_version_range_type&) const =
            	&multi_version_range_type::intersects;
            bool(multi_version_range_type::*fn_intersects_vr)(const version_range_type&) const =
            	&multi_version_range_type::intersects;

            bool(multi_version_range_type::*fn_intersect_mvr)(const multi_version_range_type&) =
            	&multi_version_range_type::intersect;
            bool(multi_version_range_type::*fn_intersect_vr)(const version_range_type&) =
            	&multi_version_range_type::intersect;

        	bp::class_<multi_version_range_type> cl(name);
            cl
            .def(bp::init<const std::string&>())
            .def(bp::init<const version_range_type&>())
            .def_pickle(_str_pickle<multi_version_range_type>())
            .def("__repr__", boost::lexical_cast<std::string,multi_version_range_type>)
            .def("set",&multi_version_range_type::set)
            .def("set_any",&multi_version_range_type::set_any)
            .def("set_none",&multi_version_range_type::set_none)
            .def("set_empty",&multi_version_range_type::set_empty)
            .def("is_any",&multi_version_range_type::is_any)
            .def("is_none",&multi_version_range_type::is_none)
            .def("is_empty",&multi_version_range_type::is_empty)
            .def("num_ranges",&multi_version_range_type::num_ranges)
            .def("inverse", _inverse)
            .def("union_with", fn_union_with_mvr)
            .def("union_with", fn_union_with_vr)
            .def("union_of", _union_of<multi_version_range_type>)
            .def("union_of", _union_of<version_range_type>)
            .def("intersects", fn_intersects_mvr)
            .def("intersects", fn_intersects_vr)
            .def("intersect", fn_intersect_mvr)
            .def("intersect", fn_intersect_vr)
            .def("intersection", _intersection<multi_version_range_type>)
            .def("intersection", _intersection<version_range_type>)
            ;
        }
    };
}}

#endif

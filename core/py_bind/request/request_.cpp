#include "util/util.h"
#include "request/request.h"

using namespace certus;
using namespace certus::req;


void _export_request()
{
    bp::class_<request> cl("request", bp::init<const std::string&>());
    cl
    .def_pickle(_str_pickle<request>())
    .def("__repr__", boost::lexical_cast<std::string,request>)
    .def("set", &request::set)
    .def("is_anti", &request::is_anti)
    .def("set_anti", &request::set_anti)
    .add_property("name", bp::make_function(&request::name, bp_ret_val()), &request::set_name)
    .add_property("range", bp::make_function(&request::range, bp_ret_val()), &request::set_range)
    .def(bp::self == bp::self)
    .def(bp::self != bp::self)
    ;
}

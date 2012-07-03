#include "util/util.h"
#include "request/request_list.h"

using namespace certus;
using namespace certus::req;


void setContents(request_list& self, const bp::object &o)
{
	std::vector<request> requests;
	unsigned int nreq = bp::len(o);
	for(unsigned int i=0; i<nreq; ++i)
		requests.push_back(bp::extract<const request&>(o[i]));

	self.set(requests.begin(), requests.end());
}


request_list* sequenceInit(const bp::object &o)
{
	request_list* rl = new request_list();
	setContents(*rl, o);
	return rl;
}


void _export_request_list()
{
	void (request_list::*fn_add)(const request&) = &request_list::add;

    bp::class_<request_list> cl("request_list");
    cl
    .def(bp::init<const std::string&>())
    .def("__init__", bp::make_constructor(sequenceInit))
    .def_pickle(_str_pickle<request>())
    .def("__repr__", boost::lexical_cast<std::string,request_list>)
    .def("set", setContents)
    .def("add", fn_add)
    .def("remove", &request_list::remove)
    .def("clear", &request_list::clear)
    ;
}









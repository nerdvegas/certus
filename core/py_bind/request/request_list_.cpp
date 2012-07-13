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


bp::list _requests(const request_list& self)
{
	bp::list l;
	const request_list::request_list_type& reqs = self.requests();
	for(request_list::request_list_type::const_iterator it=reqs.begin(); it!=reqs.end(); ++it)
		l.append(bp::object(*it));

	return l;
}


void _add1(request_list& self, const request& r) {
	self.add(r);
}

void _add2(request_list& self, const request& r, bool replace = false) {
	self.add(r, replace);
}


void _export_request_list()
{
    bp::class_<request_list> cl("request_list");
    cl
    .def(bp::init<>())
    .def("__init__", bp::make_constructor(sequenceInit))
    .def_pickle(_str_pickle<request>())
    .def("__repr__", boost::lexical_cast<std::string,request_list>)
    .def("set", setContents)
    .def("add", _add1)
    .def("add", _add2)
    .def("remove", &request_list::remove)
    .def("clear", &request_list::clear)
    .def("empty", &request_list::empty)
    .def("requests", _requests)
    ;
}









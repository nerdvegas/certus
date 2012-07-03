#include <boost/python.hpp>

extern void _export_exceptions();
extern void _export_request();
extern void _export_request_list();


BOOST_PYTHON_MODULE(request)
{
    _export_exceptions();
    _export_request();
    _export_request_list();
}

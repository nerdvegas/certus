#include <boost/python.hpp>

extern void _export_exceptions();
extern void _export_resolve_context();


BOOST_PYTHON_MODULE(resolve)
{
	_export_exceptions();
	_export_resolve_context();
}

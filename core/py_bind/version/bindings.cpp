#include <boost/python.hpp>

extern void _export_version();
extern void _export_version_range();
extern void _export_multi_version_range();
extern void _export_exceptions();

BOOST_PYTHON_MODULE(version)
{
    _export_version();
    _export_version_range();
    _export_multi_version_range();
    _export_exceptions();
}

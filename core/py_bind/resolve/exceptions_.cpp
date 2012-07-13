#include "util/util.h"
#include "resolve/exceptions.h"


void _export_exceptions()
{
    certus::exception_bind<certus::res::nonexistent_object_error>			("NonexistentObjectError");
    certus::exception_bind<certus::res::nonexistent_object_version_error>	("NonexistentObjectVersionError");
}


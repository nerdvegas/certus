#include "exceptions_bindings.h"

PyObject *certus::ver::exception_bind::gExeType = NULL;

void _export_exceptions()
{
    certus::ver::exception_bind("InvalidVersionError");
}


#include "util/util.h"
#include "request/exceptions.h"


void _export_exceptions()
{
    certus::exception_bind<certus::req::invalid_request_error>		("InvalidRequestError");
    certus::exception_bind<certus::req::request_conflict_error>		("RequestConflictError");
}


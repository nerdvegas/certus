#include "util/util.h"
#include "version/exceptions.h"


void _export_exceptions()
{
	// todo move from certus.version module into certus
    certus::exception_bind<certus::certus_error>					("CertusError");

    certus::exception_bind<certus::ver::invalid_version_error>		("InvalidVersionError");
}


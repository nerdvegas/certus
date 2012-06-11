#include "version_.h"
#include "version/ver_token_alphanumeric.h"

void _export_version()
{
    typedef certus::ver::ver_token_alphanumeric<unsigned int> tok_type;
    certus::ver::version_bind<tok_type>("version");
}
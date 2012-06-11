#include "version_range_.h"
#include "version/ver_token_alphanumeric.h"

void _export_version_range()
{
    typedef certus::ver::ver_token_alphanumeric<unsigned int> tok_type;
    certus::ver::version_range_bind<tok_type>("version_range");
}

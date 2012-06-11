#include "multi_version_range_.h"
#include "version/ver_token_alphanumeric.h"


void _export_multi_version_range()
{
    typedef certus::ver::ver_token_alphanumeric<unsigned int> tok_type;
    certus::ver::multi_version_range_bind<tok_type>("multi_version_range");
}

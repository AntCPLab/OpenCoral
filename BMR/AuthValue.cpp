/*
 * AuthValue.cpp
 *
 */

#include "GC/Secret.h"

namespace GC
{

void AuthValue::assign(const word& value, const int128& mac_key, bool not_first_player)
{
    if (not_first_player)
        share = 0;
    else
        share = value;
#ifdef __PCLMUL__
    mac = _mm_clmulepi64_si128(_mm_cvtsi64_si128(mac_key.get_lower()), _mm_cvtsi64_si128(value), 0);
#else
    (void) mac_key;
    throw runtime_error("need to compile with PCLMUL support");
#endif
}

ostream& operator<<(ostream& o, const AuthValue& auth_value)
{
	o << hex << auth_value.share << " " << auth_value.mac;
	return o;
}

}

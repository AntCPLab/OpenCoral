/*
 * AtlasSecret.cpp
 *
 */

#include "AtlasSecret.h"
#include "TinyMC.h"

#include "Protocols/Shamir.hpp"
#include "Protocols/ShamirMC.hpp"
#include "Protocols/MAC_Check_Base.hpp"
#include "Secret.hpp"

namespace GC
{

typename AtlasSecret::MC* AtlasSecret::new_mc(typename AtlasSecret::mac_key_type mac_key)
{
    return new MC(mac_key);
}

AtlasShare::AtlasShare(const AtlasSecret& other) :
        AtlasShare(other.get_bit(0))
{
}

void AtlasShare::random()
{
    AtlasSecret tmp;
    this->get_party().DataF.get_one(DATA_BIT, tmp);
    *this = tmp.get_reg(0);
}

}

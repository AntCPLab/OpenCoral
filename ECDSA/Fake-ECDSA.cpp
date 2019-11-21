/*
 * Fake-ECDSA.cpp
 *
 */

#include "ECDSA/P256Element.h"
#include "Tools/mkpath.h"

#include "Protocols/fake-stuff.hpp"
#include "Protocols/Share.hpp"
#include "Processor/Data_Files.hpp"

int main()
{
    P256Element::init();
    P256Element::Scalar key;
    gf2n key2;
    string prefix = PREP_DIR "ECDSA/";
    mkdir_p(prefix.c_str());
    ofstream outf;
    write_online_setup_without_init(outf, prefix, P256Element::Scalar::pr(), 0);
    generate_mac_keys<Share<P256Element::Scalar>>(key, key2, 2, prefix);
    make_mult_triples<Share<P256Element::Scalar>>(key, 2, 1000, false, prefix);
    make_inverse<Share<P256Element::Scalar>>(key, 2, 1000, false, prefix);
}

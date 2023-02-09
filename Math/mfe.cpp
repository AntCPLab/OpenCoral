#include "NTL/GF2X.h"
#include "NTL/GF2E.h"
#include "NTL/GF2XFactoring.h"

void basic_mfe() {
    long n = 2;
    GF2X P = BuildIrred_GF2X(n);
    GF2E::init(P);
}
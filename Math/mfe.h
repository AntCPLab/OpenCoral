#ifndef MFE_H_
#define MFE_H_

#include "NTL/GF2EX.h"
#include "NTL/vec_GF2E.h"
#include "NTL/mat_GF2.h"
#include "NTL/GF2X.h"
#include "Tools/Exceptions.h"
#include <memory>

void test_composite_to_binary();

void basic_mfe();
void basic_rmfe();

void sigma(NTL::vec_GF2E& h, const NTL::GF2EX& g, const NTL::vec_GF2EX& basis, const NTL::vec_GF2E& beta);
void rho(NTL::GF2EX& g, const NTL::vec_GF2E& h, const NTL::vec_GF2EX& basis, const NTL::vec_GF2E& beta, const NTL::GF2EXModulus& modulus);

void mu_1(NTL::GF2EX& g, const NTL::GF2EX& f, const NTL::vec_GF2EX& basis);
void inv_mu_1(NTL::GF2EX& f, const NTL::GF2EX& g, const NTL::vec_GF2EX& basis);

void nu_1(NTL::vec_GF2E& g, const NTL::GF2EX& f, const NTL::vec_GF2E& beta);

void mu_2(NTL::vec_GF2E& g, const NTL::GF2EX& f, const NTL::vec_GF2E& beta);
void inv_mu_2(NTL::GF2EX& f, const NTL::vec_GF2E& g, const NTL::vec_GF2E& beta);

void nu_2(NTL::GF2EX& g, const NTL::GF2EX& f, const NTL::vec_GF2EX& basis, const NTL::GF2EXModulus& modulus);

void mul(NTL::vec_GF2E& x, const NTL::vec_GF2E& a, const NTL::vec_GF2E& b);

void phi(NTL::GF2EX& g, const NTL::vec_GF2E& h, const NTL::vec_GF2EX& basis, const NTL::vec_GF2E& beta);
void psi(NTL::vec_GF2E& h, const NTL::GF2EX& g, const NTL::vec_GF2EX& basis, const NTL::vec_GF2E& beta);

NTL::GF2EX lift(const NTL::GF2X& x);
NTL::GF2X shrink(const NTL::GF2EX& x);
NTL::vec_GF2E lift(const NTL::vec_GF2& x);
NTL::vec_GF2 shrink(const NTL::vec_GF2E& x);
NTL::GF2E to_GF2E(const NTL::GF2X& x, const NTL::GF2X& poly_mod);
NTL::GF2E to_GF2E(const NTL::vec_GF2& x, const NTL::GF2X& poly_mod);

class FieldConverter {
private:
long k_, m_, n_;
NTL::GF2X p_;
NTL::GF2X u_;
NTL::GF2EX q_;
NTL::mat_GF2 mat_T_;
NTL::mat_GF2 mat_T_inv_;

public:
FieldConverter(long k, long m, long n);
const NTL::GF2X& binary_field_poly();
const NTL::GF2X& base_field_poly();
const NTL::GF2EX& composite_field_poly();
void raw_composite_to_binary(NTL::vec_GF2& y, const NTL::vec_GF2& x);
NTL::vec_GF2 raw_composite_to_binary(const NTL::vec_GF2& x);
NTL::GF2E composite_to_binary(const NTL::GF2EX& x);
void raw_binary_to_composite(NTL::vec_GF2& y, const NTL::vec_GF2& x);
NTL::vec_GF2 raw_binary_to_composite(const NTL::vec_GF2& x);
NTL::GF2EX binary_to_composite(const NTL::GF2E& x);
};

template <class T1, class T2, class T3, class T4>
class MFE {
public:
    virtual long m() = 0;
    virtual long t() = 0;
    virtual long base_field_size() = 0;
    virtual const T3& base_field_mod() = 0;
    virtual const T4& ex_field_mod() = 0;
    virtual void encode(T2& h, const T1& g) = 0;
    T2 encode(const T1& g) {
        T2 h;
        encode(h, g);
        return h;
    }
    virtual void decode(T1& g, const T2& h) = 0;
    T1 decode(const T2& h) {
        T1 g;
        decode(g, h);
        return g;
    }
};

typedef MFE<NTL::GF2EX, NTL::vec_GF2E, NTL::GF2X, NTL::GF2EX> Gf2eMFE;
typedef MFE<NTL::GF2X, NTL::vec_GF2, long, NTL::GF2X> Gf2MFE;

class BasicMFE: public Gf2eMFE {
private:
long m_;
long t_;
NTL::GF2X base_field_poly_mod_;
NTL::GF2EXModulus ex_field_poly_mod_;
NTL::GF2EX alpha_;
NTL::vec_GF2EX basis_;
NTL::vec_GF2E beta_;

void initialize();

public:
// A fucntion named f in derived class will hide all other members named f in the base class, regardless of return types or arguments.
// So should expose them with `using`.

using MFE::encode;
using MFE::decode;

BasicMFE(long m, long t, long base_field_poly_mod_deg);

BasicMFE(NTL::GF2X base_field_poly, NTL::GF2EX ex_field_poly, long t);

long m() {
    return m_;
}

long t() {
    return t_;
}

const NTL::GF2X& base_field_mod() {
    return base_field_poly_mod_;
}

const NTL::GF2EX& ex_field_mod() {
    return ex_field_poly_mod_.val();
}

long base_field_size() {
    return 1 << (deg(base_field_poly_mod_));
}

void encode(NTL::vec_GF2E& h, const NTL::GF2EX& g);
void decode(NTL::GF2EX& g, const NTL::vec_GF2E& h);
};

class BasicGf2MFE: public Gf2MFE {
private:
// Now we simply use BasicMFE to implement the special case. Might be able to optimize.
unique_ptr<BasicMFE> internal_;
long base_field_mod_ = 2;
NTL::GF2XModulus ex_field_poly_;

public:
using MFE::encode;
using MFE::decode;

BasicGf2MFE(long m, long t);

long m() {
    return internal_->m();
}

long t() {
    return internal_->t();
}

const long& base_field_mod() {
    return base_field_mod_;
}

const NTL::GF2X& ex_field_mod() {
    return ex_field_poly_.val();
}

long base_field_size() {
    return 2;
}

void encode(NTL::vec_GF2& h, const NTL::GF2X& g);
void decode(NTL::GF2X& g, const NTL::vec_GF2& h);
};

class CompositeGf2MFE: public Gf2MFE {
private:
long m_;
long t_;

std::shared_ptr<FieldConverter> converter_;
std::shared_ptr<Gf2eMFE> mfe1_;
std::shared_ptr<Gf2MFE> mfe2_;

long base_field_mod_ = 2;
NTL::GF2XModulus ex_field_poly_;

public:
using MFE::encode;
using MFE::decode;

CompositeGf2MFE(std::shared_ptr<FieldConverter> converter, std::shared_ptr<Gf2eMFE> mfe1, std::shared_ptr<Gf2MFE> mfe2);

long m() {
    return m_;
}

long t() {
    return t_;
}

const long& base_field_mod() {
    return base_field_mod_;
}

const NTL::GF2X& ex_field_mod() {
    return ex_field_poly_.val();
}

long base_field_size() {
    return 2;
}

void encode(NTL::vec_GF2& h, const NTL::GF2X& g);
void decode(NTL::GF2X& g, const NTL::vec_GF2& h);
};

// class BasicRMFE {

// };

#endif /* MFE_H_ */
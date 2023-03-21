#include "NTL/GF2EX.h"
#include "NTL/vec_GF2E.h"
#include "NTL/mat_GF2.h"
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

void composite_to_binary();

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

// class MFE {
// public:
//     long m() { throw not_implemented(); }
//     long t() { throw not_implemented(); }
//     long base_field_size() { throw not_implemented(); }
//     void sigma(NTL::vec_GF2E& h, const NTL::GF2EX& g) { throw not_implemented(); }
//     void rho(NTL::GF2EX& g, const NTL::vec_GF2E& h) { throw not_implemented(); }
// };

// class BasicMFE: public MFE {
// private:
// long m_;
// long t_;
// GF2X base_field_poly_mod_;
// GF2EXModulus ex_field_poly_mod_;
// GF2EX alpha_;
// vec_GF2EX basis_;
// vec_GF2E beta_;

// public:
// BasicMFE(long m, long t, long base_field_poly_mod_deg);

// long base_field_size() {
//     return 1 << (deg(base_field_poly_mod_));
// }

// void sigma(NTL::vec_GF2E& h, const NTL::GF2EX& g);
// void rho(NTL::GF2EX& g, const NTL::vec_GF2E& h);
// };

// class CompositeMFE: public MFE {
// private:
// long m_;
// long t_;

// std::shared_ptr<MFE> mfe1_;
// std::shared_ptr<MFE> mfe2_;

// public:
// CompositeMFE(std::shared_ptr<MFE> mfe1, std::shared_ptr<MFE> mfe2);

// long base_field_size() {
//     return mfe2_->base_field_size();
// }

// void sigma(NTL::vec_GF2E& h, const NTL::GF2EX& g);
// void rho(NTL::GF2EX& g, const NTL::vec_GF2E& h);
// };

// class BasicRMFE {

// };
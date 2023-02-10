#include "NTL/GF2EX.h"
#include "NTL/vec_GF2E.h"

void basic_mfe();

void sigma(NTL::vec_GF2E& h, const NTL::GF2EX& g, const NTL::vec_GF2EX& basis, const NTL::vec_GF2E& beta);
void rho(NTL::GF2EX& g, const NTL::vec_GF2E& h, const NTL::vec_GF2EX& basis, const NTL::vec_GF2E& beta, const NTL::GF2EXModulus& modulus);

void mu_1(NTL::GF2EX& g, const NTL::GF2EX& f, const NTL::vec_GF2EX& basis);
void inv_mu_1(NTL::GF2EX& f, const NTL::GF2EX& g, const NTL::vec_GF2EX& basis, long m);

void nu_1(NTL::vec_GF2E& g, const NTL::GF2EX& f, const NTL::vec_GF2E& beta);

void mu_2(NTL::vec_GF2E& g, const NTL::GF2EX& f, const NTL::vec_GF2E& beta);
void inv_mu_2(NTL::GF2EX& f, const NTL::vec_GF2E& g, const NTL::vec_GF2E& beta);

void nu_2(NTL::GF2EX& g, const NTL::GF2EX& f, const NTL::vec_GF2EX& basis, const NTL::GF2EXModulus& modulus);

void mul(NTL::vec_GF2E& x, const NTL::vec_GF2E& a, const NTL::vec_GF2E& b);
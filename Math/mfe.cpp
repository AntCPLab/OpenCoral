#include "mfe.h"
#include "NTL/ZZ.h"
#include "NTL/ZZ_p.h"
#include "NTL/ZZ_pX.h"
#include "NTL/ZZ_pE.h"
#include "NTL/GF2X.h"
#include "NTL/GF2E.h"
#include "NTL/GF2XFactoring.h"
#include "NTL/GF2EXFactoring.h"
#include "NTL/vec_vec_GF2E.h"
#include "NTL/vec_GF2E.h"
#include "NTL/mat_GF2E.h"
#include "NTL/vec_vec_GF2.h"
#include "NTL/mat_GF2.h"
#include "primitive_polys.h"
#include <iostream>
#include <unordered_set>
#include <cmath>
#include <vector>

using namespace std;
using namespace NTL;

void mul(vec_GF2E& x, const vec_GF2E& a, const vec_GF2E& b) {
    if (a.length() != b.length())
        LogicError("Mul operands have different length");
    x.SetLength(a.length());
    for(int i = 0; i < a.length(); i++)
        x[i] = a[i] * b[i];
}

void Enumerate_GF2X(vec_GF2X& x, size_t start, size_t n) {
    x.SetLength(n);
    if (n == 0)
        return;

    int len = int(log2(n)) + 1;
    for (size_t i = start; i < start + n; i++) {
        vec_GF2 a({}, len);
        for (int k = 0; k < len; k++)
            a[k] = (i >> k) & 1;
        conv(x[i], a);
    }
}

void Enumerate_GF2E(vec_GF2E& x, size_t start, size_t n) {
    if ((start + n) > GF2E::cardinality())
        LogicError("# of requested elements larger than field size");
    vec_GF2X y;
    Enumerate_GF2X(y, start, n);
    x.SetLength(n);
    for (size_t i = 0; i < n; i++) {
        conv(x[i], y[i]);
    }
}

void sigma(vec_GF2E& h, const GF2EX& g, const vec_GF2EX& basis, const vec_GF2E& beta) {
    GF2EX f;
    inv_mu_1(f, g, basis);
    nu_1(h, f, beta);
}

void mu_1(GF2EX& g, const GF2EX& f, const vec_GF2EX& basis) {
    clear(g);
    for(int i = 0; i <= deg(f); i++) {
        g += basis.at(i) * coeff(f, i);
    }
}

void inv_mu_1(GF2EX& f, const GF2EX& g, const vec_GF2EX& basis) {
    long m = basis.length();
    if (deg(g) + 1 > m)
        LogicError("inv_mu_1: Invalid input g");
    vec_vec_GF2E basis_({}, basis.length());
    for(int i = 0; i < basis.length(); i++) {
        conv(basis_[i], basis[i]);
        basis_[i].SetLength(m);
    }
    mat_GF2E basis_mat;
    MakeMatrix(basis_mat, basis_);

    vec_GF2E g_vec;
    conv(g_vec, g);
    g_vec.SetLength(m);

    GF2E d;
    vec_GF2E f_vec;
    // f(\alpha) = g, equivalent to
    // f * basis = g, solve f
    solve(d, f_vec, basis_mat, g_vec);
    conv(f, f_vec);
}

/**
 * deg(f) = m-1
 * len(beta) = 2m-2
 * f --> (f(beta[0]), f(beta[1]), ..., f(beta[2m-3]), f_{m-1})
*/ 
void nu_1(vec_GF2E& g, const GF2EX& f, const vec_GF2E& beta) {
    g.SetLength(beta.length() + 1);
    for(int i = 0; i < beta.length(); i++) {
        eval(g[i], f, beta[i]);
    }
    // f_{m-1}
    GetCoeff(g[beta.length()], f, beta.length() / 2);
}

void rho(GF2EX& g, const vec_GF2E& h, const vec_GF2EX& basis, const vec_GF2E& beta, const GF2EXModulus& modulus) {
    GF2EX f;
    inv_mu_2(f, h, beta);
    nu_2(g, f, basis, modulus);
}

/**
 * deg(f) = m-1
 * len(beta) = m-1
 * f --> (f(beta[0]), f(beta[1]), ..., f(beta[m-2]), f_{m-1})
*/
void mu_2(vec_GF2E& g, const GF2EX& f, const vec_GF2E& beta) {
    g.SetLength(beta.length() + 1);
    for(int i = 0; i < beta.length(); i++) {
        eval(g[i], f, beta[i]);
    }
    GetCoeff(g[beta.length()], f, beta.length());
}

/**
 * len(g) = m
 * deg(f) = m-1
 * len(beta) = m-1
 * (f(beta[0]), f(beta[1]), ..., f(beta[m-2]), f_{m-1}) --> f
*/
void inv_mu_2(GF2EX& f, const vec_GF2E& g, const vec_GF2E& beta) {
    if (g.length() != beta.length() + 1)
        LogicError("Invalid input to inv_mu_2");
    vec_GF2E corrected_g({}, g.length() - 1);
    for(int i = 0; i < corrected_g.length(); i++) {
        corrected_g[i] = g[i] - g[g.length() - 1] * power(beta[i], corrected_g.length());
    }
    interpolate(f, beta, corrected_g);
    SetCoeff(f, corrected_g.length(), g[g.length() - 1]);
}

// /**
//  * len(g) = m
//  * deg(f) = m-1
//  * len(beta) = m-1
//  * (f(beta[0]), f(beta[1]), ..., f(beta[m-2]), f_{0}) --> f
// */
// void inv_mu_2(GF2EX& f, const vec_GF2E& g, const vec_GF2E& beta) {
//     if (g.length() != beta.length() + 1)
//         LogicError("Invalid input to inv_mu_2");
//     vec_GF2E corrected_g({}, g.length() - 1);
//     for(int i = 0; i < corrected_g.length(); i++) {
//         corrected_g[i] = g[i] - g[g.length() - 1];
//     }
//     interpolate(f, beta, corrected_g);
//     f <<= 1;
//     SetCoeff(f, 0, g[g.length() - 1]);
// }

void nu_2(GF2EX& g, const GF2EX& f, const vec_GF2EX& basis, const GF2EXModulus& modulus) {
    clear(g);
    GF2EX extended_alpha_power = basis[basis.length() - 1];
    for(int i = 0; i <= deg(f); i++) {
        if (i < basis.length())
            g += basis[i] * coeff(f, i);
        else {
            MulMod(extended_alpha_power, extended_alpha_power, basis[1], modulus);
            g += extended_alpha_power * coeff(f, i);
        }
    }
}



const auto& xi_1 = mu_2;

// const auto& inv_xi_1 = inv_mu_2;
/**
 * len(g) = m
 * deg(f) = m-1
 * len(beta) = m
 * beta[0] = 0
 * (f_{beta[0]}, f(beta[1]), f(beta[2]), ..., f(beta[m-1])) --> f
*/
void inv_xi_1(GF2EX& f, const vec_GF2E& g, const vec_GF2E& beta) {
    if (g.length() != beta.length())
        LogicError("Invalid input to inv_xi_1");
    // Make sure beta[0] is 0 so that we can map identity to identity in RMFE
    if (!IsZero(beta[0]))
        LogicError("beta[0] must be zero in RMFE");

    interpolate(f, beta, g);
}

const auto& pi_1 = mu_1;

const auto& inv_pi_1 = inv_mu_1;

// /**
//  * deg(f) = 2m-2
//  * len(beta) = m-1
//  * f --> (f(beta[0]), f(beta[1]), ..., f(beta[m-2]), f_{2m-2})
// */
// void xi_2(vec_GF2E& g, const GF2EX& f, const vec_GF2E& beta) {
//     long k = beta.length() + 1;
//     g.SetLength(k);
//     for(int i = 0; i < beta.length(); i++) {
//         eval(g[i], f, beta[i]);
//     }
//     GetCoeff(g[k-1], f, 2*k-2);
// }

/**
 * deg(f) = 2k-2 (type1) for 2k-1 (type2)
 * len(beta) = k
 * beta[0] = 0
 * f --> (f(beta[0]), f(beta[1]), ..., f(beta[k-1]))
*/
void xi_2(vec_GF2E& g, const GF2EX& f, const vec_GF2E& beta) {
    if (!IsZero(beta[0]))
        LogicError("beta[0] must be zero in RMFE");

    long k = beta.length();
    g.SetLength(k);
    for(int i = 0; i < beta.length(); i++) {
        eval(g[i], f, beta[i]);
    }
}

void phi(GF2EX& g, const vec_GF2E& h, const vec_GF2EX& basis, const vec_GF2E& beta) {
    GF2EX f;
    inv_xi_1(f, h, beta);
    pi_1(g, f, basis);
}

void psi(vec_GF2E& h, const GF2EX& g, const vec_GF2EX& basis, const vec_GF2E& beta) {
    GF2EX f;
    inv_pi_1(f, g, basis);
    xi_2(h, f, beta);
}

GF2EX lift(const GF2X& x) {
    GF2EX y;
    y.SetLength(deg(x) + 1);
    for (int i = 0; i <= deg(x); i++)
        SetCoeff(y, i, coeff(x, i));
    return y;
}

GF2X shrink(const GF2EX& x) {
    GF2X y;
    y.SetLength(deg(x) + 1);
    for (int i = 0; i <= deg(x); i++) {
        const GF2E& xi = coeff(x, i);
        if (!IsZero(xi) && ! IsOne(xi))
            LogicError("Invalid coefficient in x");
        SetCoeff(y, i, coeff(rep(coeff(x, i)), 0));
    }
    return y;
}

vec_GF2E lift(const vec_GF2& x) {
    vec_GF2E y;
    y.SetLength(x.length());
    for (int i = 0; i < y.length(); i++)
        y.at(i) = to_GF2E(x.at(i));
    return y;
}

vec_GF2 shrink(const vec_GF2E& x) {
    vec_GF2 y;
    y.SetLength(x.length());
    for (int i = 0; i < y.length(); i++) {
        const GF2E& xi = x.at(i);
        if (!IsZero(xi) && ! IsOne(xi))
            LogicError("Invalid element in x");
        y.at(i) = coeff(rep(xi), 0);
    }
    return y;
}


BasicMFE::BasicMFE(long m, long base_field_poly_mod_deg)
    : m_(m), t_(2*m-1) {
    if (!ProbPrime(m))
        LogicError("m must be prime for construcing a basic MFE");
    base_field_poly_mod_ = BuildIrred_GF2X(base_field_poly_mod_deg);
    GF2EPush push;
    GF2E::init(base_field_poly_mod_);

    ex_field_poly_mod_ = GF2EXModulus(BuildIrred_GF2EX(m));

    initialize();
}

BasicMFE::BasicMFE(NTL::GF2X base_field_poly, NTL::GF2EX ex_field_poly) {
    base_field_poly_mod_ = move(base_field_poly);
    GF2EPush push;
    GF2E::init(base_field_poly_mod_);
    ex_field_poly_mod_ = GF2EXModulus(ex_field_poly);

    m_ = deg(ex_field_poly_mod_);
    t_ = 2 * m_ - 1;

    initialize();
}

void BasicMFE::initialize() {
    alpha_ = GF2EX(1, 1); // alpha = 0 + 1*Y + 0*Y^2 + ... = Y
    basis_ = vec_GF2EX({}, m_);
    basis_[0] = 1;
    for (int i = 1; i < m_; i++) {
        MulMod(basis_[i], basis_[i-1], alpha_, ex_field_poly_mod_);
    }
    // TODO: actually, in this setting, the basis is just {1, Y, Y^2, ..., Y^(m-1)}
    // Hence mu_1 and inv_mu_1 are both just identity functions (just run them to verify)
    // So things can be simpfiled a lot. Keep algorithm code of mu_1 and inv_mu_1 here
    // in case we could not use "alpha = Y" in some setting.

    // get 2m-2 distinct elements
    Enumerate_GF2E(beta_, 0, 2*m_-2);
}

void BasicMFE::encode(vec_GF2E& h, const GF2EX& g) {
    GF2EPush push;
    GF2E::init(base_field_poly_mod_);
    ::sigma(h, g, basis_, beta_);
}

void BasicMFE::decode(GF2EX& g, const vec_GF2E& h) {
    GF2EPush push;
    GF2E::init(base_field_poly_mod_);
    ::rho(g, h, basis_, beta_, ex_field_poly_mod_);
}

BasicGf2MFE::BasicGf2MFE(long m) {
    internal_ = unique_ptr<BasicMFE>(new BasicMFE(m, 1));
    const GF2EX& f = internal_->ex_field_mod();
    ex_field_poly_ = GF2XModulus(shrink(f));
}

void BasicGf2MFE::encode(vec_GF2& h, const GF2X& g) {
    if (deg(g) + 1 > m())
        LogicError("Input polynomial g has an invalid length");
    GF2EPush push;
    GF2E::init(GF2X(1, 1));
    GF2EX g_ = lift(g);

    vec_GF2E h_ = internal_->encode(g_);
    if (h_.length() != t())
        LogicError("Output vector h has an invalid length");

    h = shrink(h_);
}

void BasicGf2MFE::decode(GF2X& g, const vec_GF2& h) {
    if (h.length() != t())
        LogicError("Input vector h has an invalid length");

    GF2EPush push;
    GF2E::init(GF2X(1, 1));
    vec_GF2E h_ = lift(h);
    
    GF2EX g_ = internal_->decode(h_);
    if (deg(g_) + 1 > m())
        LogicError("Output polynomial g has an invalid length");

    g = shrink(g_);
}

CompositeGf2MFE::CompositeGf2MFE(std::shared_ptr<FieldConverter> converter, std::shared_ptr<Gf2eMFE> mfe1, std::shared_ptr<Gf2MFE> mfe2)
    : converter_(converter), mfe1_(mfe1), mfe2_(mfe2) {
    if (converter->base_field_poly() != mfe1->base_field_mod())
        LogicError("Converter and mfe1's base field polys are different");
    if (converter->composite_field_poly() != mfe1->ex_field_mod())
        LogicError("Converter and mfe1's composite field polys are different");
    if (mfe1->base_field_size() != (long) pow(mfe2->base_field_size(), mfe2->m()))
        LogicError("Cannot concatenate two MFEs with incompatible field sizes");
    if (mfe2->base_field_size() != 2)
        LogicError("mfe2's base field must be GF(2)");

    m_ = mfe1->m() * mfe2->m();
    t_ = mfe1->t() * mfe2->t();

    ex_field_poly_ = converter_->binary_field_poly();
}

void CompositeGf2MFE::encode(NTL::vec_GF2& h, const NTL::GF2X& g) {
    if (deg(g) + 1 > m())
        LogicError("Input polynomial g has an invalid length");
    GF2E g_ = to_GF2E(g, converter_->binary_field_poly());
    GF2EX g_comp = converter_->binary_to_composite(g_);

    vec_GF2E y = mfe1_->encode(g_comp);

    h.SetLength(t());
    for (int i = 0; i < y.length(); i++) {
        vec_GF2 yi = mfe2_->encode(rep(y.at(i)));
        for (int j = 0; j < yi.length(); j++)
            h.at(i * mfe2_->t() + j) = yi.at(j);
    }
}

void CompositeGf2MFE::decode(NTL::GF2X& g, const NTL::vec_GF2& h) {
    if (h.length() != t())
        LogicError("Input vector h has an invalid length");
    GF2EPush push;
    GF2E::init(converter_->base_field_poly());
    vec_GF2E y({}, mfe1_->t());

    for (int i = 0; i < y.length(); i++) {
        vec_GF2 yi({}, mfe2_->t());
        for (int j = 0; j < yi.length(); j++)
            yi.at(j) = h.at(i * mfe2_->t() + j);
        y.at(i) = to_GF2E(mfe2_->decode(yi), converter_->base_field_poly());
    }
    GF2EX g_comp = mfe1_->decode(y);
    g = rep(converter_->composite_to_binary(g_comp));
}

void indices_to_gf2x(GF2X& f, const vector<long>& indices) {
    clear(f);
    for (size_t i = 0; i < indices.size(); i++) {
        SetCoeff(f, indices[i]);
    }
}

GF2X indices_to_gf2x(const vector<long>& indices) {
    GF2X f;
    indices_to_gf2x(f, indices);
    return f;
}

GF2E to_GF2E(const GF2X& x, const GF2X& poly_mod) {
    GF2EPush push;
    GF2E::init(poly_mod);
    return to_GF2E(x);
}

GF2E to_GF2E(const vec_GF2& x, const GF2X& poly_mod) {
    return to_GF2E(to_GF2X(x), poly_mod);
}

vec_GF2 to_vec_GF2(const GF2E& x, long target_len) {
    if (deg(rep(x)) + 1 > target_len)
        LogicError("Invalid target length (too small)");
    vec_GF2 y = to_vec_GF2(rep(x));
    y.SetLength(target_len);
    return y;
}


GF2EX to_GF2EX(const vec_GF2& x, const GF2X& base_field_poly) {
    if (x.length() % deg(base_field_poly) != 0)
        LogicError("Input vector length is not a multiple of base field polynomial degree.");
    GF2EPush push;
    GF2E::init(base_field_poly);
    vec_GF2E y({}, x.length() / deg(base_field_poly));
    for (long i = 0; i < y.length(); i++) {
        vec_GF2 tmp({}, deg(base_field_poly));
        for (long j = 0; j < deg(base_field_poly); j++) {
            tmp[j] = x[i * deg(base_field_poly) + j];
        }
        y[i] = to_GF2E(to_GF2X(tmp));
    }
    return to_GF2EX(y);
}

vec_GF2 to_vec_GF2(const GF2EX& x, const GF2X& base_field_poly, long target_len) {
    if ((deg(x)+1) * deg(base_field_poly) > target_len ||
        target_len % deg(base_field_poly) != 0)
        LogicError("Invalid target length");

    vec_GF2 y({}, target_len);
    for (long i = 0; i <= deg(x); i++) {
        for (long j = 0; j < deg(base_field_poly); j++) {
            y[i * deg(base_field_poly) + j] = coeff(rep(coeff(x, i)), j);
        }
    }
    return y;
}



FieldConverter::FieldConverter(long k, long m, long n): k_(k), m_(m), n_(n) {
    if (k != m * n)
        LogicError("Invalid field composition parameters");
    if (PRIMITIVE_POLYS.count(k) == 0)
        LogicError("No primitive polynomial is defined for this binary field");
    ZZ r = ((ZZ(1) << k) - 1) / ((ZZ(1) << n) - 1);

    p_ = indices_to_gf2x(PRIMITIVE_POLYS[k]);
    GF2EPush push;
    GF2E::init(p_);

    GF2E alpha = to_GF2E(GF2X(1, 1));
    GF2E gamma = power(alpha, r);

    vec_vec_GF2E alpha_power({}, m);
    for (long j = 0; j < m; j++) {
        alpha_power[j].SetLength(n);
        for (long i = 0; i < n; i++) {
            alpha_power[j][i] = power(alpha, r * i + j);
        }
    }
    vec_vec_GF2 T({}, k);
    for (long h = 0; h < k; h++) {
        T[h].SetLength(k);
        for (long j = 0; j < m; j++) {
            for (long i = 0; i < n; i++) {
                T[h][j*n + i] = coeff(rep(alpha_power[j][i]), h);
            }
        }
    }
    MakeMatrix(mat_T_, T);
    mat_T_inv_ = inv(mat_T_);

    GF2EX u(1);
    for (long i = 0; i < n; i++) {
        GF2EX tmp;
        tmp.SetLength(2);
        tmp[0] = power(gamma, (ZZ(1) << (n-i-1)));
        tmp[1] = 1;
        u = u * tmp;
    }
    u_.SetLength(deg(u) + 1);
    for (long i = 0; i <= deg(u); i++) {
        // All coefficients of u should belong to GF(2), otherwise something is wrong.
        if (!(IsZero(u[i]) || IsOne(u[i]))) {
            LogicError("Error computing base field minimal polynomial");
        }
        u_[i] = coeff(rep(u[i]), 0);
    }

    GF2EX q(1);
    for (long i = 0; i < m; i++) {
        GF2EX tmp;
        tmp.SetLength(2);
        tmp[0] = power(alpha, (ZZ(1) << (i*n)));
        tmp[1] = 1;
        q = q * tmp;
    }
    q_.SetLength(deg(q) + 1);
    for (long i = 0; i <= deg(q); i++) {
        vec_GF2 a = to_vec_GF2(q[i], k_), b;
        raw_binary_to_composite(b, a);
        // All coefficients of q should belong to GF(2^n), hence any value after index n should be 0.
        for (long j = n; j < k; j++) {
            if (IsOne(b[j])) {
                LogicError("Error computing composite field polynomial");
            }
        }
        vec_GF2 c = VectorCopy(b, n);
        // Here we should use u_ as the base field irreducible polynomial for q_'s coefficients,
        // but since deg(u_) < deg(p_), the conversion here should be fine.
        q_[i] = to_GF2E(c, u_);
    }

}

const GF2X& FieldConverter::binary_field_poly() {
    return p_;
}
const GF2X& FieldConverter::base_field_poly() {
    return u_;
}
const GF2EX& FieldConverter::composite_field_poly() {
    return q_;
}

void FieldConverter::raw_composite_to_binary(vec_GF2& y, const vec_GF2& x) {
    if (x.length() != k_) {
        LogicError("Input vector has invalid length");
    }
    mul(y, mat_T_, x);
}

vec_GF2 FieldConverter::raw_composite_to_binary(const vec_GF2& x) {
    vec_GF2 y;
    raw_composite_to_binary(y, x);
    return y;
}

GF2E FieldConverter::composite_to_binary(const GF2EX& x) {
    vec_GF2 y = raw_composite_to_binary(to_vec_GF2(x, base_field_poly(), k_));
    return to_GF2E(y, binary_field_poly());
}

void FieldConverter::raw_binary_to_composite(vec_GF2& y, const vec_GF2& x) {
    if (x.length() != k_) {
        LogicError("Input vector has invalid length");
    }
    mul(y, mat_T_inv_, x);
}

vec_GF2 FieldConverter::raw_binary_to_composite(const vec_GF2& x) {
    vec_GF2 y;
    raw_binary_to_composite(y, x);
    return y;
}

GF2EX FieldConverter::binary_to_composite(const GF2E& x) {
    return to_GF2EX(raw_binary_to_composite(to_vec_GF2(x, k_)), base_field_poly());
}

BasicRMFE::BasicRMFE(long k, long base_field_poly_mod_deg, bool is_type1): k_(k) {
    if (is_type1)
        m_ = 2*k-1;
    else
        m_ = 2*k;
    base_field_poly_mod_ = BuildIrred_GF2X(base_field_poly_mod_deg);
    GF2EPush push;
    GF2E::init(base_field_poly_mod_);

    ex_field_poly_mod_ = GF2EXModulus(BuildIrred_GF2EX(m_));

    initialize();
}

BasicRMFE::BasicRMFE(NTL::GF2X base_field_poly, NTL::GF2EX ex_field_poly) {
    base_field_poly_mod_ = move(base_field_poly);
    GF2EPush push;
    GF2E::init(base_field_poly_mod_);
    ex_field_poly_mod_ = GF2EXModulus(ex_field_poly);

    m_ = deg(ex_field_poly_mod_);
    if (m_ % 2 == 1)
        k_ = (m_ + 1) / 2;
    else
        k_ = m_ / 2;
    // Different from Lemma 4 in [CCXY18], we require q >= k instead of q >= k+1, in order
    // to make this RMFE have a mapping of identity to identity.
    if (k_ > GF2E::cardinality())
        LogicError("Constrait violation: q >= k");

    initialize();
}

void BasicRMFE::initialize() {
    alpha_ = GF2EX(1, 1); // alpha = 0 + 1*Y + 0*Y^2 + ... = Y
    basis_ = vec_GF2EX({}, m_);
    basis_[0] = 1;
    for (int i = 1; i < m_; i++) {
        MulMod(basis_[i], basis_[i-1], alpha_, ex_field_poly_mod_);
    }
    // TODO: actually, in this setting, the basis is just {1, Y, Y^2, ..., Y^(m-1)}
    // Hence mu_1 and inv_mu_1 are both just identity functions (just run them to verify)
    // So things can be simpfiled a lot. Keep algorithm code of mu_1 and inv_mu_1 here
    // in case we could not use "alpha = Y" in some setting.

    // get k distinct elements
    Enumerate_GF2E(beta_, 0, k_);
}

void BasicRMFE::encode(GF2EX& g, const vec_GF2E& h) {
    GF2EPush push;
    GF2E::init(base_field_poly_mod_);
    ::phi(g, h, basis_, beta_);
}

void BasicRMFE::decode(vec_GF2E& h, const GF2EX& g) {
    GF2EPush push;
    GF2E::init(base_field_poly_mod_);
    ::psi(h, g, basis_, beta_);
}

BasicGf2RMFE::BasicGf2RMFE(long k, bool is_type1) {
    internal_ = unique_ptr<BasicRMFE>(new BasicRMFE(k, 1, is_type1));
    const GF2EX& f = internal_->ex_field_mod();
    ex_field_poly_ = GF2XModulus(shrink(f));
}

void BasicGf2RMFE::decode(vec_GF2& h, const GF2X& g) {
    if (deg(g) + 1 > m())
        LogicError("Input polynomial g has an invalid length");
    GF2EPush push;
    GF2E::init(GF2X(1, 1));
    GF2EX g_ = lift(g);

    vec_GF2E h_ = internal_->decode(g_);
    if (h_.length() != k())
        LogicError("Output vector h has an invalid length");

    h = shrink(h_);
}

void BasicGf2RMFE::encode(GF2X& g, const vec_GF2& h) {
    if (h.length() != k())
        LogicError("Input vector h has an invalid length");

    GF2EPush push;
    GF2E::init(GF2X(1, 1));
    vec_GF2E h_ = lift(h);
    
    GF2EX g_ = internal_->encode(h_);
    if (deg(g_) + 1 > m())
        LogicError("Output polynomial g has an invalid length");

    g = shrink(g_);
}

CompositeGf2RMFE::CompositeGf2RMFE(std::shared_ptr<FieldConverter> converter, std::shared_ptr<Gf2eRMFE> rmfe1, std::shared_ptr<Gf2RMFE> rmfe2)
    : converter_(converter), rmfe1_(rmfe1), rmfe2_(rmfe2) {
    if (converter->base_field_poly() != rmfe1->base_field_mod())
        LogicError("Converter and rmfe1's base field polys are different");
    if (converter->composite_field_poly() != rmfe1->ex_field_mod())
        LogicError("Converter and rmfe1's composite field polys are different");
    if (rmfe1->base_field_size() != (long) pow(rmfe2->base_field_size(), rmfe2->m()))
        LogicError("Cannot concatenate two RMFEs with incompatible field sizes");
    if (rmfe2->base_field_size() != 2)
        LogicError("rmfe2's base field must be GF(2)");

    m_ = rmfe1->m() * rmfe2->m();
    k_ = rmfe1->k() * rmfe2->k();

    ex_field_poly_ = converter_->binary_field_poly();
}

void CompositeGf2RMFE::encode(NTL::GF2X& g, const NTL::vec_GF2& h) {
    if (h.length() != k())
        LogicError("Input vector h has an invalid length");
    GF2EPush push;
    GF2E::init(converter_->base_field_poly());
    vec_GF2E y({}, rmfe1_->k());

    for (int i = 0; i < y.length(); i++) {
        vec_GF2 yi({}, rmfe2_->k());
        for (int j = 0; j < yi.length(); j++)
            yi.at(j) = h.at(i * rmfe2_->k() + j);
        y.at(i) = to_GF2E(rmfe2_->encode(yi), converter_->base_field_poly());
    }
    GF2EX g_comp = rmfe1_->encode(y);
    g = rep(converter_->composite_to_binary(g_comp));
}

void CompositeGf2RMFE::decode(NTL::vec_GF2& h, const NTL::GF2X& g) {
    if (deg(g) + 1 > m())
        LogicError("Input polynomial g has an invalid length");
    GF2E g_ = to_GF2E(g, converter_->binary_field_poly());
    GF2EX g_comp = converter_->binary_to_composite(g_);

    vec_GF2E y = rmfe1_->decode(g_comp);

    h.SetLength(k());
    for (int i = 0; i < y.length(); i++) {
        vec_GF2 yi = rmfe2_->decode(rep(y.at(i)));
        for (int j = 0; j < yi.length(); j++)
            h.at(i * rmfe2_->k() + j) = yi.at(j);
    }
}

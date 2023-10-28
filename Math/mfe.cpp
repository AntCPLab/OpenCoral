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
#include "Tools/performance.h"
#include <assert.h>

using namespace std;
using namespace NTL;

void mul(vec_GF2E& x, const vec_GF2E& a, const vec_GF2E& b) {
    if (a.length() != b.length())
        LogicError("Mul operands have different length");
    x.SetLength(a.length());
    for(int i = 0; i < a.length(); i++)
        x[i] = a[i] * b[i];
}

void mul_precomp_aux(vec_GF2E& x, const mat_GF2E& A, const vec_GF2E& b) {
    acc_time_log("mul_precomp_aux");
    long n = A.NumRows();  
    long l = A.NumCols();  
    
    if (l != b.length())  
        LogicError("matrix mul: dimension mismatch");  
    
    x.SetLength(n);  
    
    long i, k;  
    GF2E tmp;
    const vector<vector<GF2E>>& mul_table = GF2EPrecomp::get_mul_table(GF2E::modulus().f);
    const vector<vector<GF2E>>& add_table = GF2EPrecomp::get_add_table(GF2E::modulus().f);
    
    for (i = 1; i <= n; i++) {
        clear(x(i));
        const GF2E* acc = &(x(i));
        for (k = 1; k <= l; k++) {
            const GF2E& entry = mul_table[ntl_GF2E_to_num(A(i,k))][ntl_GF2E_to_num(b(k))];
            acc = &(add_table[ntl_GF2E_to_num(*acc)][ntl_GF2E_to_num(entry)]);
        }
        // [zico] Without the following assignment, it drops from 75 ms to 14 ms. Can we optimize this?
        x(i) = *acc;
    }
    acc_time_log("mul_precomp_aux");
}


void mul_precomp(vec_GF2E& x, const mat_GF2E& A, const vec_GF2E& b) {
    if (&b == &x || A.alias(x)) {
        vec_GF2E tmp;
        mul_precomp_aux(tmp, A, b);
        x = tmp;
   }
   else
        mul_precomp_aux(x, A, b);
}

void mul_precomp(GF2E& x, const GF2E& a, const GF2E& b) {
    const vector<vector<GF2E>>& mul_table = GF2EPrecomp::get_mul_table(GF2E::modulus().f);
    x = mul_table[ntl_GF2E_to_num(a)][ntl_GF2E_to_num(b)];
}

void add_precomp(GF2E& x, const GF2E& a, const GF2E& b) {
    const vector<vector<GF2E>>& add_table = GF2EPrecomp::get_add_table(GF2E::modulus().f);
    x = add_table[ntl_GF2E_to_num(a)][ntl_GF2E_to_num(b)];
}


void interpolate_precomp(GF2EX& f, const vec_GF2E& a, const vec_GF2E& b)
{
   long m = a.length();
   if (b.length() != m) LogicError("interpolate: vector length mismatch");

   if (m == 0) {
      clear(f);
      return;
   }

   vec_GF2E prod;
   prod = a;

   GF2E t1, t2;

   long k, i;

   vec_GF2E res;
   res.SetLength(m);

   for (k = 0; k < m; k++) {

      const GF2E& aa = a[k];

      set(t1);
      for (i = k-1; i >= 0; i--) {
         mul_precomp(t1, t1, aa);
         add_precomp(t1, t1, prod[i]);
      }

      clear(t2);
      for (i = k-1; i >= 0; i--) {
         mul_precomp(t2, t2, aa);
         add_precomp(t2, t2, res[i]);
      }


      inv(t1, t1);
      sub(t2, b[k], t2);
      mul_precomp(t1, t1, t2);

      for (i = 0; i < k; i++) {
         mul_precomp(t2, prod[i], t1);
         add_precomp(res[i], res[i], t2);
      }

      res[k] = t1;

      if (k < m-1) {
         if (k == 0)
            NTL::negate(prod[0], prod[0]);
         else {
            NTL::negate(t1, a[k]);
            add(prod[k], t1, prod[k-1]);
            for (i = k-1; i >= 1; i--) {
               mul_precomp(t2, prod[i], t1);
               add(prod[i], t2, prod[i-1]);
            }
            mul_precomp(prod[0], prod[0], t1);
         }
      }
   }

   while (m > 0 && IsZero(res[m-1])) m--;
   res.SetLength(m);
   f.rep = res;
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
        conv(x[i-start], a);
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

void sigma_fast(vec_GF2E& h, const GF2EX& g, const vec_GF2E& beta) {
    nu_1(h, g, beta);
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

/**
 * deg(f) = 2k-2 (type1) for 2k-1 (type2)
 * len(beta) = k
 * beta[0] = 0
 * f --> (f(beta[0]), f(beta[1]), ..., f(beta[k-1]))
*/
void xi_2_precomp(vec_GF2E& g, const GF2EX& f, const mat_GF2E& beta) {
    long m = beta.NumCols();
    if (deg(f) == m-1) {
        mul_precomp(g, beta, f.rep);
    }
    else {
        vec_GF2E ext_f({}, m);
        auto f_it = f.rep.begin();
        auto ext_f_it = ext_f.begin();
        for (; f_it != f.rep.end(); f_it++, ext_f_it++)
            *ext_f_it = *f_it;
        mul_precomp(g, beta, ext_f);
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

void psi_fast(vec_GF2E& h, const GF2EX& g, const vec_GF2E& beta) {
    xi_2(h, g, beta);
}

void psi_fast_precomp(vec_GF2E& h, const GF2EX& g, const mat_GF2E& beta) {
    xi_2_precomp(h, g, beta);
}

void psi_precomp(vec_GF2E& h, const GF2EX& g, const vec_GF2EX& basis, const mat_GF2E& beta) {
    GF2EX f;
    inv_pi_1(f, g, basis);
    xi_2_precomp(h, g, beta);
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

GF2E num_to_ntl_GF2E(unsigned long x) {
    GF2E res;
    num_to_ntl_GF2E(res, x);
    return res;
}

void num_to_ntl_GF2E(GF2E& res, unsigned long x) {
    if (x == 0) {
        clear(res);
        return;
    }
    res.allocate();
    res._GF2E__rep.SetLength(GF2E::degree());
    res._GF2E__rep.xrep[0] = x;
}

unsigned long ntl_GF2E_to_num(const GF2E& x) {
    // This is much more efficient than IsZero(x), which seems weird.
    if (x._GF2E__rep.xrep.length() == 0)
        return 0;
    return x._GF2E__rep.xrep[0];
    // bool b = (x._GF2E__rep.xrep.length() == 0);
    // // bool b = IsZero(x);
    // return 1;
}

std::unordered_map<unsigned long, std::vector<std::vector<NTL::GF2E>>> GF2EPrecomp::mul_table_;
std::unordered_map<unsigned long, std::vector<std::vector<NTL::GF2E>>> GF2EPrecomp::add_table_;
std::mutex GF2EPrecomp::mtx_;

void GF2EPrecomp::generate_table(const GF2X& poly_mod) {
    if (deg(poly_mod) > 63)
        LogicError("Poly modulus degree > 63 for GF2EPrecomp");
    if (deg(poly_mod) < 0)
        return;
    mtx_.lock();
    GF2EPush push;
    GF2E::init(poly_mod);
    long idx = poly_mod.xrep[0];
    if (mul_table_.count(idx) != 1) {
        cout << "poly_mod: " << poly_mod << endl;
        mul_table_[idx] = {};
        add_table_[idx] = {};
        size_t n = 1 << deg(poly_mod);
        vector<vector<GF2E>>& t_mul = mul_table_[idx];
        vector<vector<GF2E>>& t_add = add_table_[idx];
        t_mul.resize(n, vector<GF2E>(n));
        t_add.resize(n, vector<GF2E>(n));
        for (size_t i = 0; i < n; i++) {
            GF2E x = num_to_ntl_GF2E(i);
            for (size_t j = 0; j < n; j++) {
                GF2E y = num_to_ntl_GF2E(j);
                t_mul[i][j] = x * y;
                t_add[i][j] = x + y;
            }
        }
    }
    mtx_.unlock();
}

const std::vector<std::vector<NTL::GF2E>>& GF2EPrecomp::get_mul_table(const NTL::GF2X& poly_mod) {
    if (deg(poly_mod) > 63)
        LogicError("Poly modulus degree > 63 for GF2EPrecomp");
    if (deg(poly_mod) < 0)
        LogicError("Poly modulus is 0 for GF2EPrecomp");
    long idx = poly_mod.xrep[0];
    if (mul_table_.count(idx) != 1)
        LogicError(("Mul table has not been generated: " + to_string(idx)).c_str());

    return mul_table_[idx];
}


const std::vector<std::vector<NTL::GF2E>>& GF2EPrecomp::get_add_table(const NTL::GF2X& poly_mod) {
    if (deg(poly_mod) > 63)
        LogicError("Poly modulus degree > 63 for GF2EPrecomp");
    if (deg(poly_mod) < 0)
        LogicError("Poly modulus is 0 for GF2EPrecomp");
    long idx = poly_mod.xrep[0];
    if (add_table_.count(idx) != 1)
        LogicError(("Mul table has not been generated: " + to_string(idx)).c_str());

    return add_table_[idx];
}


BasicMFE::BasicMFE(long m, long base_field_poly_mod_deg)
    : m_(m), t_(2*m-1) {
    // if (!ProbPrime(m))
    //     LogicError("m must be prime for construcing a basic MFE");
    base_field_poly_mod_ = BuildIrred_GF2X(base_field_poly_mod_deg);
    GF2EPush push;
    GF2E::init(base_field_poly_mod_);

    ex_field_poly_mod_ = GF2EXModulus(BuildIrred_GF2EX(m));

    initialize();
}

BasicMFE::BasicMFE(NTL::GF2X base_field_poly, NTL::GF2EX ex_field_poly) {
    base_field_poly_mod_ = std::move(base_field_poly);
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

    base_field_context_ = GF2EContext(base_field_poly_mod_);
}

void BasicMFE::encode(vec_GF2E& h, const GF2EX& g) {
    GF2EPush push;
    // GF2E::init(base_field_poly_mod_);
    base_field_context_.restore();
    if (use_fast_basis_)
        ::sigma_fast(h, g, beta_);
    else
        ::sigma(h, g, basis_, beta_);
}

void BasicMFE::decode(GF2EX& g, const vec_GF2E& h) {
    GF2EPush push;
    // GF2E::init(base_field_poly_mod_);
    base_field_context_.restore();
    ::rho(g, h, basis_, beta_, ex_field_poly_mod_);
}

BasicGf2MFE::BasicGf2MFE(long m) {
    internal_ = unique_ptr<BasicMFE>(new BasicMFE(m, 1));
    const GF2EX& f = internal_->ex_field_mod();
    ex_field_poly_ = GF2XModulus(shrink(f));

    // To use cache, we need to constrain the input size.
    if (use_cache_) {
        assert(internal_->m() < 22);
        assert(internal_->t() < 22);
        
        encode_table_.resize(1 << internal_->m());
        encode_table_cached_.resize(1 << internal_->m());
        decode_table_.resize(1 << internal_->t());
        decode_table_cached_.resize(1 << internal_->t());
    }

    base_field_context_ = GF2EContext(GF2X(1, 1));
}

void BasicGf2MFE::encode(vec_GF2& h, const GF2X& g) {
    if (deg(g) + 1 > m())
        LogicError("Input polynomial g has an invalid length");

    long idx = deg(g) == -1 ? 0 : g.xrep[0];
    if (use_cache_ && encode_table_cached_[idx]) {
        h = encode_table_[idx];
        return;
    }
    
    GF2EPush push;
    // GF2E::init(GF2X(1, 1));
    base_field_context_.restore();
    GF2EX g_ = lift(g);

    vec_GF2E h_ = internal_->encode(g_);
    if (h_.length() != t())
        LogicError("Output vector h has an invalid length");

    h = shrink(h_);

    if (use_cache_) {
        encode_table_[idx] = h;
        encode_table_cached_[idx] = true;
    }
}

void BasicGf2MFE::decode(GF2X& g, const vec_GF2& h) {
    if (h.length() != t())
        LogicError("Input vector h has an invalid length");

    if (use_cache_ && decode_table_cached_[h.rep[0]]) {
        g = decode_table_[h.rep[0]];
        return;
    }

    GF2EPush push;
    // GF2E::init(GF2X(1, 1));
    base_field_context_.restore();
    vec_GF2E h_ = lift(h);
    
    GF2EX g_ = internal_->decode(h_);
    if (deg(g_) + 1 > m())
        LogicError("Output polynomial g has an invalid length");

    g = shrink(g_);

    if (use_cache_) {
        decode_table_[h.rep[0]] = g;
        decode_table_cached_[h.rep[0]] = true;
    }
}

CompositeGf2MFE::CompositeGf2MFE(std::shared_ptr<FieldConverter> converter, std::shared_ptr<Gf2MFE> mfe1, std::shared_ptr<Gf2eMFE> mfe2)
    : converter_(converter), mfe1_(mfe1), mfe2_(mfe2) {
    if (converter->base_field_poly() != mfe2->base_field_mod())
        LogicError("Converter and mfe2's base field polys are different");
    if (converter->composite_field_poly() != mfe2->ex_field_mod())
        LogicError("Converter and mfe2's composite field polys are different");
    if (mfe2->base_field_size() != (long) pow(mfe1->base_field_size(), mfe1->m()))
        LogicError("Cannot concatenate two MFEs with incompatible field sizes");
    if (mfe1->base_field_size() != 2)
        LogicError("mfe1's base field must be GF(2)");

    m_ = mfe1->m() * mfe2->m();
    t_ = mfe1->t() * mfe2->t();

    ex_field_poly_ = converter_->binary_field_poly();

    // To use cache, we need to constrain the input size.
    if (use_cache_) {
        if (this->m() < 22) {
            use_encode_table_ = true;
            encode_table_.resize(1 << this->m());
            encode_table_cached_.resize(1 << this->m());
        }
        else if (this->m() < 64) {
            use_encode_map_ = true;
        }
        if (this->t() < 22) {
            use_decode_table_ = true;
            decode_table_.resize(1 << this->t());
            decode_table_cached_.resize(1 << this->t());
        }
    }

    base_field_context_ = GF2EContext(converter_->base_field_poly());
}

void CompositeGf2MFE::encode(NTL::vec_GF2& h, const NTL::GF2X& g) {
    if (deg(g) + 1 > m())
        LogicError("Input polynomial g has an invalid length");

    long idx = deg(g) == -1 ? 0 : g.xrep[0];
    if (use_cache_) {
        if (use_encode_table_ && encode_table_cached_[idx]) {
            h = encode_table_[idx];
            return;
        }
        if (use_encode_map_ && encode_map_.contains(idx)) {
            h = encode_map_.get(idx);
            return;
        }
    }

    GF2E g_ = to_GF2E(g, converter_->binary_field_poly());
    GF2EX g_comp = converter_->binary_to_composite(g_);

    vec_GF2E y = mfe2_->encode(g_comp);

    h.SetLength(t());
    for (int i = 0; i < y.length(); i++) {
        vec_GF2 yi = mfe1_->encode(rep(y.at(i)));
        for (int j = 0; j < yi.length(); j++)
            h.at(i * mfe1_->t() + j) = yi.at(j);
    }

    if (use_cache_) {
        if (use_encode_table_) {
            encode_table_[idx] = h;
            encode_table_cached_[idx] = true;
        }
        else if (use_encode_map_) {
            encode_map_.insert(idx, h);
        }
    }
}

void CompositeGf2MFE::decode(NTL::GF2X& g, const NTL::vec_GF2& h) {
    if (h.length() != t())
        LogicError("Input vector h has an invalid length");

    if (use_cache_) {
        if (use_decode_table_ && decode_table_cached_[h.rep[0]]) {
            g = decode_table_[h.rep[0]];
            return;
        }
    }

    GF2EPush push;
    // GF2E::init(converter_->base_field_poly());
    base_field_context_.restore();
    vec_GF2E y({}, mfe2_->t());

    acc_time_log("MFE 1 decode " + to_string(mfe1_->t()) + "," + to_string(mfe1_->m()));
    for (int i = 0; i < y.length(); i++) {
        
        // vec_GF2 yi({}, mfe1_->t());
        // for (int j = 0; j < yi.length(); j++)
        //     yi.at(j) = h.at(i * mfe1_->t() + j);
        // GF2X yi_dec = mfe1_->decode(yi);
        // y.at(i) = to_GF2E(yi_dec, converter_->base_field_poly());

        vec_GF2 yi({}, mfe1_->t());
        auto h_it = h.begin() + i*mfe1_->t();
        auto yi_it = yi.begin();
        for (; yi_it != yi.end(); h_it++, yi_it++)
            *yi_it = *h_it;
        mfe1_->decode(y.at(i)._GF2E__rep, yi);
    }
    acc_time_log("MFE 1 decode " + to_string(mfe1_->t()) + "," + to_string(mfe1_->m()));
    acc_time_log("MFE 2 decode " + to_string(mfe1_->t()) + "," + to_string(mfe1_->m()));
    GF2EX g_comp = mfe2_->decode(y);
    acc_time_log("MFE 2 decode " + to_string(mfe1_->t()) + "," + to_string(mfe1_->m()));
    acc_time_log("MFE c2b " + to_string(t()) + "," + to_string(m()));
    g = rep(converter_->composite_to_binary(g_comp));
    acc_time_log("MFE c2b " + to_string(t()) + "," + to_string(m()));
    assert(deg(g) < m());

    if (use_cache_ && use_decode_table_) {
        decode_table_[h.rep[0]] = g;
        decode_table_cached_[h.rep[0]] = true;
    }
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
    if (deg(x) >= deg(poly_mod)) {
        GF2EPush push;
        GF2E::init(poly_mod);
        return to_GF2E(x);
    }
    else {
        GF2E res;
        res._GF2E__rep = x;
        return res;
    }
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


GF2EX to_GF2EX(const vec_GF2& x) {
    acc_time_log("to_GF2EX");
    long k = GF2E::degree();
    if (x.length() % k != 0)
        LogicError("Input vector length is not a multiple of base field polynomial degree.");
    
    vec_GF2E y({}, x.length() / k);
    for (long i = 0; i < y.length(); i++) {
        vec_GF2 tmp({}, k);
        for (long j = 0; j < k; j++) {
            tmp[j] = x[i * k + j];
        }
        y[i] = to_GF2E(to_GF2X(tmp));
    }
    GF2EX res = to_GF2EX(y);
    acc_time_log("to_GF2EX");
    return res;
}

vec_GF2 to_vec_GF2(const GF2EX& x, const GF2X& base_field_poly, long target_len) {
    long x_deg = deg(x);
    long base_field_poly_deg = deg(base_field_poly);
    if ((x_deg+1) * base_field_poly_deg > target_len ||
        target_len % base_field_poly_deg != 0)
        LogicError("Invalid target length");

    vec_GF2 y({}, target_len);
    for (long i = 0; i <= x_deg; i++) {
        for (long j = 0; j < base_field_poly_deg; j++) {
            y[i * base_field_poly_deg + j] = coeff(rep(coeff(x, i)), j);
        }
    }
    return y;
}



FieldConverter::FieldConverter(long binary_field_deg, long base_field_deg, long extension_deg, GF2X prespecified_base_field_poly)
    : k_(binary_field_deg), n_(base_field_deg), m_(extension_deg), prespecified_base_field_poly_(prespecified_base_field_poly) {
    if (k_ != m_ * n_)
        LogicError("Invalid field composition parameters");
    if (PRIMITIVE_POLYS.count(k_) == 0)
        LogicError("No primitive polynomial is defined for this binary field");
    if (prespecified_base_field_poly != GF2X(0) && deg(prespecified_base_field_poly) != n_)
        LogicError("Invalid degree of prespecified base field polynomial");
    ZZ r = ((ZZ(1) << k_) - 1) / ((ZZ(1) << n_) - 1);

    p_ = indices_to_gf2x(PRIMITIVE_POLYS[k_]);
    GF2EPush push;
    GF2E::init(p_);

    GF2E alpha = to_GF2E(GF2X(1, 1));
    GF2E gamma = power(alpha, r);

    vec_vec_GF2E alpha_power({}, m_);
    for (long j = 0; j < m_; j++) {
        alpha_power[j].SetLength(n_);
        for (long i = 0; i < n_; i++) {
            alpha_power[j][i] = power(alpha, r * i + j);
        }
    }
    vec_vec_GF2 T({}, k_);
    for (long h = 0; h < k_; h++) {
        T[h].SetLength(k_);
        for (long j = 0; j < m_; j++) {
            for (long i = 0; i < n_; i++) {
                T[h][j*n_ + i] = coeff(rep(alpha_power[j][i]), h);
            }
        }
    }
    MakeMatrix(mat_T_, T);
    mat_T_inv_ = inv(mat_T_);
    pre_isomorphic_mat_ = ident_mat_GF2(k_);
    pre_isomorphic_mat_inv_ = ident_mat_GF2(k_);

    GF2EX u(1);
    for (long i = 0; i < n_; i++) {
        GF2EX tmp;
        tmp.SetLength(2);
        tmp[0] = power(gamma, (ZZ(1) << (n_-i-1)));
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

    if (prespecified_base_field_poly != GF2X(0) && prespecified_base_field_poly != u_) {
        // The isomorphic mapping is calculated based on this post:
        // https://mathwiki.cs.ut.ee/finite_fields/04_isomorphisms
        GF2EPush push;
        GF2E::init(prespecified_base_field_poly);
        vec_GF2E sb_vec;
        Enumerate_GF2E(sb_vec, 2, (1<<n_) - 2);
        int i = 0;
        for (; i < sb_vec.length(); i++) {
            if (IsZero(eval(u, sb_vec[i])))
                break;
        }
        if (i >= sb_vec.length())
            LogicError("Invalid prespecified base field polynomial: can't find a satisfying element");
        GF2E sb = sb_vec[i];
        vec_vec_GF2 mat({}, n_);
        for (long h = 0; h < n_; h++) {
            mat[h].SetLength(n_);
            GF2E sb_power = power(sb, h);
            for (long i = 0; i < n_; i++) {
                mat[h][i] = coeff(rep(sb_power), i);
            }
        }
        mat_GF2 single_mat, single_mat_inv, tmp;
        MakeMatrix(single_mat, mat);
        transpose(tmp, single_mat);
        transpose(single_mat, single_mat);
        single_mat_inv = inv(single_mat);

        // replicate the single matrix m times on the diagnal
        pre_isomorphic_mat_ = mat_GF2({}, k_, k_);
        pre_isomorphic_mat_inv_ = mat_GF2({}, k_, k_);
        for (i = 0; i < m_; i++) {
            for (int j = 0; j < n_; j++) {
                for (int k = 0; k < n_; k++) {
                    pre_isomorphic_mat_[i*n_+j][i*n_+k] = single_mat[j][k];
                    pre_isomorphic_mat_inv_[i*n_+j][i*n_+k] = single_mat_inv[j][k];
                }
            }
        }

        // MakeMatrix(pre_isomorphic_mat_, mat);
        // transpose(pre_isomorphic_mat_, pre_isomorphic_mat_);
        // pre_isomorphic_mat_inv_ = inv(pre_isomorphic_mat_);
    }

    combined_c2b_mat_ = mat_T_ * pre_isomorphic_mat_inv_;
    combined_b2c_mat_ = pre_isomorphic_mat_ * mat_T_inv_;

    GF2EX q(1);
    for (long i = 0; i < m_; i++) {
        GF2EX tmp;
        tmp.SetLength(2);
        tmp[0] = power(alpha, (ZZ(1) << (i*n_)));
        tmp[1] = 1;
        q = q * tmp;
    }
    q_.SetLength(deg(q) + 1);
    for (long i = 0; i <= deg(q); i++) {
        vec_GF2 a = to_vec_GF2(q[i], k_), b;
        raw_binary_to_composite(b, a);
        // All coefficients of q should belong to GF(2^n), hence any value after index n should be 0.
        for (long j = n_; j < k_; j++) {
            if (IsOne(b[j])) {
                LogicError("Error computing composite field polynomial");
            }
        }
        vec_GF2 c = VectorCopy(b, n_);
        // Here we should use u_ as the base field irreducible polynomial for q_'s coefficients,
        // but since deg(u_) < deg(p_), the conversion here should be fine.
        q_[i] = to_GF2E(c, u_);
    }

    base_field_context_ = GF2EContext(base_field_poly());
}

const GF2X& FieldConverter::binary_field_poly() {
    return p_;
}
const GF2X& FieldConverter::base_field_poly() {
    if (prespecified_base_field_poly_ == GF2X(0))
        return u_;
    else 
        return prespecified_base_field_poly_;
}
const GF2EX& FieldConverter::composite_field_poly() {
    return q_;
}

void FieldConverter::raw_composite_to_binary(vec_GF2& y, const vec_GF2& x) {
    acc_time_log("raw_composite_to_binary");
    if (x.length() != k_) {
        LogicError("FieldConverter c2b: Input vector has invalid length");
    }

    mul(y, combined_c2b_mat_, x);
    acc_time_log("raw_composite_to_binary");
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
    acc_time_log("raw_binary_to_composite");
    if (x.length() != k_) {
        LogicError("FielConverter b2c: Input vector has invalid length");
    }

    mul(y, combined_b2c_mat_, x);
    acc_time_log("raw_binary_to_composite");
}

vec_GF2 FieldConverter::raw_binary_to_composite(const vec_GF2& x) {
    vec_GF2 y;
    raw_binary_to_composite(y, x);
    return y;
}

GF2EX FieldConverter::binary_to_composite(const GF2E& x) {
    vec_GF2 b = raw_binary_to_composite(to_vec_GF2(x, k_));
    GF2EPush push;
    base_field_context_.restore();
    return to_GF2EX(b);
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
    base_field_poly_mod_ = std::move(base_field_poly);
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

    vec_vec_GF2E beta_power({}, k_);
    for (long j = 0; j < k_; j++) {
        beta_power[j].SetLength(m_);
        for (long i = 0; i < m_; i++) {
            beta_power[j][i] = power(beta_[j], i);
        }
    }
    MakeMatrix(beta_matrix_, beta_power);

    base_field_context_ = GF2EContext(base_field_poly_mod_);
}

void BasicRMFE::encode(GF2EX& g, const vec_GF2E& h) {
    GF2EPush push;
    // GF2E::init(base_field_poly_mod_);
    base_field_context_.restore();
    ::phi(g, h, basis_, beta_);
}

void BasicRMFE::decode(vec_GF2E& h, const GF2EX& g) {
    acc_time_log("BasicRMFE decode " + to_string(k_) + ", " + to_string(m_));
    GF2EPush push;
    // GF2E::init(base_field_poly_mod_);
    base_field_context_.restore();
    if (use_fast_basis_) {
        if (use_precompute_beta_matrix_)
            ::psi_fast_precomp(h, g, beta_matrix_);
        else
            ::psi_fast(h, g, beta_);
    }
    else {
        if (use_precompute_beta_matrix_)
            ::psi_precomp(h, g, basis_, beta_matrix_);
        else
            ::psi(h, g, basis_, beta_);
    }
    acc_time_log("BasicRMFE decode " + to_string(k_) + ", " + to_string(m_));
}

void BasicRMFE::random_preimage(GF2EX& h, const vec_GF2E& g) {

    if (g.length() != k())
        LogicError("Invalid input length to random_preimage");
    // Sample a random sub polynomial
    GF2EX tmp = random_GF2EX(m() - k() + 1);
    if (deg(tmp) < 0)
        tmp.SetLength(1);
    tmp[0] = g[0];

    // Correct the evaluation vector by subtracting the evaluation of the above random poly.
    vec_GF2E corrected_g({}, k() - 1), tmp_beta({}, k() - 1);
    for(int i = 0; i < corrected_g.length(); i++) {
        GF2E q;
        eval(q, tmp, beta_[i + 1]);
        // [zico] TODO: might be able to optimize by storing the inv
        GF2E inv_beta = inv(power(beta_[i+1], m()-k()+1));
        corrected_g[i] = (g[i+1] -  q) * inv_beta;
        tmp_beta[i] = beta_[i+1];
    }
    // Interpolate the remaining part
    GF2EX remaining_f;
    interpolate(remaining_f, tmp_beta, corrected_g);

    // Set the final polynomial
    h.SetLength(m());
    for (int i = 0; i < m() - k() + 1; i++)
        h[i] = tmp[i];
    for (int i = 0; i <= deg(remaining_f); i++)
        h[i + m() - k() + 1] = remaining_f[i];
}

BasicGf2RMFE::BasicGf2RMFE(long k, bool is_type1) {
    internal_ = unique_ptr<BasicRMFE>(new BasicRMFE(k, 1, is_type1));
    const GF2EX& f = internal_->ex_field_mod();
    ex_field_poly_ = GF2XModulus(shrink(f));

    // To use cache, we need to constrain the input size.
    if (use_cache_) {
        assert(internal_->k() < 22);
        assert(internal_->m() < 22);
        
        encode_table_.resize(1 << internal_->k());
        encode_table_cached_.resize(1 << internal_->k());
        decode_table_.resize(1 << internal_->m());
        decode_table_cached_.resize(1 << internal_->m());
    }

    base_field_context_ = GF2EContext(GF2X(1, 1));
}

void BasicGf2RMFE::decode(vec_GF2& h, const GF2X& g) {
    if (deg(g) + 1 > m())
        LogicError("Input polynomial g has an invalid length");

    long idx = deg(g) == -1 ? 0 : g.xrep[0];
    if (use_cache_ && decode_table_cached_[idx]) {
        h = decode_table_[idx];
        return;
    }

    GF2EPush push;
    // GF2E::init(GF2X(1, 1));
    base_field_context_.restore();
    GF2EX g_ = lift(g);

    vec_GF2E h_ = internal_->decode(g_);
    if (h_.length() != k())
        LogicError("Output vector h has an invalid length");

    h = shrink(h_);

    if (use_cache_) {
        decode_table_[idx] = h;
        decode_table_cached_[idx] = true;
    }
}

void BasicGf2RMFE::encode(GF2X& g, const vec_GF2& h) {
    if (h.length() != k())
        LogicError("Input vector h has an invalid length");

    GF2EPush push;
    // GF2E::init(GF2X(1, 1));
    base_field_context_.restore();
    vec_GF2E h_ = lift(h);
    
    GF2EX g_ = internal_->encode(h_);
    if (deg(g_) + 1 > m())
        LogicError("Output polynomial g has an invalid length");

    g = shrink(g_);
}

void BasicGf2RMFE::random_preimage(NTL::GF2X& g, const NTL::vec_GF2& h) {
    GF2EPush push;
    // GF2E::init(GF2X(1, 1));
    base_field_context_.restore();
    vec_GF2E h_ = lift(h);

    GF2EX g_ = internal_->random_preimage(h_);
    if (deg(g_) + 1 > m())
        LogicError("Output polynomial g has an invalid length");

    g = shrink(g_);
}

CompositeGf2RMFE::CompositeGf2RMFE(std::shared_ptr<FieldConverter> converter, std::shared_ptr<Gf2RMFE> rmfe1, std::shared_ptr<Gf2eRMFE> rmfe2)
    : converter_(converter), rmfe1_(rmfe1), rmfe2_(rmfe2) {
    if (converter->base_field_poly() != rmfe2->base_field_mod())
        LogicError("Converter and rmfe1's base field polys are different");
    if (converter->composite_field_poly() != rmfe2->ex_field_mod())
        LogicError("Converter and rmfe1's composite field polys are different");
    if (rmfe2->base_field_size() != (long) pow(rmfe1->base_field_size(), rmfe1->m()))
        LogicError("Cannot concatenate two RMFEs with incompatible field sizes");
    if (rmfe1->base_field_size() != 2)
        LogicError("rmfe2's base field must be GF(2)");

    m_ = rmfe1->m() * rmfe2->m();
    k_ = rmfe1->k() * rmfe2->k();

    ex_field_poly_ = converter_->binary_field_poly();

    // To use cache, we need to constrain the input size.
    if (use_cache_) {
        assert(k_ < 64);
        assert(m_ < 64);
        
        encode_table_.resize(1 << k_);
        encode_table_cached_.resize(1 << k_);
    }

    base_field_context_ = GF2EContext(converter_->base_field_poly());
}

void CompositeGf2RMFE::encode(NTL::GF2X& g, const NTL::vec_GF2& h) {
    if (h.length() != k())
        LogicError("Input vector h has an invalid length");
    
    if (use_cache_ && encode_table_cached_[h.rep[0]]) {
        g = encode_table_[h.rep[0]];
        return;
    }

    GF2EPush push;
    // GF2E::init(converter_->base_field_poly());
    base_field_context_.restore();
    vec_GF2E y({}, rmfe2_->k());

    for (int i = 0; i < y.length(); i++) {
        vec_GF2 yi({}, rmfe1_->k());
        for (int j = 0; j < yi.length(); j++)
            yi.at(j) = h.at(i * rmfe1_->k() + j);
        y.at(i) = to_GF2E(rmfe1_->encode(yi), converter_->base_field_poly());
    }
    GF2EX g_comp = rmfe2_->encode(y);
    g = rep(converter_->composite_to_binary(g_comp));

    if (use_cache_) {
        encode_table_[h.rep[0]] = g;
        encode_table_cached_[h.rep[0]] = true;
    }
}

void CompositeGf2RMFE::decode(NTL::vec_GF2& h, const NTL::GF2X& g) {
    if (deg(g) + 1 > m())
        LogicError("Input polynomial g has an invalid length");

    long idx = deg(g) == -1 ? 0 : g.xrep[0];
    if (use_cache_ && decode_map_.contains(idx)) {
        h = decode_map_.get(idx);
        return;
    }

    GF2E g_ = to_GF2E(g, converter_->binary_field_poly());
    acc_time_log("RMFE b2c " + to_string(k()) + "," + to_string(m()));
    GF2EX g_comp = converter_->binary_to_composite(g_);
    acc_time_log("RMFE b2c " + to_string(k()) + "," + to_string(m()));

    acc_time_log("RMFE 2 decode " + to_string(rmfe2_->k()) + "," + to_string(rmfe2_->m()));
    vec_GF2E y = rmfe2_->decode(g_comp);
    acc_time_log("RMFE 2 decode " + to_string(rmfe2_->k()) + "," + to_string(rmfe2_->m()));

    acc_time_log("RMFE 1 decode " + to_string(rmfe1_->k()) + "," + to_string(rmfe1_->m()));
    h.SetLength(k());
    // for (int i = 0; i < y.length(); i++) {
    //     vec_GF2 yi = rmfe1_->decode(rep(y.at(i)));
    //     for (int j = 0; j < yi.length(); j++)
    //         h.at(i * rmfe1_->k() + j) = yi.at(j);
    // }

    auto h_it = h.begin();
    for (int i = 0; i < y.length(); i++) {
        vec_GF2 yi = rmfe1_->decode(rep(y.at(i)));
        auto yi_it = yi.begin();
        for (; yi_it != yi.end(); yi_it++, h_it++)
            *h_it = *yi_it;
    }
    acc_time_log("RMFE 1 decode " + to_string(rmfe1_->k()) + "," + to_string(rmfe1_->m()));

    if (use_cache_)
        decode_map_.insert(idx, h);
}

void CompositeGf2RMFE::random_preimage(NTL::GF2X& g, const NTL::vec_GF2& h) {
    if (h.length() != k())
        LogicError("Input vector h has an invalid length");

    GF2EPush push;
    // GF2E::init(converter_->base_field_poly());
    base_field_context_.restore();
    vec_GF2E y({}, rmfe2_->k());

    for (int i = 0; i < y.length(); i++) {
        vec_GF2 yi({}, rmfe1_->k());
        for (int j = 0; j < yi.length(); j++)
            yi.at(j) = h.at(i * rmfe1_->k() + j);
        y.at(i) = to_GF2E(rmfe1_->random_preimage(yi), converter_->base_field_poly());
    }
    GF2EX g_comp = rmfe2_->random_preimage(y);
    g = rep(converter_->composite_to_binary(g_comp));
}

std::unique_ptr<Gf2RMFE> get_composite_gf2_rmfe_type2(long k1, long k2) {
    long m1 = 2*k1, m2 = 2*k2;
    shared_ptr<FieldConverter> converter = make_shared<FieldConverter>(m1 * m2, m1, m2);
    shared_ptr<Gf2RMFE> rmfe1 = make_shared<BasicGf2RMFE>(k1, false);
    shared_ptr<Gf2eRMFE> rmfe2 = make_shared<BasicRMFE>(converter->base_field_poly(), converter->composite_field_poly());

    // Precompute small GF2E fields
    GF2EPrecomp::generate_table(GF2X(1, 1));
    if (deg(converter->base_field_poly()) < 10)
        GF2EPrecomp::generate_table(converter->base_field_poly());
    
    return unique_ptr<Gf2RMFE>(new CompositeGf2RMFE(converter, rmfe1, rmfe2));
}

std::unique_ptr<Gf2MFE> get_double_composite_gf2_mfe(long m1, long m2, long m3) {
    shared_ptr<FieldConverter> converter1 = make_shared<FieldConverter>(m1 * m2, m1, m2);
    shared_ptr<FieldConverter> converter2 = make_shared<FieldConverter>(m1 * m2 * m3, m1*m2, m3, converter1->binary_field_poly());
    shared_ptr<Gf2MFE> mfe1 = make_shared<BasicGf2MFE>(m1);
    shared_ptr<Gf2eMFE> mfe2 = make_shared<BasicMFE>(converter1->base_field_poly(), converter1->composite_field_poly());
    shared_ptr<CompositeGf2MFE> mfe3 = make_shared<CompositeGf2MFE>(converter1, mfe1, mfe2);
    
    shared_ptr<Gf2eMFE> mfe4 = make_shared<BasicMFE>(converter2->base_field_poly(), converter2->composite_field_poly());

    // Precompute small GF2E fields
    GF2EPrecomp::generate_table(GF2X(1, 1));
    if (deg(converter1->base_field_poly()) < 10)
        GF2EPrecomp::generate_table(converter1->base_field_poly());
    if (deg(converter2->base_field_poly()) < 10)
        GF2EPrecomp::generate_table(converter2->base_field_poly());

    return unique_ptr<Gf2MFE>(new CompositeGf2MFE(converter2, mfe3, mfe4));
}
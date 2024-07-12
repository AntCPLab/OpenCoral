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
#include "NTL/tools.h"
#include <iostream>
#include <unordered_set>
#include <cmath>
#include <vector>
#include "Tools/performance.h"
#include <assert.h>
#include "mfe64.h"

using namespace std;

ostream& operator<<(ostream& s, const vec_gf2e& a) { 
   long i, n;   
  
   n = a.size();  
   
   s << '[';   
   
   for (i = 0; i < n; i++) {   
      s << a[i];   
      if (i < n-1) s << " ";   
   }   
   
   s << ']';   
      
   return s;   
}

bool operator==(const vec_gf2e& a, const vec_gf2e& b) {
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); i++)
        if (a[i] != b[i])
            return false;
    return true;
}

void enumerate_gf2e(vec_gf2e& x, size_t start, size_t n) {
    if ((start + n) > NTL::GF2E::cardinality())
        NTL::LogicError("# of requested elements larger than field size");
    x.resize(n);
    for (size_t i = start; i < n; i++) {
        x[i-start] = i;
    }
}

/**
 * https://en.m.wikipedia.org/wiki/Hamming_weight#Efficient_implementation
*/
long count_ones(uint64_t x) {
    int count;
    for (count=0; x; count++)
        x &= x - 1;
    return count;
}

void mul(vec_gf2_64& x, const mat_gf2_64& A, const vec_gf2_64& b) {
    if (A.size() > 64)
        NTL::LogicError("Matrix row count is larger than 64.");
    x = 0;
    for (size_t i = 0; i < A.size(); i++) {
        x ^= (count_ones(A[i] & b) & 1) << i;
    }
}


void gf2e_precomp::generate_table(const NTL::GF2X& poly_mod) {
    if (deg(poly_mod) > 63)
        NTL::LogicError("Poly modulus degree > 63 for gf2e_precomp");
    if (deg(poly_mod) < 0)
        return;
    NTL::GF2EPush push;
    NTL::GF2E::init(poly_mod);
    long idx = poly_mod.xrep[0];
    size_t n = 1 << deg(poly_mod);
    vector<vector<gf2e>>& t_mul = mul_table_;
    vector<gf2e>& t_inv = inv_table_;
    vector<vector<long>>& t_pow = pow_table_;
    t_mul.resize(n, vector<gf2e>(n));
    t_inv.resize(n);
    t_pow.resize(n, vector<long>(MAX_POWER + 1));
    for (size_t i = 0; i < n; i++) {
        NTL::GF2E x = gf2e_to_ntl_GF2E(i);
        for (size_t j = 0; j < n; j++) {
            NTL::GF2E y = gf2e_to_ntl_GF2E(j);
            t_mul[i][j] = ntl_GF2E_to_gf2e(x * y);
        }
        for (long j = 0; j <= MAX_POWER; j++)
            t_pow[i][j] = ntl_GF2E_to_gf2e(NTL::power(x, j));
        if (NTL::IsZero(x))
            t_inv[i] = 0;
        else
            t_inv[i] = ntl_GF2E_to_gf2e(NTL::inv(x));
    }
}

void gf2e_precomp::interpolate(gf2ex& f, const vec_gf2e& a, const vec_gf2e& b)
{
   long m = a.size();
   if (b.size() != (size_t) m) NTL::LogicError("interpolate: vector length mismatch");

   if (m == 0) {
      f.clear();
      return;
   }

   vec_gf2e prod;
   prod = a;

   gf2e t1, t2;

   long k, i;

   vec_gf2e res;
   res.resize(m);

   for (k = 0; k < m; k++) {

      const gf2e& aa = a[k];

      t1 = 1;
      for (i = k-1; i >= 0; i--) {
         t1 = mul(t1, aa);
         t1 = add(t1, prod[i]);
      }

      t2 = 0;
      for (i = k-1; i >= 0; i--) {
         t2 = mul(t2, aa);
         t2 = add(t2, res[i]);
      }
      
      t1 = inv(t1);
      t2 = sub(b[k], t2);
      t1 = mul(t1, t2);

      for (i = 0; i < k; i++) {
         t2 = mul(prod[i], t1);
         res[i] = add(res[i], t2);
      }

      res[k] = t1;

      if (k < m-1) {
         if (k == 0) {

         }
         else {
            t1 = a[k];
            prod[k] = add(t1, prod[k-1]);
            for (i = k-1; i >= 1; i--) {
               t2 = mul(prod[i], t1);
               prod[i] = add(t2, prod[i-1]);
            }
            prod[0] = mul(prod[0], t1);
         }
      }
   }

   while (m > 0 && res[m-1] == 0) m--;
   res.resize(m);
   f = std::move(res);
}

void gf2e_precomp::eval(gf2e& b, const gf2ex& f, const gf2e& a)
// does a Horner evaluation
{
    gf2e acc = 0;
    long i;

    for (i = deg(f); i >= 0; i--) {
        acc = mul(acc, a);
        acc = add(acc, f[i]);
    }

    b = acc;
}


gf2ex& operator +=(gf2ex& a, const gf2ex& b) {
    if (a.size() < b.size())
        a.resize(b.size());
    auto ita = a.begin();
    auto itb = b.begin();
    for(; ita != a.end() && itb != b.end(); ita++, itb++) {
        *ita = add(*ita, *itb);
    }
    return a;
}

void mfe_precomp::mu_1(gf2ex& g, const gf2ex& f, const vec_gf2ex& basis) {
    g.clear();
    for(int i = 0; i <= deg(f); i++) {
        g += mul(basis.at(i), coeff(f, i));
    }
}

// void inv_mu_1(gf2ex& f, const gf2ex& g, const vec_gf2ex& basis) {
//     long m = basis.length();
//     if (deg(g) + 1 > m)
//         LogicError("inv_mu_1: Invalid input g");
//     vec_vec_GF2E basis_({}, basis.length());
//     for(int i = 0; i < basis.length(); i++) {
//         conv(basis_[i], basis[i]);
//         basis_[i].SetLength(m);
//     }
//     mat_GF2E basis_mat;
//     MakeMatrix(basis_mat, basis_);

//     vec_GF2E g_vec;
//     conv(g_vec, g);
//     g_vec.SetLength(m);

//     GF2E d;
//     vec_GF2E f_vec;
//     // f(\alpha) = g, equivalent to
//     // f * basis = g, solve f
//     solve(d, f_vec, basis_mat, g_vec);
//     conv(f, f_vec);
// }

/**
 * len(g) = m
 * deg(f) = m-1
 * len(beta) = m
 * beta[0] = 0
 * (f_{beta[0]}, f(beta[1]), f(beta[2]), ..., f(beta[m-1])) --> f
*/
void mfe_precomp::inv_xi_1(gf2ex& f, const vec_gf2e& g, const vec_gf2e& beta) {
    if (g.size() != beta.size())
        NTL::LogicError("Invalid input to inv_xi_1");
    // Make sure beta[0] is 0 so that we can map identity to identity in RMFE
    if (beta[0] != 0)
        NTL::LogicError("beta[0] must be zero in RMFE");

    interpolate(f, beta, g);
}

/**
 * deg(f) = 2k-2 (type1) for 2k-1 (type2)
 * len(beta) = k
 * beta[0] = 0
 * f --> (f(beta[0]), f(beta[1]), ..., f(beta[k-1]))
*/
void mfe_precomp::xi_2(vec_gf2e& g, const gf2ex& f, const vec_gf2e& beta) {
    if (beta[0] != 0)
        NTL::LogicError("beta[0] must be zero in RMFE");

    long k = beta.size();
    g.resize(k);
    for(size_t i = 0; i < beta.size(); i++) {
        eval(g[i], f, beta[i]);
    }
}

void mfe_precomp::pi_1(gf2ex& g, const gf2ex& f, const vec_gf2ex& basis) {
    mu_1(g, f, basis);
}

void mfe_precomp::phi(gf2ex& g, const vec_gf2e& h, const vec_gf2ex& basis, const vec_gf2e& beta) {
    gf2ex f;
    inv_xi_1(f, h, beta);
    pi_1(g, f, basis);
}

// void psi(vec_gf2e& h, const gf2ex& g, const vec_gf2ex& basis, const vec_gf2e& beta) {
//     gf2ex f;
//     inv_pi_1(f, g, basis);
//     xi_2(h, f, beta);
// }

void mfe_precomp::psi_fast(vec_gf2e& h, const gf2ex& g, const vec_gf2e& beta) {
    xi_2(h, g, beta);
}

/**
 * deg(f) = m-1
 * len(beta) = 2m-2
 * f --> (f(beta[0]), f(beta[1]), ..., f(beta[2m-3]), f_{m-1})
*/ 
void mfe_precomp::nu_1(vec_gf2e& g, const gf2ex& f, const vec_gf2e& beta) {
    g.resize(beta.size() + 1);
    for(size_t i = 0; i < beta.size(); i++) {
        eval(g[i], f, beta[i]);
    }
    // f_{m-1}
    g[beta.size()] = coeff(f, beta.size() / 2);
}

/**
 * len(g) = m
 * deg(f) = m-1
 * len(beta) = m-1
 * (f(beta[0]), f(beta[1]), ..., f(beta[m-2]), f_{m-1}) --> f
*/
void mfe_precomp::inv_mu_2(gf2ex& f, const vec_gf2e& g, const vec_gf2e& beta) {
    if (g.size() != beta.size() + 1)
        NTL::LogicError("Invalid input to inv_mu_2");
    vec_gf2e corrected_g(g.size() - 1);
    for(size_t i = 0; i < corrected_g.size(); i++) {
        corrected_g[i] = sub(g[i], mul(g[g.size() - 1], power(beta[i], corrected_g.size())));
    }
    interpolate(f, beta, corrected_g);
    set_coeff(f, corrected_g.size(), g[g.size() - 1]);
}

/**
 * deg(f) = 2*m-2
 * len(alpha_power) = 2*m-1
 * deg(g) = m-1
*/
void mfe_precomp::nu_2(gf2ex& g, const gf2ex& f, const vec_gf2ex& alpha_power) {
    g.clear();
    for(int i = 0; i <= deg(f); i++) {
        g += mul(alpha_power[i], coeff(f, i));
    }
}

void mfe_precomp::sigma_fast(vec_gf2e& h, const gf2ex& g, const vec_gf2e& beta) {
    nu_1(h, g, beta);
}

void mfe_precomp::rho(gf2ex& g, const vec_gf2e& h, const vec_gf2ex& alpha_power, const vec_gf2e& beta) {
    gf2ex f;
    inv_mu_2(f, h, beta);
    nu_2(g, f, alpha_power);
}

BasicMFE64::BasicMFE64(long m, long base_field_poly_mod_deg)
    : m_(m), t_(2*m-1) {
    base_field_poly_mod_ = NTL::BuildIrred_GF2X(base_field_poly_mod_deg);
    NTL::GF2EPush push;
    NTL::GF2E::init(base_field_poly_mod_);

    ex_field_poly_mod_ = NTL::GF2EXModulus(NTL::BuildIrred_GF2EX(m));

    initialize();
}

BasicMFE64::BasicMFE64(NTL::GF2X base_field_poly, NTL::GF2EX ex_field_poly) {
    base_field_poly_mod_ = std::move(base_field_poly);
    NTL::GF2EPush push;
    NTL::GF2E::init(base_field_poly_mod_);
    ex_field_poly_mod_ = NTL::GF2EXModulus(ex_field_poly);

    m_ = deg(ex_field_poly_mod_);
    t_ = 2 * m_ - 1;

    initialize();
}

void BasicMFE64::initialize() {
    alpha_ = NTL::GF2EX(1, 1); // alpha = 0 + 1*Y + 0*Y^2 + ... = Y
    extented_alpha_power_ = vec_gf2ex(2*m_-1);
    extented_alpha_power_[0] = gf2ex({1});
    NTL::GF2EX prev(1), next;
    for (int i = 1; i <= 2*m_-2; i++) {
        MulMod(next, prev, alpha_, ex_field_poly_mod_);
        ntl_GF2EX_to_gf2ex(extented_alpha_power_[i], next);
        prev = std::move(next);
    }
    // TODO: actually, in this setting, the basis is just {1, Y, Y^2, ..., Y^(m-1)}
    // Hence mu_1 and inv_mu_1 are both just identity functions (just run them to verify)
    // So things can be simpfiled a lot. Keep algorithm code of mu_1 and inv_mu_1 here
    // in case we could not use "alpha = Y" in some setting.

    // get 2m-2 distinct elements
    enumerate_gf2e(beta_, 0, 2*m_-2);

    base_field_context_ = NTL::GF2EContext(base_field_poly_mod_);

    generate_table(base_field_poly_mod_);
}


void BasicMFE64::encode(vec_gf2e& h, const gf2ex& g) {
    NTL::GF2EPush push;
    base_field_context_.restore();
    sigma_fast(h, g, beta_);
}

void BasicMFE64::decode(gf2ex& g, const vec_gf2e& h) {
    NTL::GF2EPush push;
    base_field_context_.restore();
    rho(g, h, extented_alpha_power_, beta_);
}

BasicGf2MFE64::BasicGf2MFE64(long m) {
    internal_ = unique_ptr<BasicMFE64>(new BasicMFE64(m, 1));
    const NTL::GF2EX& f = internal_->ex_field_mod();
    ex_field_poly_ = NTL::GF2XModulus(shrink(f));

    assert(internal_->m() < 22);
    assert(internal_->t() < 22);
    
    encode_table_.resize(1 << internal_->m());
    decode_table_.resize(1 << internal_->t());

    if (m == 1) {
        encode_table_ = {0, 1};
        decode_table_ = {0, 1};
    }
    else if (m == 2) {
        encode_table_ = {0, 3, 6, 5};
        decode_table_ = {0, 3, 2, 1, 1, 2, 3, 0};
    }
}

gf2x64 BasicGf2MFE64::encode(vec_gf2_64 g) {
    return encode_table_[g];
}

vec_gf2_64 BasicGf2MFE64::decode(gf2x64 h) {
    return decode_table_[h];
}

CompositeGf2MFE64::CompositeGf2MFE64(std::shared_ptr<FieldConverter64> converter, std::shared_ptr<Gf2MFE64> mfe1, std::shared_ptr<Gf2eMFE64> mfe2)
    : converter_(converter), mfe1_(mfe1), mfe2_(mfe2) {
    if (converter->base_field_poly() != mfe2->base_field_mod())
        NTL::LogicError("Converter and mfe2's base field polys are different");
    if (converter->composite_field_poly() != mfe2->ex_field_mod())
        NTL::LogicError("Converter and mfe2's composite field polys are different");
    if (mfe2->base_field_size() != (long) pow(mfe1->base_field_size(), mfe1->m()))
        NTL::LogicError("Cannot concatenate two MFEs with incompatible field sizes");
    if (mfe1->base_field_size() != 2)
        NTL::LogicError("mfe1's base field must be GF(2)");

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
        else if (this->t() < 64) {
            use_decode_map_ = true;
        }
    }

    base_field_context_ = NTL::GF2EContext(converter_->base_field_poly());
}

vec_gf2_64 CompositeGf2MFE64::encode(gf2x64 g) {

    if (use_cache_) {
        if (use_encode_table_ && encode_table_cached_[g]) {
            return encode_table_[g];
        }
        if (use_encode_map_ && encode_map_.contains(g)) {
            return encode_map_.get(g);
        }
    }

    gf2ex g_comp = converter_->binary_to_composite(g);

    vec_gf2e y = mfe2_->encode(g_comp);

    vec_gf2_64 h = 0;
    for (size_t i = 0; i < y.size(); i++) {
        vec_gf2_64 yi = mfe1_->encode(y.at(i));
        h ^= (yi << (i * mfe1_->t()));
    }

    if (use_cache_) {
        if (use_encode_table_) {
            encode_table_[g] = h;
            encode_table_cached_[g] = true;
        }
        else if (use_encode_map_) {
            encode_map_.insert(g, h);
        }
    }
    return h;
}

gf2x64 CompositeGf2MFE64::decode(vec_gf2_64 h) {

    if (use_cache_) {
        if (use_decode_table_ && decode_table_cached_[h]) {
            return decode_table_[h];
        }
        if (use_decode_map_ && decode_map_.contains(h)) {
            return decode_map_.get(h);
        }
    }

    NTL::GF2EPush push;
    base_field_context_.restore();
    vec_gf2e y(mfe2_->t());

    uint64_t mask = (1 << mfe1_->t()) - 1;
    for (size_t i = 0; i < y.size(); i++) {
        vec_gf2_64 yi = (h >> (i * mfe1_->t())) & mask;
        y.at(i) = mfe1_->decode(yi);
    }
    gf2ex g_comp = mfe2_->decode(y);
    gf2e g = converter_->composite_to_binary(g_comp);

    if (use_cache_ && use_decode_table_) {
        decode_table_[h] = g;
        decode_table_cached_[h] = true;
    }
    if (use_cache_ && use_decode_map_) {
        decode_map_.insert(h, g);
    }
    return g;
}

CompositeGf2MFEInternal64::CompositeGf2MFEInternal64(std::shared_ptr<FieldConverter64> converter, std::shared_ptr<Gf2MFE64> mfe1, std::shared_ptr<Gf2eMFE64> mfe2)
    : converter_(converter), mfe1_(mfe1), mfe2_(mfe2) {
    if (converter->base_field_poly() != mfe2->base_field_mod())
        NTL::LogicError("Converter and mfe2's base field polys are different");
    if (converter->composite_field_poly() != mfe2->ex_field_mod())
        NTL::LogicError("Converter and mfe2's composite field polys are different");
    if (mfe2->base_field_size() != (long) pow(mfe1->base_field_size(), mfe1->m()))
        NTL::LogicError("Cannot concatenate two MFEs with incompatible field sizes");
    if (mfe1->base_field_size() != 2)
        NTL::LogicError("mfe1's base field must be GF(2)");

    m_ = mfe1->m() * mfe2->m();
    t_ = mfe1->t() * mfe2->t();

    if (m_ > 64 || mfe1->t() > 64 || mfe2->t() > 64)
        NTL::LogicError("Param m or t is larger than 64");

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
        else if (this->t() < 64) {
            use_decode_map_ = true;
        }
    }

    base_field_context_ = NTL::GF2EContext(converter_->base_field_poly());
}

NTL::vec_GF2 CompositeGf2MFEInternal64::encode(gf2x64 g) {

    if (use_cache_) {
        if (use_encode_table_ && encode_table_cached_[g]) {
            return encode_table_[g];
        }
        if (use_encode_map_ && encode_map_.contains(g)) {
            return encode_map_.get(g);
        }
    }

    gf2ex g_comp = converter_->binary_to_composite(g);

    vec_gf2e y = mfe2_->encode(g_comp);

    // acc_time_log("MFE 1 encode " + to_string(mfe1_->t()) + "," + to_string(mfe1_->m()));
    // NTL::vec_GF2 h({}, t());
    // auto h_it = h.begin();
    // for (size_t i = 0; i < y.size(); i++) {
    //     vec_gf2_64 yi = mfe1_->encode(y.at(i));
    //     for (int j = 0; j < mfe1_->t(); j++, h_it++)
    //         *h_it = (yi >> j) & 1;
    // }
    // acc_time_log("MFE 1 encode " + to_string(mfe1_->t()) + "," + to_string(mfe1_->m()));

    NTL::WordVector rep({}, (t() + 63) / 64);
    int rep_idx = 0, word_bit_idx = 0;
    rep[rep_idx] = 0;
    for (size_t i = 0; i < y.size(); i++) {
        vec_gf2_64 yi = mfe1_->encode(y.at(i));
        update_ntl_word_vec_rep(rep, rep_idx, word_bit_idx, yi, mfe1_->t());
    }
    NTL::vec_GF2 h({}, t());
    h.rep.swap(rep);

    if (use_cache_) {
        if (use_encode_table_) {
            encode_table_[g] = h;
            encode_table_cached_[g] = true;
        }
        else if (use_encode_map_) {
            encode_map_.insert(g, h);
        }
    }
    return h;
}

gf2x64 CompositeGf2MFEInternal64::decode(const NTL::vec_GF2& h) {
    NTL::GF2EPush push;
    base_field_context_.restore();
    vec_gf2e y(mfe2_->t());

    const NTL::WordVector& rep = h.rep;
    int rep_idx = 0, word_bit_idx = 0;
    for (size_t i = 0; i < y.size(); i++) {
        vec_gf2_64 yi = 0;
        get_ntl_word_vec_rep(rep, rep_idx, word_bit_idx, yi, mfe1_->t());
        // auto h_it = h.begin() + i*mfe1_->t();
        // for (int j = 0; j < mfe1_->t(); j++, h_it++)
        //     yi ^= (*h_it)._GF2__rep << j;

        y.at(i) = mfe1_->decode(yi);
    }
    gf2ex g_comp = mfe2_->decode(y);
    gf2e g = converter_->composite_to_binary(g_comp);
    return g;
}



BasicRMFE64::BasicRMFE64(long k, long base_field_poly_mod_deg, bool is_type1): k_(k) {
    if (is_type1)
        m_ = 2*k-1;
    else
        m_ = 2*k;
    base_field_poly_mod_ = NTL::BuildIrred_GF2X(base_field_poly_mod_deg);
    NTL::GF2EPush push;
    NTL::GF2E::init(base_field_poly_mod_);

    ex_field_poly_mod_ = NTL::GF2EXModulus(NTL::BuildIrred_GF2EX(m_));

    // Different from Lemma 4 in [CCXY18], we require q >= k instead of q >= k+1, in order
    // to make this RMFE have a mapping of identity to identity.
    if (k_ > NTL::GF2E::cardinality())
        NTL::LogicError("Constrait violation: q >= k");

    initialize();
}

BasicRMFE64::BasicRMFE64(NTL::GF2X base_field_poly, NTL::GF2EX ex_field_poly) {
    base_field_poly_mod_ = std::move(base_field_poly);
    NTL::GF2EPush push;
    NTL::GF2E::init(base_field_poly_mod_);
    ex_field_poly_mod_ = NTL::GF2EXModulus(ex_field_poly);

    m_ = deg(ex_field_poly_mod_);
    if (m_ % 2 == 1)
        k_ = (m_ + 1) / 2;
    else
        k_ = m_ / 2;
    // Different from Lemma 4 in [CCXY18], we require q >= k instead of q >= k-1, in order
    // to make this RMFE have a mapping of identity to identity.
    if (k_ > NTL::GF2E::cardinality())
        NTL::LogicError("Constrait violation: q >= k");

    initialize();
}

void BasicRMFE64::initialize() {
    alpha_ = NTL::GF2EX(1, 1); // alpha = 0 + 1*Y + 0*Y^2 + ... = Y
    basis_ = vec_gf2ex(m_);
    basis_[0] = gf2ex({1});
    NTL::GF2EX prev(1), next;
    for (int i = 1; i < m_; i++) {
        MulMod(next, prev, alpha_, ex_field_poly_mod_);
        ntl_GF2EX_to_gf2ex(basis_[i], next);
        prev = std::move(next);
    }
    // TODO: actually, in this setting, the basis is just {1, Y, Y^2, ..., Y^(m-1)}
    // Hence mu_1 and inv_mu_1 are both just identity functions (just run them to verify)
    // So things can be simpfiled a lot. Keep algorithm code of mu_1 and inv_mu_1 here
    // in case we could not use "alpha = Y" in some setting.

    // get k distinct elements
    enumerate_gf2e(beta_, 0, k_);

    // beta_matrix_.resize(k_, vec_gf2e(m_));
    // for (long j = 0; j < k_; j++) {
    //     for (long i = 0; i < m_; i++) {
    //         beta_matrix_[j][i] = power(beta_[j], i);
    //     }
    // }
    // MakeMatrix(beta_matrix_, beta_power);

    base_field_context_ = NTL::GF2EContext(base_field_poly_mod_);

    generate_table(base_field_poly_mod_);
}

void BasicRMFE64::encode(gf2ex& g, const vec_gf2e& h) {
    if ((long) h.size() != k())
        NTL::LogicError("Invalid input length to encode");
    NTL::GF2EPush push;
    base_field_context_.restore();
    phi(g, h, basis_, beta_);
    // // Expand g to have length m
    // g.resize(m(), 0);
}

void BasicRMFE64::decode(vec_gf2e& h, const gf2ex& g) {
    NTL::GF2EPush push;
    base_field_context_.restore();
    psi_fast(h, g, beta_);
}

void BasicRMFE64::random_preimage(gf2ex& h, const vec_gf2e& g) {

    if ((long) g.size() != k())
        NTL::LogicError("Invalid input length to random_preimage");
    // Sample a random sub polynomial
    gf2ex tmp = random_gf2ex(m() - k() + 1);
    if (deg(tmp) < 0)
        tmp.resize(1);
    tmp[0] = g[0];

    // Correct the evaluation vector by subtracting the evaluation of the above random poly.
    vec_gf2e corrected_g(k() - 1), tmp_beta(k() - 1);
    for(size_t i = 0; i < corrected_g.size(); i++) {
        gf2e q;
        eval(q, tmp, beta_[i + 1]);
        gf2e inv_beta = inv(power(beta_[i+1], m()-k()+1));
        corrected_g[i] = mul(sub(g[i+1], q), inv_beta);
        tmp_beta[i] = beta_[i+1];
    }
    // Interpolate the remaining part
    gf2ex remaining_f;
    interpolate(remaining_f, tmp_beta, corrected_g);

    // Set the final polynomial
    h.resize(m());
    for (int i = 0; i < m() - k() + 1; i++)
        h[i] = tmp[i];
    for (int i = 0; i <= deg(remaining_f); i++)
        h[i + m() - k() + 1] = remaining_f[i];
}

BasicGf2RMFE64::BasicGf2RMFE64(long k, bool is_type1) {
    internal_ = unique_ptr<BasicRMFE64>(new BasicRMFE64(k, 1, is_type1));
    const NTL::GF2EX& f = internal_->ex_field_mod();
    ex_field_poly_ = NTL::GF2XModulus(shrink(f));

    // To use cache, we need to constrain the input size.
    if (use_cache_) {
        assert(internal_->k() < 22);
        assert(internal_->m() < 22);
        
        encode_table_.resize(1 << internal_->k());
        decode_table_.resize(1 << internal_->m());

        // [zico] With q = 2, there is not that many choices for k, 
        // so we can just enumerate all cases here
        if (is_type1) {
            if (k == 1) {
                encode_table_ = {0, 1};
                decode_table_ = {0, 1};
            }
            else if (k == 2) {
                encode_table_ = {0, 3, 2, 1};
                decode_table_ = {0, 3, 2, 1, 2, 1, 0, 3};
            }
        }
        else {
            if (k == 1) {
                encode_table_ = {0, 1};
                decode_table_ = {0, 1, 0, 1};
            }
            else if (k == 2) {
                encode_table_ = {0, 3, 2, 1};
                decode_table_ = {0, 3, 2, 1, 2, 1, 0, 3, 2, 1, 0, 3, 0, 3, 2, 1};
            }
        }
        
    }
}

gf2x64 BasicGf2RMFE64::encode(vec_gf2_64 h) {
    return encode_table_[h];
}

vec_gf2_64 BasicGf2RMFE64::decode(gf2x64 g) {
    return decode_table_[g];
}

gf2x64 BasicGf2RMFE64::random_preimage(vec_gf2_64 h) {
    size_t choices = (1<<(m() - k()));
    size_t choice = rd() % choices;
    for (size_t i = 0; i < decode_table_.size(); i++) {
        if (decode_table_[i] == h) {
            if (choice == 0)
                return i;
            else 
                choice--;
        }
    }
    NTL::LogicError("Random preimage failed. Should not reach here.");
    return 0;
}

CompositeGf2RMFE64::CompositeGf2RMFE64(std::shared_ptr<FieldConverter64> converter, std::shared_ptr<Gf2RMFE64> rmfe1, std::shared_ptr<Gf2eRMFE64> rmfe2)
    : converter_(converter), rmfe1_(rmfe1), rmfe2_(rmfe2) {
    if (converter->base_field_poly() != rmfe2->base_field_mod())
        NTL::LogicError("Converter and rmfe1's base field polys are different");
    if (converter->composite_field_poly() != rmfe2->ex_field_mod())
        NTL::LogicError("Converter and rmfe1's composite field polys are different");
    if (rmfe2->base_field_size() != (long) pow(rmfe1->base_field_size(), rmfe1->m()))
        NTL::LogicError("Cannot concatenate two RMFEs with incompatible field sizes");
    if (rmfe1->base_field_size() != 2)
        NTL::LogicError("rmfe2's base field must be GF(2)");

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

    base_field_context_ = NTL::GF2EContext(converter_->base_field_poly());
}



gf2x64 CompositeGf2RMFE64::encode(vec_gf2_64 h) {
    
    if (use_cache_ && encode_table_cached_[h]) {
        return encode_table_[h];
    }

    NTL::GF2EPush push;
    base_field_context_.restore();
    vec_gf2e y(rmfe2_->k());

    uint64_t mask = (1 << rmfe1_->k()) - 1;
    for (size_t i = 0; i < y.size(); i++) {
        vec_gf2_64 yi = (h >> (i * rmfe1_->k())) & mask;
        y.at(i) = rmfe1_->encode(yi);
    }
    gf2ex g_comp = rmfe2_->encode(y);
    gf2e g = converter_->composite_to_binary(g_comp);

    if (use_cache_) {
        encode_table_[h] = g;
        encode_table_cached_[h] = true;
    }
    return g;
}

vec_gf2_64 CompositeGf2RMFE64::decode(gf2x64 g) {
    if (use_cache_ && decode_map_.contains(g)) {
        return decode_map_.get(g);
    }

    gf2ex g_comp_ = converter_->binary_to_composite(g);

    vec_gf2e y = rmfe2_->decode(g_comp_);

    vec_gf2_64 h = 0;
    for (size_t i = 0; i < y.size(); i++) {
        vec_gf2_64 yi = rmfe1_->decode(y.at(i));
        h ^= (yi << (i * rmfe1_->k()));
    }

    if (use_cache_)
        decode_map_.insert(g, h);
    return h;
}

gf2x64 CompositeGf2RMFE64::random_preimage(vec_gf2_64 h) {
    NTL::GF2EPush push;
    base_field_context_.restore();
    vec_gf2e y(rmfe2_->k());

    uint64_t mask = (1 << rmfe1_->k()) - 1;
    for (size_t i = 0; i < y.size(); i++) {
        vec_gf2_64 yi = (h >> (i * rmfe1_->k())) & mask;
        y.at(i) = rmfe1_->random_preimage(yi);
    }

    gf2ex g_comp = rmfe2_->random_preimage(y);
    gf2e g = converter_->composite_to_binary(g_comp);
    return g;
}

FieldConverter64::FieldConverter64(long binary_field_deg, long base_field_deg, long extension_deg, NTL::GF2X prespecified_base_field_poly)
: FieldConverter(binary_field_deg, base_field_deg, extension_deg, prespecified_base_field_poly) {
    combined_c2b_mat_64_.resize(combined_c2b_mat_.NumRows());
    for (long i = 0; i < combined_c2b_mat_.NumRows(); i++) {
        combined_c2b_mat_64_[i] = ntl_vec_GF2_to_vec_gf2_64(combined_c2b_mat_[i]);
    }
    combined_b2c_mat_64_.resize(combined_b2c_mat_.NumRows());
    for (long i = 0; i < combined_b2c_mat_.NumRows(); i++) {
        combined_b2c_mat_64_[i] = ntl_vec_GF2_to_vec_gf2_64(combined_b2c_mat_[i]);
    }
}


gf2ex FieldConverter64::binary_to_composite(gf2e x) {
    vec_gf2_64 b;
    mul(b, combined_b2c_mat_64_, x);
    NTL::GF2EPush push;
    base_field_context_.restore();
    return vec_gf2_64_to_gf2ex(b, k_);
}

gf2e FieldConverter64::composite_to_binary(gf2ex x) {
    vec_gf2_64 b = gf2ex_to_vec_gf2_64(x, k_);
    vec_gf2_64 y;
    mul(y, combined_c2b_mat_64_, b);
    return y;
}

std::unique_ptr<Gf2RMFE> get_composite_gf2_rmfe64_type2(long k1, long k2) {
    long m1 = 2*k1, m2 = 2*k2;
    assert(m1 * m2 < 64);
    shared_ptr<FieldConverter64> converter = make_shared<FieldConverter64>(m1 * m2, m1, m2);
    shared_ptr<Gf2RMFE64> rmfe1 = make_shared<BasicGf2RMFE64>(k1, false);
    shared_ptr<Gf2eRMFE64> rmfe2 = make_shared<BasicRMFE64>(converter->base_field_poly(), converter->composite_field_poly());
    
    return unique_ptr<Gf2RMFE>(new CompositeGf2RMFE64(converter, rmfe1, rmfe2));
}

std::unique_ptr<Gf2RMFE> get_composite_gf2_rmfe64_type1_type2(long k1, long k2) {
    long m1 = 2*k1-1, m2 = 2*k2;
    assert(m1 * m2 < 64);
    shared_ptr<FieldConverter64> converter = make_shared<FieldConverter64>(m1 * m2, m1, m2);
    shared_ptr<Gf2RMFE64> rmfe1 = make_shared<BasicGf2RMFE64>(k1, true);
    shared_ptr<Gf2eRMFE64> rmfe2 = make_shared<BasicRMFE64>(converter->base_field_poly(), converter->composite_field_poly());
    
    return unique_ptr<Gf2RMFE>(new CompositeGf2RMFE64(converter, rmfe1, rmfe2));
}

std::unique_ptr<Gf2MFE64> get_composite_gf2_mfe64(long m1, long m2) {
    long t1 = 2*m1 - 1, t2 = 2*m2 - 1;
    assert(t1 * t2 < 64);
    shared_ptr<FieldConverter64> converter1 = make_shared<FieldConverter64>(m1 * m2, m1, m2);
    shared_ptr<Gf2MFE64> mfe1 = make_shared<BasicGf2MFE64>(m1);
    shared_ptr<Gf2eMFE64> mfe2 = make_shared<BasicMFE64>(converter1->base_field_poly(), converter1->composite_field_poly());
    
    return unique_ptr<Gf2MFE64>(new CompositeGf2MFE64(converter1, mfe1, mfe2));
}

std::unique_ptr<Gf2MFE> get_composite_gf2_mfe64(long m1, long m2, long m3) {
    long t1 = 2*m1 - 1, t2 = 2*m2 - 1;
    assert(t1 * t2 < 64); // Assert 1st level is smaller than 64 bits
    assert(m1 * m2 * m3 < 64); // Assert the largest field is smaller than 64 bits
    shared_ptr<FieldConverter64> converter1 = make_shared<FieldConverter64>(m1 * m2, m1, m2);
    shared_ptr<FieldConverter64> converter2 = make_shared<FieldConverter64>(m1 * m2 * m3, m1*m2, m3, converter1->binary_field_poly());
    shared_ptr<Gf2MFE64> mfe1 = make_shared<BasicGf2MFE64>(m1);
    shared_ptr<Gf2eMFE64> mfe2 = make_shared<BasicMFE64>(converter1->base_field_poly(), converter1->composite_field_poly());
    shared_ptr<CompositeGf2MFE64> mfe3 = make_shared<CompositeGf2MFE64>(converter1, mfe1, mfe2);
    
    shared_ptr<Gf2eMFE64> mfe4 = shared_ptr<Gf2eMFE64>(new BasicMFE64(converter2->base_field_poly(), converter2->composite_field_poly()));

    // return unique_ptr<Gf2MFE>(new CompositeGf2MFE(converter2, mfe3, mfe4));

    return unique_ptr<Gf2MFE>(new CompositeGf2MFEInternal64(converter2, mfe3, mfe4));
}




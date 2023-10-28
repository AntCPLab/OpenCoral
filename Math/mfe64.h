#ifndef MFE64_H_
#define MFE64_H_

#include "NTL/GF2EX.h"
#include "NTL/vec_GF2E.h"
#include "NTL/mat_GF2.h"
#include "NTL/mat_GF2E.h"
#include "NTL/GF2X.h"
#include "Tools/Exceptions.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <assert.h>
#include "mfe.h"
#include "Tools/performance.h"
#include <random>

#define USE_CACHE 1
#define USE_PRECOMP 1
#define USE_OPTIMIZED_MAPPING 1

typedef uint64_t vec_gf2_64;
typedef uint64_t gf2x64;
typedef uint64_t gf2e;
typedef vector<gf2e> vec_gf2e;
typedef vector<gf2e> gf2ex;
typedef vector<gf2ex> vec_gf2ex;
typedef vector<vec_gf2e> mat_gf2e;

ostream& operator<<(ostream& s, const vec_gf2e& a);
bool operator==(const vec_gf2e& a, const vec_gf2e& b);

gf2e mul(gf2e a, gf2e b);

inline long deg(const gf2ex& f) {
    return ((long) f.size()) - 1;
}

inline long coeff(const gf2ex& f, int i) {
    if (i > deg(f))
        return 0;
    return f[i];
}

inline gf2x64 ntl_GF2X_to_gf2x64(const NTL::GF2X& f) {
    // [zico] This is more efficient than calling deg(f)
    long d = f.xrep.length();
    if (d > 1)
        NTL::LogicError("Poly modulus degree > 63");
    if (d == 0)
        return 0;
    return f.xrep[0];
}

inline NTL::GF2X gf2x64_to_ntl_GF2X(gf2x64 f) {
    if (f == 0)
        return NTL::GF2X::zero();
    NTL::GF2X g(1);
    g.xrep[0] = f;
    return g;
}

inline gf2e ntl_GF2E_to_gf2e(const NTL::GF2E& f) {
    return ntl_GF2X_to_gf2x64(f._GF2E__rep);
}

inline NTL::GF2E gf2e_to_ntl_GF2E(gf2e f) {
    if (f == 0)
        return NTL::GF2E::zero();
    NTL::GF2E g(1);
    g._GF2E__rep.xrep[0] = f;
    return g;
}

inline void ntl_GF2EX_to_gf2ex(gf2ex& g, const NTL::GF2EX& f) {
    long d = deg(f);
    if (d < 0) {
        g.clear();
        return;
    }
    g.resize(d + 1);
    for (int i = 0; i <= d; i++) {
        g[i] = ntl_GF2E_to_gf2e(f[i]);
    }
}

inline gf2ex ntl_GF2EX_to_gf2ex(const NTL::GF2EX& f) {
    gf2ex g;
    ntl_GF2EX_to_gf2ex(g, f);
    return g;
}

inline void gf2ex_to_ntl_GF2EX(NTL::GF2EX& g, const gf2ex& f) {
    long d = deg(f);
    if (d < 0) {
        clear(g);
        return;
    }
    g.SetLength(d + 1);
    for (int i = 0; i <= d; i++) {
        g[i] = gf2e_to_ntl_GF2E(f[i]);
    }
}

inline NTL::GF2EX gf2ex_to_ntl_GF2EX(const gf2ex& f) {
    NTL::GF2EX g;
    gf2ex_to_ntl_GF2EX(g, f);
    return g;
}

inline void vec_gf2e_to_ntl_vec_GF2E(NTL::vec_GF2E& g, const vec_gf2e& f) {
    g.SetLength(f.size());
    for (size_t i = 0; i < f.size(); i++)
        g[i] = gf2e_to_ntl_GF2E(f[i]);
}

inline NTL::vec_GF2E vec_gf2e_to_ntl_vec_GF2E(const vec_gf2e& f) {
    NTL::vec_GF2E g;
    vec_gf2e_to_ntl_vec_GF2E(g, f);
    return g;
}

inline void ntl_vec_GF2E_to_vec_gf2e(vec_gf2e& g, const NTL::vec_GF2E& f) {
    g.resize(f.length());
    for (long i = 0; i < f.length(); i++)
        g[i] = ntl_GF2E_to_gf2e(f[i]);
}

inline gf2e ntl_GF2X_to_gf2e(const NTL::GF2X& x) {
    return ntl_GF2X_to_gf2x64(x);
}

inline NTL::GF2X gf2e_to_ntl_GF2X(gf2e x) {
    return gf2x64_to_ntl_GF2X(x);
}

inline NTL::vec_GF2 gf2e_to_ntl_vec_GF2(gf2e x, long target_len) {
    if (NTL::GF2E::degree() > target_len)
        NTL::LogicError("Invalid target length (too small)");
    NTL::vec_GF2 y({}, target_len);
    y.rep[0] = x;
    return y;
}

inline gf2e ntl_vec_GF2_to_gf2e(const NTL::vec_GF2& x) {
    if (x.length() == 0)
        return 0;
    return x.rep[0];
}

/**
 * Unroll the gf2ex and put all coefficients in a vector of GF2.
*/
inline NTL::vec_GF2 gf2ex_to_ntl_vec_GF2(gf2ex x, long target_len) {
    long d = NTL::GF2E::degree();
    if (target_len % d != 0 || (long)x.size() * d > target_len)
        NTL::LogicError("Invalid target length");
    NTL::vec_GF2 y({}, target_len);
    for (size_t i = 0; i < x.size(); i++) {
        for (long j = 0; j < d; j++) {
            y[i * d + j] = (x[i] >> j) & 1;
        }
    }
    return y;
}

inline gf2ex to_gf2ex(const NTL::vec_GF2& x) {
    acc_time_log("to_gf2ex");
    long k = NTL::GF2E::degree();
    if (x.length() % k != 0)
        NTL::LogicError("Input vector length is not a multiple of base field polynomial degree.");

    gf2ex y(x.length() / k);
    for (size_t i = 0; i < y.size(); i++) {
        y[i] = 0;
        for (long j = 0; j < k; j++) {
            y[i] ^= NTL::rep(x[i * k + j]) << j;
        }
    }
    acc_time_log("to_gf2ex");
    return y;
}

inline vec_gf2_64 ntl_vec_GF2_to_vec_gf2_64(const NTL::vec_GF2& x) {
    if (x.length() == 0)
        return 0;
    return x.rep[0];
}

inline NTL::vec_GF2 vec_gf2_64_to_ntl_vec_GF2(vec_gf2_64 x, long target_len) {
    if (target_len > 64)
        NTL::LogicError("Invalid target length larger than 64");
    NTL::vec_GF2 y({}, target_len);
    y.rep[0] = x;
    return y;
}

inline void lift(gf2ex& y, const NTL::GF2X& x) {
    long d = deg(x);
    y.resize(d + 1);
    for (int i = 0; i <= d; i++)
        y[i] = NTL::coeff(x, i)._GF2__rep;
}

inline void lift(vec_gf2e& y, const NTL::vec_GF2& x) {
    y.resize(x.length());
    for (size_t i = 0; i < y.size(); i++)
        y[i] = x.at(i)._GF2__rep;
}

inline void shrink(NTL::vec_GF2& y, const vec_gf2e& x) {
    y.SetLength(x.size());
    for (int i = 0; i < y.length(); i++) {
        if (x[i]!=0 && x[i]!=1)
            NTL::LogicError("Invalid element in x");
        y.at(i) = x[i];
    }
}

inline void shrink(NTL::GF2X& y, const gf2ex& x) {
    y.SetLength(deg(x) + 1);
    for (int i = 0; i <= deg(x); i++) {
        if (x[i]!=0 && x[i]!=1)
            NTL::LogicError("Invalid coefficient in x");
        SetCoeff(y, i, x[i]);
    }
}

inline gf2e add(gf2e a, gf2e b) {
    return a^b;
}

inline gf2e sub(gf2e a, gf2e b) {
    return a^b;
}

inline gf2e negate(gf2e a) {
    return a;
}

// inline gf2e random_gf2e() {

// }

inline gf2ex random_gf2ex(int n) {
    // [zico] Any better random generation?
    gf2ex res(n);
    ntl_GF2EX_to_gf2ex(res, NTL::random_GF2EX(n));
    return res;
}

inline vec_gf2e random_vec_gf2e(int n) {
    vec_gf2e res;
    ntl_vec_GF2E_to_vec_gf2e(res, NTL::random_vec_GF2E(n));
    return res;
}

class gf2e_precomp {
    static std::unordered_map<gf2x64, std::vector<std::vector<gf2e>>> mul_table_;
    static std::unordered_map<gf2x64, std::vector<std::vector<gf2e>>> add_table_;
    static std::unordered_map<gf2x64, std::vector<gf2e>> inv_table_;
    static std::unordered_map<gf2x64, std::vector<std::vector<long>>> pow_table_;
    static std::mutex mtx_;
public:

    static void generate_table(const NTL::GF2X& poly_mod);

    static const std::vector<std::vector<gf2e>>& get_mul_table(const NTL::GF2X& poly_mod);
    static const std::vector<std::vector<gf2e>>& get_add_table(const NTL::GF2X& poly_mod);
    static const std::vector<gf2e>& get_inv_table(const NTL::GF2X& poly_mod);
    static const std::vector<std::vector<long>>& get_pow_table(const NTL::GF2X& poly_mod);

};

class FieldConverter64 : public FieldConverter {
public:
FieldConverter64(long binary_field_deg, long base_field_deg, long extension_deg, NTL::GF2X prespecified_base_field_poly=NTL::GF2X(0))
: FieldConverter(binary_field_deg, base_field_deg, extension_deg, prespecified_base_field_poly) {}

gf2ex binary_to_composite(gf2e x);
gf2e composite_to_binary(gf2ex x);
};

/**
 * This class restricts input and output of the functionalities
 * to be within 64 bits.
*/
class Gf2RMFE64 : public Gf2RMFE {
public:
using RMFE::encode;
using RMFE::decode;
using RMFE::random_preimage;

virtual gf2x64 encode(vec_gf2_64 h) = 0;
virtual vec_gf2_64 decode(gf2x64 g) = 0;
virtual gf2x64 random_preimage(vec_gf2_64 h) = 0;
};

typedef RMFE<vec_gf2e, gf2ex, NTL::GF2X, NTL::GF2EX> Gf2eRMFE64;

class BasicRMFE64 : public Gf2eRMFE64{
private:
long k_;
long m_;
NTL::GF2X base_field_poly_mod_;
NTL::GF2EXModulus ex_field_poly_mod_;
NTL::GF2EX alpha_;
vec_gf2ex basis_;
vec_gf2e beta_;

mat_gf2e beta_matrix_;
// bool use_precompute_beta_matrix_ = USE_PRECOMP;
// bool use_fast_basis_ = USE_OPTIMIZED_MAPPING;

// Using GF2E::init frequently is very expensive, so we should save the context here.
NTL::GF2EContext base_field_context_;

void initialize();

public:

/**
 * "type1" mapping uses m = 2*k-1, otherwise uses m = 2*k
*/
BasicRMFE64(long k, long base_field_poly_mod_deg, bool is_type1=true);

BasicRMFE64(NTL::GF2X base_field_poly, NTL::GF2EX ex_field_poly);

bool is_type1() {
    return m_ == 2 * k_ - 1;
}

long m() {
    return m_;
}

long k() {
    return k_;
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

void encode(gf2ex& g, const vec_gf2e& h);
void decode(vec_gf2e& h, const gf2ex& g);
void random_preimage(gf2ex& h, const vec_gf2e& g);

gf2ex encode(const vec_gf2e& h) {
    gf2ex g;
    encode(g, h);
    return g;
}
vec_gf2e decode(const gf2ex& g) {
    vec_gf2e h;
    decode(h, g);
    return h;
}

gf2ex random_preimage(const vec_gf2e& g) {
    gf2ex h;
    random_preimage(h, g);
    return h;
}

};

class BasicGf2RMFE64 : public Gf2RMFE64 {
private:
// Now we simply use BasicRMFE to implement the special case. Might be able to optimize.
unique_ptr<BasicRMFE64> internal_;
long base_field_mod_ = 2;
NTL::GF2XModulus ex_field_poly_;

bool use_cache_ = USE_CACHE;
vector<gf2x64> encode_table_;
vector<bool> encode_table_cached_;
vector<vec_gf2_64> decode_table_;
vector<bool> decode_table_cached_;

// Using GF2E::init frequently is very expensive, so we should save the context here.
NTL::GF2EContext base_field_context_;

std::random_device rd;

public:
using RMFE::encode;
using RMFE::decode;
using RMFE::random_preimage;

/**
 * "type1" mapping uses m = 2*k-1, otherwise uses m = 2*k
*/
BasicGf2RMFE64(long k, bool is_type1=true);

bool is_type1() {
    return internal_->is_type1();
}

long m() {
    return internal_->m();
}

long k() {
    return internal_->k();
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

void encode(NTL::GF2X& g, const NTL::vec_GF2& h) {
    // [zico] TODO: can redirect to the integer version of encode
}
void decode(NTL::vec_GF2& h, const NTL::GF2X& g) {
    // [zico] TODO: can redirect to the integer version of encode
}
void random_preimage(NTL::GF2X& g, const NTL::vec_GF2& h) {
    // [zico] TODO: can redirect to the integer version of encode
}

gf2x64 encode(vec_gf2_64 h);
vec_gf2_64 decode(gf2x64 g);
gf2x64 random_preimage(vec_gf2_64 h);
};

class CompositeGf2RMFE64: public Gf2RMFE64 {
private:
long k_;
long m_;

std::shared_ptr<FieldConverter64> converter_;
std::shared_ptr<Gf2RMFE64> rmfe1_;
std::shared_ptr<Gf2eRMFE64> rmfe2_;

long base_field_mod_ = 2;
NTL::GF2XModulus ex_field_poly_;

bool use_cache_ = USE_CACHE;
// vector<NTL::GF2X> encode_table_;
// vector<bool> encode_table_cached_;
// LRU<long, NTL::vec_GF2> decode_map_;

vector<gf2x64> encode_table_;
vector<bool> encode_table_cached_;
LRU<long, vec_gf2_64> decode_map_;

// Using GF2E::init frequently is very expensive, so we should save the context here.
NTL::GF2EContext base_field_context_;

public:
using RMFE::encode;
using RMFE::decode;
using RMFE::random_preimage;

CompositeGf2RMFE64(std::shared_ptr<FieldConverter64> converter, std::shared_ptr<Gf2RMFE64> rmfe1, std::shared_ptr<Gf2eRMFE64> rmfe2);

long m() {
    return m_;
}

long k() {
    return k_;
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

void encode(NTL::GF2X& g, const NTL::vec_GF2& h) {
    vec_gf2_64 h_ = ntl_vec_GF2_to_vec_gf2_64(h);
    gf2x64 g_ = encode(h_);
    g = gf2x64_to_ntl_GF2X(g_);
}
void decode(NTL::vec_GF2& h, const NTL::GF2X& g) {
    gf2x64 g_ = ntl_GF2X_to_gf2x64(g);
    vec_gf2_64 h_ = decode(g_);
    h = vec_gf2_64_to_ntl_vec_GF2(h_, k());
}
void random_preimage(NTL::GF2X& g, const NTL::vec_GF2& h) {
    vec_gf2_64 h_ = ntl_vec_GF2_to_vec_gf2_64(h);
    gf2x64 g_ = random_preimage(h_);
    g = gf2x64_to_ntl_GF2X(g_);
}

gf2x64 encode(vec_gf2_64 h);
vec_gf2_64 decode(gf2x64 g);
gf2x64 random_preimage(vec_gf2_64 h);
};


std::unique_ptr<Gf2RMFE> get_composite_gf2_rmfe64_type2(long k1, long k2);

// std::unique_ptr<Gf2MFE> get_double_composite_gf2_mfe(long m1, long m2, long m3);


#endif /* MFE64_H_ */
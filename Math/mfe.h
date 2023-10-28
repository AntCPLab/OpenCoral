#ifndef MFE_H_
#define MFE_H_

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

#define USE_CACHE 1
#define USE_PRECOMP 1
#define USE_OPTIMIZED_MAPPING 1

NTL::GF2X shrink(const NTL::GF2EX& x);

NTL::GF2X indices_to_gf2x(const std::vector<long>& indices);
NTL::GF2E num_to_ntl_GF2E(unsigned long x);
void num_to_ntl_GF2E(NTL::GF2E& res, unsigned long x);
unsigned long ntl_GF2E_to_num(const NTL::GF2E& x);


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

void inv_xi_1(NTL::GF2EX& f, const NTL::vec_GF2E& g, const NTL::vec_GF2E& beta);

NTL::GF2EX lift(const NTL::GF2X& x);
NTL::GF2X shrink(const NTL::GF2EX& x);
NTL::vec_GF2E lift(const NTL::vec_GF2& x);
NTL::vec_GF2 shrink(const NTL::vec_GF2E& x);
/**
 * Convert `x` to a GF2E type which takes `poly_mod` as the polynomial modulus.
 * The conversion does NOT necessarily set GF2E's modulus, so the caller should
 * make sure the modulus is expected when using the returned result.
*/
NTL::GF2E to_GF2E(const NTL::GF2X& x, const NTL::GF2X& poly_mod);
/**
 * Convert `x` to a GF2E type which takes `poly_mod` as the polynomial modulus.
 * The conversion does NOT necessarily set GF2E's modulus, so the caller should
 * make sure the modulus is expected when using the returned result.
*/
NTL::GF2E to_GF2E(const NTL::vec_GF2& x, const NTL::GF2X& poly_mod);

template<class TKey, class TValue>
class LRU {
    static const int MAX_SIZE = 2000000;
    std::unordered_map<TKey, TValue> map_;
    std::queue<TKey> keys_;
public:
    LRU() {}

    bool contains(const TKey& key) {
        return map_.count(key) == 1;
    }

    void insert(const TKey& key, const TValue& value) {
        if (contains(key)) {
            assert(map_.at(key) == value);
            return;
        }
        if (keys_.size() >= MAX_SIZE) {
            map_.erase(keys_.front());
            keys_.pop();
        }
        keys_.push(key);
        map_[key] = value;
    }

    const TValue& get(const TKey& key) {
        return map_.at(key);
    }
};

class GF2EPrecomp {
    // static std::unique_ptr<GF2EPrecomp> singleton;
    static std::unordered_map<unsigned long, std::vector<std::vector<NTL::GF2E>>> mul_table_;
    static std::unordered_map<unsigned long, std::vector<std::vector<NTL::GF2E>>> add_table_;
    static std::mutex mtx_;
public:
    // static void set_singleton(std::unique_ptr<GF2EPrecomp> s) {
    //     singleton = std::move(s);
    // }
    // static GF2EPrecomp& s() {
    //     if (singleton)
    //         return *singleton;
    //     else
    //         throw runtime_error("no singleton: " + std::string(typeid(GF2EPrecomp).name()));
    // }
    // static bool has_singleton() {
    //     return bool(singleton);
    // }
    // static void reset_singleton() {
    //     if (has_singleton())
    //         singleton.reset(nullptr);
    // }

    // GF2EPrecomp() {}

    static void generate_table(const NTL::GF2X& poly_mod);

    static const std::vector<std::vector<NTL::GF2E>>& get_mul_table(const NTL::GF2X& poly_mod);
    static const std::vector<std::vector<NTL::GF2E>>& get_add_table(const NTL::GF2X& poly_mod);

};

class FieldConverter {
protected:
long k_, n_, m_;
NTL::GF2X p_;
NTL::GF2X u_;
NTL::GF2EX q_;
NTL::mat_GF2 mat_T_;
NTL::mat_GF2 mat_T_inv_;

NTL::GF2X prespecified_base_field_poly_;
NTL::mat_GF2 pre_isomorphic_mat_;
NTL::mat_GF2 pre_isomorphic_mat_inv_;

NTL::mat_GF2 combined_c2b_mat_;
NTL::mat_GF2 combined_b2c_mat_;

// Using GF2E::init frequently is very expensive, so we should save the context here.
NTL::GF2EContext base_field_context_;

public:
FieldConverter(long binary_field_deg, long base_field_deg, long extension_deg, NTL::GF2X prespecified_base_field_poly=NTL::GF2X(0));
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


/**
 * T1: Input type of encoding (Output type of decoding)
 * T2: Input type of decoding (Output type of encoding)
 * T3: Base field modulus type
 * T4: Extension field modulus type 
*/
template <class T1, class T2, class T3, class T4>
class MFE {
    typedef MFE<T1, T2, T3, T4> This;
    // use thread_local because we need to use cache for encode/decode and the cache is not thread-safe
    static thread_local unique_ptr<This> singleton;
public:
    virtual ~MFE() {};

    static void set_singleton(unique_ptr<This> s);
    static MFE& s();
    static bool has_singleton() {
        return bool(singleton);
    }
    static void reset_singleton() {
        if (has_singleton())
            singleton.reset(nullptr);
    }

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

template<class T1, class T2, class T3, class T4>
thread_local unique_ptr<MFE<T1, T2, T3, T4>> MFE<T1, T2, T3, T4>::singleton(nullptr);

template<class T1, class T2, class T3, class T4>
inline MFE<T1, T2, T3, T4>& MFE<T1, T2, T3, T4>::s()
{
    if (singleton)
        return *singleton;
    else
        throw runtime_error("no singleton: " + std::string(typeid(MFE<T1, T2, T3, T4>).name()));
}

template<class T1, class T2, class T3, class T4>
inline void MFE<T1, T2, T3, T4>::set_singleton(unique_ptr<MFE<T1, T2, T3, T4>> s) {
    singleton = std::move(s);
}


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

bool use_fast_basis_ = USE_OPTIMIZED_MAPPING;

// Using GF2E::init frequently is very expensive, so we should save the context here.
NTL::GF2EContext base_field_context_;

void initialize();

public:
// A fucntion named f in derived class will hide all other members named f in the base class, regardless of return types or arguments.
// So should expose them with `using`.

using MFE::encode;
using MFE::decode;

BasicMFE(long m, long base_field_poly_mod_deg);

BasicMFE(NTL::GF2X base_field_poly, NTL::GF2EX ex_field_poly);

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

bool use_cache_ = USE_CACHE;
vector<NTL::vec_GF2> encode_table_;
vector<bool> encode_table_cached_;
vector<NTL::GF2X> decode_table_;
vector<bool> decode_table_cached_;

// Using GF2E::init frequently is very expensive, so we should save the context here.
NTL::GF2EContext base_field_context_;

public:
using MFE::encode;
using MFE::decode;

BasicGf2MFE(long m);

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
std::shared_ptr<Gf2MFE> mfe1_;
std::shared_ptr<Gf2eMFE> mfe2_;

long base_field_mod_ = 2;
NTL::GF2XModulus ex_field_poly_;

bool use_cache_ = USE_CACHE;
vector<NTL::vec_GF2> encode_table_;
vector<bool> encode_table_cached_;
bool use_encode_table_ = false;
vector<NTL::GF2X> decode_table_;
vector<bool> decode_table_cached_;
bool use_decode_table_ = false;
LRU<long, NTL::vec_GF2> encode_map_;
bool use_encode_map_ = false;

// Using GF2E::init frequently is very expensive, so we should save the context here.
NTL::GF2EContext base_field_context_;

public:
using MFE::encode;
using MFE::decode;

CompositeGf2MFE(std::shared_ptr<FieldConverter> converter, std::shared_ptr<Gf2MFE> mfe1, std::shared_ptr<Gf2eMFE> mfe2);

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


/**
 * T1: Input type of encoding (Output type of decoding)
 * T2: Input type of decoding (Output type of encoding)
 * T3: Base field modulus type
 * T4: Extension field modulus type 
*/
template <class T1, class T2, class T3, class T4>
class RMFE {
    typedef RMFE<T1, T2, T3, T4> This;
    // use thread_local because we need to use cache for encode/decode and the cache is not thread-safe
    static thread_local unique_ptr<This> singleton;

public:
    virtual ~RMFE() {};

    static void set_singleton(unique_ptr<This> s);
    static RMFE& s();
    static bool has_singleton() {
        return bool(singleton);
    }
    static void reset_singleton() {
        if (has_singleton())
            singleton.reset(nullptr);
    }

    virtual long m() = 0;
    virtual long k() = 0;
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

    void tau(T2& y, const T2& x) {
        y = encode(decode(x));
    }
    T2 tau(const T2& x) {
        T2 y;
        tau(y, x);
        return y;
    }

    virtual void random_preimage(T2& h, const T1& g) = 0;

    T2 random_preimage(const T1& g) {
        T2 h;
        random_preimage(h, g);
        return h;
    }

};

template<class T1, class T2, class T3, class T4>
thread_local unique_ptr<RMFE<T1, T2, T3, T4>> RMFE<T1, T2, T3, T4>::singleton(nullptr);

template<class T1, class T2, class T3, class T4>
inline RMFE<T1, T2, T3, T4>& RMFE<T1, T2, T3, T4>::s()
{
    if (singleton)
        return *singleton;
    else
        throw runtime_error("no singleton: " + std::string(typeid(RMFE<T1, T2, T3, T4>).name()));
}

template<class T1, class T2, class T3, class T4>
inline void RMFE<T1, T2, T3, T4>::set_singleton(unique_ptr<RMFE<T1, T2, T3, T4>> s) {
    singleton = std::move(s);
}

typedef RMFE<NTL::vec_GF2E, NTL::GF2EX, NTL::GF2X, NTL::GF2EX> Gf2eRMFE;
typedef RMFE<NTL::vec_GF2, NTL::GF2X, long, NTL::GF2X> Gf2RMFE;

class BasicRMFE : public Gf2eRMFE {
private:
long k_;
long m_;
NTL::GF2X base_field_poly_mod_;
NTL::GF2EXModulus ex_field_poly_mod_;
NTL::GF2EX alpha_;
NTL::vec_GF2EX basis_;
NTL::vec_GF2E beta_;

NTL::mat_GF2E beta_matrix_;
bool use_precompute_beta_matrix_ = USE_PRECOMP;
bool use_fast_basis_ = USE_OPTIMIZED_MAPPING;

// Using GF2E::init frequently is very expensive, so we should save the context here.
NTL::GF2EContext base_field_context_;

void initialize();

public:
// A fucntion named f in derived class will hide all other members named f in the base class, regardless of return types or arguments.
// So should expose them with `using`.

using RMFE::encode;
using RMFE::decode;
using RMFE::random_preimage;

/**
 * "type1" mapping uses m = 2*k-1, otherwise uses m = 2*k
*/
BasicRMFE(long k, long base_field_poly_mod_deg, bool is_type1=true);

BasicRMFE(NTL::GF2X base_field_poly, NTL::GF2EX ex_field_poly);

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

void encode(NTL::GF2EX& g, const NTL::vec_GF2E& h);
void decode(NTL::vec_GF2E& h, const NTL::GF2EX& g);

void random_preimage(NTL::GF2EX& h, const NTL::vec_GF2E& g);
};

class BasicGf2RMFE: public Gf2RMFE {
private:
// Now we simply use BasicRMFE to implement the special case. Might be able to optimize.
unique_ptr<BasicRMFE> internal_;
long base_field_mod_ = 2;
NTL::GF2XModulus ex_field_poly_;

bool use_cache_ = USE_CACHE;
vector<NTL::GF2X> encode_table_;
vector<bool> encode_table_cached_;
vector<NTL::vec_GF2> decode_table_;
vector<bool> decode_table_cached_;

// Using GF2E::init frequently is very expensive, so we should save the context here.
NTL::GF2EContext base_field_context_;

public:
using RMFE::encode;
using RMFE::decode;
using RMFE::random_preimage;

/**
 * "type1" mapping uses m = 2*k-1, otherwise uses m = 2*k
*/
BasicGf2RMFE(long k, bool is_type1=true);

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

void encode(NTL::GF2X& g, const NTL::vec_GF2& h);
void decode(NTL::vec_GF2& h, const NTL::GF2X& g);
void random_preimage(NTL::GF2X& g, const NTL::vec_GF2& h);
};


class CompositeGf2RMFE: public Gf2RMFE {
private:
long k_;
long m_;

std::shared_ptr<FieldConverter> converter_;
std::shared_ptr<Gf2RMFE> rmfe1_;
std::shared_ptr<Gf2eRMFE> rmfe2_;

long base_field_mod_ = 2;
NTL::GF2XModulus ex_field_poly_;

bool use_cache_ = USE_CACHE;
vector<NTL::GF2X> encode_table_;
vector<bool> encode_table_cached_;
LRU<long, NTL::vec_GF2> decode_map_;

// Using GF2E::init frequently is very expensive, so we should save the context here.
NTL::GF2EContext base_field_context_;

public:
using RMFE::encode;
using RMFE::decode;
using RMFE::random_preimage;

CompositeGf2RMFE(std::shared_ptr<FieldConverter> converter, std::shared_ptr<Gf2RMFE> rmfe1, std::shared_ptr<Gf2eRMFE> rmfe2);

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

void encode(NTL::GF2X& g, const NTL::vec_GF2& h);
void decode(NTL::vec_GF2& h, const NTL::GF2X& g);
void random_preimage(NTL::GF2X& g, const NTL::vec_GF2& h);
};

std::unique_ptr<Gf2RMFE> get_composite_gf2_rmfe_type2(long k1, long k2);

std::unique_ptr<Gf2MFE> get_double_composite_gf2_mfe(long m1, long m2, long m3);


#endif /* MFE_H_ */
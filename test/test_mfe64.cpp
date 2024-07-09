#include "Math/mfe64.h"
#include <random>
#include "Tools/performance.h"

using namespace NTL;

/*
Helper function: Prints the name of the example in a fancy banner.
*/
inline void print_banner(std::string title)
{
    if (!title.empty())
    {
        std::size_t title_length = title.length();
        std::size_t banner_length = title_length + 2 * 10;
        std::string banner_top = "+" + std::string(banner_length - 2, '-') + "+";
        std::string banner_middle =
            "|" + std::string(9, ' ') + title + std::string(9, ' ') + "|";

        std::cout << std::endl
            << banner_top << std::endl
            << banner_middle << std::endl
            << banner_top << std::endl;
    }
}


// void test_composite_to_binary() {
//     print_banner("test_composite_to_binary");
    
//     long n = 2, m = 3, k = n*m;
//     FieldConverter converter(k, n, m);
//     cout << "q: " << converter.composite_field_poly() << endl;

//     GF2E::init(converter.binary_field_poly());
//     GF2E a = random_GF2E(), b = random_GF2E();
//     GF2E c = a * b;
//     GF2EX a_composite = converter.binary_to_composite(a);
//     GF2EX b_composite = converter.binary_to_composite(b);
//     GF2EX c_composite = converter.binary_to_composite(c);

//     {
//         GF2EPush push;
//         GF2E::init(converter.base_field_poly());
//         GF2EX c_composite_ = MulMod(a_composite, b_composite, converter.composite_field_poly());

//         cout << "c_composite:\t" << c_composite << endl;
//         cout << "c_composite_:\t" << c_composite_ << endl;
//         {
//             GF2EPush push;
//             GF2E::init(converter.binary_field_poly());
//             GF2E c_ = converter.composite_to_binary(c_composite_);
//             cout << "c:\t" << c << endl;
//             cout << "c_:\t" << c_ << endl;
//         }
//     }

//     GF2E one(1);
//     GF2EX one_ = converter.binary_to_composite(one);
//     cout << "one:\t" << one << endl;
//     cout << "one_:\t" << one_ << endl;
//  }

//  void test_composite_to_binary_with_base_poly() {
//     print_banner("test_composite_to_binary_with_base_poly");

//     GF2X base_poly = indices_to_gf2x(std::vector<long>({6, 5, 4, 1, 0}));
//     cout << "base poly: " << base_poly << endl;
    
//     long n = 6, m = 8, k = n*m;
//     FieldConverter converter(k, n, m, base_poly);
//     cout << "q: " << converter.composite_field_poly() << endl;
//     cout << "u: " << converter.base_field_poly() << endl;

//     GF2E::init(converter.binary_field_poly());
//     GF2E a = random_GF2E(), b = random_GF2E();
//     GF2E c = a * b;
//     GF2EX a_composite = converter.binary_to_composite(a);
//     GF2EX b_composite = converter.binary_to_composite(b);
//     GF2EX c_composite = converter.binary_to_composite(c);

//     {
//         GF2EPush push;
//         GF2E::init(converter.base_field_poly());
//         GF2EX c_composite_ = MulMod(a_composite, b_composite, converter.composite_field_poly());

//         cout << "c_composite:\t" << c_composite << endl;
//         cout << "c_composite_:\t" << c_composite_ << endl;
//         {
//             GF2EPush push;
//             GF2E::init(converter.binary_field_poly());
//             GF2E c_ = converter.composite_to_binary(c_composite_);
//             cout << "c:\t" << c << endl;
//             cout << "c_:\t" << c_ << endl;
//         }
//     }

//     GF2E one(1);
//     GF2EX one_ = converter.binary_to_composite(one);
//     cout << "one:\t" << one << endl;
//     cout << "one_:\t" << one_ << endl;
//  }

void test_basic_mfe64() {
    print_banner("test_basic_mfe64");

    long m = 3, n = 2;
    BasicMFE64 mfe(m, n);
    mfe.print_config();
    // gf2e_precomp::generate_table(mfe.base_field_mod());
    GF2E::init(mfe.base_field_mod());
    gf2ex a = random_gf2ex(m), b = random_gf2ex(m);
    vec_gf2e enc_a = mfe.encode(a), enc_b = mfe.encode(b);

    vec_gf2e enc_c(enc_a.size());
    for (size_t i = 0; i < enc_a.size(); i++) {
        enc_c[i] = mfe.mul(enc_a[i], enc_b[i]);
    }

    gf2ex c = mfe.decode(enc_c);
    GF2EX a_ = gf2ex_to_ntl_GF2EX(a), b_ = gf2ex_to_ntl_GF2EX(b);
    a_.normalize();
    b_.normalize();
    gf2ex c_ = ntl_GF2EX_to_gf2ex(MulMod(a_, b_, mfe.ex_field_mod()));

    normalize(c);
    normalize(c_);
    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;

    assert(c == c_);
}

void test_basic_gf2_mfe64() {
    print_banner("test_basic_gf2_mfe64");
    random_device rd;
    long m = 2;
    BasicGf2MFE64 mfe(m);
    // gf2e_precomp::generate_table(GF2X(1, 1));
    gf2x64 a = rd() % (1<<m), b = rd() % (1<<m);
    vec_gf2_64 enc_a = mfe.encode(a), enc_b = mfe.encode(b);

    vec_gf2_64 enc_c = enc_a & enc_b;

    gf2x64 c = mfe.decode(enc_c);
    gf2x64 c_ = ntl_GF2X_to_gf2x64(MulMod(gf2x64_to_ntl_GF2X(a), gf2x64_to_ntl_GF2X(b), mfe.ex_field_mod()));

    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;

    assert(c == c_);
}

void test_composite_gf2_mfe64() {
    print_banner("test_composite_gf2_mfe64");
    long m1 = 2, m2 = 3;
    unique_ptr<Gf2MFE> mfe = get_composite_gf2_mfe64(m1, m2);

    GF2X a = random_GF2X(mfe->m()), b = random_GF2X(mfe->m());
    vec_GF2 enc_a = mfe->encode(a), enc_b = mfe->encode(b);

    vec_GF2 enc_c({}, enc_a.length());
    for (int i = 0; i < enc_a.length(); i++) {
        enc_c[i] = enc_a[i] * enc_b[i];
    }

    GF2X c = mfe->decode(enc_c);
    GF2X c_ = MulMod(a, b, mfe->ex_field_mod());

    cout << "m:\t" << mfe->m() << ", t:\t" << mfe->t() << endl;
    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;
    
    assert(c == c_);

    GF2E::init(mfe->ex_field_mod());
    cout << "a * b:\t" << to_GF2E(a) * to_GF2E(b) << endl;

    // mfe.encode(a);
    // mfe.decode(enc_a);
    // cout << "a:\t" << a << endl;
    // cout << "a_:\t" <<  << endl;
}

void test_double_composite_gf2_mfe64(long m1, long m2, long m3) {
    print_banner("test_double_composite_gf2_mfe64");
    unique_ptr<Gf2MFE> mfe = get_composite_gf2_mfe64(m1, m2, m3);
    mfe->print_config();

    GF2X a = random_GF2X(mfe->m()), b = random_GF2X(mfe->m());
    vec_GF2 enc_a = mfe->encode(a), enc_b = mfe->encode(b);

    vec_GF2 enc_c({}, enc_a.length());
    for (int i = 0; i < enc_a.length(); i++) {
        enc_c[i] = enc_a[i] * enc_b[i];
    }

    GF2X c = mfe->decode(enc_c);
    GF2X c_ = MulMod(a, b, mfe->ex_field_mod());

    cout << "m:\t" << mfe->m() << ", t:\t" << mfe->t() << endl;
    cout << "a: " << a << endl;
    cout << "b: " << b << endl;
    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;

    assert(c == c_);

    GF2E::init(mfe->ex_field_mod());
    cout << "a * b:\t" << to_GF2E(a) * to_GF2E(b) << endl;

    // mfe.encode(a);
    // mfe.decode(enc_a);
    // cout << "a:\t" << a << endl;
    // cout << "a_:\t" <<  << endl;
}

void test_basic_rmfe64() {
    print_banner("test_basic_rmfe64");

    long k = 7, n = 5;
    BasicRMFE64 rmfe(k, n);
    // gf2e_precomp::generate_table(rmfe.base_field_mod());
    GF2E::init(rmfe.base_field_mod());
    vec_gf2e a = random_vec_gf2e(k), b = random_vec_gf2e(k);
    gf2ex enc_a = rmfe.encode(a), enc_b = rmfe.encode(b);

    GF2EX enc_a_ = gf2ex_to_ntl_GF2EX(enc_a), enc_b_ = gf2ex_to_ntl_GF2EX(enc_b);
    enc_a_.normalize();
    enc_b_.normalize();
    // Mul without normalization will result in a segmentation fault, not sure whether it's a bug of NTL
    GF2EX enc_c_ = MulMod(enc_a_, enc_b_, rmfe.ex_field_mod());
    gf2ex enc_c = ntl_GF2EX_to_gf2ex(enc_c_);

    vec_gf2e c = rmfe.decode(enc_c);
    vec_gf2e c_(a.size());
    for (size_t i = 0; i < a.size(); i++) {
        c_[i] = rmfe.mul(a[i], b[i]);
    }

    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;

    assert(c == c_);

    cout << "a:\t" << a << endl;
    cout << "a_:\t" << rmfe.decode(enc_a) << endl;
}

void test_basic_gf2_rmfe64() {
    print_banner("test_basic_gf2_rmfe64");
    random_device rd;
    long k = 2;
    BasicGf2RMFE64 rmfe(k);
    // gf2e_precomp::generate_table(GF2X(1, 1));
    vec_gf2_64 a = rd() % (1<<k), b= rd() % (1<<k);
    gf2x64 enc_a = rmfe.encode(a), enc_b = rmfe.encode(b);

    gf2x64 enc_c = ntl_GF2X_to_gf2x64(MulMod(gf2x64_to_ntl_GF2X(enc_a), gf2x64_to_ntl_GF2X(enc_b), rmfe.ex_field_mod()));

    vec_gf2_64 c = rmfe.decode(enc_c);
    vec_gf2_64 c_ = a & b;

    cout << "a:\t" << a << endl;
    cout << "b:\t" << b << endl;
    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;

    assert(c == c_);

    // k = 1;
    // BasicGf2RMFE64 rmfe1(k, false);
    // gf2e_precomp::generate_table(GF2X(1, 1));
    // cout << "encode: " << endl;
    // a.SetLength(k);
    // for (int i = 0; i < (1 << k); i++) {
    //     a.rep[0] = i;
    //     cout << i << " --> " << rmfe1.encode(a) << endl;
    // }
    // cout << "decode: " << endl;
    // for (int i = 0; i < (1<<(2*k)); i++) {
    //     enc_a.SetLength(2*k);
    //     enc_a.xrep[0] = i;
    //     enc_a.normalize();
    //     cout << i << " --> " << rmfe1.decode(enc_a) << endl;
    // }
}

// void test_basic_gf2_rmfe64() {
//     print_banner("test_basic_gf2_rmfe64");
//     long k = 2;
//     BasicGf2RMFE64 rmfe(k, false);
//     gf2e_precomp::generate_table(GF2X(1, 1));
//     // vec_GF2 a = random_vec_GF2(k), b = random_vec_GF2(k);
//     vec_GF2 a({}, 2), b({}, 2);
//     a[0] = 1;
//     a[1] = 1;
//     b[0] = 0;
//     b[1] = 1;
//     GF2X enc_a = rmfe.encode(a), enc_b = rmfe.encode(b);

//     GF2X enc_c = MulMod(enc_a, enc_b, rmfe.ex_field_mod());

//     vec_GF2 c = rmfe.decode(enc_c);
//     vec_GF2 c_({}, a.length());
//     for (int i = 0; i < a.length(); i++) {
//         c_[i] = a[i] * b[i];
//     }

//     cout << "a:\t" << a << endl;
//     cout << "b:\t" << b << endl;
//     cout << "c:\t" << c << endl;
//     cout << "c_:\t" << c_ << endl;

//     assert(c == c_);
// }

// void test_basic_gf2_rmfe_type2() {
//     print_banner("test_basic_gf2_rmfe_type2");
//     long k = 2;
//     BasicGf2RMFE rmfe(k, false);
//     vec_GF2 a = random_vec_GF2(k), b = random_vec_GF2(k);
//     GF2X enc_a = rmfe.encode(a), enc_b = rmfe.encode(b);

//     GF2X enc_c = MulMod(enc_a, enc_b, rmfe.ex_field_mod());

//     vec_GF2 c = rmfe.decode(enc_c);
//     vec_GF2 c_({}, a.length());
//     for (int i = 0; i < a.length(); i++) {
//         c_[i] = a[i] * b[i];
//     }

//     cout << "a:\t" << a << endl;
//     cout << "b:\t" << b << endl;
//     cout << "c:\t" << c << endl;
//     cout << "c_:\t" << c_ << endl;
// }

// void test_composite_gf2_rmfe() {
//     print_banner("test_composite_gf2_rmfe");
//     long k1 = 2, m1 = 2*k1 - 1, k2 = 8, m2 = 2*k2-1;
//     shared_ptr<FieldConverter> converter = make_shared<FieldConverter>(m1 * m2, m1, m2);
//     shared_ptr<Gf2RMFE> rmfe1 = make_shared<BasicGf2RMFE>(k1);
//     shared_ptr<Gf2eRMFE> rmfe2 = make_shared<BasicRMFE>(converter->base_field_poly(), converter->composite_field_poly());
//     CompositeGf2RMFE rmfe(converter, rmfe1, rmfe2);

//     vec_GF2 a = random_vec_GF2(rmfe.k()), b = random_vec_GF2(rmfe.k());
//     GF2X enc_a = rmfe.encode(a), enc_b = rmfe.encode(b);

//     GF2X enc_c = MulMod(enc_a, enc_b, rmfe.ex_field_mod());
//     vec_GF2 c = rmfe.decode(enc_c);

//     vec_GF2 c_({}, a.length());
//     for (int i = 0; i < a.length(); i++) {
//         c_[i] = a[i] * b[i];
//     }

//     cout << "k:\t" << rmfe.k() << ", m:\t" << rmfe.m() << endl;
//     cout << "a:\t" << a << endl;
//     cout << "b:\t" << b << endl;
//     cout << "c:\t" << c << endl;
//     cout << "c_:\t" << c_ << endl;

//     // mfe.encode(a);
//     // mfe.decode(enc_a);
//     cout << "a:\t" << a << endl;
//     cout << "a_:\t" << rmfe.decode(enc_a) << endl;
// }

void test_composite_gf2_rmfe64_type2(long k1, long k2) {
    print_banner("test_composite_gf2_rmfe64_type2");
    unique_ptr<Gf2RMFE> rmfe = get_composite_gf2_rmfe64_type2(k1, k2);

    vec_GF2 a = random_vec_GF2(rmfe->k()), b = random_vec_GF2(rmfe->k());
    GF2X enc_a = rmfe->encode(a), enc_b = rmfe->encode(b);

    GF2X enc_c = MulMod(enc_a, enc_b, rmfe->ex_field_mod());
    vec_GF2 c = rmfe->decode(enc_c);

    vec_GF2 c_({}, a.length());
    for (int i = 0; i < a.length(); i++) {
        c_[i] = a[i] * b[i];
    }
    assert(c_ == c);
    
    vec_GF2 d = rmfe->decode(enc_a + enc_b);
    vec_GF2 d_({}, a.length());
    for (int i = 0; i < a.length(); i++) {
        d_[i] = a[i] + b[i];
    }
    assert(d_ == d);

    cout << "k:\t" << rmfe->k() << ", m:\t" << rmfe->m() << endl;
    cout << "a:\t" << a << endl;
    cout << "b:\t" << b << endl;
    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;
    cout << "d:\t" << d << endl;
    cout << "d_:\t" << d_ << endl;

    // mfe.encode(a);
    // mfe.decode(enc_a);
    cout << "a:\t" << a << endl;
    cout << "a_:\t" << rmfe->decode(enc_a) << endl;
}

void test_composite_gf2_rmfe64_type1_type2(long k1, long k2) {
    print_banner("test_composite_gf2_rmfe64_type1_type2");
    unique_ptr<Gf2RMFE> rmfe = get_composite_gf2_rmfe64_type1_type2(k1, k2);

    vec_GF2 a = random_vec_GF2(rmfe->k()), b = random_vec_GF2(rmfe->k());
    GF2X enc_a = rmfe->encode(a), enc_b = rmfe->encode(b);

    GF2X enc_c = MulMod(enc_a, enc_b, rmfe->ex_field_mod());
    vec_GF2 c = rmfe->decode(enc_c);

    vec_GF2 c_({}, a.length());
    for (int i = 0; i < a.length(); i++) {
        c_[i] = a[i] * b[i];
    }
    assert(c_ == c);
    
    vec_GF2 d = rmfe->decode(enc_a + enc_b);
    vec_GF2 d_({}, a.length());
    for (int i = 0; i < a.length(); i++) {
        d_[i] = a[i] + b[i];
    }
    assert(d_ == d);

    cout << "k:\t" << rmfe->k() << ", m:\t" << rmfe->m() << endl;
    cout << "a:\t" << a << endl;
    cout << "b:\t" << b << endl;
    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;
    cout << "d:\t" << d << endl;
    cout << "d_:\t" << d_ << endl;

    cout << "a:\t" << a << endl;
    cout << "a_:\t" << rmfe->decode(enc_a) << endl;
}

// void test_rmfe_then_mfe() {
//     print_banner("test_rmfe_then_mfe");
//     unique_ptr<Gf2RMFE> rmfe = get_composite_gf2_rmfe_type2(2, 6);
//     CompositeGf2MFE mfe = test_double_composite_gf2_mfe(2, 3, 8);

//     vec_GF2 a = random_vec_GF2(rmfe->k()), b = random_vec_GF2(rmfe->k());
//     GF2X enc_a = rmfe->encode(a), enc_b = rmfe->encode(b);

//     vec_GF2 enc_enc_a = mfe.encode(enc_a), enc_enc_b = mfe.encode(enc_b);

//     vec_GF2 enc_enc_c({}, enc_enc_a.length());
//     for (int i = 0; i < enc_enc_a.length(); i++) {
//         enc_enc_c[i] = enc_enc_a[i] * enc_enc_b[i];
//     }

//     GF2X enc_c = mfe.decode(enc_enc_c);
//     vec_GF2 c = rmfe->decode(enc_c);

//     vec_GF2 c_({}, a.length());
//     for (int i = 0; i < a.length(); i++) {
//         c_[i] = a[i] * b[i];
//     }

//     cout << "k:\t" << rmfe->k() << ", m:\t" << rmfe->m() << ", t:\t" << mfe.t() << endl;
//     cout << "a:\t" << a << endl;
//     cout << "b:\t" << b << endl;
//     cout << "c:\t" << c << endl;
//     cout << "c_:\t" << c_ << endl;

// }

// void test_rmfe_tau(long k1, long k2) {
//     print_banner("test_rmfe_tau");
//     long m1 = 2*k1, m2 = 2*k2;
//     shared_ptr<FieldConverter> converter = make_shared<FieldConverter>(m1 * m2, m1, m2);
//     shared_ptr<Gf2RMFE> rmfe1 = make_shared<BasicGf2RMFE>(k1, false);
//     shared_ptr<Gf2eRMFE> rmfe2 = make_shared<BasicRMFE>(converter->base_field_poly(), converter->composite_field_poly());
//     CompositeGf2RMFE rmfe(converter, rmfe1, rmfe2);

//     vec_GF2 a = random_vec_GF2(rmfe.k()), b = random_vec_GF2(rmfe.k());
//     GF2X enc_a = rmfe.encode(a), enc_b = rmfe.encode(b);

//     GF2X enc_a_ = rmfe.tau(enc_a), enc_b_ = rmfe.tau(enc_b);

//     cout << "k:\t" << rmfe.k() << ", m:\t" << rmfe.m() << endl;
//     cout << "enc_a:\t" << enc_a << endl;
//     cout << "enc_a_:\t" << enc_a_ << endl;
//     cout << "enc_b:\t" << enc_b_ << endl;
//     cout << "enc_b_:\t" << enc_b_ << endl;
// }


// void test_basic_gf2_rmfe_type2_random_preimage() {
//     print_banner("test_basic_gf2_rmfe_type2");
//     long k = 2;
//     BasicGf2RMFE rmfe(k, false);
//     vec_GF2 a = random_vec_GF2(k);
//     a[0] = 1;
//     a[1] = 1;
//     GF2X pre_a = rmfe.random_preimage(a);

//     vec_GF2 a_ = rmfe.decode(pre_a);

//     cout << "a:\t" << a << endl;
//     cout << "a_:\t" << a_ << endl;
//     cout << "pre_a:\t" << pre_a << endl;
// }

// CompositeGf2RMFE test_composite_gf2_rmfe_type2_random_preimage(long k1, long k2) {
//     print_banner("test_composite_gf2_rmfe_type2_random_preimage");
//     // long k1 = 2, k2 = 6;
//     long m1 = 2*k1, m2 = 2*k2;
//     shared_ptr<FieldConverter> converter = make_shared<FieldConverter>(m1 * m2, m1, m2);
//     shared_ptr<Gf2RMFE> rmfe1 = make_shared<BasicGf2RMFE>(k1, false);
//     shared_ptr<Gf2eRMFE> rmfe2 = make_shared<BasicRMFE>(converter->base_field_poly(), converter->composite_field_poly());
//     CompositeGf2RMFE rmfe(converter, rmfe1, rmfe2);

//     vec_GF2 a = random_vec_GF2(rmfe.k());
//     GF2X pre_a = rmfe.random_preimage(a);

//     vec_GF2 a_ = rmfe.decode(pre_a);

//     cout << "k:\t" << rmfe.k() << ", m:\t" << rmfe.m() << endl;
//     cout << "a:\t" << a << endl;
//     cout << "a_:\t" << a_ << endl;

//     return rmfe;
// }

void benchmark_rmfe64_then_mfe64_12_48_225() {
    print_banner("test_benchmark_rmfe64_then_mfe64_12_48_225");
    auto rmfe = get_composite_gf2_rmfe64_type2(2, 6);
    auto mfe = get_composite_gf2_mfe64(2, 3, 8);

    int n = 100000;
    for (int i = 0; i < n; i++) {
        {
            vec_GF2 a = random_vec_GF2(rmfe->k()), b = random_vec_GF2(rmfe->k());
            acc_time_log("RMFE encode (12 --> 48)");
            GF2X enc_a = rmfe->encode(a);
            GF2X enc_b = rmfe->encode(b);
            acc_time_log("RMFE encode (12 --> 48)");

            GF2X enc_c = MulMod(enc_a, enc_b, rmfe->ex_field_mod());
            acc_time_log("RMFE decode (48 --> 12)");
            vec_GF2 c = rmfe->decode(enc_c);
            acc_time_log("RMFE decode (48 --> 12)");

            vec_GF2 c_({}, a.length());
            for (int i = 0; i < a.length(); i++) {
                c_[i] = a[i] * b[i];
            }

            assert(c_ == c);

            acc_time_log("RMFE preimage (12 --> 48)");
            GF2X r = rmfe->random_preimage(c);
            acc_time_log("RMFE preimage (12 --> 48)");
        }

        {
            GF2X a = random_GF2X(mfe->m()), b = random_GF2X(mfe->m());
            acc_time_log("MFE encode (48 --> 225)");
            vec_GF2 enc_a = mfe->encode(a);
            vec_GF2 enc_b = mfe->encode(b);
            acc_time_log("MFE encode (48 --> 225)");

            vec_GF2 enc_c({}, enc_a.length());
            for (int i = 0; i < enc_a.length(); i++) {
                enc_c[i] = enc_a[i] * enc_b[i];
            }

            acc_time_log("MFE decode (225 --> 48)");
            GF2X c = mfe->decode(enc_c);
            acc_time_log("MFE decode (225 --> 48)");

            GF2X c_ = MulMod(a, b, mfe->ex_field_mod());

            assert(c_ == c);
        }
    }

    print_profiling();
    cout << "Throughput: " << endl;
    cout << "RMFE encode (12 --> 48): " << n / (get_acc_time_log("RMFE encode (12 --> 48)") / 1000.0 + 1e-30) << endl;
    cout << "RMFE decode (48 --> 12): " << n / (get_acc_time_log("RMFE decode (48 --> 12)") / 1000.0 + 1e-30) << endl;
    cout << "MFE encode (48 --> 225): " << n / (get_acc_time_log("MFE encode (48 --> 225)") / 1000.0 + 1e-30) << endl;
    cout << "MFE decode (225 --> 48): " << n / (get_acc_time_log("MFE decode (225 --> 48)") / 1000.0 + 1e-30) << endl;
    cout << "RMFE preimage (12 --> 48): " << n / (get_acc_time_log("RMFE preimage (12 --> 48)") / 1000.0 + 1e-30) << endl;
}

void benchmark_rmfe64_then_mfe64_14_42_195() {
    print_banner("benchmark_rmfe64_then_mfe64_14_42_195");
    auto rmfe = get_composite_gf2_rmfe64_type1_type2(2, 7);
    auto mfe = get_composite_gf2_mfe64(2, 3, 7);

    int n = 100000;
    for (int i = 0; i < n; i++) {
        {
            vec_GF2 a = random_vec_GF2(rmfe->k()), b = random_vec_GF2(rmfe->k());
            acc_time_log("RMFE encode (12 --> 42)");
            GF2X enc_a = rmfe->encode(a);
            GF2X enc_b = rmfe->encode(b);
            acc_time_log("RMFE encode (12 --> 42)");

            GF2X enc_c = MulMod(enc_a, enc_b, rmfe->ex_field_mod());
            acc_time_log("RMFE decode (42 --> 12)");
            vec_GF2 c = rmfe->decode(enc_c);
            acc_time_log("RMFE decode (42 --> 12)");

            vec_GF2 c_({}, a.length());
            for (int i = 0; i < a.length(); i++) {
                c_[i] = a[i] * b[i];
            }

            assert(c_ == c);

            acc_time_log("RMFE preimage (12 --> 42)");
            GF2X r = rmfe->random_preimage(c);
            acc_time_log("RMFE preimage (12 --> 42)");
        }

        {
            GF2X a = random_GF2X(mfe->m()), b = random_GF2X(mfe->m());
            acc_time_log("MFE encode (42 --> 195)");
            vec_GF2 enc_a = mfe->encode(a);
            vec_GF2 enc_b = mfe->encode(b);
            acc_time_log("MFE encode (42 --> 195)");

            vec_GF2 enc_c({}, enc_a.length());
            for (int i = 0; i < enc_a.length(); i++) {
                enc_c[i] = enc_a[i] * enc_b[i];
            }

            acc_time_log("MFE decode (195 --> 42)");
            GF2X c = mfe->decode(enc_c);
            acc_time_log("MFE decode (195 --> 42)");

            GF2X c_ = MulMod(a, b, mfe->ex_field_mod());

            assert(c_ == c);
        }
    }

    print_profiling();
    cout << "Throughput: " << endl;
    cout << "RMFE encode (12 --> 42): " << n / (get_acc_time_log("RMFE encode (12 --> 42)") / 1000.0 + 1e-30) << endl;
    cout << "RMFE decode (42 --> 12): " << n / (get_acc_time_log("RMFE decode (42 --> 12)") / 1000.0 + 1e-30) << endl;
    cout << "MFE encode (42 --> 195): " << n / (get_acc_time_log("MFE encode (42 --> 195)") / 1000.0 + 1e-30) << endl;
    cout << "MFE decode (195 --> 42): " << n / (get_acc_time_log("MFE decode (195 --> 42)") / 1000.0 + 1e-30) << endl;
    cout << "RMFE preimage (12 --> 42): " << n / (get_acc_time_log("RMFE preimage (12 --> 42)") / 1000.0 + 1e-30) << endl;
}

void rmfe64_then_mfe64_demo() {
    print_banner("rmfe64_then_mfe64_demo");
    auto rmfe = get_composite_gf2_rmfe64_type1_type2(2, 1);
    auto mfe = get_composite_gf2_mfe64(2, 3);

    int n = 1;
    for (int i = 0; i < n; i++) {
        
        vec_GF2 a = random_vec_GF2(rmfe->k());
        a[0] = NTL::GF2(0);
        a[1] = NTL::GF2(1);
        cout << "a:\t" << a << endl;
        GF2X enc_a = rmfe->encode(a);
        cout << "enc_a:\t" << enc_a << endl;
        vec_GF2 enc_enc_a = mfe->encode(enc_a);
        cout << "enc_enc_a:\t" << enc_enc_a << endl;
        GF2X enc_a_ = mfe->decode(enc_enc_a);
        cout << "enc_a_:\t" << enc_a_ << endl;
        vec_GF2 a_ = rmfe->decode(enc_a_);
        cout << "a_:\t" << a_ << endl;
    }
}


int main() {
    // test_composite_to_binary();
    // test_composite_to_binary_with_base_poly();
    // test_basic_mfe64();
    test_basic_gf2_mfe64();
    // test_composite_gf2_mfe64();
    // test_double_composite_gf2_mfe64(2, 3, 8);
    // test_basic_rmfe64();
    // test_basic_gf2_rmfe64();
    // test_basic_gf2_rmfe_type2();
    // test_composite_gf2_rmfe();
    // test_composite_gf2_rmfe64_type2(2, 6);
    test_composite_gf2_rmfe64_type1_type2(2, 7);
    // test_rmfe_then_mfe();
    // test_rmfe_tau(2, 6);
    // test_basic_gf2_rmfe_type2_random_preimage();
    // test_composite_gf2_rmfe_type2_random_preimage(2, 6);
    benchmark_rmfe64_then_mfe64_12_48_225();
    benchmark_rmfe64_then_mfe64_14_42_195();
    rmfe64_then_mfe64_demo();
}
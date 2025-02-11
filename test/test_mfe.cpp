#include "Math/mfe.h"
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


void test_composite_to_binary() {
    print_banner("test_composite_to_binary");
    
    long n = 2, m = 3, k = n*m;
    FieldConverter converter(k, n, m);
    cout << "q: " << converter.composite_field_poly() << endl;

    GF2E::init(converter.binary_field_poly());
    GF2E a = random_GF2E(), b = random_GF2E();
    GF2E c = a * b;
    GF2EX a_composite = converter.binary_to_composite(a);
    GF2EX b_composite = converter.binary_to_composite(b);
    GF2EX c_composite = converter.binary_to_composite(c);

    {
        GF2EPush push;
        GF2E::init(converter.base_field_poly());
        GF2EX c_composite_ = MulMod(a_composite, b_composite, converter.composite_field_poly());

        cout << "c_composite:\t" << c_composite << endl;
        cout << "c_composite_:\t" << c_composite_ << endl;
        {
            GF2EPush push;
            GF2E::init(converter.binary_field_poly());
            GF2E c_ = converter.composite_to_binary(c_composite_);
            cout << "c:\t" << c << endl;
            cout << "c_:\t" << c_ << endl;
        }
    }

    GF2E one(1);
    GF2EX one_ = converter.binary_to_composite(one);
    cout << "one:\t" << one << endl;
    cout << "one_:\t" << one_ << endl;
 }

 void test_composite_to_binary_with_base_poly() {
    print_banner("test_composite_to_binary_with_base_poly");

    GF2X base_poly = indices_to_gf2x(std::vector<long>({6, 5, 4, 1, 0}));
    cout << "base poly: " << base_poly << endl;
    
    long n = 6, m = 8, k = n*m;
    FieldConverter converter(k, n, m, base_poly);
    cout << "q: " << converter.composite_field_poly() << endl;
    cout << "u: " << converter.base_field_poly() << endl;

    GF2E::init(converter.binary_field_poly());
    GF2E a = random_GF2E(), b = random_GF2E();
    GF2E c = a * b;
    GF2EX a_composite = converter.binary_to_composite(a);
    GF2EX b_composite = converter.binary_to_composite(b);
    GF2EX c_composite = converter.binary_to_composite(c);

    {
        GF2EPush push;
        GF2E::init(converter.base_field_poly());
        GF2EX c_composite_ = MulMod(a_composite, b_composite, converter.composite_field_poly());

        cout << "c_composite:\t" << c_composite << endl;
        cout << "c_composite_:\t" << c_composite_ << endl;
        {
            GF2EPush push;
            GF2E::init(converter.binary_field_poly());
            GF2E c_ = converter.composite_to_binary(c_composite_);
            cout << "c:\t" << c << endl;
            cout << "c_:\t" << c_ << endl;
        }
    }

    GF2E one(1);
    GF2EX one_ = converter.binary_to_composite(one);
    cout << "one:\t" << one << endl;
    cout << "one_:\t" << one_ << endl;
 }

void test_basic_mfe() {
    print_banner("test_basic_mfe");

    long m = 3, n = 2;
    BasicMFE mfe(m, n);
    GF2E::init(mfe.base_field_mod());
    GF2EX a = random_GF2EX(m), b = random_GF2EX(m);
    vec_GF2E enc_a = mfe.encode(a), enc_b = mfe.encode(b);

    vec_GF2E enc_c({}, enc_a.length());
    for (int i = 0; i < enc_a.length(); i++) {
        enc_c[i] = enc_a[i] * enc_b[i];
    }

    GF2EX c = mfe.decode(enc_c);
    GF2EX c_ = MulMod(a, b, mfe.ex_field_mod());

    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;
}

void test_basic_gf2_mfe() {
    print_banner("test_basic_gf2_mfe");
    long m = 2;
    BasicGf2MFE mfe(m);
    GF2X a = random_GF2X(m), b = random_GF2X(m);
    vec_GF2 enc_a = mfe.encode(a), enc_b = mfe.encode(b);

    vec_GF2 enc_c({}, enc_a.length());
    for (int i = 0; i < enc_a.length(); i++) {
        enc_c[i] = enc_a[i] * enc_b[i];
    }

    GF2X c = mfe.decode(enc_c);
    GF2X c_ = MulMod(a, b, mfe.ex_field_mod());

    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;
}

void test_composite_gf2_mfe() {
    print_banner("test_composite_gf2_mfe");
    long m1 = 2, m2 = 3;
    shared_ptr<FieldConverter> converter = make_shared<FieldConverter>(m1 * m2, m1, m2);
    shared_ptr<Gf2MFE> mfe1 = make_shared<BasicGf2MFE>(m1);
    shared_ptr<Gf2eMFE> mfe2 = make_shared<BasicMFE>(converter->base_field_poly(), converter->composite_field_poly());
    CompositeGf2MFE mfe(converter, mfe1, mfe2);

    GF2X a = random_GF2X(mfe.m()), b = random_GF2X(mfe.m());
    vec_GF2 enc_a = mfe.encode(a), enc_b = mfe.encode(b);

    vec_GF2 enc_c({}, enc_a.length());
    for (int i = 0; i < enc_a.length(); i++) {
        enc_c[i] = enc_a[i] * enc_b[i];
    }

    GF2X c = mfe.decode(enc_c);
    GF2X c_ = MulMod(a, b, mfe.ex_field_mod());

    cout << "m:\t" << mfe.m() << ", t:\t" << mfe.t() << endl;
    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;
    // cout << "field poly mod:\t" << mfe.ex_field_poly() << endl;

    GF2E::init(mfe.ex_field_mod());
    cout << "a * b:\t" << to_GF2E(a) * to_GF2E(b) << endl;

    // mfe.encode(a);
    // mfe.decode(enc_a);
    // cout << "a:\t" << a << endl;
    // cout << "a_:\t" <<  << endl;
}

CompositeGf2MFE test_double_composite_gf2_mfe(long m1, long m2, long m3) {
    print_banner("test_double_composite_gf2_mfe");
    // long m1 = 2, m2 = 3, m3 = 8;
    shared_ptr<FieldConverter> converter1 = make_shared<FieldConverter>(m1 * m2, m1, m2);
    shared_ptr<FieldConverter> converter2 = make_shared<FieldConverter>(m1 * m2 * m3, m1*m2, m3, converter1->binary_field_poly());
    shared_ptr<Gf2MFE> mfe1 = make_shared<BasicGf2MFE>(m1);
    shared_ptr<Gf2eMFE> mfe2 = make_shared<BasicMFE>(converter1->base_field_poly(), converter1->composite_field_poly());
    shared_ptr<CompositeGf2MFE> mfe3 = make_shared<CompositeGf2MFE>(converter1, mfe1, mfe2);
    
    shared_ptr<Gf2eMFE> mfe4 = make_shared<BasicMFE>(converter2->base_field_poly(), converter2->composite_field_poly());
    CompositeGf2MFE mfe(converter2, mfe3, mfe4);

    GF2X a = random_GF2X(mfe.m()), b = random_GF2X(mfe.m());
    vec_GF2 enc_a = mfe.encode(a), enc_b = mfe.encode(b);

    vec_GF2 enc_c({}, enc_a.length());
    for (int i = 0; i < enc_a.length(); i++) {
        enc_c[i] = enc_a[i] * enc_b[i];
    }

    GF2X c = mfe.decode(enc_c);
    GF2X c_ = MulMod(a, b, mfe.ex_field_mod());

    cout << "m:\t" << mfe.m() << ", t:\t" << mfe.t() << endl;
    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;
    // cout << "field poly mod:\t" << mfe.ex_field_poly() << endl;

    GF2E::init(mfe.ex_field_mod());
    cout << "a * b:\t" << to_GF2E(a) * to_GF2E(b) << endl;

    // mfe.encode(a);
    // mfe.decode(enc_a);
    // cout << "a:\t" << a << endl;
    // cout << "a_:\t" <<  << endl;
    return mfe;
}

void test_basic_rmfe() {
    print_banner("test_basic_rmfe");

    long k = 7, n = 5;
    BasicRMFE rmfe(k, n);
    GF2E::init(rmfe.base_field_mod());
    vec_GF2E a = random_vec_GF2E(k), b = random_vec_GF2E(k);
    GF2EX enc_a = rmfe.encode(a), enc_b = rmfe.encode(b);

    GF2EX enc_c = enc_a * enc_b;

    vec_GF2E c = rmfe.decode(enc_c);
    vec_GF2E c_({}, a.length());
    for (int i = 0; i < a.length(); i++) {
        c_[i] = a[i] * b[i];
    }

    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;

    cout << "a:\t" << a << endl;
    cout << "a_:\t" << rmfe.decode(enc_a) << endl;
}

void test_basic_gf2_rmfe() {
    print_banner("test_basic_gf2_rmfe");
    long k = 2;
    BasicGf2RMFE rmfe(k);
    vec_GF2 a = random_vec_GF2(k), b = random_vec_GF2(k);
    GF2X enc_a = rmfe.encode(a), enc_b = rmfe.encode(b);

    GF2X enc_c = MulMod(enc_a, enc_b, rmfe.ex_field_mod());

    vec_GF2 c = rmfe.decode(enc_c);
    vec_GF2 c_({}, a.length());
    for (int i = 0; i < a.length(); i++) {
        c_[i] = a[i] * b[i];
    }

    cout << "a:\t" << a << endl;
    cout << "b:\t" << b << endl;
    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;
}

void test_basic_gf2_rmfe_type2() {
    print_banner("test_basic_gf2_rmfe_type2");
    long k = 2;
    BasicGf2RMFE rmfe(k, false);
    vec_GF2 a = random_vec_GF2(k), b = random_vec_GF2(k);
    GF2X enc_a = rmfe.encode(a), enc_b = rmfe.encode(b);

    GF2X enc_c = MulMod(enc_a, enc_b, rmfe.ex_field_mod());

    vec_GF2 c = rmfe.decode(enc_c);
    vec_GF2 c_({}, a.length());
    for (int i = 0; i < a.length(); i++) {
        c_[i] = a[i] * b[i];
    }

    cout << "a:\t" << a << endl;
    cout << "b:\t" << b << endl;
    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;
}

void test_composite_gf2_rmfe() {
    print_banner("test_composite_gf2_rmfe");
    long k1 = 2, m1 = 2*k1 - 1, k2 = 8, m2 = 2*k2-1;
    shared_ptr<FieldConverter> converter = make_shared<FieldConverter>(m1 * m2, m1, m2);
    shared_ptr<Gf2RMFE> rmfe1 = make_shared<BasicGf2RMFE>(k1);
    shared_ptr<Gf2eRMFE> rmfe2 = make_shared<BasicRMFE>(converter->base_field_poly(), converter->composite_field_poly());
    CompositeGf2RMFE rmfe(converter, rmfe1, rmfe2);

    vec_GF2 a = random_vec_GF2(rmfe.k()), b = random_vec_GF2(rmfe.k());
    GF2X enc_a = rmfe.encode(a), enc_b = rmfe.encode(b);

    GF2X enc_c = MulMod(enc_a, enc_b, rmfe.ex_field_mod());
    vec_GF2 c = rmfe.decode(enc_c);

    vec_GF2 c_({}, a.length());
    for (int i = 0; i < a.length(); i++) {
        c_[i] = a[i] * b[i];
    }

    cout << "k:\t" << rmfe.k() << ", m:\t" << rmfe.m() << endl;
    cout << "a:\t" << a << endl;
    cout << "b:\t" << b << endl;
    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;

    // mfe.encode(a);
    // mfe.decode(enc_a);
    cout << "a:\t" << a << endl;
    cout << "a_:\t" << rmfe.decode(enc_a) << endl;
}

void test_composite_gf2_rmfe_type2(long k1, long k2) {
    print_banner("test_composite_gf2_rmfe_type2");
    // long k1 = 2, k2 = 6;
    // long m1 = 2*k1, m2 = 2*k2;
    // shared_ptr<FieldConverter> converter = make_shared<FieldConverter>(m1 * m2, m1, m2);
    // shared_ptr<Gf2RMFE> rmfe1 = make_shared<BasicGf2RMFE>(k1, false);
    // shared_ptr<Gf2eRMFE> rmfe2 = make_shared<BasicRMFE>(converter->base_field_poly(), converter->composite_field_poly());
    // CompositeGf2RMFE rmfe(converter, rmfe1, rmfe2);
    unique_ptr<Gf2RMFE> rmfe = get_composite_gf2_rmfe_type2(k1, k2);
    cout << "after get instance" << endl;

    vec_GF2 a = random_vec_GF2(rmfe->k()), b = random_vec_GF2(rmfe->k());
    GF2X enc_a = rmfe->encode(a), enc_b = rmfe->encode(b);
    cout << "after encode" << endl;

    GF2X enc_c = MulMod(enc_a, enc_b, rmfe->ex_field_mod());
    vec_GF2 c = rmfe->decode(enc_c);
    cout << "after decode" << endl;

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

void test_rmfe_then_mfe() {
    print_banner("test_rmfe_then_mfe");
    unique_ptr<Gf2RMFE> rmfe = get_composite_gf2_rmfe_type2(2, 6);
    CompositeGf2MFE mfe = test_double_composite_gf2_mfe(2, 3, 8);

    vec_GF2 a = random_vec_GF2(rmfe->k()), b = random_vec_GF2(rmfe->k());
    GF2X enc_a = rmfe->encode(a), enc_b = rmfe->encode(b);

    vec_GF2 enc_enc_a = mfe.encode(enc_a), enc_enc_b = mfe.encode(enc_b);

    vec_GF2 enc_enc_c({}, enc_enc_a.length());
    for (int i = 0; i < enc_enc_a.length(); i++) {
        enc_enc_c[i] = enc_enc_a[i] * enc_enc_b[i];
    }

    GF2X enc_c = mfe.decode(enc_enc_c);
    vec_GF2 c = rmfe->decode(enc_c);

    vec_GF2 c_({}, a.length());
    for (int i = 0; i < a.length(); i++) {
        c_[i] = a[i] * b[i];
    }

    cout << "k:\t" << rmfe->k() << ", m:\t" << rmfe->m() << ", t:\t" << mfe.t() << endl;
    cout << "a:\t" << a << endl;
    cout << "b:\t" << b << endl;
    cout << "c:\t" << c << endl;
    cout << "c_:\t" << c_ << endl;

}

void test_rmfe_tau(long k1, long k2) {
    print_banner("test_rmfe_tau");
    long m1 = 2*k1, m2 = 2*k2;
    shared_ptr<FieldConverter> converter = make_shared<FieldConverter>(m1 * m2, m1, m2);
    shared_ptr<Gf2RMFE> rmfe1 = make_shared<BasicGf2RMFE>(k1, false);
    shared_ptr<Gf2eRMFE> rmfe2 = make_shared<BasicRMFE>(converter->base_field_poly(), converter->composite_field_poly());
    CompositeGf2RMFE rmfe(converter, rmfe1, rmfe2);

    vec_GF2 a = random_vec_GF2(rmfe.k()), b = random_vec_GF2(rmfe.k());
    GF2X enc_a = rmfe.encode(a), enc_b = rmfe.encode(b);

    GF2X enc_a_ = rmfe.tau(enc_a), enc_b_ = rmfe.tau(enc_b);

    cout << "k:\t" << rmfe.k() << ", m:\t" << rmfe.m() << endl;
    cout << "enc_a:\t" << enc_a << endl;
    cout << "enc_a_:\t" << enc_a_ << endl;
    cout << "enc_b:\t" << enc_b_ << endl;
    cout << "enc_b_:\t" << enc_b_ << endl;
}


void test_basic_gf2_rmfe_type2_random_preimage() {
    print_banner("test_basic_gf2_rmfe_type2");
    long k = 2;
    BasicGf2RMFE rmfe(k, false);
    vec_GF2 a = random_vec_GF2(k);
    a[0] = 1;
    a[1] = 1;
    GF2X pre_a = rmfe.random_preimage(a);

    vec_GF2 a_ = rmfe.decode(pre_a);

    cout << "a:\t" << a << endl;
    cout << "a_:\t" << a_ << endl;
    cout << "pre_a:\t" << pre_a << endl;
}

CompositeGf2RMFE test_composite_gf2_rmfe_type2_random_preimage(long k1, long k2) {
    print_banner("test_composite_gf2_rmfe_type2_random_preimage");
    // long k1 = 2, k2 = 6;
    long m1 = 2*k1, m2 = 2*k2;
    shared_ptr<FieldConverter> converter = make_shared<FieldConverter>(m1 * m2, m1, m2);
    shared_ptr<Gf2RMFE> rmfe1 = make_shared<BasicGf2RMFE>(k1, false);
    shared_ptr<Gf2eRMFE> rmfe2 = make_shared<BasicRMFE>(converter->base_field_poly(), converter->composite_field_poly());
    CompositeGf2RMFE rmfe(converter, rmfe1, rmfe2);

    vec_GF2 a = random_vec_GF2(rmfe.k());
    GF2X pre_a = rmfe.random_preimage(a);

    vec_GF2 a_ = rmfe.decode(pre_a);

    cout << "k:\t" << rmfe.k() << ", m:\t" << rmfe.m() << endl;
    cout << "a:\t" << a << endl;
    cout << "a_:\t" << a_ << endl;

    return rmfe;
}

void benchmark_rmfe_then_mfe() {
    print_banner("test_benchmark_rmfe_then_mfe");
    auto rmfe = get_composite_gf2_rmfe_type2(2, 6);
    auto mfe = get_double_composite_gf2_mfe(2, 3, 8);

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

void test_weird() {
    time_log("test");
    for (int i = 0; i < 10000000; i++) {
        GF2E x;
        // bool b = (x._GF2E__rep.xrep.length() == 0);
        long b = IsZero(x);
    }

    time_log("test");
}

int main() {
    // test_composite_to_binary();
    // test_composite_to_binary_with_base_poly();
    // test_basic_mfe();
    // test_basic_gf2_mfe();
    // test_composite_gf2_mfe();
    // test_double_composite_gf2_mfe(2, 3, 8);
    // test_basic_rmfe();
    // test_basic_gf2_rmfe();
    // test_basic_gf2_rmfe_type2();
    // test_composite_gf2_rmfe();
    // test_composite_gf2_rmfe_type2(2, 6);
    // test_rmfe_then_mfe();
    // test_rmfe_tau(2, 6);
    // test_basic_gf2_rmfe_type2_random_preimage();
    // test_composite_gf2_rmfe_type2_random_preimage(2, 6);
    benchmark_rmfe_then_mfe();
    test_weird();
}
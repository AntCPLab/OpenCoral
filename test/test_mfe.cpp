#include "Math/mfe.h"

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
    FieldConverter converter(k, m, n);
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
}

void test_basic_mfe() {
    print_banner("test_basic_mfe");

    long m = 3, t = 5, n = 2;
    BasicMFE mfe(m, t, n);
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
    long m = 2, t = 3;
    BasicGf2MFE mfe(m, t);
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
    long m1 = 3, t1 = 5, m2 = 2, t2 = 3;
    shared_ptr<FieldConverter> converter = make_shared<FieldConverter>(m1 * m2, m1, m2);
    shared_ptr<Gf2eMFE> mfe1 = make_shared<BasicMFE>(converter->base_field_poly(), converter->composite_field_poly(), t1);
    shared_ptr<Gf2MFE> mfe2 = make_shared<BasicGf2MFE>(m2, t2);
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

int main() {
    // basic_mfe();
    // basic_rmfe();
    // composite_to_binary();
    test_composite_to_binary();
    test_basic_mfe();
    // test_basic_gf2_mfe();
    test_composite_gf2_mfe();
}
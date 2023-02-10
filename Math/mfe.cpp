#include "mfe.h"
#include "NTL/GF2X.h"
#include "NTL/GF2E.h"
#include "NTL/GF2XFactoring.h"
#include "NTL/GF2EXFactoring.h"
#include "NTL/vec_vec_GF2E.h"
#include "NTL/vec_GF2E.h"
#include "NTL/mat_GF2E.h"
#include <iostream>
#include <unordered_set>

using namespace std;
using namespace NTL;

void mul(vec_GF2E& x, const vec_GF2E& a, const vec_GF2E& b) {
    if (a.length() != b.length())
        LogicError("Mul operands have different length");
    x.SetLength(a.length());
    for(int i = 0; i < a.length(); i++)
        x[i] = a[i] * b[i];
}

void Enumerate_GF2X(vec_GF2X& x, size_t n) {
    x.SetLength(n);
    if (n == 0)
        return;

    int len = int(log2(n)) + 1;
    for (size_t i = 0; i < n; i++) {
        vec_GF2 a({}, len);
        for (int k = 0; k < len; k++)
            a[k] = (i >> k) & 1;
        conv(x[i], a);
    }
}

void Enumerate_GF2E(vec_GF2E& x, size_t n) {
    if (n > GF2E::cardinality())
        LogicError("# of requested elements larger than field size");
    vec_GF2X y;
    Enumerate_GF2X(y, n);
    x.SetLength(n);
    for (size_t i = 0; i < n; i++) {
        conv(x[i], y[i]);
    }
}

void basic_mfe() {
    long n = 2, m = 3;//, t = 5;
    GF2X base_field_poly_mod = BuildIrred_GF2X(n);
    GF2E::init(base_field_poly_mod);

    // construct basis
    GF2EXModulus ex_field_poly_mod = GF2EXModulus(BuildIrred_GF2EX(m));
    GF2EX alpha(1, 1); // alpha = 0 + 1*Y + 0*Y^2 + ... = Y
    vec_GF2EX basis({}, m);
    basis[0] = 1;
    for (int i = 1; i < m; i++) {
        MulMod(basis[i], basis[i-1], alpha, ex_field_poly_mod);
    }
    // TODO: actually, in this setting, the basis is just {1, Y, Y^2, ..., Y^(m-1)}
    // Hence mu_1 and inv_mu_1 are both just identity functions (just run them to verify)
    // So things can be simpfiled a lot. Keep algorithm code of mu_1 and inv_mu_1 here
    // in case we could not use "alpha = Y" in some setting.

    // get 2m-2 distinct elements
    vec_GF2E beta;
    Enumerate_GF2E(beta, 2*m-2);


    GF2EX mu_1_f = random_GF2EX(m), mu_1_g, mu_1_f_;
    mu_1(mu_1_g, mu_1_f, basis);
    inv_mu_1(mu_1_f_, mu_1_g, basis, m);


    GF2EX mu_2_f = random_GF2EX(2*m-1), mu_2_f_;
    vec_GF2E mu_2_g;
    mu_2(mu_2_g, mu_2_f, beta);
    inv_mu_2(mu_2_f_, mu_2_g, beta);


    GF2EX x = random_GF2EX(m), y = random_GF2EX(m), z, z_;
    vec_GF2E a, b, c;
    sigma(a, x, basis, beta);
    sigma(b, y, basis, beta);
    mul(c, a, b);
    rho(z, c, basis, beta, ex_field_poly_mod);
    MulMod(z_, x, y, ex_field_poly_mod);

    cout << base_field_poly_mod << endl;
    cout << ex_field_poly_mod << endl;
    cout << beta << endl;
    cout << basis << endl;
    cout << "mu_1 f:\t" << mu_1_f << endl;
    cout << "mu_1 g:\t" << mu_1_g << endl;
    cout << "mu_1 f_:\t" << mu_1_f_ << endl;
    cout << "mu_2 f:\t" << mu_2_f << endl;
    cout << "mu_2 g:\t" << mu_2_g << endl;
    cout << "mu_2 f_:\t" << mu_2_f_ << endl;
    cout << "z:\t" << z <<endl;
    cout << "z_:\t" << z_ << endl;
}

void sigma(vec_GF2E& h, const GF2EX& g, const vec_GF2EX& basis, const vec_GF2E& beta) {
    long m = beta.length() / 2 + 1;
    GF2EX f;
    inv_mu_1(f, g, basis, m);
    nu_1(h, f, beta);
}

void mu_1(GF2EX& g, const GF2EX& f, const vec_GF2EX& basis) {
    clear(g);
    for(int i = 0; i <= deg(f); i++) {
        g += basis[i] * f[i];
    }
}

void inv_mu_1(GF2EX& f, const GF2EX& g, const vec_GF2EX& basis, long m) {
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

void nu_1(vec_GF2E& g, const GF2EX& f, const vec_GF2E& beta) {
    g.SetLength(beta.length() + 1);
    for(int i = 0; i < beta.length(); i++) {
        eval(g[i], f, beta[i]);
    }
    GetCoeff(g[beta.length()], f, beta.length() / 2);
}

void rho(GF2EX& g, const vec_GF2E& h, const vec_GF2EX& basis, const vec_GF2E& beta, const GF2EXModulus& modulus) {
    GF2EX f;
    inv_mu_2(f, h, beta);
    nu_2(g, f, basis, modulus);
}

void mu_2(vec_GF2E& g, const GF2EX& f, const vec_GF2E& beta) {
    g.SetLength(beta.length() + 1);
    for(int i = 0; i < beta.length(); i++) {
        eval(g[i], f, beta[i]);
    }
    GetCoeff(g[beta.length()], f, beta.length());
}

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

void nu_2(GF2EX& g, const GF2EX& f, const vec_GF2EX& basis, const GF2EXModulus& modulus) {
    clear(g);
    GF2EX extended_alpha_power = basis[basis.length() - 1];
    for(int i = 0; i <= deg(f); i++) {
        if (i < basis.length())
            g += basis[i] * f[i];
        else {
            MulMod(extended_alpha_power, extended_alpha_power, basis[1], modulus);
            g += extended_alpha_power * f[i];
        }
    }
}
/*
 * NoiseBound.cpp
 *
 */

#include <FHE/NoiseBounds.h>
#include "FHEOffline/Proof.h"
#include "Protocols/CowGearOptions.h"
#include <math.h>


SemiHomomorphicNoiseBounds::SemiHomomorphicNoiseBounds(const bigint& p,
        int phi_m, int n, int sec, int slack_param, bool extra_h, double sigma, int h) :
        p(p), phi_m(phi_m), n(n), sec(sec),
        slack(numBits(Proof::slack(slack_param, sec, phi_m))), sigma(sigma), h(h)
{
    if (sigma <= 0)
        this->sigma = sigma = FHE_Params().get_R();
#ifdef VERBOSE
    cerr << "Standard deviation: " << this->sigma << endl;
#endif
    h += extra_h * sec;
    produce_epsilon_constants();

    if (CowGearOptions::singleton.top_gear())
    {
        // according to documentation of SCALE-MAMBA 1.7
        // excluding a factor of n because we don't always add up n ciphertexts
        B_clean = (bigint(phi_m) << (sec + 2)) * p
                * (20.5 + c1 * sigma * sqrt(phi_m) + 20 * c1 * sqrt(h));
        mpf_class V_s;
        if (h > 0)
            V_s = sqrt(h);
        else
            V_s = sigma * sqrt(phi_m);
        B_scale = (c1 + c2 * V_s) * p * sqrt(phi_m / 12.0);
#ifdef NOISY
        cout << "p * sqrt(phi(m) / 12): " << p * sqrt(phi_m / 12.0) << endl;
        cout << "V_s: " << V_s << endl;
        cout << "c1: " << c1 << endl;
        cout << "c2: " << c2 << endl;
        cout << "c1 + c2 * V_s: " << c1 + c2 * V_s << endl;
        cout << "B_scale: " << B_scale << endl;
#endif
    }
    else
    {
        B_clean = (phi_m * p / 2
                + p * sigma
                        * (16 * phi_m * sqrt(n / 2) + 6 * sqrt(phi_m)
                                + 16 * sqrt(n * h * phi_m))) << slack;
        B_scale = p * sqrt(3 * phi_m) * (1 + 8 * sqrt(n * h) / 3);
        cout << "log(slack): " << slack << endl;
    }

    drown = 1 + n * (bigint(1) << sec);
}

bigint SemiHomomorphicNoiseBounds::min_p0(const bigint& p1)
{
    return p * drown * n * B_clean / p1 + B_scale;
}

bigint SemiHomomorphicNoiseBounds::min_p0()
{
    // slack is already in B_clean
    return B_clean * drown * p;
}

double SemiHomomorphicNoiseBounds::min_phi_m(int log_q, double sigma)
{
    if (sigma <= 0)
        sigma = FHE_Params().get_R();
    // the constant was updated using Martin Albrecht's LWE estimator in Sep 2019
    return 37.8 * (log_q - log2(sigma));
}

void SemiHomomorphicNoiseBounds::produce_epsilon_constants()
{
    double C[3];

    for (int i = 0; i < 3; i++)
    {
        C[i] = -1;
    }
    for (double x = 0.1; x < 10.0; x += .1)
    {
        double t = erfc(x), tp = 1;
        for (int i = 1; i < 3; i++)
        {
            tp *= t;
            double lgtp = log(tp) / log(2.0);
            if (C[i] < 0 && lgtp < FHE_epsilon)
            {
                C[i] = pow(x, i);
            }
        }
    }

    c1 = C[1];
    c2 = C[2];
}

NoiseBounds::NoiseBounds(const bigint& p, int phi_m, int n, int sec, int slack,
        double sigma, int h) :
        SemiHomomorphicNoiseBounds(p, phi_m, n, sec, slack, false, sigma, h)
{
    if (CowGearOptions::singleton.top_gear())
    {
        B_KS = p * c2 * this->sigma * phi_m / sqrt(12);
    }
    else
    {
        B_KS = p * phi_m * mpf_class(this->sigma)
                * (pow(n, 2.5) * (1.49 * sqrt(h * phi_m) + 2.11 * h)
                        + 2.77 * n * n * sqrt(h)
                        + pow(n, 1.5) * (1.96 * sqrt(phi_m) * 2.77 * sqrt(h))
                        + 4.62 * n);
    }
#ifdef NOISY
    cout << "p size: " << numBits(p) << endl;
    cout << "phi(m): " << phi_m << endl;
    cout << "n: " << n << endl;
    cout << "sec: " << sec << endl;
    cout << "sigma: " << this->sigma << endl;
    cout << "h: " << h << endl;
    cout << "B_clean size: " << numBits(B_clean) << endl;
    cout << "B_scale size: " << numBits(B_scale) << endl;
    cout << "B_KS size: " << numBits(B_KS) << endl;
    cout << "drown size: " << numBits(drown) << endl;
#endif
}

bigint NoiseBounds::U1(const bigint& p0, const bigint& p1)
{
    bigint tmp = n * B_clean / p1 + B_scale;
    return tmp * tmp + B_KS * p0 / p1 + B_scale;
}

bigint NoiseBounds::U2(const bigint& p0, const bigint& p1)
{
    return U1(p0, p1) + n * B_clean / p1 + B_scale;
}

bigint NoiseBounds::min_p0(const bigint& p0, const bigint& p1)
{
    return 2 * U2(p0, p1) * drown;
}

bigint NoiseBounds::min_p0(const bigint& p1)
{
    bigint U = n * B_clean / p1 + 1 + B_scale;
    bigint res = 2 * (U * U + U + B_scale) * drown;
    mpf_class div = (1 - 1. * min_p1() / p1);
    res = ceil(res / div);
#ifdef NOISY
    cout << "U size: " << numBits(U) << endl;
    cout << "before div size: " << numBits(res) << endl;
    cout << "div: " << div << endl;
    cout << "minimal p0 size: " << numBits(res) << endl;
#endif
    return res;
}

bigint NoiseBounds::min_p1()
{
    return drown * B_KS + 1;
}

bigint NoiseBounds::opt_p1()
{
    // square equation parameters
    bigint a, b, c;
    a = B_scale * B_scale + B_scale;
    b = -2 * a * min_p1();
    c = -n * B_clean * (2 * B_scale + 1) * min_p1() + n * n * B_scale * B_scale;
    // solve
    mpf_class s = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
    bigint res = ceil(s);
    cout << "Optimal p1 vs minimal: " << numBits(res) << "/"
            << numBits(min_p1()) << endl;
    return res;
}

double NoiseBounds::optimize(int& lg2p0, int& lg2p1)
{
    bigint min_p1 = opt_p1();
    bigint min_p0 = this->min_p0(min_p1);
    while (this->min_p0(min_p0, min_p1) > min_p0)
      {
        min_p0 *= 2;
        min_p1 *= 2;
        cout << "increasing lengths: " << numBits(min_p0) << "/"
            << numBits(min_p1) << endl;
      }
    lg2p1 = numBits(min_p1);
    lg2p0 = numBits(min_p0);
    return min_phi_m(lg2p0 + lg2p1);
}
